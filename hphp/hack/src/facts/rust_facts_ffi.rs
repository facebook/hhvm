// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use facts_rust as facts;
use hhbc_string_utils_rust::without_xhp_mangling;
use ocamlrep_ocamlpool::ocaml_ffi_no_panic;
use oxidized::relative_path::RelativePath;

use facts::facts_parser::*;

ocaml_ffi_no_panic! {
    fn extract_as_json_ffi(
        flags: i32,
        filename: RelativePath,
        text: String,
        mangle_xhp: bool,
    ) -> Option<String> {
        extract_as_json_ffi0(
            ((1 << 0) & flags) != 0, // php5_compat_mode
            ((1 << 1) & flags) != 0, // hhvm_compat_mode
            ((1 << 2) & flags) != 0, // allow_new_attribute_syntax
            filename,
            &text,
            mangle_xhp,
        )
    }
}

fn extract_as_json_ffi0(
    php5_compat_mode: bool,
    hhvm_compat_mode: bool,
    allow_new_attribute_syntax: bool,
    filename: RelativePath,
    text: &str,
    mangle_xhp: bool,
) -> Option<String> {
    let opts = ExtractAsJsonOpts {
        php5_compat_mode,
        hhvm_compat_mode,
        allow_new_attribute_syntax,
        filename,
    };
    if mangle_xhp {
        extract_as_json(text, opts)
    } else {
        without_xhp_mangling(|| extract_as_json(text, opts))
    }
}
