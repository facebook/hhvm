// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::class::Requirement;
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
use ir::SymbolRefs;
// This is reasonable because if we compare everything then we'll end up pulling
// in everything...
use ir::*;

use crate::cmp_eq;
use crate::cmp_map_t;
use crate::cmp_option;
use crate::cmp_slice;
use crate::CmpContext;
use crate::CmpError;
use crate::MapName;
use crate::Result;

pub fn cmp_ir(a: &Unit<'_>, b: &Unit<'_>) -> Result {
    cmp_unit(a, b).with_raw(|| "unit".to_string())
}

fn cmp_attribute(
    (a, a_strings): (&Attribute, &StringInterner),
    (b, b_strings): (&Attribute, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    cmp_id(a.name.id, b.name.id).qualified("name")?;
    cmp_slice(
        a.arguments.iter().map(|a| (a, a_strings)),
        b.arguments.iter().map(|b| (b, b_strings)),
        cmp_typed_value,
    )
    .qualified("arguments")?;
    Ok(())
}

fn cmp_attributes(
    (a, a_strings): (&[Attribute], &StringInterner),
    (b, b_strings): (&[Attribute], &StringInterner),
) -> Result {
    cmp_map_t(
        a.iter().map(|a| (a, a_strings)),
        b.iter().map(|b| (b, b_strings)),
        cmp_attribute,
    )
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

fn cmp_cc_reified(a: &CcReified<'_>, b: &CcReified<'_>) -> Result {
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

fn cmp_cc_this(a: &CcThis<'_>, b: &CcThis<'_>) -> Result {
    let CcThis { types: a_types } = a;
    let CcThis { types: b_types } = b;
    cmp_slice(a_types, b_types, cmp_eq).qualified("types")?;
    Ok(())
}

fn cmp_class(
    (a, a_strings): (&Class<'_>, &StringInterner),
    (b, b_strings): (&Class<'_>, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

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
        src_loc: a_src_loc,
        type_constants: a_type_constants,
        upper_bounds: a_upper_bounds,
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
        src_loc: b_src_loc,
        type_constants: b_type_constants,
        upper_bounds: b_upper_bounds,
        uses: b_uses,
    } = b;

    cmp_attributes((a_attributes, a_strings), (b_attributes, b_strings)).qualified("attributes")?;
    cmp_option(a_base.map(|i| i.id), b_base.map(|i| i.id), cmp_id).qualified("base")?;
    cmp_map_t(
        a_constants.iter().map(|a| (a, a_strings)),
        b_constants.iter().map(|b| (b, b_strings)),
        cmp_hack_constant,
    )
    .qualified("constants")?;
    cmp_map_t(
        a_ctx_constants.iter(),
        b_ctx_constants.iter(),
        cmp_ctx_constant,
    )
    .qualified("ctx_constants")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    cmp_option(
        a_enum_type.as_ref().map(|i| (i, a_strings)),
        b_enum_type.as_ref().map(|i| (i, b_strings)),
        cmp_type_info,
    )
    .qualified("enum_type")?;
    cmp_slice(
        a_enum_includes.iter().map(|i| i.id),
        b_enum_includes.iter().map(|i| i.id),
        cmp_id,
    )
    .qualified("enum_includes")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_slice(
        a_implements.iter().map(|i| i.id),
        b_implements.iter().map(|i| i.id),
        cmp_id,
    )
    .qualified("implements")?;
    cmp_map_t(
        a_methods.iter().map(|i| (i, a_strings)),
        b_methods.iter().map(|i| (i, b_strings)),
        cmp_method,
    )
    .qualified("methods")?;
    cmp_id(a_name.id, b_name.id).qualified("name")?;
    cmp_map_t(
        a_properties.iter().map(|i| (i, a_strings)),
        b_properties.iter().map(|i| (i, b_strings)),
        cmp_property,
    )
    .qualified("properties")?;
    cmp_slice(
        a_requirements.iter().map(|i| (i, a_strings)),
        b_requirements.iter().map(|i| (i, b_strings)),
        cmp_requirement,
    )
    .qualified("requirements")?;
    cmp_src_loc((a_src_loc, a_strings), (b_src_loc, b_strings)).qualified("src_loc")?;
    cmp_slice(
        a_type_constants.iter().map(|i| (i, a_strings)),
        b_type_constants.iter().map(|i| (i, b_strings)),
        cmp_type_constant,
    )
    .qualified("type_constants")?;
    cmp_slice(
        a_upper_bounds.iter().map(|i| (i, a_strings)),
        b_upper_bounds.iter().map(|i| (i, b_strings)),
        cmp_upper_bounds,
    )
    .qualified("upper_bounds")?;
    cmp_slice(
        a_uses.iter().map(|i| i.id),
        b_uses.iter().map(|i| i.id),
        cmp_id,
    )
    .qualified("uses")?;
    Ok(())
}

fn cmp_coeffects(a: &Coeffects<'_>, b: &Coeffects<'_>) -> Result {
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

fn cmp_constant(
    (a_const, a_strings): (&Constant<'_>, &StringInterner),
    (b_const, b_strings): (&Constant<'_>, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    cmp_eq(
        std::mem::discriminant(a_const),
        std::mem::discriminant(b_const),
    )?;

    match (a_const, b_const) {
        (Constant::Array(a), Constant::Array(b)) => {
            cmp_typed_value((a, a_strings), (b, b_strings)).qualified("array")?
        }
        (Constant::Bool(a), Constant::Bool(b)) => cmp_eq(a, b).qualified("bool")?,
        (Constant::EnumClassLabel(a), Constant::EnumClassLabel(b)) => {
            cmp_id(*a, *b).qualified("enum_class_label")?
        }
        (Constant::Float(a), Constant::Float(b)) => cmp_eq(a, b).qualified("float")?,
        (Constant::Int(a), Constant::Int(b)) => cmp_eq(a, b).qualified("int")?,
        (Constant::LazyClass(a), Constant::LazyClass(b)) => {
            cmp_id(a.id, b.id).qualified("lazy_class")?
        }
        (Constant::Named(a), Constant::Named(b)) => cmp_eq(a, b).qualified("named")?,
        (Constant::NewCol(a), Constant::NewCol(b)) => cmp_eq(a, b).qualified("new_col")?,
        (Constant::String(a), Constant::String(b)) => cmp_id(*a, *b).qualified("string")?,
        (Constant::Dir, Constant::Dir)
        | (Constant::File, Constant::File)
        | (Constant::FuncCred, Constant::FuncCred)
        | (Constant::Method, Constant::Method)
        | (Constant::Null, Constant::Null)
        | (Constant::Uninit, Constant::Uninit) => {}

        // these should never happen
        (
            Constant::Array(_)
            | Constant::Bool(_)
            | Constant::EnumClassLabel(_)
            | Constant::Float(_)
            | Constant::Int(_)
            | Constant::LazyClass(_)
            | Constant::Named(_)
            | Constant::NewCol(_)
            | Constant::String(_)
            | Constant::Dir
            | Constant::File
            | Constant::FuncCred
            | Constant::Method
            | Constant::Null
            | Constant::Uninit,
            _,
        ) => unreachable!(),
    }

    Ok(())
}

fn cmp_ctx_constant(a: &CtxConstant<'_>, b: &CtxConstant<'_>) -> Result {
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

fn cmp_fatal(
    (a, a_strings): (&Fatal, &StringInterner),
    (b, b_strings): (&Fatal, &StringInterner),
) -> Result {
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
    cmp_src_loc((a_loc, a_strings), (b_loc, b_strings)).qualified("loc")?;
    cmp_eq(a_message, b_message).qualified("message")?;
    Ok(())
}

fn cmp_func(
    (a, a_strings): (&Func<'_>, &StringInterner),
    (b, b_strings): (&Func<'_>, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    let Func {
        attributes: a_attributes,
        attrs: a_attrs,
        blocks: a_blocks,
        coeffects: a_coeffects,
        constants: _,
        doc_comment: a_doc_comment,
        ex_frames: a_ex_frames,
        instrs: a_instrs,
        is_memoize_wrapper: a_is_memoize_wrapper,
        is_memoize_wrapper_lsb: a_is_memoize_wrapper_lsb,
        loc_id: a_loc_id,
        locs: _,
        num_iters: a_num_iters,
        params: a_params,
        return_type: a_return_type,
        shadowed_tparams: a_shadowed_tparams,
        tparams: a_tparams,
    } = a;
    let Func {
        attributes: b_attributes,
        attrs: b_attrs,
        blocks: b_blocks,
        coeffects: b_coeffects,
        constants: _,
        doc_comment: b_doc_comment,
        ex_frames: b_ex_frames,
        instrs: b_instrs,
        is_memoize_wrapper: b_is_memoize_wrapper,
        is_memoize_wrapper_lsb: b_is_memoize_wrapper_lsb,
        loc_id: b_loc_id,
        locs: _,
        num_iters: b_num_iters,
        params: b_params,
        return_type: b_return_type,
        shadowed_tparams: b_shadowed_tparams,
        tparams: b_tparams,
    } = b;

    cmp_attributes((a_attributes, a_strings), (b_attributes, b_strings)).qualified("attributes")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    cmp_coeffects(a_coeffects, b_coeffects).qualified("coeffects")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    cmp_eq(a_is_memoize_wrapper, b_is_memoize_wrapper).qualified("is_memoize_wrapper")?;
    cmp_eq(a_is_memoize_wrapper_lsb, b_is_memoize_wrapper_lsb)
        .qualified("is_memoize_wrapper_lsb")?;
    cmp_eq(a_num_iters, b_num_iters).qualified("num_iters")?;
    cmp_slice(
        a_params.iter().map(|a| (a, a_strings)),
        b_params.iter().map(|b| (b, b_strings)),
        cmp_param,
    )
    .qualified("params")?;
    cmp_type_info((a_return_type, a_strings), (b_return_type, b_strings))
        .qualified("return_type")?;
    cmp_slice(
        a_shadowed_tparams.iter().map(|i| i.id),
        b_shadowed_tparams.iter().map(|i| i.id),
        cmp_id,
    )
    .qualified("shadowed_tparams")?;

    cmp_map_t(
        a_tparams.iter().map(|(i, j)| (i, j, a_strings)),
        b_tparams.iter().map(|(i, j)| (i, j, b_strings)),
        cmp_tparam_bounds,
    )
    .qualified("tparams")?;

    cmp_slice(a_blocks.iter(), b_blocks.iter(), cmp_block).qualified("blocks")?;
    cmp_map_t(a_ex_frames.iter(), b_ex_frames.iter(), cmp_ex_frame).qualified("ex_frames")?;
    cmp_slice(
        a_instrs.iter().map(|i| (i, a, a_strings)),
        b_instrs.iter().map(|i| (i, b, b_strings)),
        cmp_instr,
    )
    .qualified("instrs")?;

    cmp_loc_id((*a_loc_id, a, a_strings), (*b_loc_id, b, b_strings)).qualified("loc_id")?;

    Ok(())
}

fn cmp_function(
    (a, a_strings): (&Function<'_>, &StringInterner),
    (b, b_strings): (&Function<'_>, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    let Function {
        flags: a_flags,
        name: a_name,
        func: a_func,
    } = a;
    let Function {
        flags: b_flags,
        name: b_name,
        func: b_func,
    } = b;

    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_id(a_name.id, b_name.id).qualified("name")?;
    cmp_func((a_func, a_strings), (b_func, b_strings)).qualified("func")?;

    Ok(())
}

fn cmp_hack_constant(
    (a, a_strings): (&HackConstant, &StringInterner),
    (b, b_strings): (&HackConstant, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    let HackConstant {
        name: a_name,
        value: a_value,
        attrs: a_attrs,
    } = a;
    let HackConstant {
        name: b_name,
        value: b_value,
        attrs: b_attrs,
    } = b;
    cmp_id(a_name.id, b_name.id).qualified("name")?;
    cmp_option(
        a_value.as_ref().map(|i| (i, a_strings)),
        b_value.as_ref().map(|i| (i, b_strings)),
        cmp_typed_value,
    )
    .qualified("value")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    Ok(())
}

fn cmp_id(
    (a, a_strings): (UnitBytesId, &StringInterner),
    (b, b_strings): (UnitBytesId, &StringInterner),
) -> Result {
    match (a, b) {
        (UnitBytesId::NONE, UnitBytesId::NONE) => {}
        (UnitBytesId::NONE, b) => {
            let b = b_strings.lookup_bytes(b);
            bail!("UnitBytesId NONE vs \"{}\"", String::from_utf8_lossy(&b));
        }
        (a, UnitBytesId::NONE) => {
            let a = a_strings.lookup_bytes(a);
            bail!("UnitBytesId \"{}\" vs NONE", String::from_utf8_lossy(&a));
        }
        (a, b) => {
            let a = a_strings.lookup_bytes(a);
            let b = b_strings.lookup_bytes(b);
            cmp_eq(a.as_ref() as &[u8], b.as_ref() as &[u8])?;
        }
    }
    Ok(())
}

fn cmp_instr(
    (a_instr, a_func, a_strings): (&Instr, &Func<'_>, &StringInterner),
    (b_instr, b_func, b_strings): (&Instr, &Func<'_>, &StringInterner),
) -> Result {
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
        (a_instr, a_func, a_strings): (&Instr, &Func<'_>, &StringInterner),
        (b_instr, b_func, b_strings): (&Instr, &Func<'_>, &StringInterner),
    ) -> Result {
        cmp_eq(a_instr.operands().len(), b_instr.operands().len()).qualified("operands.len")?;
        cmp_slice(
            a_instr.operands().iter().map(|i| (*i, a_func, a_strings)),
            b_instr.operands().iter().map(|i| (*i, b_func, b_strings)),
            cmp_operand,
        )
        .qualified("operands")?;

        cmp_eq(a_instr.locals().len(), b_instr.locals().len()).qualified("locals.len")?;
        cmp_slice(
            a_instr.locals().iter().map(|i| (*i, a_strings)),
            b_instr.locals().iter().map(|i| (*i, b_strings)),
            cmp_local,
        )
        .qualified("locals")?;
        cmp_slice(a_instr.edges(), b_instr.edges(), cmp_eq).qualified("edges")?;

        cmp_loc_id(
            (a_instr.loc_id(), a_func, a_strings),
            (b_instr.loc_id(), b_func, b_strings),
        )
        .qualified("loc_id")?;

        match (a_instr, b_instr) {
            (Instr::Call(a), Instr::Call(b)) => {
                cmp_instr_call((a, a_strings), (b, b_strings)).qualified("call")
            }
            (Instr::Hhbc(a), Instr::Hhbc(b)) => {
                cmp_instr_hhbc((a, a_func, a_strings), (b, b_func, b_strings)).qualified("hhbc")
            }
            (Instr::MemberOp(a), Instr::MemberOp(b)) => {
                cmp_instr_member_op((a, a_func, a_strings), (b, b_func, b_strings))
                    .qualified("member_op")
            }
            (Instr::Special(a), Instr::Special(b)) => cmp_instr_special(a, b).qualified("special"),
            (Instr::Terminator(a), Instr::Terminator(b)) => {
                cmp_instr_terminator((a, a_strings), (b, b_strings)).qualified("terminator")
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

    cmp_instr_((a_instr, a_func, a_strings), (b_instr, b_func, b_strings))
        .with_raw(|| format!("::<{:?}>", std::mem::discriminant(a_instr)))
}

fn cmp_local(
    (a, a_strings): (LocalId, &StringInterner),
    (b, b_strings): (LocalId, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    match (a, b) {
        (LocalId::Named(a_id), LocalId::Named(b_id)) => cmp_id(a_id, b_id).qualified("named"),
        (LocalId::Unnamed(a_id), LocalId::Unnamed(b_id)) => cmp_eq(a_id, b_id).qualified("unnamed"),
        (LocalId::Named(a_id), LocalId::Unnamed(b_id)) => bail!(
            "LocalId mismatch {} vs {b_id}",
            String::from_utf8_lossy(&a_strings.lookup_bytes(a_id))
        ),
        (LocalId::Unnamed(a_id), LocalId::Named(b_id)) => bail!(
            "LocalId mismatch {a_id} vs {}",
            String::from_utf8_lossy(&b_strings.lookup_bytes(b_id))
        ),
    }
}

fn cmp_operand(
    (a, a_func, a_strings): (ValueId, &Func<'_>, &StringInterner),
    (b, b_func, b_strings): (ValueId, &Func<'_>, &StringInterner),
) -> Result {
    use FullInstrId as I;
    match (a.full(), b.full()) {
        (I::None, I::None) => {}
        (I::None, _) => bail!("Mismatch in ValueId (none)"),

        (I::Instr(a), I::Instr(b)) => cmp_eq(a, b).qualified("instr")?,
        (I::Instr(_), _) => bail!("Mismatch in ValueId (instr)"),

        (I::Constant(a), I::Constant(b)) => cmp_constant(
            (a_func.constant(a), a_strings),
            (b_func.constant(b), b_strings),
        )
        .qualified("constant")?,
        (I::Constant(_), _) => bail!("Mismatch in ValueId (const)"),
    }

    Ok(())
}

fn cmp_instr_call(
    (a, a_strings): (&Call, &StringInterner),
    (b, b_strings): (&Call, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

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
    cmp_id(*a_context, *b_context).qualified("context")?;
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
            cmp_id(a_clsid.id, b_clsid.id).qualified("clsid")?;
            cmp_id(a_method.id, b_method.id).qualified("method")?;
        }
        (CallDetail::FCallClsMethodM { method: a_method, log: a_log },
         CallDetail::FCallClsMethodM { method: b_method, log: b_log }) => {
            cmp_id(a_method.id, b_method.id).qualified("method")?;
            cmp_eq(a_log, b_log).qualified("log")?;
        }
        (CallDetail::FCallClsMethodS { clsref: a_clsref },
         CallDetail::FCallClsMethodS { clsref: b_clsref }) => {
            cmp_eq(a_clsref, b_clsref).qualified("clsref")?;
        }
        (CallDetail::FCallClsMethodSD { clsref: a_clsref, method: a_method },
         CallDetail::FCallClsMethodSD { clsref: b_clsref, method: b_method }) => {
            cmp_eq(a_clsref, b_clsref).qualified("clsref")?;
            cmp_id(a_method.id, b_method.id).qualified("method")?;
        }
        (CallDetail::FCallFuncD { func: a_func },
         CallDetail::FCallFuncD { func: b_func }) => {
            cmp_id(a_func.id, b_func.id).qualified("func")?;
        }
        (CallDetail::FCallObjMethod { flavor: a_flavor },
         CallDetail::FCallObjMethod { flavor: b_flavor }) => {
            cmp_eq(a_flavor, b_flavor).qualified("flavor")?;
        }
        (CallDetail::FCallObjMethodD { flavor: a_flavor, method: a_method },
         CallDetail::FCallObjMethodD { flavor: b_flavor, method: b_method }) => {
            cmp_eq(a_flavor, b_flavor).qualified("flavor")?;
            cmp_id(a_method.id, b_method.id).qualified("method")?;
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

fn cmp_instr_hhbc(
    (a, a_func, a_strings): (&Hhbc, &Func<'_>, &StringInterner),
    (b, b_func, b_strings): (&Hhbc, &Func<'_>, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

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
            cmp_id(x0.id, x1.id).qualified("CheckProp param x")?;
        }
        (Hhbc::ClsCns(_, x0, _), Hhbc::ClsCns(_, x1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ClsCns param x")?;
        }
        (Hhbc::ClsCnsD(x0, y0, _), Hhbc::ClsCnsD(x1, y1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ClsCnsD param x")?;
            cmp_id(y0.id, y1.id).qualified("ClsCnsD param y")?;
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
            cmp_id(x0.id, x1.id).qualified("CreateCl param x")?;
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
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings))
                .qualified("loc")?;
        }
        (Hhbc::InitProp(_, x0, y0, _), Hhbc::InitProp(_, x1, y1, _)) => {
            cmp_id(x0.id, x1.id).qualified("InitProp param x")?;
            cmp_eq(y0, y1).qualified("InitProp param y")?;
        }
        (Hhbc::InstanceOfD(_, x0, _), Hhbc::InstanceOfD(_, x1, _)) => {
            cmp_id(x0.id, x1.id).qualified("InstanceOfD param x")?;
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
        (Hhbc::NewDictArray(x0, _), Hhbc::NewDictArray(x1, _)) => {
            cmp_eq(x0, x1).qualified("NewDictArray param x")?;
        }
        (Hhbc::NewObjD(x0, _), Hhbc::NewObjD(x1, _)) => {
            cmp_id(x0.id, x1.id).qualified("NewObjD param x")?;
        }
        (Hhbc::NewObjS(x0, _), Hhbc::NewObjS(x1, _)) => {
            cmp_eq(x0, x1).qualified("NewObjS param x")?;
        }
        (Hhbc::NewStructDict(x0, _, _), Hhbc::NewStructDict(x1, _, _)) => {
            cmp_slice(x0.iter().copied(), x1.iter().copied(), cmp_id)
                .qualified("NewStructDict param x")?;
        }
        (Hhbc::OODeclExists(_, x0, _), Hhbc::OODeclExists(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("OODeclExists param x")?;
        }
        (Hhbc::ResolveClass(x0, _), Hhbc::ResolveClass(x1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ResolveClass param x")?;
        }
        (Hhbc::ResolveClsMethod(_, x0, _), Hhbc::ResolveClsMethod(_, x1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ResolveClsMethod param x")?;
        }
        (Hhbc::ResolveClsMethodD(x0, y0, _), Hhbc::ResolveClsMethodD(x1, y1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ResolveClsMethodD param x")?;
            cmp_id(y0.id, y1.id).qualified("ResolveClsMethodD param y")?;
        }
        (Hhbc::ResolveClsMethodS(x0, y0, _), Hhbc::ResolveClsMethodS(x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveClsMethodS param x")?;
            cmp_id(y0.id, y1.id).qualified("ResolveClsMethodS param y")?;
        }
        (Hhbc::ResolveRClsMethod(_, x0, _), Hhbc::ResolveRClsMethod(_, x1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ResolveRClsMethod param x")?;
        }
        (Hhbc::ResolveRClsMethodS(_, x0, y0, _), Hhbc::ResolveRClsMethodS(_, x1, y1, _)) => {
            cmp_eq(x0, x1).qualified("ResolveRClsMethodS param x")?;
            cmp_id(y0.id, y1.id).qualified("ResolveRClsMethodS param y")?;
        }
        (Hhbc::ResolveFunc(x0, _), Hhbc::ResolveFunc(x1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ResolveFunc param x")?;
        }
        (Hhbc::ResolveRClsMethodD(_, x0, y0, _), Hhbc::ResolveRClsMethodD(_, x1, y1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ResolveRClsMethodD param x")?;
            cmp_id(y0.id, y1.id).qualified("ResolveRClsMethodD param y")?;
        }
        (Hhbc::ResolveRFunc(_, x0, _), Hhbc::ResolveRFunc(_, x1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ResolveRFunc param x")?;
        }
        (Hhbc::ResolveMethCaller(x0, _), Hhbc::ResolveMethCaller(x1, _)) => {
            cmp_id(x0.id, x1.id).qualified("ResolveMethCaller param x")?;
        }
        (Hhbc::SetOpL(_, _, x0, _), Hhbc::SetOpL(_, _, x1, _)) => {
            cmp_eq(x0, x1).qualified("SetOpL param x")?;
        }
        (Hhbc::SetOpG(_, x0, _), Hhbc::SetOpG(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("SetOpG param x")?;
        }
        (Hhbc::SetOpS(_, x0, _), Hhbc::SetOpS(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("SetOpS param x")?;
        }
        (Hhbc::SetS(_, x0, _), Hhbc::SetS(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("SetS param x")?;
        }
        (Hhbc::Silence(_, x0, _), Hhbc::Silence(_, x1, _)) => {
            cmp_eq(x0, x1).qualified("Silence param x")?;
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
        | (Hhbc::CreateSpecialImplicitContext(_, _), _)
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
        | (Hhbc::LateBoundCls(_), _)
        | (Hhbc::LazyClassFromClass(_, _), _)
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
        | (Hhbc::VerifyImplicitContextState(_), _)
        | (Hhbc::VerifyOutType(_, _, _), _)
        | (Hhbc::VerifyParamType(_, _, _), _)
        | (Hhbc::VerifyParamTypeTS(_, _, _), _)
        | (Hhbc::VerifyRetTypeC(_, _), _)
        | (Hhbc::VerifyRetTypeTS(_, _), _)
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
            | Hhbc::SetOpG(..)
            | Hhbc::SetOpS(..)
            | Hhbc::SetS(..)
            | Hhbc::Silence(..),
            _,
        ) => unreachable!(),
    }
    Ok(())
}

fn cmp_instr_member_op(
    (a, a_func, a_strings): (&MemberOp, &Func<'_>, &StringInterner),
    (b, b_func, b_strings): (&MemberOp, &Func<'_>, &StringInterner),
) -> Result {
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

    cmp_instr_member_op_base(
        (a_base_op, a_func, a_strings),
        (b_base_op, b_func, b_strings),
    )
    .qualified("base")?;
    cmp_slice(
        a_intermediate_ops.iter().map(|i| (i, a_func, a_strings)),
        b_intermediate_ops.iter().map(|i| (i, b_func, b_strings)),
        cmp_instr_member_op_intermediate,
    )
    .qualified("intermediate")?;
    cmp_instr_member_op_final(
        (a_final_op, a_func, a_strings),
        (b_final_op, b_func, b_strings),
    )
    .qualified("final")?;
    Ok(())
}

fn cmp_instr_member_op_base(
    (a, a_func, a_strings): (&BaseOp, &Func<'_>, &StringInterner),
    (b, b_func, b_strings): (&BaseOp, &Func<'_>, &StringInterner),
) -> Result {
    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    #[rustfmt::skip]
    match (a, b) {
        (BaseOp::BaseC { mode: a_mode, loc: a_loc },
         BaseOp::BaseC { mode: b_mode, loc: b_loc }) |
        (BaseOp::BaseGC { mode: a_mode, loc: a_loc },
         BaseOp::BaseGC { mode: b_mode, loc: b_loc }) => {
            cmp_eq(a_mode, b_mode).qualified("mode")?;
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
        }
        (BaseOp::BaseH { loc: a_loc },
         BaseOp::BaseH { loc: b_loc }) => {
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
        }
        (BaseOp::BaseL { mode: a_mode, readonly: a_readonly, loc: a_loc },
         BaseOp::BaseL { mode: b_mode, readonly: b_readonly, loc: b_loc }) |
        (BaseOp::BaseSC { mode: a_mode, readonly: a_readonly, loc: a_loc },
         BaseOp::BaseSC { mode: b_mode, readonly: b_readonly, loc: b_loc }) => {
            cmp_eq(a_mode, b_mode).qualified("mode")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
        }
        (BaseOp::BaseST { mode: a_mode, readonly: a_readonly, loc: a_loc, prop: a_prop },
         BaseOp::BaseST { mode: b_mode, readonly: b_readonly, loc: b_loc, prop: b_prop }) => {
            cmp_eq(a_mode, b_mode).qualified("mode")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
            cmp_id((a_prop.id, a_strings), (b_prop.id, b_strings)).qualified("prop")?;
        }

        // these should never happen
        (
            BaseOp::BaseC { .. }
            | BaseOp::BaseGC { .. }
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
    (a, a_func, a_strings): (&IntermediateOp, &Func<'_>, &StringInterner),
    (b, b_func, b_strings): (&IntermediateOp, &Func<'_>, &StringInterner),
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

    cmp_member_key((a_key, a_strings), (b_key, b_strings)).qualified("key")?;
    cmp_eq(a_mode, b_mode).qualified("mode")?;
    cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
    cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;

    Ok(())
}

fn cmp_instr_member_op_final(
    (a, a_func, a_strings): (&FinalOp, &Func<'_>, &StringInterner),
    (b, b_func, b_strings): (&FinalOp, &Func<'_>, &StringInterner),
) -> Result {
    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    #[rustfmt::skip]
    match (a, b) {
        (FinalOp::IncDecM { key: a_key, readonly: a_readonly, inc_dec_op: a_inc_dec_op, loc: a_loc },
         FinalOp::IncDecM { key: b_key, readonly: b_readonly, inc_dec_op: b_inc_dec_op, loc: b_loc }) => {
            cmp_member_key((a_key, a_strings), (b_key, b_strings)).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_eq(a_inc_dec_op, b_inc_dec_op).qualified("inc_dec_op")?;
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
        }
        (FinalOp::QueryM { key: a_key, readonly: a_readonly, query_m_op: a_query_m_op, loc: a_loc },
         FinalOp::QueryM { key: b_key, readonly: b_readonly, query_m_op: b_query_m_op, loc: b_loc }) => {
            cmp_member_key((a_key, a_strings), (b_key, b_strings)).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_eq(a_query_m_op, b_query_m_op).qualified("query_m_op")?;
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
        }
        (FinalOp::SetM { key: a_key, readonly: a_readonly, loc: a_loc },
         FinalOp::SetM { key: b_key, readonly: b_readonly, loc: b_loc }) => {
            cmp_member_key((a_key, a_strings), (b_key, b_strings)).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
        }
        (FinalOp::SetRangeM { sz: a_sz, set_range_op: a_set_range_op, loc: a_loc },
         FinalOp::SetRangeM { sz: b_sz, set_range_op: b_set_range_op, loc: b_loc }) => {
            cmp_eq(a_sz, b_sz).qualified("sz")?;
            cmp_eq(a_set_range_op, b_set_range_op).qualified("set_range_op")?;
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
        }
        (FinalOp::SetOpM { key: a_key, readonly: a_readonly, set_op_op: a_set_op_op, loc: a_loc },
         FinalOp::SetOpM { key: b_key, readonly: b_readonly, set_op_op: b_set_op_op, loc: b_loc }) => {
            cmp_member_key((a_key, a_strings), (b_key, b_strings)).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_eq(a_set_op_op, b_set_op_op).qualified("set_op_op")?;
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
        }
        (FinalOp::UnsetM { key: a_key, readonly: a_readonly, loc: a_loc },
         FinalOp::UnsetM { key: b_key, readonly: b_readonly, loc: b_loc }) => {
            cmp_member_key((a_key, a_strings), (b_key, b_strings)).qualified("key")?;
            cmp_eq(a_readonly, b_readonly).qualified("readonly")?;
            cmp_loc_id((*a_loc, a_func, a_strings), (*b_loc, b_func, b_strings)).qualified("loc")?;
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

fn cmp_member_key(
    (a, a_strings): (&MemberKey, &StringInterner),
    (b, b_strings): (&MemberKey, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    match (a, b) {
        (MemberKey::EI(a), MemberKey::EI(b)) => cmp_eq(a, b)?,
        (MemberKey::ET(a), MemberKey::ET(b)) => {
            cmp_id(*a, *b)?;
        }
        (MemberKey::PT(a), MemberKey::PT(b)) | (MemberKey::QT(a), MemberKey::QT(b)) => {
            cmp_id(a.id, b.id)?;
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

fn cmp_instr_terminator(
    (a, a_strings): (&Terminator, &StringInterner),
    (b, b_strings): (&Terminator, &StringInterner),
) -> Result {
    cmp_eq(&std::mem::discriminant(a), &std::mem::discriminant(b))?;

    fn cmp_instr_terminator_(
        (a, a_strings): (&Terminator, &StringInterner),
        (b, b_strings): (&Terminator, &StringInterner),
    ) -> Result {
        // Ignore LocId, ValueIds, LocalIds and BlockIds - those are checked elsewhere.
        #[rustfmt::skip]
        match (a, b) {
            (Terminator::CallAsync(a_call, _),
             Terminator::CallAsync(b_call, _)) => {
                cmp_instr_call((a_call, a_strings), (b_call, b_strings))?;
            }
            (Terminator::Fatal(_, a_op, _),
             Terminator::Fatal(_, b_op, _)) => {
                cmp_eq(a_op, b_op)?;
            }
            (Terminator::IterInit(a_iterator, _),
             Terminator::IterInit(b_iterator, _))
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
                cmp_slice(a_cases.iter().map(|i| (*i, a_strings)), b_cases.iter().map(|i| (*i, b_strings)), cmp_id)?;
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

    cmp_instr_terminator_((a, a_strings), (b, b_strings))
        .with_raw(|| format!("::<{:?}>", std::mem::discriminant(a)))
}

fn cmp_instr_iterator(a: &IteratorArgs, b: &IteratorArgs) -> Result {
    // Ignore LocId, ValueIds and LocalIds - those are checked elsewhere.
    let IteratorArgs {
        iter_id: a_iter_id,
        locals: _,
        targets: _,
        loc: _,
    } = a;
    let IteratorArgs {
        iter_id: b_iter_id,
        locals: _,
        targets: _,
        loc: _,
    } = b;
    cmp_eq(a_iter_id, b_iter_id).qualified("iter_id")?;
    Ok(())
}

fn cmp_loc_id(
    (a, a_func, a_strings): (LocId, &Func<'_>, &StringInterner),
    (b, b_func, b_strings): (LocId, &Func<'_>, &StringInterner),
) -> Result {
    let a_src_loc = a_func.get_loc(a);
    let b_src_loc = b_func.get_loc(b);
    cmp_option(
        a_src_loc.map(|i| (i, a_strings)),
        b_src_loc.map(|i| (i, b_strings)),
        cmp_src_loc,
    )
}

fn cmp_method(
    (a, a_strings): (&Method<'_>, &StringInterner),
    (b, b_strings): (&Method<'_>, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    let Method {
        flags: a_flags,
        func: a_func,
        name: a_name,
        visibility: a_visibility,
    } = a;
    let Method {
        flags: b_flags,
        func: b_func,
        name: b_name,
        visibility: b_visibility,
    } = b;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_func((a_func, a_strings), (b_func, b_strings)).qualified("func")?;
    cmp_id(a_name.id, b_name.id).qualified("name")?;
    cmp_eq(a_visibility, b_visibility).qualified("visibility")?;
    Ok(())
}

fn cmp_module(
    (a, a_strings): (&Module, &StringInterner),
    (b, b_strings): (&Module, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    let Module {
        attributes: a_attributes,
        name: a_name,
        src_loc: a_src_loc,
        doc_comment: a_doc_comment,
    } = a;
    let Module {
        attributes: b_attributes,
        name: b_name,
        src_loc: b_src_loc,
        doc_comment: b_doc_comment,
    } = b;
    cmp_id(a_name.id, b_name.id).qualified("name")?;
    cmp_attributes((a_attributes, a_strings), (b_attributes, b_strings)).qualified("attributes")?;
    cmp_src_loc((a_src_loc, a_strings), (b_src_loc, b_strings)).qualified("src_loc")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    Ok(())
}

fn cmp_param(
    (a, a_strings): (&Param<'_>, &StringInterner),
    (b, b_strings): (&Param<'_>, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    let Param {
        name: a_name,
        is_variadic: a_is_variadic,
        is_inout: a_is_inout,
        is_readonly: a_is_readonly,
        user_attributes: a_user_attributes,
        ty: a_ty,
        default_value: a_default_value,
    } = a;
    let Param {
        name: b_name,
        is_variadic: b_is_variadic,
        is_inout: b_is_inout,
        is_readonly: b_is_readonly,
        user_attributes: b_user_attributes,
        ty: b_ty,
        default_value: b_default_value,
    } = b;

    cmp_id(*a_name, *b_name)
        .qualified("name")
        .qualified("name")?;
    cmp_eq(a_is_variadic, b_is_variadic).qualified("is_variadic")?;
    cmp_eq(a_is_inout, b_is_inout).qualified("is_inout")?;
    cmp_eq(a_is_readonly, b_is_readonly).qualified("is_readonly")?;
    cmp_attributes(
        (a_user_attributes, a_strings),
        (b_user_attributes, b_strings),
    )
    .qualified("user_attributes")?;
    cmp_type_info((a_ty, a_strings), (b_ty, b_strings)).qualified("ty")?;
    cmp_option(
        a_default_value.as_ref(),
        b_default_value.as_ref(),
        cmp_default_value,
    )
    .qualified("default_value")?;
    Ok(())
}

fn cmp_default_value(a: &DefaultValue<'_>, b: &DefaultValue<'_>) -> Result {
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

fn cmp_property(
    (a, a_strings): (&Property, &StringInterner),
    (b, b_strings): (&Property, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

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

    cmp_id(a_name.id, b_name.id).qualified("name")?;
    cmp_eq(a_flags, b_flags).qualified("flagsr")?;
    cmp_attributes((a_attributes, a_strings), (b_attributes, b_strings)).qualified("attributes")?;
    cmp_eq(a_visibility, b_visibility).qualified("visibility")?;
    cmp_option(
        a_initial_value.as_ref().map(|i| (i, a_strings)),
        b_initial_value.as_ref().map(|i| (i, b_strings)),
        cmp_typed_value,
    )
    .qualified("initial_value")?;
    cmp_type_info((a_type_info, a_strings), (b_type_info, b_strings)).qualified("type_info")?;
    cmp_option(
        a_doc_comment.as_ref().into_option(),
        b_doc_comment.as_ref().into_option(),
        cmp_eq,
    )
    .qualified("doc_comment")?;
    Ok(())
}

fn cmp_requirement(
    (a, a_strings): (&Requirement, &StringInterner),
    (b, b_strings): (&Requirement, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    let Requirement {
        name: a_name,
        kind: a_kind,
    } = a;
    let Requirement {
        name: b_name,
        kind: b_kind,
    } = b;
    cmp_id(a_name.id, b_name.id).qualified("name")?;
    cmp_eq(a_kind, b_kind).qualified("kind")?;
    Ok(())
}

fn cmp_src_loc(
    (a, a_strings): (&SrcLoc, &StringInterner),
    (b, b_strings): (&SrcLoc, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    cmp_id(a.filename.0, b.filename.0).qualified("filename")?;
    cmp_eq(a.line_begin, b.line_begin).qualified("line_begin")?;
    cmp_eq(a.line_end, b.line_end).qualified("line_end")?;
    cmp_eq(a.col_begin, b.col_begin).qualified("col_begin")?;
    cmp_eq(a.col_end, b.col_end).qualified("col_end")?;
    Ok(())
}

fn cmp_symbol_refs(a: &SymbolRefs<'_>, b: &SymbolRefs<'_>) -> Result {
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

fn cmp_tparam_bounds(
    (a_id, a, a_strings): (&ClassId, &TParamBounds, &StringInterner),
    (b_id, b, b_strings): (&ClassId, &TParamBounds, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    cmp_id(a_id.id, b_id.id).qualified("0")?;
    cmp_slice(
        a.bounds.iter().map(|i| (i, a_strings)),
        b.bounds.iter().map(|i| (i, b_strings)),
        cmp_type_info,
    )
    .qualified("bounds")
    .qualified("1")?;
    Ok(())
}

fn cmp_type_constant(
    (a, a_strings): (&TypeConstant, &StringInterner),
    (b, b_strings): (&TypeConstant, &StringInterner),
) -> Result {
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
        a_initializer.as_ref().map(|i| (i, a_strings)),
        b_initializer.as_ref().map(|i| (i, b_strings)),
        cmp_typed_value,
    )
    .qualified("initializer")?;
    cmp_eq(a_is_abstract, b_is_abstract).qualified("is_abstract")?;
    Ok(())
}

fn cmp_type_info(
    (a, a_strings): (&TypeInfo, &StringInterner),
    (b, b_strings): (&TypeInfo, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    let TypeInfo {
        user_type: a_user_type,
        enforced: a_enforced,
    } = a;
    let TypeInfo {
        user_type: b_user_type,
        enforced: b_enforced,
    } = b;

    cmp_option(*a_user_type, *b_user_type, cmp_id).qualified("user_type")?;

    let EnforceableType {
        ty: a_ty,
        modifiers: a_modifiers,
    } = a_enforced;
    let EnforceableType {
        ty: b_ty,
        modifiers: b_modifiers,
    } = b_enforced;

    cmp_base_ty((a_ty, a_strings), (b_ty, b_strings))
        .qualified("ty")
        .qualified("enforced")?;
    cmp_eq(a_modifiers, b_modifiers)
        .qualified("modifiers")
        .qualified("enforced")?;
    Ok(())
}

fn cmp_base_ty(
    (a, a_strings): (&BaseType, &StringInterner),
    (b, b_strings): (&BaseType, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    cmp_eq(std::mem::discriminant(a), std::mem::discriminant(b))?;
    match (a, b) {
        (BaseType::Class(a), BaseType::Class(b)) => cmp_id(a.id, b.id),

        (BaseType::AnyArray, _)
        | (BaseType::Arraykey, _)
        | (BaseType::Bool, _)
        | (BaseType::Classname, _)
        | (BaseType::Darray, _)
        | (BaseType::Dict, _)
        | (BaseType::Float, _)
        | (BaseType::Int, _)
        | (BaseType::Keyset, _)
        | (BaseType::Mixed, _)
        | (BaseType::None, _)
        | (BaseType::Nonnull, _)
        | (BaseType::Noreturn, _)
        | (BaseType::Nothing, _)
        | (BaseType::Null, _)
        | (BaseType::Num, _)
        | (BaseType::Resource, _)
        | (BaseType::String, _)
        | (BaseType::This, _)
        | (BaseType::Typename, _)
        | (BaseType::Varray, _)
        | (BaseType::VarrayOrDarray, _)
        | (BaseType::Vec, _)
        | (BaseType::VecOrDict, _)
        | (BaseType::Void, _) => Ok(()),

        // these should never happen
        (BaseType::Class(_), _) => unreachable!(),
    }
}

fn cmp_typed_value(
    (a, a_strings): (&TypedValue, &StringInterner),
    (b, b_strings): (&TypedValue, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    cmp_eq(std::mem::discriminant(a), std::mem::discriminant(b))?;

    match (a, b) {
        (TypedValue::Uninit, TypedValue::Uninit) | (TypedValue::Null, TypedValue::Null) => {}
        (TypedValue::Int(a), TypedValue::Int(b)) => cmp_eq(a, b).qualified("int")?,
        (TypedValue::Bool(a), TypedValue::Bool(b)) => cmp_eq(a, b).qualified("bool")?,
        (TypedValue::Float(a), TypedValue::Float(b)) => cmp_eq(a, b).qualified("float")?,
        (TypedValue::String(a), TypedValue::String(b)) => cmp_id(*a, *b).qualified("string")?,
        (TypedValue::LazyClass(ClassId { id: a }), TypedValue::LazyClass(ClassId { id: b })) => {
            cmp_id(*a, *b).qualified("lazy_class")?
        }
        (TypedValue::Vec(a), TypedValue::Vec(b)) => {
            cmp_slice(
                a.iter().map(|i| (i, a_strings)),
                b.iter().map(|i| (i, b_strings)),
                cmp_typed_value,
            )
            .qualified("vec")?;
        }
        (TypedValue::Keyset(a), TypedValue::Keyset(b)) => {
            cmp_slice(
                a.0.iter().map(|i| (i, a_strings)),
                b.0.iter().map(|i| (i, b_strings)),
                cmp_array_key,
            )
            .qualified("keyset")?;
        }
        (TypedValue::Dict(a), TypedValue::Dict(b)) => {
            cmp_slice(
                a.0.keys().map(|i| (i, a_strings)),
                b.0.keys().map(|i| (i, b_strings)),
                cmp_array_key,
            )
            .qualified("dict keys")?;
            cmp_slice(
                a.0.values().map(|i| (i, a_strings)),
                b.0.values().map(|i| (i, b_strings)),
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

fn cmp_array_key(
    (a, a_strings): (&ArrayKey, &StringInterner),
    (b, b_strings): (&ArrayKey, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    cmp_eq(std::mem::discriminant(a), std::mem::discriminant(b))?;

    match (a, b) {
        (ArrayKey::Int(a), ArrayKey::Int(b)) => cmp_eq(a, b).qualified("int")?,
        (ArrayKey::String(a), ArrayKey::String(b)) => cmp_id(*a, *b).qualified("string")?,

        (ArrayKey::LazyClass(ClassId { id: a }), ArrayKey::LazyClass(ClassId { id: b })) => {
            cmp_id(*a, *b).qualified("lazy_class")?
        }

        // these should never happen
        (ArrayKey::Int(_) | ArrayKey::String(_) | ArrayKey::LazyClass(_), _) => {
            unreachable!()
        }
    }

    Ok(())
}

fn cmp_typedef(
    (a, a_strings): (&Typedef, &StringInterner),
    (b, b_strings): (&Typedef, &StringInterner),
) -> Result {
    let cmp_id = |a: UnitBytesId, b: UnitBytesId| cmp_id((a, a_strings), (b, b_strings));

    let Typedef {
        name: a_name,
        attributes: a_attributes,
        type_info_union: a_type_info_union,
        type_structure: a_type_structure,
        loc: a_loc,
        attrs: a_attrs,
        case_type: a_case_type,
    } = a;
    let Typedef {
        name: b_name,
        attributes: b_attributes,
        type_info_union: b_type_info_union,
        type_structure: b_type_structure,
        loc: b_loc,
        attrs: b_attrs,
        case_type: b_case_type,
    } = b;
    cmp_id(a_name.id, b_name.id).qualified("name")?;
    cmp_attributes((a_attributes, a_strings), (b_attributes, b_strings)).qualified("attributes")?;
    cmp_slice(
        a_type_info_union.iter().map(|i| (i, a_strings)),
        b_type_info_union.iter().map(|i| (i, b_strings)),
        cmp_type_info,
    )?;
    cmp_typed_value((a_type_structure, a_strings), (b_type_structure, b_strings))
        .qualified("type_structure")?;
    cmp_src_loc((a_loc, a_strings), (b_loc, b_strings)).qualified("loc")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    cmp_eq(a_case_type, b_case_type).qualified("case_type")?;
    Ok(())
}

fn cmp_unit(a_unit: &Unit<'_>, b_unit: &Unit<'_>) -> Result {
    let Unit {
        classes: a_classes,
        constants: a_constants,
        file_attributes: a_file_attributes,
        functions: a_functions,
        fatal: a_fatal,
        modules: a_modules,
        module_use: a_module_use,
        strings: a_strings,
        symbol_refs: a_symbol_refs,
        typedefs: a_typedefs,
    } = a_unit;
    let Unit {
        classes: b_classes,
        constants: b_constants,
        file_attributes: b_file_attributes,
        functions: b_functions,
        fatal: b_fatal,
        modules: b_modules,
        module_use: b_module_use,
        strings: b_strings,
        symbol_refs: b_symbol_refs,
        typedefs: b_typedefs,
    } = b_unit;

    let a_strings = a_strings.as_ref();
    let b_strings = b_strings.as_ref();

    cmp_map_t(
        a_classes.iter().map(|a| (a, a_strings)),
        b_classes.iter().map(|b| (b, b_strings)),
        cmp_class,
    )
    .qualified("classes")?;

    cmp_map_t(
        a_constants.iter().map(|a| (a, a_strings)),
        b_constants.iter().map(|b| (b, b_strings)),
        cmp_hack_constant,
    )
    .qualified("constants")?;

    cmp_attributes(
        (a_file_attributes, a_strings),
        (b_file_attributes, b_strings),
    )
    .qualified("file_attributes")?;

    cmp_map_t(
        a_functions.iter().map(|a| (a, a_strings)),
        b_functions.iter().map(|b| (b, b_strings)),
        cmp_function,
    )
    .qualified("functions")?;

    cmp_option(
        a_fatal.as_ref().map(|a| (a, a_strings)),
        b_fatal.as_ref().map(|b| (b, b_strings)),
        cmp_fatal,
    )
    .qualified("fatal")?;

    cmp_map_t(
        a_modules.iter().map(|a| (a, a_strings)),
        b_modules.iter().map(|b| (b, b_strings)),
        cmp_module,
    )
    .qualified("modules")?;

    cmp_option(a_module_use.as_ref(), b_module_use.as_ref(), |a, b| {
        cmp_id((a.id, a_strings), (b.id, b_strings))
    })
    .qualified("module_use")?;

    cmp_symbol_refs(a_symbol_refs, b_symbol_refs).qualified("symbol_refs")?;

    cmp_map_t(
        a_typedefs.iter().map(|a| (a, a_strings)),
        b_typedefs.iter().map(|b| (b, b_strings)),
        cmp_typedef,
    )
    .qualified("typedefs")?;

    Ok(())
}

fn cmp_upper_bounds(
    (a, a_strings): (&(StringId, Vec<TypeInfo>), &StringInterner),
    (b, b_strings): (&(StringId, Vec<TypeInfo>), &StringInterner),
) -> Result {
    cmp_eq(a.0, b.0).qualified("key")?;
    cmp_slice(
        a.1.iter().map(|i| (i, a_strings)),
        b.1.iter().map(|i| (i, b_strings)),
        cmp_type_info,
    )
    .qualified("value")?;
    Ok(())
}

mod mapping {
    use super::*;

    impl MapName for (&Attribute, &StringInterner) {
        fn get_name(&self) -> String {
            self.0.name.as_bstr(self.1).to_string()
        }
    }

    impl MapName for (&Class<'_>, &StringInterner) {
        fn get_name(&self) -> String {
            self.0.name.as_bstr(self.1).to_string()
        }
    }

    impl MapName for (&ExFrameId, &ExFrame) {
        fn get_name(&self) -> String {
            self.0.to_string()
        }
    }

    impl MapName for (&Function<'_>, &StringInterner) {
        fn get_name(&self) -> String {
            self.0.name.as_bstr(self.1).to_string()
        }
    }

    impl MapName for (&HackConstant, &StringInterner) {
        fn get_name(&self) -> String {
            self.0.name.as_bstr(self.1).to_string()
        }
    }

    impl MapName for (&Method<'_>, &StringInterner) {
        fn get_name(&self) -> String {
            self.0.name.as_bstr(self.1).to_string()
        }
    }

    impl MapName for (&Module, &StringInterner) {
        fn get_name(&self) -> String {
            self.0.name.as_bstr(self.1).to_string()
        }
    }

    impl MapName for (&Property, &StringInterner) {
        fn get_name(&self) -> String {
            self.0.name.as_bstr(self.1).to_string()
        }
    }

    impl MapName for (&ClassId, &TParamBounds, &StringInterner) {
        fn get_name(&self) -> String {
            self.0.as_bstr(self.2).to_string()
        }
    }

    impl MapName for (&Typedef, &StringInterner) {
        fn get_name(&self) -> String {
            self.0.name.as_bstr(self.1).to_string()
        }
    }
}
