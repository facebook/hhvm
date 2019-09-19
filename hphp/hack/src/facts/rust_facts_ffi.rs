// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use facts_rust as facts;
use ocaml::caml;
use oxidized::relative_path::RelativePath;

use facts::facts_parser::*;

caml!(extract_as_json_ffi(
    flags,
    filename,
    text,
    mangle_xhp
) {
    use ocaml::ToValue;
    let flags = flags.i32_val();
    return extract_as_json_ffi0(
        ((1 << 0) & flags) != 0, // php5_compat_mode
        ((1 << 1) & flags) != 0, // hhvm_compat_mode
        ((1 << 2) & flags) != 0, // allow_new_attribute_syntax
        RelativePath::from_ocamlvalue(&filename),
        ocaml::Str::from(text).as_str(),
        mangle_xhp.i32_val() != 0,
    ).to_value();
});

fn extract_as_json_ffi0(
    php5_compat_mode: bool,
    hhvm_compat_mode: bool,
    allow_new_attribute_syntax: bool,
    filename: RelativePath,
    text: &str,
    mangle_xhp: bool,
) -> String {
    let opts = ExtractAsJsonOpts {
        php5_compat_mode,
        hhvm_compat_mode,
        allow_new_attribute_syntax,
        filename,
    };
    // return empty string in case of failure (ambiguous because "" is not a valid JSON)
    // as ocaml-rs doesn't offer conversion from/to option (caller will use None for failure)
    let f = || extract_as_json(text, opts).unwrap_or(String::from(""));
    if mangle_xhp {
        f()
    } else {
        without_xhp_mangling(f)
    }
}
