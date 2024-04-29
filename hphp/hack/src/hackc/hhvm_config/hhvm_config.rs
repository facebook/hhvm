// Copyright (c) 2019; Facebook; Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use anyhow::Result;
use hhvm_options::HhvmConfig;
use options::ParserOptions;

/*
These helper functions are best-effort utilities for CLI tools like hackc
to read HHVM configuration. No guarantees are made about coverage;
ultimately the source of truth is HHVM, for how .hdf and .ini
files are turned into hackc/compile::NativeEnv (and its embedded options)

see hphp/runtime/base/config.{cpp,h} and runtime_option.{cpp,h}
*/

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
        &mut flags.po_const_static_props,
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
