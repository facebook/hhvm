// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::Requirement;
use ir::SymbolRefs;
use ir::func::DefaultValue;
use ir::func::ExFrame;
use ir::instr::BaseOp;
use ir::instr::FinalOp;
use ir::instr::Hhbc;
use ir::instr::IntermediateOp;
use ir::instr::IteratorArgs;
use ir::instr::MemberKey;
use ir::instr::MemberOp;
use ir::instr::Special;
use ir::instr::Terminator;
// This is reasonable because if we compare everything then we'll end up pulling
// in everything...
use ir::*;

use crate::CmpContext;
use crate::CmpError;
use crate::MapName;
use crate::Result;
use crate::cmp_eq;
use crate::cmp_map_t;
use crate::cmp_option;
use crate::cmp_slice;

pub fn cmp_ir(a: &Unit, b: &Unit) -> Result {
    cmp_unit(a, b).with_raw(|| "unit".to_string())
}

fn cmp_attribute(a: &Attribute, b: &Attribute) -> Result {
    cmp_eq(a.name, b.name).qualified("name")?;
    cmp_slice(&a.arguments, &b.arguments, cmp_typed_value).qualified("arguments")?;
    Ok(())
}

fn cmp_attributes(a: &[Attribute], b: &[Attribute]) -> Result {
    cmp_map_t(a, b, cmp_attribute)
}

fn cmp_block(a_block: &Block, b_block: &Block) -> Result {
    cmp_eq(a_block.params.len(), b_block.params.len()).qualified("params.len")?;
    cmp_eq(a_block.iids.len(), b_block.iids.len()).qualified("iids.len")?;

    cmp_option(
        a_block.pname_hint.as_ref(),
        b_block.pname_hint.as_ref(),
        cmp_eq,
    )
    .qualified("pname_hint")?;

    cmp_eq(a_block.tcid, b_block.tcid).qualified("tcid")?;

    cmp_slice(&a_block.iids, &b_block.iids, cmp_eq).qualified("iids")?;

    Ok(())
}

fn cmp_cc_reified(a: &CcReified, b: &CcReified) -> Result {
    let CcReified {
        is_class: a_is_class,
        index: a_index,
        types: a_types,
    } = a;
    let CcReified {
        is_class: b_is_class,
        index: b_index,
        types: b_types,
    } = b;
    cmp_eq(a_is_class, b_is_class).qualified("is_class")?;
    cmp_eq(a_index, b_index).qualified("index")?;
    cmp_slice(a_types, b_types, cmp_eq).qualified("types")?;
    Ok(())
}

fn cmp_cc_this(a: &CcThis, b: &CcThis) -> Result {
    let CcThis { types: a_types } = a;
    let CcThis { types: b_types } = b;
    cmp_slice(a_types, b_types, cmp_eq).qualified("types")?;
    Ok(())
}

fn cmp_class(a: &Class, b: &Class) -> Result {
    let Class {
        attributes: a_attributes,
        base: a_base,
        constants: a_constants,
        ctx_constants: a_ctx_constants,
        doc_comment: a_doc_comment,
        enum_type: a_enum_type,
        enum_includes: a_enum_includes,
        flags: a_flags,
        implements: a_implements,
        methods: a_methods,
        name: a_name,
        properties: a_properties,
        requirements: a_requirements,
        span: a_span,
        type_constants: a_type_constants,
        upper_bounds: a_upper_bounds,
        tparams: a_tparams,
        uses: a_uses,
    } = a;
    let Class {
        attributes: b_attributes,
        base: b_base,
        constants: b_constants,
        ctx_constants: b_ctx_constants,
        doc_comment: b_doc_comment,
        enum_type: b_enum_type,
        enum_includes: b_enum_includes,
        flags: b_flags,
        implements: b_implements,
        methods: b_methods,
        name: b_name,
        properties: b_properties,
        requirements: b_requirements,
        span: b_span,
        type_constants: b_type_constants,
        upper_bounds: b_upper_bounds,
        tparams: b_tparams,
        uses: b_uses,
    } = b;

    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_option(
        a_base.as_ref().into_option(),
        b_base.as_ref().into_option(),
        cmp_eq,
    )
    .qualified("base")?;
    cmp_map_t(a_constants, b_constants, cmp_constant).qualified("constants")?;
    cmp_map_t(a_ctx_constants, b_ctx_constants, cmp_ctx_constant).qualified("ctx_constants")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    cmp_option(
        a_enum_type.as_ref().into_option(),
        b_enum_type.as_ref().into_option(),
        cmp_type_info,
    )
    .qualified("enum_type")?;
    cmp_slice(a_enum_includes.iter(), b_enum_includes.iter(), cmp_eq).qualified("enum_includes")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_slice(a_implements.iter(), b_implements.iter(), cmp_eq).qualified("implements")?;
    cmp_map_t(a_methods, b_methods, cmp_method).qualified("methods")?;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_map_t(a_properties, b_properties, cmp_property).qualified("properties")?;
    cmp_slice(a_requirements, b_requirements, cmp_requirement).qualified("requirements")?;
    cmp_span(a_span, b_span).qualified("span")?;
    cmp_slice(a_type_constants, b_type_constants, cmp_type_constant).qualified("type_constants")?;
    cmp_slice(a_upper_bounds, b_upper_bounds, cmp_upper_bounds).qualified("upper_bounds")?;
    cmp_slice(a_tparams.iter(), b_tparams.iter(), cmp_eq).qualified("tparams")?;
    cmp_slice(a_uses.iter(), b_uses.iter(), cmp_eq).qualified("uses")?;
    Ok(())
}

fn cmp_coeffects(a: &Coeffects, b: &Coeffects) -> Result {
    let Coeffects {
        static_coeffects: a_static_coeffects,
        unenforced_static_coeffects: a_unenforced_static_coeffects,
        fun_param: a_fun_param,
        cc_param: a_cc_param,
        cc_this: a_cc_this,
        cc_reified: a_cc_reified,
        closure_parent_scope: a_closure_parent_scope,
        generator_this: a_generator_this,
        caller: a_caller,
    } = a;
    let Coeffects {
        static_coeffects: b_static_coeffects,
        unenforced_static_coeffects: b_unenforced_static_coeffects,
        fun_param: b_fun_param,
        cc_param: b_cc_param,
        cc_this: b_cc_this,
        cc_reified: b_cc_reified,
        closure_parent_scope: b_closure_parent_scope,
        generator_this: b_generator_this,
        caller: b_caller,
    } = b;

    cmp_slice(a_static_coeffects, b_static_coeffects, cmp_eq).qualified("static_coeffects")?;

    cmp_slice(
        a_unenforced_static_coeffects,
        b_unenforced_static_coeffects,
        cmp_eq,
    )
    .qualified("unenforced_static_coeffects")?;

    cmp_slice(a_fun_param, b_fun_param, cmp_eq).qualified("fun_param")?;

    cmp_slice(a_cc_param, b_cc_param, cmp_eq).qualified("cc_param")?;

    cmp_slice(a_cc_this, b_cc_this, cmp_cc_this).qualified("cc_this")?;

    cmp_slice(a_cc_reified, b_cc_reified, cmp_cc_reified).qualified("cc_reified")?;

    cmp_eq(a_closure_parent_scope, b_closure_parent_scope).qualified("closure_parent_scope")?;
    cmp_eq(a_generator_this, b_generator_this).qualified("generator_this")?;
    cmp_eq(a_caller, b_caller).qualified("caller")?;

    Ok(())
}

