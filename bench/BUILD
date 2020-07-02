load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")


py_binary(
    name = "bench",
    srcs = ["scripts/bench.py"],
)

cc_library(
    name = "benchlib",
    srcs = [],
    hdrs = ["include/bench/benc.h", "include/bench/colors.h"],
    includes = ["include"],
    deps = [],
    visibility = ["//visibility:public"]
)