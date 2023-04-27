// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ir::instr::Call;
use ir::instr::CallDetail;
use ir::instr::Hhbc;
use ir::FCallArgsFlags;
use ir::Func;
use ir::FuncBuilder;
use ir::Instr;
use ir::LocalId;
use ir::MemberOpBuilder;
use ir::MethodFlags;
use ir::MethodId;
use ir::SpecialClsRef;
use ir::StringInterner;
use ir::UnitBytesId;
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

            // We want 86pinit to be 'instance' but hackc marks it as 'static'.
            if method_info.name.is_86pinit(&strings) {
                method_info.is_static = crate::class::IsStatic::NonStatic;
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

    // 86pinit needs to call its base
    match func_info {
        FuncInfo::Method(mi) if mi.class.base.is_some() && mi.name.is_86pinit(&strings) => {
            let clsref = SpecialClsRef::ParentCls;
            let method = MethodId::_86pinit(&strings);
            let detail = CallDetail::FCallClsMethodSD { clsref, method };
            let call = Instr::call(Call {
                operands: Default::default(),
                context: UnitBytesId::NONE,
                detail,
                flags: FCallArgsFlags::default(),
                num_rets: 0,
                inouts: None,
                readonly: None,
                loc: builder.func.loc_id,
            });
            let iid = builder.func.alloc_instr(call);
            builder.func.blocks[Func::ENTRY_BID].iids.insert(0, iid);
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