fn cmp_imm(a_const: &Immediate, b_const: &Immediate) -> Result {
    cmp_eq(
        std::mem::discriminant(a_const),
        std::mem::discriminant(b_const),
    )?;

    match (a_const, b_const) {
        (Immediate::EnumClassLabel(a), Immediate::EnumClassLabel(b)) => {
            cmp_eq(*a, *b).qualified("enum_class_label")?
        }
        (Immediate::Named(a), Immediate::Named(b)) => cmp_eq(a, b).qualified("named")?,
        (Immediate::NewCol(a), Immediate::NewCol(b)) => cmp_eq(a, b).qualified("new_col")?,
        (Immediate::Dir, Immediate::Dir)
        | (Immediate::File, Immediate::File)
        | (Immediate::FuncCred, Immediate::FuncCred)
        | (Immediate::Method, Immediate::Method) => {}
        (a, b) => cmp_typed_value(
            &a.clone().try_into().unwrap(),
            &b.clone().try_into().unwrap(),
        )
        .qualified("typed_value")?,
    }

    Ok(())
}

fn cmp_ctx_constant(a: &CtxConstant, b: &CtxConstant) -> Result {
    let CtxConstant {
        name: a_name,
        recognized: a_recognized,
        unrecognized: a_unrecognized,
        is_abstract: a_is_abstract,
    } = a;
    let CtxConstant {
        name: b_name,
        recognized: b_recognized,
        unrecognized: b_unrecognized,
        is_abstract: b_is_abstract,
    } = b;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_eq(a_recognized, b_recognized).qualified("recognized")?;
    cmp_eq(a_unrecognized, b_unrecognized).qualified("unrecognized")?;
    cmp_eq(a_is_abstract, b_is_abstract).qualified("is_abstract")?;
    Ok(())
}

fn cmp_ex_frame(a_frame: (&ExFrameId, &ExFrame), b_frame: (&ExFrameId, &ExFrame)) -> Result {
    cmp_eq(a_frame.0, b_frame.0).qualified("id")?;
    cmp_eq(a_frame.1.parent, b_frame.1.parent).qualified("parent")?;
    cmp_eq(a_frame.1.catch_bid, b_frame.1.catch_bid).qualified("catch_bid")?;
    Ok(())
}

fn cmp_fatal(a: &Fatal, b: &Fatal) -> Result {
    let Fatal {
        op: a_op,
        loc: a_loc,
        message: a_message,
    } = a;
    let Fatal {
        op: b_op,
        loc: b_loc,
        message: b_message,
    } = b;
    cmp_eq(a_op, b_op).qualified("op")?;
    cmp_src_loc(a_loc, b_loc).qualified("loc")?;
    cmp_eq(a_message, b_message).qualified("message")?;
    Ok(())
}

fn cmp_body(a: &Func, b: &Func) -> Result {
    let Func {
        attributes: a_attributes,
        attrs: a_attrs,
        coeffects: a_coeffects,
        doc_comment: a_doc_comment,
        is_memoize_wrapper: a_is_memoize_wrapper,
        is_memoize_wrapper_lsb: a_is_memoize_wrapper_lsb,
        span: a_span,
        num_iters: a_num_iters,
        return_type: a_return_type,
        upper_bounds: a_upper_bounds,
        tparam_info: a_tparam_info,
        repr:
            ir::IrRepr {
                blocks: a_blocks,
                imms: _,
                ex_frames: a_ex_frames,
                instrs: a_instrs,
                locs: _,
                params: a_params,
            },
    } = a;
    let Func {
        attributes: b_attributes,
        attrs: b_attrs,
        coeffects: b_coeffects,
        doc_comment: b_doc_comment,
        is_memoize_wrapper: b_is_memoize_wrapper,
        is_memoize_wrapper_lsb: b_is_memoize_wrapper_lsb,
        span: b_span,
        num_iters: b_num_iters,
        return_type: b_return_type,
        upper_bounds: b_upper_bounds,
        tparam_info: b_tparam_info,
        repr:
            ir::IrRepr {
                blocks: b_blocks,
                imms: _,
                ex_frames: b_ex_frames,
                instrs: b_instrs,
                locs: _,
                params: b_params,
            },
    } = b;

    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    cmp_coeffects(a_coeffects, b_coeffects).qualified("coeffects")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    cmp_eq(a_is_memoize_wrapper, b_is_memoize_wrapper).qualified("is_memoize_wrapper")?;
    cmp_eq(a_is_memoize_wrapper_lsb, b_is_memoize_wrapper_lsb)
        .qualified("is_memoize_wrapper_lsb")?;
    cmp_eq(a_num_iters, b_num_iters).qualified("num_iters")?;
    cmp_slice(a_params, b_params, cmp_param).qualified("params")?;
    cmp_option(
        a_return_type.as_ref().into(),
        b_return_type.as_ref().into(),
        cmp_type_info,
    )
    .qualified("return_type")?;
    cmp_slice(a_tparam_info.iter(), b_tparam_info.iter(), cmp_eq).qualified("tparam_info")?;

    cmp_slice(
        a_upper_bounds.iter(),
        b_upper_bounds.iter(),
        cmp_tparam_bounds,
    )
    .qualified("upper_bounds")?;

    cmp_slice(a_blocks.iter(), b_blocks.iter(), cmp_block).qualified("blocks")?;
    cmp_map_t(a_ex_frames.iter(), b_ex_frames.iter(), cmp_ex_frame).qualified("ex_frames")?;
    cmp_slice(
        a_instrs.iter().map(|i| (i, a)),
        b_instrs.iter().map(|i| (i, b)),
        cmp_instr,
    )
    .qualified("instrs")?;

    cmp_span(a_span, b_span).qualified("span")?;

    Ok(())
}

fn cmp_function(a: &Function, b: &Function) -> Result {
    let Function {
        flags: a_flags,
        name: a_name,
        body: a_body,
    } = a;
    let Function {
        flags: b_flags,
        name: b_name,
        body: b_body,
    } = b;

    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_body(a_body, b_body).qualified("body")?;

    Ok(())
}

fn cmp_constant(a: &Constant, b: &Constant) -> Result {
    let Constant {
        name: a_name,
        value: a_value,
        attrs: a_attrs,
    } = a;
    let Constant {
        name: b_name,
        value: b_value,
        attrs: b_attrs,
    } = b;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_option(
        a_value.as_ref().into(),
        b_value.as_ref().into(),
        cmp_typed_value,
    )
    .qualified("value")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    Ok(())
}

fn cmp_instr((a_instr, a_func): (&Instr, &Func), (b_instr, b_func): (&Instr, &Func)) -> Result {
    use ir::instr::HasLoc;
    use ir::instr::HasLocals;
    use ir::instr::HasOperands;

    // eprintln!("A: {a_instr:?}");
    // eprintln!("B: {b_instr:?}");

    // Do a little extra work to make the errors nicer.
    cmp_eq(
        &std::mem::discriminant(a_instr),
        &std::mem::discriminant(b_instr),
    )
    .qualified("discriminant")?;

    fn cmp_instr_(
        (a_instr, a_func): (&Instr, &Func),
        (b_instr, b_func): (&Instr, &Func),
    ) -> Result {
        cmp_eq(a_instr.operands().len(), b_instr.operands().len()).qualified("operands.len")?;
        cmp_slice(
            a_instr.operands().iter().map(|i| (*i, a_func)),
            b_instr.operands().iter().map(|i| (*i, b_func)),
            cmp_operand,
        )
        .qualified("operands")?;

        cmp_eq(a_instr.locals().len(), b_instr.locals().len()).qualified("locals.len")?;
        cmp_slice(
            a_instr.locals().iter().copied(),
            b_instr.locals().iter().copied(),
            cmp_local,
        )
        .qualified("locals")?;
        cmp_slice(a_instr.edges(), b_instr.edges(), cmp_eq).qualified("edges")?;

        cmp_loc_id((a_instr.loc_id(), a_func), (b_instr.loc_id(), b_func)).qualified("loc_id")?;

        match (a_instr, b_instr) {
            (Instr::Call(a), Instr::Call(b)) => cmp_instr_call(a, b).qualified("call"),
            (Instr::Hhbc(a), Instr::Hhbc(b)) => {
                cmp_instr_hhbc((a, a_func), (b, b_func)).qualified("hhbc")
            }
            (Instr::MemberOp(a), Instr::MemberOp(b)) => {
                cmp_instr_member_op((a, a_func), (b, b_func)).qualified("member_op")
            }
            (Instr::Special(a), Instr::Special(b)) => cmp_instr_special(a, b).qualified("special"),
            (Instr::Terminator(a), Instr::Terminator(b)) => {
                cmp_instr_terminator(a, b).qualified("terminator")
            }

            // these should never happen
            (
                Instr::Call(_)
                | Instr::Hhbc(_)
                | Instr::MemberOp(_)
                | Instr::Special(_)
                | Instr::Terminator(_),
                _,
            ) => unreachable!(),
        }
    }

    cmp_instr_((a_instr, a_func), (b_instr, b_func))
        .with_raw(|| format!("::<{:?}>", std::mem::discriminant(a_instr)))
}

