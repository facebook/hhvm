// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ir::instr::Hhbc;
use ir::Func;
use ir::FuncBuilder;
use ir::Instr;
use ir::LocId;
use ir::LocalId;
use ir::MemberOpBuilder;
use ir::MethodFlags;
use ir::SpecialClsRef;
use ir::StringInterner;
use log::trace;

use crate::func::FuncInfo;
use crate::func::MethodInfo;

pub(crate) fn lower_func<'a>(
    mut func: Func<'a>,
    func_info: &mut FuncInfo<'_>,
    strings: Arc<StringInterner>,
) -> Func<'a> {
    trace!(
        "{} Before Lower: {}",
        func_info.name_id().display(&strings),
        ir::print::DisplayFunc::new(&func, true, &strings)
    );

    // In a closure we implicitly load all the properties as locals - so start
    // with that as a prelude to all entrypoints.
    match *func_info {
        FuncInfo::Method(ref mut method_info) => {
            if method_info.flags.contains(MethodFlags::IS_CLOSURE_BODY) {
                load_closure_vars(&mut func, method_info, &strings);
            }
        }
        FuncInfo::Function(_) => {}
    }

    // Start by 'unasync'ing the Func.
    ir::passes::unasync(&mut func);
    trace!(
        "After unasync: {}",
        ir::print::DisplayFunc::new(&func, true, &strings)
    );

    let mut builder = FuncBuilder::with_func(func, Arc::clone(&strings));

    match func_info {
        FuncInfo::Method(mi) if mi.name.is_86pinit(&strings) => {
            rewrite_86pinit(&mut builder, mi);
        }
        _ => {}
    }

    // Simplify various Instrs.
    super::instrs::lower_instrs(&mut builder, func_info);

    trace!(
        "After lower_instrs: {}",
        ir::print::DisplayFunc::new(&builder.func, true, &strings)
    );

    // Write the complex constants out as a prelude to the function.
    super::constants::write_constants(&mut builder);

    let mut func = builder.finish();

    ir::passes::split_critical_edges(&mut func, true);

    ir::passes::clean::run(&mut func);

    trace!(
        "After Lower: {}",
        ir::print::DisplayFunc::new(&func, true, &strings)
    );

    func
}

fn call_base_func(builder: &mut FuncBuilder<'_>, method_info: &MethodInfo<'_>, loc: LocId) {
    if method_info.class.base.is_some() {
        let clsref = SpecialClsRef::ParentCls;
        let method = method_info.name;
        builder.emit(Instr::method_call_special(clsref, method, &[], loc));
    }
}

fn rewrite_86pinit(builder: &mut FuncBuilder<'_>, method_info: &MethodInfo<'_>) {
    // In HHVM 86pinit is only used to initialize "complex" properties (and
    // doesn't exist if there aren't any). For textual we change that to use it
    // to initialize all properties and be guaranteed to exist.

    builder.start_block(Func::ENTRY_BID);
    let saved = std::mem::take(&mut builder.cur_block_mut().iids);
    let loc = builder.func.loc_id;

    call_base_func(builder, method_info, loc);

    // Init the properties.
    for prop in &method_info.class.properties {
        match prop {
            ir::Property {
                name,
                flags,
                initial_value: Some(initial_value),
                ..
            } if !flags.is_static() => {
                let vid = builder.emit_constant(initial_value.clone().into());
                MemberOpBuilder::base_h(loc).emit_set_m_pt(builder, *name, vid);
            }
            _ => {}
        }
    }

    builder.cur_block_mut().iids.extend(saved);
}

fn load_closure_vars(func: &mut Func<'_>, method_info: &MethodInfo<'_>, strings: &StringInterner) {
    let mut instrs = Vec::new();

    let loc = func.loc_id;

    for prop in &method_info.class.properties {
        // Property names are the variable names without the '$'.
        let prop_str = strings.lookup_bstr(prop.name.id);
        let mut var = prop_str.to_vec();
        var.insert(0, b'$');
        let lid = LocalId::Named(strings.intern_bytes(var));

        let iid = func.alloc_instr(Instr::MemberOp(
            MemberOpBuilder::base_h(loc).query_pt(prop.name),
        ));
        instrs.push(iid);
        instrs.push(func.alloc_instr(Instr::Hhbc(Hhbc::SetL(iid.into(), lid, loc))));
    }

    func.block_mut(Func::ENTRY_BID).iids.splice(0..0, instrs);
}
