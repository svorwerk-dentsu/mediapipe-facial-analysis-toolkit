load("//mediapipe/framework/port:build_config.bzl", "mediapipe_proto_library")

licenses(["notice"])

package(default_visibility = ["//visibility:private"])

cc_library(name = "landmark_standardization",
    srcs        = ["landmark_standardization.cc"],
    visibility  = ["//visibility:public"],
    deps        = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/formats:landmark_cc_proto",
        "//mediapipe/framework/formats:detection_cc_proto",
    ],
    alwayslink = 1,
)

cc_library(name = "blank_image_calculator",
    srcs        = ["blank_image_calculator.cc"],
    visibility  = ["//visibility:public"],
    deps        = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework:timestamp",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/stream_handler:immediate_input_stream_handler",
        ":blank_image_calculator_cc_proto",
    ],
    alwayslink = 1,
)

exports_files(
    srcs = [
        "proctor_result.h",
    ]
)

cc_library(name = "proctor_result",
    hdrs        = ["proctor_result.h"],
    include_prefix = ".",
    visibility  = ["//visibility:public"],
)

cc_library(name = "proctor_result_calculator",
    srcs        = ["proctor_result_calculator.cc"],
    visibility  = ["//visibility:public"],
    deps        = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:timestamp",
        ":proctor_result",
        "//mediapipe/calculators/core:end_loop_calculator",
        "//mediapipe/calculators/core:begin_loop_calculator",
    ],
    alwayslink = 1,
)

mediapipe_proto_library(
    name = "blank_image_calculator_proto",
    srcs = ["blank_image_calculator.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_options_proto",
        "//mediapipe/framework:calculator_proto",
        "//mediapipe/util:color_proto",
    ],
)

cc_library(name = "constant_matrix_calculator",
    srcs        = ["constant_matrix_calculator.cc"],
    visibility  = ["//visibility:public"],
    deps        = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        ":constant_matrix_calculator_cc_proto",
    ],
    alwayslink = 1,
)

mediapipe_proto_library(
    name = "constant_matrix_calculator_proto",
    srcs = ["constant_matrix_calculator.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_options_proto",
        "//mediapipe/framework:calculator_proto",
        "//mediapipe/util:color_proto",
    ],
)

cc_library(name = "proctor_result_to_render_data_calculator",
    srcs = ["proctor_result_to_render_data_calculator.cc"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        "//mediapipe/util:render_data_cc_proto",
        ":proctor_result",
    ],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)
