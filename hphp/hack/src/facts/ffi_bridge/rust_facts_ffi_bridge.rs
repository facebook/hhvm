// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

use cxx::CxxString;
use oxidized::relative_path::RelativePath;
use rust_facts_ffi::extract_as_json_ffi0;

#[cxx::bridge]
mod ffi {
    extern "Rust" {
        pub fn hackc_extract_as_json_cpp_ffi(
            flags: i32,
            filename: &CxxString,
            source_text: &CxxString,
        ) -> String;
    }
}

pub fn hackc_extract_as_json_cpp_ffi(
    flags: i32,
    filename: &CxxString,
    source_text: &CxxString,
) -> String {
    use std::os::unix::ffi::OsStrExt;
    let filepath = RelativePath::make(
        oxidized::relative_path::Prefix::Dummy,
        std::path::PathBuf::from(std::ffi::OsStr::from_bytes(filename.as_bytes())),
    );
    match extract_as_json_ffi0(
        ((1 << 0) & flags) != 0, // php5_compat_mode
        ((1 << 1) & flags) != 0, // hhvm_compat_mode
        ((1 << 2) & flags) != 0, // allow_new_attribute_syntax
        ((1 << 3) & flags) != 0, // enable_xhp_class_modifier
        ((1 << 4) & flags) != 0, // disable_xhp_element_mangling
        ((1 << 5) & flags) != 0, // disallow_hash_comments
        filepath,
        source_text.as_bytes(),
        true, // mangle_xhp
    ) {
        Some(s) => s,
        None => String::new(),
    }
}
