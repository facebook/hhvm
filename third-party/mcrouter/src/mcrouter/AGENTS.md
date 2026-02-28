# mcrouter Agent Instructions

## OSS Build

- OSS CMakeLists.txt is at `mcrouter/public_tld/CMakeLists.txt`
- OSS stub files (e.g., `mcrouter_sr_deps-impl.h`) provide minimal implementations for open source builds; real implementations are in `mcrouter/facebook/`
- Build output paths like `/fbcode_builder_getdeps/shipit/mcrouter/` map to `fbcode/mcrouter/` in the repo
- When `install(TARGETS ...)` includes targets with `PUBLIC_HEADER` property, add `PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}` to avoid warnings
