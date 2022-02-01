// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use cxx::CxxString;

#[cxx::bridge]
pub mod compile_ffi {
    extern "Rust" {
        fn hackc_parser_flags_ffi(
            config_path: &CxxString,
            hdf_options: &CxxString,
            ini_options: &CxxString,
        ) -> u32;

        fn hackc_hhbc_flags_ffi(
            config_path: &CxxString,
            hdf_options: &CxxString,
            ini_options: &CxxString,
        ) -> u32;
    }
}

///////////////////////////////////////////////////////////////////////////////////
// Opaque to C++.

fn get_hhvm_options(
    config_path: &CxxString,
    hdf_options: &CxxString,
    ini_options: &CxxString,
) -> hhvm_options::HhvmOptions {
    use std::os::unix::ffi::OsStrExt;

    let path = std::ffi::OsStr::from_bytes(config_path.as_bytes()); // std::path::PathBuf::from();
    let config_files = if path == "" {
        vec![]
    } else {
        vec![std::path::PathBuf::from(path)]
    };

    let hdf_values = std::ffi::OsStr::from_bytes(hdf_options.as_bytes())
        .to_str()
        .expect("hdf_options don't work")
        .split(' ')
        .map(str::to_string)
        .collect();

    let ini_values = std::ffi::OsStr::from_bytes(ini_options.as_bytes())
        .to_str()
        .expect("ini_values don't work")
        .split(' ')
        .map(str::to_string)
        .collect();

    hhvm_options::HhvmOptions {
        config_files,
        hdf_values,
        ini_values,
    }
}

fn hackc_parser_flags_ffi(
    config_path: &CxxString,
    hdf_options: &CxxString,
    ini_options: &CxxString,
) -> u32 {
    use compile::ParserFlags;
    let options = get_hhvm_options(config_path, hdf_options, ini_options);

    let mut parser_options = ParserFlags::empty();
    let config = options.to_config().expect("Invalid configuration options");

    // Note: Could only find examples of Hack.Lang.AbstractStaticProps
    if let Some(true) = config.get_bool("Hack.Lang.AbstractStaticProps") {
        parser_options |= ParserFlags::ABSTRACT_STATIC_PROPS;
    }
    // TODO: I'm pretty sure allow_new_attribute_syntax is dead and we can kill this option
    if let Some(true) = config.get_bool("hack.lang.allow_new_attribute_syntax") {
        parser_options |= ParserFlags::ALLOW_NEW_ATTRIBUTE_SYNTAX;
    }
    // Both hdf and ini versions are being used
    if let Some(true) = config.get_bool("Hack.Lang.AllowUnstableFeatures") {
        parser_options |= ParserFlags::ALLOW_UNSTABLE_FEATURES;
    }
    // TODO: could not find examples of const_default_func_args, kill it in options_cli.rs
    if let Some(true) = config.get_bool("Hack.Lang.ConstDefaultFuncArgs") {
        parser_options |= ParserFlags::CONST_DEFAULT_FUNC_ARGS;
    }
    // Only hdf version found in use
    if let Some(true) = config.get_bool("Hack.Lang.ConstStaticProps") {
        parser_options |= ParserFlags::CONST_STATIC_PROPS;
    }
    // TODO: Only seems to exist in HackFuzzEmitter.php, look to kill this
    if let Some(true) = config.get_bool("Hack.Lang.DisableArray") {
        parser_options |= ParserFlags::DISABLE_ARRAY;
    }
    // TODO: Kill disable_array_typehint
    // TODO: Kill disable_lval_as_an_expression
    // Only hdf option in use. Kill variant in options_cli.rs
    if let Some(true) = config.get_bool("Hack.Lang.DisableUnsetClassConst") {
        parser_options |= ParserFlags::DISABLE_UNSET_CLASS_CONST;
    }
    // Only hdf option in use
    if let Some(true) = config.get_bool("Hack.Lang.DisallowInstMeth") {
        parser_options |= ParserFlags::DISALLOW_INST_METH;
    }
    // Both ini and hdf variants in use
    if let Some(true) = config.get_bool("Hack.Lang.DisableXHPElementMangling") {
        parser_options |= ParserFlags::DISABLE_XHP_ELEMENT_MANGLING;
    }
    // Both ini and hdf variants in use
    if let Some(true) = config.get_bool("Hack.Lang.DisallowFunAndClsMethPseudoFuncs") {
        parser_options |= ParserFlags::DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS;
    }
    // Only hdf option in use
    if let Some(true) = config.get_bool("Hack.Lang.DisallowFuncPtrsInConstants") {
        parser_options |= ParserFlags::DISALLOW_FUNC_PTRS_IN_CONSTANTS;
    }
    // Only hdf option in use
    if let Some(true) = config.get_bool("Hack.Lang.EnableEnumClasses") {
        parser_options |= ParserFlags::ENABLE_ENUM_CLASSES;
    }
    // Both options in use
    if let Some(true) = config.get_bool("Hack.Lang.EnableXHPClassModifier") {
        parser_options |= ParserFlags::ENABLE_XHP_CLASS_MODIFIER;
    }
    // Only hdf option in use. Kill variant in options_cli.rs
    if let Some(true) = config.get_bool("Hack.Lang.EnableClassLevelWhereClauses") {
        parser_options |= ParserFlags::ENABLE_CLASS_LEVEL_WHERE_CLAUSES;
    }
    return parser_options.bits();
}

