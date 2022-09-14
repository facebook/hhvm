// Copyright (c) 2019; Facebook; Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use anyhow::Result;
use hhvm_options::HhvmConfig;
use options::HhbcFlags;

/**!
These helper functions are best-effort utilities for CLI tools like hackc
to read HHVM configuration. No guarantees are made about coverage;
ultimately the source of truth is HHVM, for how .hdf and .ini
files are turned into hackc/compile::NativeEnv (and its embedded options)

see hphp/runtime/base/config.{cpp,h} and runtime_option.{cpp,h}
*/

pub fn init_hhbc_flags(config: &HhvmConfig) -> Result<HhbcFlags> {
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
    init(&mut flags.repo_authoritative, "Repo.Authoritative")?;
    init(
        &mut flags.jit_enable_rename_function,
        "Eval.JitEnableRenameFunction",
    )?;
    init(
        &mut flags.jit_enable_rename_function,
        "JitEnableRenameFunction",
    )?;
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

    // ini might use hhvm.array_provenance
    // hdf might use Eval.ArrayProvenance
    // But super unclear here
    init(&mut flags.array_provenance, "Eval.ArrayProvenance")?;
    init(&mut flags.array_provenance, "array_provenance")?;

    // Only hdf version
    flags.fold_lazy_class_keys = config.get_bool("Eval.FoldLazyClassKeys")?.unwrap_or(true);
    Ok(flags)
}
