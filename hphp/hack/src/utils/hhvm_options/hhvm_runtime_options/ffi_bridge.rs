// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// This is needed by cxx
#![allow(dead_code)]

// clippy doesn't like "Process_GetCPUModel"
#[allow(unknown_lints)]
#[cxx::bridge]
pub mod cxx_ffi {
    unsafe extern "C++" {
        include!("hphp/hack/src/utils/hhvm_options/hhvm_runtime_options/ffi_bridge.h");

        fn Process_GetCPUModel() -> String;
        fn Process_GetHostName() -> String;
    }
}
