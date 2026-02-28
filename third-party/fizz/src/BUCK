load("@fbcode_macros//build_defs:native_rules.bzl", "buck_genrule")

oncall("secure_pipes")

buck_genrule(
    name = "fizz-config.h",
    srcs = {file: file for file in glob([
               "fizz/cmake/*",
               "build/fbcode_builder/CMake/*",
           ])} |
           {
               "CMakeLists.txt": "//fizz:CMakeListsForBuck2.txt",
               "fizz/cmake/CheckAtomic.cmake": "//fizz:cmake/CheckAtomic.cmake",
               "fizz/cmake/FizzOptions.cmake": "//fizz:cmake/FizzOptions.cmake",
               "fizz/fizz-config.h.in": "//fizz:fizz-config.h.in",
           },
    out = "fizz-config.h",
    cmd = "cmake . && mv fizz/fizz-config.h $OUT",
    default_target_platform = "prelude//platforms:default",
    labels = ["third-party:homebrew:cmake"],
    remote = False,
)