fn hackc_hhbc_flags_ffi(
    config_path: &CxxString,
    hdf_options: &CxxString,
    ini_options: &CxxString,
) -> u32 {
    use compile::HHBCFlags;
    let options = get_hhvm_options(config_path, hdf_options, ini_options);

    let mut hhbc_options = HHBCFlags::empty();
    // Defaults
    hhbc_options |= HHBCFlags::EMIT_METH_CALLER_FUNC_POINTERS;
    hhbc_options |= HHBCFlags::FOLD_LAZY_CLASS_KEYS;
    let config = options.to_config().expect("Invalid configuration options");

    // Only ini version in use
    if let Some(true) = config.get_bool("php7.ltr_assign") {
        hhbc_options |= HHBCFlags::LTR_ASSIGN;
    }
    // Only ini version in use
    if let Some(true) = config.get_bool("php7.uvs") {
        hhbc_options |= HHBCFlags::UVS;
    }
    // Both variants in use
    if let Some(true) = config.get_bool("Repo.Authoritative") {
        hhbc_options |= HHBCFlags::AUTHORITATIVE;
    }
    // HDF uses both Eval.JitEnableRenameFunction and JitEnableRenameFunction
    // ini only uses the hhvm.jit_enable_rename_function
    if let Some(true) = config.get_bool("Eval.JitEnableRenameFunction") {
        hhbc_options |= HHBCFlags::JIT_ENABLE_RENAME_FUNCTION;
    }
    if let Some(true) = config.get_bool("JitEnableRenameFunction") {
        hhbc_options |= HHBCFlags::JIT_ENABLE_RENAME_FUNCTION;
    }
    // Only hdf version in use
    if let Some(true) = config.get_bool("Eval.LogExternCompilerPerf") {
        hhbc_options |= HHBCFlags::LOG_EXTERN_COMPILER_PERF;
    }
    // I think only the hdf is used correctly
    if let Some(true) = config.get_bool("Eval.EnableIntrinsicsExtension") {
        hhbc_options |= HHBCFlags::ENABLE_INTRINSICS_EXTENSION;
    }
    // Only the hdf versions used
    if let Some(true) = config.get_bool("Eval.EmitClsMethPointers") {
        hhbc_options |= HHBCFlags::EMIT_CLS_METH_POINTERS;
    }
    // Only the hdf versions used. Can kill variant in options_cli.rs
    if let Some(b) = config.get_bool("Eval.EmitMethCallerFuncPointers") {
        if b {
            hhbc_options |= HHBCFlags::EMIT_METH_CALLER_FUNC_POINTERS;
        } else {
            hhbc_options &= !(HHBCFlags::EMIT_METH_CALLER_FUNC_POINTERS);
        }
    }
    // ini just uses hhvm.enable_implicit_context
    // hdf uses Eval.EnableImplicitContext
    if let Some(true) = config.get_bool("Eval.EnableImplicitContext") {
        hhbc_options |= HHBCFlags::ENABLE_IMPLICIT_CONTEXT;
    }
    if let Some(true) = config.get_bool("enable_implicit_context") {
        hhbc_options |= HHBCFlags::ENABLE_IMPLICIT_CONTEXT;
    }
    // ini might use hhvm.array_provenance
    // hdf might use Eval.ArrayProvenance
    // But super unclear here
    if let Some(true) = config.get_bool("Eval.ArrayProvenance") {
        hhbc_options |= HHBCFlags::ARRAY_PROVENANCE;
    }
    if let Some(true) = config.get_bool("array_provenance") {
        hhbc_options |= HHBCFlags::ARRAY_PROVENANCE;
    }
    // Only hdf version
    if let Some(b) = config.get_bool("Eval.FoldLazyClassKeys") {
        if b {
            hhbc_options |= HHBCFlags::FOLD_LAZY_CLASS_KEYS;
        } else {
            hhbc_options &= !(HHBCFlags::FOLD_LAZY_CLASS_KEYS);
        }
    }

    return hhbc_options.bits();
}