fn cmp_local(a: LocalId, b: LocalId) -> Result {
    match (a, b) {
        (LocalId::Named(a_id), LocalId::Named(b_id)) => cmp_eq(a_id, b_id).qualified("named"),
        (LocalId::Unnamed(a_id), LocalId::Unnamed(b_id)) => cmp_eq(a_id, b_id).qualified("unnamed"),
        (LocalId::Named(a_id), LocalId::Unnamed(b_id)) => {
            bail!("LocalId mismatch Named({a_id}) vs Unnamed({b_id})")
        }
        (LocalId::Unnamed(a_id), LocalId::Named(b_id)) => {
            bail!("LocalId mismatch Unnamed({a_id}) vs Named({b_id})",)
        }
    }
}

fn cmp_operand((a, a_func): (ValueId, &Func), (b, b_func): (ValueId, &Func)) -> Result {
    use FullInstrId as I;
    match (a.full(), b.full()) {
        (I::None, I::None) => {}
        (I::None, _) => bail!("Mismatch in ValueId (none)"),

        (I::Instr(a), I::Instr(b)) => cmp_eq(a, b).qualified("instr")?,
        (I::Instr(_), _) => bail!("Mismatch in ValueId (instr)"),

        (I::Imm(a), I::Imm(b)) => {
            cmp_imm(a_func.repr.imm(a), b_func.repr.imm(b)).qualified("constant")?
        }
        (I::Imm(_), _) => bail!("Mismatch in ValueId (immediate)"),
    }

    Ok(())
}

fn cmp_instr_call(a: &Call, b: &Call) -> Result {
    // Ignore LocId, ValueIds and LocalIds - those are checked elsewhere.
    let Call {
        operands: _,
        context: a_context,
        detail: a_detail,
        flags: a_flags,
        num_rets: a_num_rets,
        inouts: a_inouts,
        readonly: a_readonly,
        loc: _,
    } = a;
    let Call {
        operands: _,
        context: b_context,
        detail: b_detail,
        flags: b_flags,
        num_rets: b_num_rets,
        inouts: b_inouts,
        readonly: b_readonly,
        loc: _,
    } = b;
    cmp_eq(a_context, b_context).qualified("context")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_eq(a_num_rets, b_num_rets).qualified("num_rets")?;
    cmp_slice(a_inouts, b_inouts, cmp_eq).qualified("inouts")?;
    cmp_slice(a_readonly, b_readonly, cmp_eq).qualified("readonly")?;

    cmp_eq(
        std::mem::discriminant(a_detail),
        std::mem::discriminant(b_detail),
    )?;
    use instr::CallDetail;
    #[rustfmt::skip]
    match (a_detail, b_detail) {
        (CallDetail::FCallCtor, CallDetail::FCallCtor)
        | (CallDetail::FCallFunc, CallDetail::FCallFunc) => {}

        (CallDetail::FCallClsMethod { log: a_log },
         CallDetail::FCallClsMethod { log: b_log }) => {
            cmp_eq(a_log, b_log).qualified("log")?;
        }
        (CallDetail::FCallClsMethodD { clsid: a_clsid, method: a_method },
         CallDetail::FCallClsMethodD { clsid: b_clsid, method: b_method }) => {
            cmp_eq(a_clsid, b_clsid).qualified("clsid")?;
            cmp_eq(a_method, b_method).qualified("method")?;
        }
        (CallDetail::FCallClsMethodM { method: a_method, log: a_log },
         CallDetail::FCallClsMethodM { method: b_method, log: b_log }) => {
            cmp_eq(a_method, b_method).qualified("method")?;
            cmp_eq(a_log, b_log).qualified("log")?;
        }
        (CallDetail::FCallClsMethodS { clsref: a_clsref },
         CallDetail::FCallClsMethodS { clsref: b_clsref }) => {
            cmp_eq(a_clsref, b_clsref).qualified("clsref")?;
        }
        (CallDetail::FCallClsMethodSD { clsref: a_clsref, method: a_method },
         CallDetail::FCallClsMethodSD { clsref: b_clsref, method: b_method }) => {
            cmp_eq(a_clsref, b_clsref).qualified("clsref")?;
            cmp_eq(a_method, b_method).qualified("method")?;
        }
        (CallDetail::FCallFuncD { func: a_func },
         CallDetail::FCallFuncD { func: b_func }) => {
            cmp_eq(a_func, b_func).qualified("func")?;
        }
        (CallDetail::FCallObjMethod { flavor: a_flavor },
         CallDetail::FCallObjMethod { flavor: b_flavor }) => {
            cmp_eq(a_flavor, b_flavor).qualified("flavor")?;
        }
        (CallDetail::FCallObjMethodD { flavor: a_flavor, method: a_method },
         CallDetail::FCallObjMethodD { flavor: b_flavor, method: b_method }) => {
            cmp_eq(a_flavor, b_flavor).qualified("flavor")?;
            cmp_eq(a_method, b_method).qualified("method")?;
        }

        // these should never happen
        (
            CallDetail::FCallCtor
            | CallDetail::FCallFunc
            | CallDetail::FCallClsMethod {..}
            | CallDetail::FCallClsMethodD {..}
            | CallDetail::FCallClsMethodM {..}
            | CallDetail::FCallClsMethodS {..}
            | CallDetail::FCallClsMethodSD {..}
            | CallDetail::FCallFuncD {..}
            | CallDetail::FCallObjMethod {..}
            | CallDetail::FCallObjMethodD {..},
            _,
        ) => unreachable!(),
    };

    Ok(())
}

