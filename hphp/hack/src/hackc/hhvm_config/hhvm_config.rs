// Copyright (c) 2019; Facebook; Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use anyhow::Result;
use hhvm_options::HhvmConfig;
use options::HhbcFlags;
use options::JitEnableRenameFunction;
use options::ParserOptions;

/*
These helper functions are best-effort utilities for CLI tools like hackc
to read HHVM configuration. No guarantees are made about coverage;
ultimately the source of truth is HHVM, for how .hdf and .ini
files are turned into hackc/compile::NativeEnv (and its embedded options)

see hphp/runtime/base/config.{cpp,h} and runtime_option.{cpp,h}
*/

pub fn hhbc_flags(config: &HhvmConfig) -> Result<HhbcFlags> {
    let mut flags = HhbcFlags::default();
    // Use the config setting if provided; otherwise preserve existing value.
    let init = |flag: &mut bool, name: &str| -> Result<()> {
        match config.get_bool(name)? {
            Some(b) => Ok(*flag = b),
            None => Ok(()),
        }
    };

    init(&mut flags.ltr_assign, "php7.ltr_assign")?;
    init(&mut flags.uvs, "php7.uvs")?;
    init(
        &mut flags.log_extern_compiler_perf,
        "Eval.LogExternCompilerPerf",
    )?;
    init(
        &mut flags.enable_intrinsics_extension,
        "Eval.EnableIntrinsicsExtension",
    )?;
    init(
        &mut flags.emit_cls_meth_pointers,
        "Eval.EmitClsMethPointers",
    )?;

    // Only the hdf versions used. Can kill variant in options_cli.rs
    flags.emit_meth_caller_func_pointers = config
        .get_bool("Eval.EmitMethCallerFuncPointers")?
        .unwrap_or(true);

    flags.readonly_nonlocal_infer = config
        .get_bool("Hack.Lang.ReadonlyNonlocalInference")?
        .unwrap_or(false);

    flags.optimize_reified_param_checks = config
        .get_bool("Hack.Lang.OptimizeReifiedParamChecks")?
        .unwrap_or(false);

    flags.enable_native_enum_class_labels = config
        .get_bool("Eval.EmitNativeEnumClassLabels")?
        .unwrap_or(false);

    // ini might use hhvm.array_provenance
    // hdf might use Eval.ArrayProvenance
    // But super unclear here
    init(&mut flags.array_provenance, "Eval.ArrayProvenance")?;
    init(&mut flags.array_provenance, "array_provenance")?;

    // Only hdf version
    flags.fold_lazy_class_keys = config.get_bool("Eval.FoldLazyClassKeys")?.unwrap_or(true);
    Ok(flags)
}

pub fn jit_enable_rename_function(config: &HhvmConfig) -> Result<JitEnableRenameFunction> {
    match config.get_uint32("Eval.JitEnableRenameFunction")? {
        Some(b) => {
            if b == 1 {
                Ok(JitEnableRenameFunction::Enable)
            } else if b == 2 {
                Ok(JitEnableRenameFunction::RestrictedEnable)
            } else {
                Ok(JitEnableRenameFunction::Disable)
            }
        }
        None => Ok(JitEnableRenameFunction::Disable),
    }
}

pub fn parser_options(config: &HhvmConfig) -> Result<ParserOptions> {
    let mut flags = ParserOptions::default();

    // Use the config setting if provided; otherwise preserve default.
    let init = |flag: &mut bool, name: &str| -> Result<()> {
        match config.get_bool(name)? {
            Some(b) => Ok(*flag = b),
            None => Ok(()),
        }
    };

    // Note: Could only find examples of Hack.Lang.AbstractStaticProps
    init(
        &mut flags.po_abstract_static_props,
        "Hack.Lang.AbstractStaticProps",
    )?;

    // Both hdf and ini versions are being used
    init(
        &mut flags.po_allow_unstable_features,
        "Hack.Lang.AllowUnstableFeatures",
    )?;

    // TODO: could not find examples of const_default_func_args, kill it in options_cli.rs
    init(
        &mut flags.po_const_default_func_args,
        "Hack.Lang.ConstDefaultFuncArgs",
    )?;

    // Only hdf version found in use
    init(
        &mut flags.tco_const_static_props,
        "Hack.Lang.ConstStaticProps",
    )?;

    // TODO: Kill disable_lval_as_an_expression

    // Both ini and hdf variants in use
    init(
        &mut flags.po_disable_xhp_element_mangling,
        "Hack.Lang.DisableXHPElementMangling",
    )?;

    // Only hdf option in use
    init(
        &mut flags.po_disallow_func_ptrs_in_constants,
        "Hack.Lang.DisallowFuncPtrsInConstants",
    )?;

    // Both options in use
    init(
        &mut flags.po_enable_xhp_class_modifier,
        "Hack.Lang.EnableXHPClassModifier",
    )?;

    // Only hdf option in use. Kill variant in options_cli.rs
    init(
        &mut flags.po_enable_class_level_where_clauses,
        "Hack.Lang.EnableClassLevelWhereClauses",
    )?;

    // Only hdf option in use
    init(
        &mut flags.po_disallow_direct_superglobals_refs,
        "Hack.Lang.DisallowDirectSuperglobalsRefs",
    )?;

    Ok(flags)
}
