/* Copyright 2023 The MediaPipe Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <optional>

#include "absl/flags/flag.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "mediapipe/framework/api2/builder.h"
#include "mediapipe/framework/api2/port.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/deps/file_path.h"
#include "mediapipe/framework/formats/image.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/packet.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/gmock.h"
#include "mediapipe/framework/port/gtest.h"
#include "mediapipe/tasks/cc/core/mediapipe_builtin_op_resolver.h"
#include "mediapipe/tasks/cc/core/proto/base_options.pb.h"
#include "mediapipe/tasks/cc/core/proto/external_file.pb.h"
#include "mediapipe/tasks/cc/core/task_runner.h"
#include "mediapipe/tasks/cc/vision/pose_detector/proto/pose_detector_graph_options.pb.h"
#include "mediapipe/tasks/cc/vision/pose_landmarker/proto/pose_landmarker_graph_options.pb.h"
#include "mediapipe/tasks/cc/vision/pose_landmarker/proto/pose_landmarks_detector_graph_options.pb.h"
#include "mediapipe/tasks/cc/vision/utils/image_utils.h"

namespace mediapipe {
namespace tasks {
namespace vision {
namespace pose_landmarker {
namespace {

using ::file::Defaults;
using ::file::GetTextProto;
using ::mediapipe::api2::Input;
using ::mediapipe::api2::Output;
using ::mediapipe::api2::builder::Graph;
using ::mediapipe::api2::builder::Source;
using ::mediapipe::file::JoinPath;
using ::mediapipe::tasks::core::TaskRunner;
using ::mediapipe::tasks::vision::DecodeImageFromFile;
using ::mediapipe::tasks::vision::pose_landmarker::proto::
    PoseLandmarkerGraphOptions;
using ::testing::EqualsProto;
using ::testing::Pointwise;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::proto::Approximately;
using ::testing::proto::Partially;

constexpr char kTestDataDirectory[] = "/mediapipe/tasks/testdata/vision/";
constexpr char kPoseLandmarkerModelBundleName[] = "pose_landmarker.task";
constexpr char kPoseImageName[] = "pose.jpg";
constexpr char kExpectedPoseLandmarksName[] =
    "expected_pose_landmarks.prototxt";

constexpr char kImageTag[] = "IMAGE";
constexpr char kImageName[] = "image";
constexpr char kNormRectTag[] = "NORM_RECT";
constexpr char kNormRectName[] = "norm_rect";
constexpr char kNormLandmarksTag[] = "NORM_LANDMARKS";
constexpr char kNormLandmarksName[] = "norm_landmarks";

constexpr float kLiteModelFractionDiff = 0.05;  // percentage

template <typename ProtoT>
ProtoT GetExpectedProto(absl::string_view filename) {
  ProtoT expected_proto;
  MP_EXPECT_OK(GetTextProto(file::JoinPath("./", kTestDataDirectory, filename),
                            &expected_proto, Defaults()));
  return expected_proto;
}

// Struct holding the parameters for parameterized PoseLandmarkerGraphTest
// class.
struct PoseLandmarkerGraphTestParams {
  // The name of this test, for convenience when displaying test results.
  std::string test_name;
  // The filename of the model to test.
  std::string input_model_name;
  // The filename of the test image.
  std::string test_image_name;
  // The expected output landmarks positions.
  std::optional<std::vector<NormalizedLandmarkList>> expected_landmarks_list;
  // The max value difference between expected_positions and detected positions.
  float landmarks_diff_threshold;
};

// Helper function to create a PoseLandmarkerGraph TaskRunner.
absl::StatusOr<std::unique_ptr<TaskRunner>> CreatePoseLandmarkerGraphTaskRunner(
    absl::string_view model_name) {
  Graph graph;

  auto& pose_landmarker = graph.AddNode(
      "mediapipe.tasks.vision.pose_landmarker."
      "PoseLandmarkerGraph");

  auto* options = &pose_landmarker.GetOptions<PoseLandmarkerGraphOptions>();
  options->mutable_base_options()->mutable_model_asset()->set_file_name(
      JoinPath("./", kTestDataDirectory, model_name));
  options->mutable_pose_detector_graph_options()->set_num_poses(1);
  options->mutable_base_options()->set_use_stream_mode(true);

  graph[Input<Image>(kImageTag)].SetName(kImageName) >>
      pose_landmarker.In(kImageTag);
  graph[Input<NormalizedRect>(kNormRectTag)].SetName(kNormRectName) >>
      pose_landmarker.In(kNormRectTag);

  pose_landmarker.Out(kNormLandmarksTag).SetName(kNormLandmarksName) >>
      graph[Output<std::vector<NormalizedLandmarkList>>(kNormLandmarksTag)];

  return TaskRunner::Create(
      graph.GetConfig(),
      absl::make_unique<tasks::core::MediaPipeBuiltinOpResolver>());
}

// Helper function to construct NormalizeRect proto.
NormalizedRect MakeNormRect(float x_center, float y_center, float width,
                            float height, float rotation) {
  NormalizedRect pose_rect;
  pose_rect.set_x_center(x_center);
  pose_rect.set_y_center(y_center);
  pose_rect.set_width(width);
  pose_rect.set_height(height);
  pose_rect.set_rotation(rotation);
  return pose_rect;
}

class PoseLandmarkerGraphTest
    : public testing::TestWithParam<PoseLandmarkerGraphTestParams> {};

TEST_P(PoseLandmarkerGraphTest, Succeeds) {
  MP_ASSERT_OK_AND_ASSIGN(
      Image image, DecodeImageFromFile(JoinPath("./", kTestDataDirectory,
                                                GetParam().test_image_name)));
  MP_ASSERT_OK_AND_ASSIGN(auto task_runner, CreatePoseLandmarkerGraphTaskRunner(
                                                GetParam().input_model_name));

  auto output_packets = task_runner->Process(
      {{kImageName, MakePacket<Image>(std::move(image))},
       {kNormRectName,
        MakePacket<NormalizedRect>(MakeNormRect(0.5, 0.5, 1.0, 1.0, 0))}});
  MP_ASSERT_OK(output_packets);

  if (GetParam().expected_landmarks_list) {
    const std::vector<NormalizedLandmarkList>& landmarks_lists =
        (*output_packets)[kNormLandmarksName]
            .Get<std::vector<NormalizedLandmarkList>>();
    EXPECT_THAT(landmarks_lists,
                Pointwise(Approximately(Partially(EqualsProto()),
                                        GetParam().landmarks_diff_threshold),
                          *GetParam().expected_landmarks_list));
  }
}

INSTANTIATE_TEST_SUITE_P(
    PoseLandmarkerGraphTests, PoseLandmarkerGraphTest,
    Values(PoseLandmarkerGraphTestParams{
        /* test_name= */ "PoseLandmarkerLite",
        /* input_model_name= */ kPoseLandmarkerModelBundleName,
        /* test_image_name= */ kPoseImageName,
        /* expected_landmarks_list= */
        {{GetExpectedProto<NormalizedLandmarkList>(
            kExpectedPoseLandmarksName)}},
        /* landmarks_diff_threshold= */ kLiteModelFractionDiff}),
    [](const TestParamInfo<PoseLandmarkerGraphTest::ParamType>& info) {
      return info.param.test_name;
    });

}  // namespace
}  // namespace pose_landmarker
}  // namespace vision
}  // namespace tasks
}  // namespace mediapipe