fn cmp_instr_hhbc((a, a_func): (&Hhbc, &Func), (b, b_func): (&Hhbc, &Func)) -> Result {
    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    // Ignore LocId, ValueIds and LocalIds - those are checked elsewhere.
    match (a, b) {
        (Hhbc::BareThis(x0, _), Hhbc::BareThis(x1, _)) => {
            cmp_eq(x0, x1).qualified("BareThis param x")?;
        }
        (Hhbc::CGetS(_, x0, _), Hhbc::CGetS(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("CGetS param x")?;
        }
        (Hhbc::CheckProp(x0, _), Hhbc::CheckProp(x1, _)) => {
            cmp_eq(x0, x1).qualified("CheckProp param x")?;
        }
        (Hhbc::ClsCns(_, x0, _), Hhbc::ClsCns(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("ClsCns param x")?;
        }
        (Hhbc::ClsCnsD(x0, y0, _), Hhbc::ClsCnsD(x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("ClsCnsD param x")?;
            cmp_eq(y0, y1).qualified("ClsCnsD param y")?;
        }
        (Hhbc::CmpOp(_, x0, _), Hhbc::CmpOp(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("CmpOp param x")?;
        }
        (Hhbc::ColFromArray(_, x0, _), Hhbc::ColFromArray(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("ColFromArray param x")?;
        }
        (Hhbc::ContCheck(x0, _), Hhbc::ContCheck(x1, _)) => {
            cmp_eq(x0, x1).qualified("ContCheck param x")?;
        }
        (
            Hhbc::CreateCl {
                operands: _,
                clsid: x0,
                loc: _,
            },
            Hhbc::CreateCl {
                operands: _,
                clsid: x1,
                loc: _,
            },
        ) => {
            cmp_eq(x0, x1).qualified("CreateCl param x")?;
        }
        (Hhbc::IncDecL(_, x0, _), Hhbc::IncDecL(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("IncDecL param x")?;
        }
        (Hhbc::IncDecS(_, x0, _), Hhbc::IncDecS(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("IncDecS param x")?;
        }
        (Hhbc::IncludeEval(x0), Hhbc::IncludeEval(x1)) => {
            let instr::IncludeEval {
                kind: a_kind,
                vid: _,
                loc: a_loc,
            } = x0;
            let instr::IncludeEval {
                kind: b_kind,
                vid: _,
                loc: b_loc,
            } = x1;
            cmp_eq(a_kind, b_kind).qualified("kind")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }
        (Hhbc::InitProp(_, x0, y0, _), Hhbc::InitProp(_, x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("InitProp param x")?;
            cmp_eq(y0, y1).qualified("InitProp param y")?;
        }
        (Hhbc::InstanceOfD(_, x0, _, _), Hhbc::InstanceOfD(_, x1, _, _)) => {
            cmp_eq(x0, x1).qualified("InstanceOfD param x")?;
        }
        (Hhbc::IsTypeC(_, x0, _), Hhbc::IsTypeC(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("IsTypeC param x")?;
        }
        (Hhbc::IsTypeL(_, x0, _), Hhbc::IsTypeL(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("IsTypeL param x")?;
        }
        (Hhbc::IsTypeStructC(_, x0, y0, _), Hhbc::IsTypeStructC(_, x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("IsTypeStructC param x")?;
            cmp_eq(y0, y1).qualified("IsTypeStructC param y")?;
        }
        (Hhbc::IterFree(x0, _), Hhbc::IterFree(x1, _)) => {
            cmp_eq(x0, x1).qualified("IterFree param x")?;
        }
        (Hhbc::IterGetKey(x0, y0, _), Hhbc::IterGetKey(x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("IterGetKey param x")?;
            cmp_eq(y0, y1).qualified("IterGetKey param y")?;
        }
        (Hhbc::IterGetValue(x0, y0, _), Hhbc::IterGetValue(x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("IterGetValue param x")?;
            cmp_eq(y0, y1).qualified("IterGetValue param y")?;
        }
        (Hhbc::IterSetValue(_, x0, y0, _), Hhbc::IterSetValue(_, x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("IterSetValue param x")?;
            cmp_eq(y0, y1).qualified("IterSetValue param y")?;
        }
        (Hhbc::NewDictArray(x0, _), Hhbc::NewDictArray(x1, _)) => {
            cmp_eq(x0, x1).qualified("NewDictArray param x")?;
        }
        (Hhbc::NewObjD(x0, _), Hhbc::NewObjD(x1, _)) => {
            cmp_eq(x0, x1).qualified("NewObjD param x")?;
        }
        (Hhbc::NewObjS(x0, _), Hhbc::NewObjS(x1, _)) => {
            cmp_eq(x0, x1).qualified("NewObjS param x")?;
        }
        (Hhbc::NewStructDict(x0, _, _), Hhbc::NewStructDict(x1, _, _)) => {
            cmp_slice(x0.iter().copied(), x1.iter().copied(), cmp_eq)
                .qualified("NewStructDict param x")?;
        }
        (Hhbc::OODeclExists(_, x0, _), Hhbc::OODeclExists(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("OODeclExists param x")?;
        }
        (Hhbc::ResolveClass(x0, _), Hhbc::ResolveClass(x1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveClass param x")?;
        }
        (Hhbc::ResolveClsMethod(_, x0, _), Hhbc::ResolveClsMethod(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveClsMethod param x")?;
        }
        (Hhbc::ResolveClsMethodD(x0, y0, _), Hhbc::ResolveClsMethodD(x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveClsMethodD param x")?;
            cmp_eq(y0, y1).qualified("ResolveClsMethodD param y")?;
        }
        (Hhbc::ResolveClsMethodS(x0, y0, _), Hhbc::ResolveClsMethodS(x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveClsMethodS param x")?;
            cmp_eq(y0, y1).qualified("ResolveClsMethodS param y")?;
        }
        (Hhbc::ResolveRClsMethod(_, x0, _), Hhbc::ResolveRClsMethod(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveRClsMethod param x")?;
        }
        (Hhbc::ResolveRClsMethodS(_, x0, y0, _), Hhbc::ResolveRClsMethodS(_, x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveRClsMethodS param x")?;
            cmp_eq(y0, y1).qualified("ResolveRClsMethodS param y")?;
        }
        (Hhbc::ResolveFunc(x0, _), Hhbc::ResolveFunc(x1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveFunc param x")?;
        }
        (Hhbc::ResolveRClsMethodD(_, x0, y0, _), Hhbc::ResolveRClsMethodD(_, x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveRClsMethodD param x")?;
            cmp_eq(y0, y1).qualified("ResolveRClsMethodD param y")?;
        }
        (Hhbc::ResolveRFunc(_, x0, _), Hhbc::ResolveRFunc(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveRFunc param x")?;
        }
        (Hhbc::ResolveMethCaller(x0, _), Hhbc::ResolveMethCaller(x1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveMethCaller param x")?;
        }
        (Hhbc::SetOpL(_, _, x0, _), Hhbc::SetOpL(_, _, x1, _)) => {
            cmp_eq(x0, x1).qualified("SetOpL param x")?;
        }
        (Hhbc::SetOpS(_, x0, _), Hhbc::SetOpS(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("SetOpS param x")?;
        }
        (Hhbc::SetS(_, x0, _), Hhbc::SetS(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("SetS param x")?;
        }
        (Hhbc::ClassGetC(x0, _, _), Hhbc::ClassGetC(x1, _, _)) => {
            cmp_eq(x0, x1).qualified("ClassGetC param x")?;
        }

        // These have no params that weren't already checked.
        (Hhbc::AKExists(_, _), _)
        | (Hhbc::Add(_, _), _)
        | (Hhbc::AddElemC(_, _), _)
        | (Hhbc::AddNewElemC(_, _), _)
        | (Hhbc::ArrayIdx(_, _), _)
        | (Hhbc::ArrayMarkLegacy(_, _), _)
        | (Hhbc::ArrayUnmarkLegacy(_, _), _)
        | (Hhbc::Await(_, _), _)
        | (Hhbc::AwaitAll(_, _), _)
        | (Hhbc::AwaitLowPri(_), _)
        | (Hhbc::BitAnd(_, _), _)
        | (Hhbc::BitNot(_, _), _)
        | (Hhbc::BitOr(_, _), _)
        | (Hhbc::BitXor(_, _), _)
        | (Hhbc::CGetG(_, _), _)
        | (Hhbc::CGetL(_, _), _)
        | (Hhbc::CGetQuietL(_, _), _)
        | (Hhbc::CUGetL(_, _), _)
        | (Hhbc::CastBool(_, _), _)
        | (Hhbc::CastDict(_, _), _)
        | (Hhbc::CastDouble(_, _), _)
        | (Hhbc::CastInt(_, _), _)
        | (Hhbc::CastKeyset(_, _), _)
        | (Hhbc::CastString(_, _), _)
        | (Hhbc::CastVec(_, _), _)
        | (Hhbc::ChainFaults(_, _), _)
        | (Hhbc::CheckClsReifiedGenericMismatch(_, _), _)
        | (Hhbc::CheckClsRGSoft(_, _), _)
        | (Hhbc::CheckThis(_), _)
        | (Hhbc::ClassGetC(_, _, _), _)
        | (Hhbc::ClassGetTS(_, _), _)
        | (Hhbc::ClassGetTSWithGenerics(_, _), _)
        | (Hhbc::ClassHasReifiedGenerics(_, _), _)
        | (Hhbc::ClassName(_, _), _)
        | (Hhbc::Clone(_, _), _)
        | (Hhbc::ClsCnsL(_, _, _), _)
        | (Hhbc::Cmp(_, _), _)
        | (Hhbc::CombineAndResolveTypeStruct(_, _), _)
        | (Hhbc::Concat(_, _), _)
        | (Hhbc::ConcatN(_, _), _)
        | (Hhbc::ConsumeL(_, _), _)
        | (Hhbc::ContCurrent(_), _)
        | (Hhbc::ContEnter(_, _), _)
        | (Hhbc::ContGetReturn(_), _)
        | (Hhbc::ContKey(_), _)
        | (Hhbc::ContRaise(_, _), _)
        | (Hhbc::ContValid(_), _)
        | (Hhbc::CreateCl { .. }, _)
        | (Hhbc::CreateCont(_), _)
        | (Hhbc::GetMemoAgnosticImplicitContext(_), _)
        | (Hhbc::Div(_, _), _)
        | (Hhbc::EnumClassLabelName(_, _), _)
        | (Hhbc::GetClsRGProp(_, _), _)
        | (Hhbc::GetMemoKeyL(_, _), _)
        | (Hhbc::HasReifiedParent(_, _), _)
        | (Hhbc::Idx(_, _), _)
        | (Hhbc::IsLateBoundCls(_, _), _)
        | (Hhbc::IssetG(_, _), _)
        | (Hhbc::IssetL(_, _), _)
        | (Hhbc::IssetS(_, _), _)
        | (Hhbc::IterBase(_, _), _)
        | (Hhbc::LateBoundCls(_), _)
        | (Hhbc::LockObj(_, _), _)
        | (Hhbc::MemoSet(_, _, _), _)
        | (Hhbc::MemoSetEager(_, _, _), _)
        | (Hhbc::Modulo(_, _), _)
        | (Hhbc::Mul(_, _), _)
        | (Hhbc::NewKeysetArray(_, _), _)
        | (Hhbc::NewObj(_, _), _)
        | (Hhbc::NewPair(_, _), _)
        | (Hhbc::NewVec(_, _), _)
        | (Hhbc::Not(_, _), _)
        | (Hhbc::ParentCls(_), _)
        | (Hhbc::Pow(_, _), _)
        | (Hhbc::Print(_, _), _)
        | (Hhbc::RaiseClassStringConversionNotice(_), _)
        | (Hhbc::RecordReifiedGeneric(_, _), _)
        | (Hhbc::ReifiedInit(_, _, _), _)
        | (Hhbc::SelfCls(_), _)
        | (Hhbc::SetG(_, _), _)
        | (Hhbc::SetImplicitContextByValue(_, _), _)
        | (Hhbc::SetL(_, _, _), _)
        | (Hhbc::Shl(_, _), _)
        | (Hhbc::Shr(_, _), _)
        | (Hhbc::Sub(_, _), _)
        | (Hhbc::This(_), _)
        | (Hhbc::ThrowNonExhaustiveSwitch(_), _)
        | (Hhbc::UnsetG(_, _), _)
        | (Hhbc::UnsetL(_, _), _)
        | (Hhbc::VerifyOutType(_, _, _), _)
        | (Hhbc::VerifyParamType(_, _, _), _)
        | (Hhbc::VerifyParamTypeTS(_, _, _), _)
        | (Hhbc::VerifyRetTypeC(_, _), _)
        | (Hhbc::VerifyRetTypeTS(_, _), _)
        | (Hhbc::VerifyTypeTS(_, _), _)
        | (Hhbc::WHResult(_, _), _)
        | (Hhbc::Yield(_, _), _)
        | (Hhbc::YieldK(_, _), _) => {}

        // these should never happen
        (
            Hhbc::BareThis(..)
            | Hhbc::CGetS(..)
            | Hhbc::CheckProp(..)
            | Hhbc::ClsCns(..)
            | Hhbc::ClsCnsD(..)
            | Hhbc::CmpOp(..)
            | Hhbc::ColFromArray(..)
            | Hhbc::ContCheck(..)
            | Hhbc::IncDecL(..)
            | Hhbc::IncDecS(..)
            | Hhbc::IncludeEval(..)
            | Hhbc::InitProp(..)
            | Hhbc::InstanceOfD(..)
            | Hhbc::IsTypeC(..)
            | Hhbc::IsTypeL(..)
            | Hhbc::IsTypeStructC(..)
            | Hhbc::IterFree(..)
            | Hhbc::IterGetKey(..)
            | Hhbc::IterGetValue(..)
            | Hhbc::IterSetValue(..)
            | Hhbc::NewDictArray(..)
            | Hhbc::NewObjD(..)
            | Hhbc::NewObjS(..)
            | Hhbc::NewStructDict(..)
            | Hhbc::OODeclExists(..)
            | Hhbc::ResolveClass(..)
            | Hhbc::ResolveClsMethod(..)
            | Hhbc::ResolveClsMethodD(..)
            | Hhbc::ResolveClsMethodS(..)
            | Hhbc::ResolveRClsMethod(..)
            | Hhbc::ResolveRClsMethodS(..)
            | Hhbc::ResolveFunc(..)
            | Hhbc::ResolveRClsMethodD(..)
            | Hhbc::ResolveRFunc(..)
            | Hhbc::ResolveMethCaller(..)
            | Hhbc::SetOpL(..)
            | Hhbc::SetOpS(..)
            | Hhbc::SetS(..),
            _,
        ) => unreachable!(),
    }
    Ok(())
}

fn cmp_instr_member_op((a, a_func): (&MemberOp, &Func), (b, b_func): (&MemberOp, &Func)) -> Result {
    let MemberOp {
        base_op: a_base_op,
        intermediate_ops: a_intermediate_ops,
        final_op: a_final_op,
        operands: _,
        locals: _,
    } = a;
    let MemberOp {
        base_op: b_base_op,
        intermediate_ops: b_intermediate_ops,
        final_op: b_final_op,
        operands: _,
        locals: _,
    } = b;

    cmp_instr_member_op_base((a_base_op, a_func), (b_base_op, b_func)).qualified("base")?;
    cmp_slice(
        a_intermediate_ops.iter().map(|i| (i, a_func)),
        b_intermediate_ops.iter().map(|i| (i, b_func)),
        cmp_instr_member_op_intermediate,
    )
    .qualified("intermediate")?;
    cmp_instr_member_op_final((a_final_op, a_func), (b_final_op, b_func)).qualified("final")?;
    Ok(())
}

fn cmp_instr_member_op_base(
    (a, a_func): (&BaseOp, &Func),
    (b, b_func): (&BaseOp, &Func),
) -> Result {
    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    #[rustfmt::skip]
    match (a, b) {
        (BaseOp::BaseC { mode: a_mode, loc: a_loc },
         BaseOp::BaseC { mode: b_mode, loc: b_loc }) 
         => {
            cmp_eq(a_mode, b_mode).qualified("mode")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }
        (BaseOp::BaseH { loc: a_loc },
         BaseOp::BaseH { loc: b_loc }) => {
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }
        (BaseOp::BaseL { mode: a_mode, readonly: a_readonly, loc: a_loc },
         BaseOp::BaseL { mode: b_mode, readonly: b_readonly, loc: b_loc }) |
        (BaseOp::BaseSC { mode: a_mode, readonly: a_readonly, loc: a_loc },
         BaseOp::BaseSC { mode: b_mode, readonly: b_readonly, loc: b_loc }) => {
            cmp_eq(a_mode, b_mode).qualified("mode")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }
        (BaseOp::BaseST { mode: a_mode, readonly: a_readonly, loc: a_loc, prop: a_prop },
         BaseOp::BaseST { mode: b_mode, readonly: b_readonly, loc: b_loc, prop: b_prop }) => {
            cmp_eq(a_mode, b_mode).qualified("mode")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
            cmp_eq(a_prop, b_prop).qualified("prop")?;
        }

        // these should never happen
        (
            BaseOp::BaseC { .. }
            | BaseOp::BaseH { .. }
            | BaseOp::BaseL { .. }
            | BaseOp::BaseSC { .. }
            | BaseOp::BaseST { .. },
            _,
        ) => unreachable!(),
    };

    Ok(())
}

fn cmp_instr_member_op_intermediate(
    (a, a_func): (&IntermediateOp, &Func),
    (b, b_func): (&IntermediateOp, &Func),
) -> Result {
    let IntermediateOp {
        key: a_key,
        mode: a_mode,
        readonly: a_readonly,
        loc: a_loc,
    } = a;
    let IntermediateOp {
        key: b_key,
        mode: b_mode,
        readonly: b_readonly,
        loc: b_loc,
    } = b;

    cmp_member_key(a_key, b_key).qualified("key")?;
    cmp_eq(a_mode, b_mode).qualified("mode")?;
    cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
    cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;

    Ok(())
}

fn cmp_instr_member_op_final(
    (a, a_func): (&FinalOp, &Func),
    (b, b_func): (&FinalOp, &Func),
) -> Result {
    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    #[rustfmt::skip]
    match (a, b) {
        (FinalOp::IncDecM { key: a_key, readonly: a_readonly, inc_dec_op: a_inc_dec_op, loc: a_loc },
         FinalOp::IncDecM { key: b_key, readonly: b_readonly, inc_dec_op: b_inc_dec_op, loc: b_loc }) => {
            cmp_member_key(a_key, b_key).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_eq(a_inc_dec_op, b_inc_dec_op).qualified("inc_dec_op")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }
        (FinalOp::QueryM { key: a_key, readonly: a_readonly, query_m_op: a_query_m_op, loc: a_loc },
         FinalOp::QueryM { key: b_key, readonly: b_readonly, query_m_op: b_query_m_op, loc: b_loc }) => {
            cmp_member_key(a_key, b_key).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_eq(a_query_m_op, b_query_m_op).qualified("query_m_op")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }
        (FinalOp::SetM { key: a_key, readonly: a_readonly, loc: a_loc },
         FinalOp::SetM { key: b_key, readonly: b_readonly, loc: b_loc }) => {
            cmp_member_key(a_key, b_key).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }
        (FinalOp::SetRangeM { sz: a_sz, set_range_op: a_set_range_op, loc: a_loc },
         FinalOp::SetRangeM { sz: b_sz, set_range_op: b_set_range_op, loc: b_loc }) => {
            cmp_eq(a_sz, b_sz).qualified("sz")?;
            cmp_eq(a_set_range_op, b_set_range_op).qualified("set_range_op")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }
        (FinalOp::SetOpM { key: a_key, readonly: a_readonly, set_op_op: a_set_op_op, loc: a_loc },
         FinalOp::SetOpM { key: b_key, readonly: b_readonly, set_op_op: b_set_op_op, loc: b_loc }) => {
            cmp_member_key(a_key, b_key).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_eq(a_set_op_op, b_set_op_op).qualified("set_op_op")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }
        (FinalOp::UnsetM { key: a_key, readonly: a_readonly, loc: a_loc },
         FinalOp::UnsetM { key: b_key, readonly: b_readonly, loc: b_loc }) => {
            cmp_member_key(a_key, b_key).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_loc_id((*a_loc, a_func), (*b_loc, b_func)).qualified("loc")?;
        }

        // these should never happen
        (
            FinalOp::IncDecM { .. }
            | FinalOp::QueryM { .. }
            | FinalOp::SetM { .. }
            | FinalOp::SetRangeM { .. }
            | FinalOp::SetOpM { .. }
            | FinalOp::UnsetM { .. },
            _,
        ) => unreachable!(),
    };

    Ok(())
}

fn cmp_member_key(a: &MemberKey, b: &MemberKey) -> Result {
    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    match (a, b) {
        (MemberKey::EI(a), MemberKey::EI(b)) => cmp_eq(a, b)?,
        (MemberKey::ET(a), MemberKey::ET(b)) => {
            cmp_eq(*a, *b)?;
        }
        (MemberKey::PT(a), MemberKey::PT(b)) | (MemberKey::QT(a), MemberKey::QT(b)) => {
            cmp_eq(a, b)?;
        }

        (MemberKey::EC, MemberKey::EC)
        | (MemberKey::EL, MemberKey::EL)
        | (MemberKey::PC, MemberKey::PC)
        | (MemberKey::PL, MemberKey::PL)
        | (MemberKey::W, MemberKey::W) => {}

        // these should never happen
        (
            MemberKey::EC
            | MemberKey::EI(..)
            | MemberKey::EL
            | MemberKey::ET(..)
            | MemberKey::PC
            | MemberKey::PL
            | MemberKey::PT(..)
            | MemberKey::QT(..)
            | MemberKey::W,
            _,
        ) => unreachable!(),
    }

    Ok(())
}

#[allow(clippy::todo)]
fn cmp_instr_special(a: &Special, b: &Special) -> Result {
    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    // Ignore LocId, ValueIds, LocalIds and BlockIds - those are checked elsewhere.
    match (a, b) {
        (Special::Copy(_), Special::Copy(_))
        | (Special::Param, Special::Param)
        | (Special::Tombstone, Special::Tombstone) => {}
        (Special::Select(_, a), Special::Select(_, b)) => cmp_eq(a, b)?,

        (Special::IrToBc(_), _) | (Special::Tmp(_), _) | (Special::Textual(_), _) => todo!(),

        // these should never happen
        (Special::Copy(..) | Special::Param | Special::Select(..) | Special::Tombstone, _) => {
            unreachable!()
        }
    }

    Ok(())
}

fn cmp_instr_terminator(a: &Terminator, b: &Terminator) -> Result {
    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    fn cmp_instr_terminator_(a: &Terminator, b: &Terminator) -> Result {
        // Ignore LocId, ValueIds, LocalIds and BlockIds - those are checked elsewhere.
        #[rustfmt::skip]
        match (a, b) {
            (Terminator::CallAsync(a_call, _),
             Terminator::CallAsync(b_call, _)) => {
                cmp_instr_call(a_call, b_call)?;
            }
            (Terminator::Fatal(_, a_op, _),
             Terminator::Fatal(_, b_op, _)) => {
                cmp_eq(a_op, b_op)?;
            }
            (Terminator::IterInit(a_iterator),
             Terminator::IterInit(b_iterator))
                | (Terminator::IterNext(a_iterator),
                   Terminator::IterNext(b_iterator)) => {
                    cmp_instr_iterator(a_iterator, b_iterator)?;
                }
            (Terminator::JmpOp { cond: _, pred: a_pred, targets: _, loc: _, },
             Terminator::JmpOp { cond: _, pred: b_pred, targets: _, loc: _, }, ) => {
                cmp_eq(a_pred, b_pred)?;
            }
            (Terminator::Switch { cond: _, bounded: a_kind, base: a_base, targets: _, loc: _, },
             Terminator::Switch { cond: _, bounded: b_kind, base: b_base, targets: _, loc: _, }, ) => {
                cmp_eq(a_kind, b_kind)?;
                cmp_eq(a_base, b_base)?;
            }
            (Terminator::SSwitch { cond: _, cases: a_cases, targets: _, loc: _, },
             Terminator::SSwitch { cond: _, cases: b_cases, targets: _, loc: _, }, ) => {
                cmp_slice(a_cases.iter().copied(), b_cases.iter().copied(), cmp_eq)?;
            }

            // These are ONLY made of LocId, ValueIds, LocalIds and BlockIds.
            (Terminator::Enter(_, _), _)
                | (Terminator::Exit(_, _), _)
                | (Terminator::Jmp(_, _), _)
                | (Terminator::JmpArgs(_, _, _), _)
                | (Terminator::MemoGet(_), _)
                | (Terminator::MemoGetEager(_), _)
                | (Terminator::NativeImpl(_), _)
                | (Terminator::Ret(_, _), _)
                | (Terminator::RetCSuspended(_, _), _)
                | (Terminator::RetM(_, _), _)
                | (Terminator::Throw(_, _), _)
                | (Terminator::ThrowAsTypeStructException(_, _, _), _)
                | (Terminator::Unreachable, _) => {}

            // these should never happen
            (
                Terminator::CallAsync(..)
                | Terminator::Fatal(..)
                | Terminator::IterInit(..)
                | Terminator::IterNext(..)
                | Terminator::JmpOp { .. }
                | Terminator::Switch { .. }
                | Terminator::SSwitch { .. },
                _,
            ) => unreachable!(),
        };

        Ok(())
    }

    cmp_instr_terminator_(a, b).with_raw(|| format!("::<{:?}>", std::mem::discriminant(a)))
}

fn cmp_instr_iterator(a: &IteratorArgs, b: &IteratorArgs) -> Result {
    // Ignore LocId, ValueIds and LocalIds - those are checked elsewhere.
    let IteratorArgs {
        iter_id: a_iter_id,
        flags: a_flags,
        base_lid: _,
        targets: _,
        loc: _,
    } = a;
    let IteratorArgs {
        iter_id: b_iter_id,
        flags: b_flags,
        base_lid: _,
        targets: _,
        loc: _,
    } = b;
    cmp_eq(a_iter_id, b_iter_id).qualified("iter_id")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    Ok(())
}

fn cmp_loc_id((a, a_func): (LocId, &Func), (b, b_func): (LocId, &Func)) -> Result {
    let a_src_loc = a_func.repr.get_loc(a);
    let b_src_loc = b_func.repr.get_loc(b);
    cmp_option(a_src_loc, b_src_loc, cmp_src_loc)
}

fn cmp_method(a: &Method, b: &Method) -> Result {
    let Method {
        flags: a_flags,
        body: a_body,
        name: a_name,
        visibility: a_visibility,
    } = a;
    let Method {
        flags: b_flags,
        body: b_body,
        name: b_name,
        visibility: b_visibility,
    } = b;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_body(a_body, b_body).qualified("body")?;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_eq(a_visibility, b_visibility).qualified("visibility")?;
    Ok(())
}

fn cmp_module(a: &Module, b: &Module) -> Result {
    let Module {
        attributes: a_attributes,
        name: a_name,
        span: a_span,
        doc_comment: a_doc_comment,
    } = a;
    let Module {
        attributes: b_attributes,
        name: b_name,
        span: b_span,
        doc_comment: b_doc_comment,
    } = b;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_span(a_span, b_span).qualified("span")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    Ok(())
}

fn cmp_param(
    (a, a_dv): &(Param, Option<DefaultValue>),
    (b, b_dv): &(Param, Option<DefaultValue>),
) -> Result {
    let Param {
        name: a_name,
        is_variadic: a_is_variadic,
        is_splat: a_is_splat,
        is_inout: a_is_inout,
        is_readonly: a_is_readonly,
        is_optional: a_is_optional,
        user_attributes: a_user_attributes,
        type_info: a_type_info,
    } = a;
    let Param {
        name: b_name,
        is_variadic: b_is_variadic,
        is_splat: b_is_splat,
        is_inout: b_is_inout,
        is_readonly: b_is_readonly,
        is_optional: b_is_optional,
        user_attributes: b_user_attributes,
        type_info: b_type_info,
    } = b;

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_eq(a_is_variadic, b_is_variadic).qualified("is_variadic")?;
    cmp_eq(a_is_splat, b_is_splat).qualified("is_splat")?;
    cmp_eq(a_is_inout, b_is_inout).qualified("is_inout")?;
    cmp_eq(a_is_readonly, b_is_readonly).qualified("is_readonly")?;
    cmp_eq(a_is_optional, b_is_optional).qualified("is_optional")?;
    cmp_attributes(a_user_attributes, b_user_attributes).qualified("user_attributes")?;
    cmp_option(
        a_type_info.as_ref().into(),
        b_type_info.as_ref().into(),
        cmp_type_info,
    )
    .qualified("type_info")?;
    cmp_option(a_dv.as_ref(), b_dv.as_ref(), cmp_default_value).qualified("default_value")?;
    Ok(())
}

fn cmp_default_value(a: &DefaultValue, b: &DefaultValue) -> Result {
    let DefaultValue {
        init: a_init,
        expr: a_expr,
    } = a;
    let DefaultValue {
        init: b_init,
        expr: b_expr,
    } = b;
    cmp_eq(a_init, b_init).qualified("init")?;
    cmp_eq(a_expr, b_expr).qualified("expr")?;
    Ok(())
}

fn cmp_property(a: &Property, b: &Property) -> Result {
    let Property {
        name: a_name,
        flags: a_flags,
        attributes: a_attributes,
        visibility: a_visibility,
        initial_value: a_initial_value,
        type_info: a_type_info,
        doc_comment: a_doc_comment,
    } = a;
    let Property {
        name: b_name,
        flags: b_flags,
        attributes: b_attributes,
        visibility: b_visibility,
        initial_value: b_initial_value,
        type_info: b_type_info,
        doc_comment: b_doc_comment,
    } = b;

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_eq(a_flags, b_flags).qualified("flagsr")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_eq(a_visibility, b_visibility).qualified("visibility")?;
    cmp_option(
        a_initial_value.as_ref().into(),
        b_initial_value.as_ref().into(),
        cmp_typed_value,
    )
    .qualified("initial_value")?;
    cmp_type_info(a_type_info, b_type_info).qualified("type_info")?;
    cmp_option(
        a_doc_comment.as_ref().into_option(),
        b_doc_comment.as_ref().into_option(),
        cmp_eq,
    )
    .qualified("doc_comment")?;
    Ok(())
}

fn cmp_requirement(a: &Requirement, b: &Requirement) -> Result {
    let Requirement {
        name: a_name,
        kind: a_kind,
    } = a;
    let Requirement {
        name: b_name,
        kind: b_kind,
    } = b;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_eq(a_kind, b_kind).qualified("kind")?;
    Ok(())
}

fn cmp_span(a: &Span, b: &Span) -> Result {
    cmp_eq(a.line_begin, b.line_begin).qualified("line_begin")?;
    cmp_eq(a.line_end, b.line_end).qualified("line_end")?;
    Ok(())
}

fn cmp_src_loc(a: &SrcLoc, b: &SrcLoc) -> Result {
    cmp_eq(a.line_begin, b.line_begin).qualified("line_begin")?;
    cmp_eq(a.line_end, b.line_end).qualified("line_end")?;
    cmp_eq(a.col_begin, b.col_begin).qualified("col_begin")?;
    cmp_eq(a.col_end, b.col_end).qualified("col_end")?;
    Ok(())
}

fn cmp_symbol_refs(a: &SymbolRefs, b: &SymbolRefs) -> Result {
    let SymbolRefs {
        classes: a_classes,
        constants: a_constants,
        functions: a_functions,
        includes: a_includes,
    } = a;
    let SymbolRefs {
        classes: b_classes,
        constants: b_constants,
        functions: b_functions,
        includes: b_includes,
    } = b;

    cmp_slice(a_classes, b_classes, cmp_eq).qualified("classes")?;
    cmp_slice(a_constants, b_constants, cmp_eq).qualified("constants")?;
    cmp_slice(a_functions, b_functions, cmp_eq).qualified("functions")?;
    cmp_slice(a_includes, b_includes, cmp_eq).qualified("includes")?;
    Ok(())
}

fn cmp_tparam_bounds(a: &UpperBound, b: &UpperBound) -> Result {
    cmp_eq(a.name, b.name).qualified("name")?;
    cmp_slice(a.bounds.iter(), b.bounds.iter(), cmp_type_info).qualified("bounds")?;
    Ok(())
}

fn cmp_type_constant(a: &TypeConstant, b: &TypeConstant) -> Result {
    let TypeConstant {
        name: a_name,
        initializer: a_initializer,
        is_abstract: a_is_abstract,
    } = a;
    let TypeConstant {
        name: b_name,
        initializer: b_initializer,
        is_abstract: b_is_abstract,
    } = b;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_option(
        a_initializer.as_ref().into(),
        b_initializer.as_ref().into(),
        cmp_typed_value,
    )
    .qualified("initializer")?;
    cmp_eq(a_is_abstract, b_is_abstract).qualified("is_abstract")?;
    Ok(())
}

fn cmp_type_info(a: &TypeInfo, b: &TypeInfo) -> Result {
    let TypeInfo {
        user_type: a_user_type,
        type_constraint: a_type_constraint,
    } = a;
    let TypeInfo {
        user_type: b_user_type,
        type_constraint: b_type_constraint,
    } = b;

    cmp_option(a_user_type.into(), b_user_type.into(), cmp_eq).qualified("user_type")?;
    cmp_eq(a_type_constraint, b_type_constraint).qualified("type_constraint")?;
    Ok(())
}

fn cmp_typed_value(a: &TypedValue, b: &TypedValue) -> Result {
    cmp_eq(std::mem::discriminant(a), std::mem::discriminant(b))?;

    match (a, b) {
        (TypedValue::Uninit, TypedValue::Uninit) | (TypedValue::Null, TypedValue::Null) => {}
        (TypedValue::Int(a), TypedValue::Int(b)) => cmp_eq(a, b).qualified("int")?,
        (TypedValue::Bool(a), TypedValue::Bool(b)) => cmp_eq(a, b).qualified("bool")?,
        (TypedValue::Float(a), TypedValue::Float(b)) => cmp_eq(a, b).qualified("float")?,
        (TypedValue::String(a), TypedValue::String(b)) => cmp_eq(*a, *b).qualified("string")?,
        (TypedValue::LazyClass(a), TypedValue::LazyClass(b)) => {
            cmp_eq(*a, *b).qualified("lazy_class")?
        }
        (TypedValue::Vec(a), TypedValue::Vec(b)) => {
            cmp_slice(a.iter(), b.iter(), cmp_typed_value).qualified("vec")?;
        }
        (TypedValue::Keyset(a), TypedValue::Keyset(b)) => {
            cmp_slice(a.iter(), b.iter(), cmp_typed_value).qualified("keyset")?;
        }
        (TypedValue::Dict(a), TypedValue::Dict(b)) => {
            cmp_slice(
                a.iter().map(|e| &e.key),
                b.iter().map(|e| &e.key),
                cmp_typed_value,
            )
            .qualified("dict keys")?;
            cmp_slice(
                a.iter().map(|e| &e.value),
                b.iter().map(|e| &e.value),
                cmp_typed_value,
            )
            .qualified("dict values")?;
        }

        // these should never happen
        (
            TypedValue::Uninit
            | TypedValue::Int(_)
            | TypedValue::Bool(_)
            | TypedValue::Float(_)
            | TypedValue::String(_)
            | TypedValue::LazyClass(_)
            | TypedValue::Null
            | TypedValue::Vec(_)
            | TypedValue::Keyset(_)
            | TypedValue::Dict(_),
            _,
        ) => unreachable!(),
    }

    Ok(())
}

fn cmp_typedef(a: &Typedef, b: &Typedef) -> Result {
    let Typedef {
        name: a_name,
        attributes: a_attributes,
        type_info_union: a_type_info_union,
        type_structure: a_type_structure,
        span: a_span,
        attrs: a_attrs,
        case_type: a_case_type,
    } = a;
    let Typedef {
        name: b_name,
        attributes: b_attributes,
        type_info_union: b_type_info_union,
        type_structure: b_type_structure,
        span: b_span,
        attrs: b_attrs,
        case_type: b_case_type,
    } = b;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_slice(
        a_type_info_union.iter(),
        b_type_info_union.iter(),
        cmp_type_info,
    )?;
    cmp_typed_value(a_type_structure, b_type_structure).qualified("type_structure")?;
    cmp_span(a_span, b_span).qualified("span")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    cmp_eq(a_case_type, b_case_type).qualified("case_type")?;
    Ok(())
}

fn cmp_unit(a_unit: &Unit, b_unit: &Unit) -> Result {
    let Unit {
        classes: a_classes,
        constants: a_constants,
        file_attributes: a_file_attributes,
        functions: a_functions,
        fatal: a_fatal,
        modules: a_modules,
        module_use: a_module_use,
        symbol_refs: a_symbol_refs,
        typedefs: a_typedefs,
        error_symbols: _,
        missing_symbols: _,
    } = a_unit;
    let Unit {
        classes: b_classes,
        constants: b_constants,
        file_attributes: b_file_attributes,
        functions: b_functions,
        fatal: b_fatal,
        modules: b_modules,
        module_use: b_module_use,
        symbol_refs: b_symbol_refs,
        typedefs: b_typedefs,
        error_symbols: _,
        missing_symbols: _,
    } = b_unit;

    cmp_map_t(a_classes, b_classes, cmp_class).qualified("classes")?;
    cmp_map_t(a_constants, b_constants, cmp_constant).qualified("constants")?;
    cmp_attributes(a_file_attributes, b_file_attributes).qualified("file_attributes")?;
    cmp_map_t(a_functions, b_functions, cmp_function).qualified("functions")?;
    cmp_option(a_fatal.as_ref().into(), b_fatal.as_ref().into(), cmp_fatal).qualified("fatal")?;
    cmp_map_t(a_modules, b_modules, cmp_module).qualified("modules")?;
    cmp_option(
        a_module_use.as_ref().into_option(),
        b_module_use.as_ref().into_option(),
        cmp_eq,
    )
    .qualified("module_use")?;
    cmp_symbol_refs(a_symbol_refs, b_symbol_refs).qualified("symbol_refs")?;
    cmp_map_t(a_typedefs, b_typedefs, cmp_typedef).qualified("typedefs")?;

    Ok(())
}

fn cmp_upper_bounds(a: &UpperBound, b: &UpperBound) -> Result {
    cmp_eq(a.name, b.name).qualified("name")?;
    cmp_slice(a.bounds.iter(), b.bounds.iter(), cmp_type_info).qualified("bounds")?;
    Ok(())
}

mod mapping {
    use super::*;

    impl MapName for &Attribute {
        fn get_name(&self) -> String {
            self.name.as_str().to_owned()
        }
    }

    impl MapName for &Class {
        fn get_name(&self) -> String {
            self.name.as_str().to_owned()
        }
    }

    impl MapName for (&ExFrameId, &ExFrame) {
        fn get_name(&self) -> String {
            self.0.to_string()
        }
    }

    impl MapName for &Function {
        fn get_name(&self) -> String {
            self.name.as_str().to_owned()
        }
    }

    impl MapName for &Method {
        fn get_name(&self) -> String {
            self.name.into_string()
        }
    }
}
