// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use anyhow::anyhow;
use anyhow::Context;
use anyhow::Result;
use bumpalo::Bump;
use ir_core::func::ExFrame;
use ir_core::instr;
use ir_core::instr::BaseOp;
use ir_core::instr::CallDetail;
use ir_core::instr::CmpOp;
use ir_core::instr::FinalOp;
use ir_core::instr::Hhbc;
use ir_core::instr::IncludeEval;
use ir_core::instr::IncludeKind;
use ir_core::instr::Instr;
use ir_core::instr::IntermediateOp;
use ir_core::instr::MemberKey;
use ir_core::instr::MemberOp;
use ir_core::instr::MemoGet;
use ir_core::instr::MemoGetEager;
use ir_core::instr::Predicate;
use ir_core::instr::Special;
use ir_core::instr::Terminator;
use ir_core::Block;
use ir_core::BlockId;
use ir_core::CcParam;
use ir_core::CcReified;
use ir_core::CcThis;
use ir_core::ClassIdMap;
use ir_core::Coeffects;
use ir_core::CollectionType;
use ir_core::Constant;
use ir_core::ExFrameId;
use ir_core::FCallArgsFlags;
use ir_core::Func;
use ir_core::FuncBuilder;
use ir_core::Function;
use ir_core::FunctionFlags;
use ir_core::InstrId;
use ir_core::InstrIdSet;
use ir_core::IterId;
use ir_core::LocId;
use ir_core::LocalId;
use ir_core::MOpMode;
use ir_core::MethodFlags;
use ir_core::ObjMethodOp;
use ir_core::PropId;
use ir_core::QueryMOp;
use ir_core::SetRangeOp;
use ir_core::SwitchKind;
use ir_core::TParamBounds;
use ir_core::TryCatchId;
use ir_core::UnitBytesId;
use ir_core::UnnamedLocalId;
use ir_core::ValueId;
use ir_core::Visibility;
use itertools::Itertools;
use parse_macro_ir::parse;

use crate::parse::convert_bid;
use crate::parse::is_block;
use crate::parse::is_int;
use crate::parse::is_lid;
use crate::parse::is_vid;
use crate::parse::parse_as_type_struct_exception_kind;
use crate::parse::parse_attr;
use crate::parse::parse_attribute;
use crate::parse::parse_bare_this_op;
use crate::parse::parse_bid;
use crate::parse::parse_class_get_c_kind;
use crate::parse::parse_class_id;
use crate::parse::parse_comma_list;
use crate::parse::parse_const_id;
use crate::parse::parse_constant;
use crate::parse::parse_constant_id;
use crate::parse::parse_dynamic_call_op;
use crate::parse::parse_fatal_op;
use crate::parse::parse_func_id;
use crate::parse::parse_i64;
use crate::parse::parse_inc_dec_op_post;
use crate::parse::parse_inc_dec_op_pre;
use crate::parse::parse_init_prop_op;
use crate::parse::parse_instr_id;
use crate::parse::parse_is_type_op;
use crate::parse::parse_m_op_mode;
use crate::parse::parse_method_id;
use crate::parse::parse_oo_decl_exists_op;
use crate::parse::parse_opt_enum;
use crate::parse::parse_param;
use crate::parse::parse_prop_id;
use crate::parse::parse_readonly;
use crate::parse::parse_set_op_op;
use crate::parse::parse_shadowed_tparams;
use crate::parse::parse_silence_op;
use crate::parse::parse_special_cls_ref;
use crate::parse::parse_special_cls_ref_opt;
use crate::parse::parse_src_loc;
use crate::parse::parse_string_id;
use crate::parse::parse_type_info;
use crate::parse::parse_type_struct_enforce_kind;
use crate::parse::parse_type_struct_resolve_op;
use crate::parse::parse_u32;
use crate::parse::parse_user_id;
use crate::parse::parse_usize;
use crate::parse::parse_visibility;
use crate::tokenizer::Token;
use crate::tokenizer::TokenLoc;
use crate::tokenizer::Tokenizer;

macro_rules! parse_instr {
    ($tok:ident, $cons:expr, $($rest:tt)+) => {{
        parse!($tok, $($rest)+);
        $cons
    }};
}

#[derive(Debug)]
enum OpKind {
    C,
    I(i64),
    L,
    T(UnitBytesId),
    W,
}

struct InstrInfo {
    iid: InstrId,
    instr: Instr,
}

struct BlockInfo {
    bid: BlockId,
    instrs: Vec<InstrInfo>,
}

pub(crate) struct ClassState {
    pub(crate) visibility: Visibility,
    pub(crate) flags: MethodFlags,
}

impl Default for ClassState {
    fn default() -> Self {
        Self {
            visibility: Visibility::Public,
            flags: Default::default(),
        }
    }
}

pub(crate) struct FunctionParser<'a, 'b> {
    alloc: &'a Bump,
    // A local cache for the (Block x (InstrId, Instr)) state.  BlockId::NONE
    // (where we put the non-block instrs like params) is guaranteed to be
    // blocks[0].
    blocks: Vec<BlockInfo>,
    builder: FuncBuilder<'a>,
    class_state: Option<&'b mut ClassState>,
    cur_loc: LocId,
    flags: FunctionFlags,
}

impl<'a, 'b> FunctionParser<'a, 'b> {
    pub(crate) fn parse(
        tokenizer: &mut Tokenizer<'_>,
        unit_state: &mut crate::assemble::UnitParser<'a>,
        mut class_state: Option<&'b mut ClassState>,
    ) -> Result<Function<'a>> {
        let alloc = unit_state.alloc;

        parse!(tokenizer, <name:parse_func_id>);

        let tparams = if tokenizer.next_is_identifier("<")? {
            let mut tparams = ClassIdMap::default();
            parse_comma_list(tokenizer, false, |tokenizer| {
                let name = parse_class_id(tokenizer)?;
                let mut bounds = TParamBounds::default();
                if tokenizer.next_is_identifier(":")? {
                    loop {
                        bounds.bounds.push(parse_type_info(tokenizer)?);
                        if !tokenizer.next_is_identifier("+")? {
                            break;
                        }
                    }
                }
                tparams.insert(name, bounds);
                Ok(())
            })?;

            tokenizer.expect_identifier(">")?;
            tparams
        } else {
            Default::default()
        };

        parse!(tokenizer, "(" <params:parse_param(alloc),*> ")" <shadowed_tparams:parse_shadowed_tparams> ":" <return_type:parse_type_info>);
        let attrs = parse_attr(tokenizer)?;

        if let Some(class_state) = &mut class_state {
            let vis = parse_visibility(tokenizer)?;
            class_state.visibility = vis;
        }

        parse!(tokenizer, "{" "\n");

        let mut builder = FuncBuilder::with_func(
            Func {
                return_type,
                params,
                tparams,
                shadowed_tparams,
                attrs,
                ..Default::default()
            },
            Arc::clone(&unit_state.unit.strings),
        );

        let cur_loc = if let Some(src_loc) = unit_state.src_loc.as_ref() {
            let loc = builder.add_loc(src_loc.clone());
            builder.func.loc_id = loc;
            loc
        } else {
            LocId::NONE
        };

        // Initialize the blocks with an empty BlockId::NONE at [0].
        let blocks = vec![BlockInfo {
            bid: BlockId::NONE,
            instrs: Vec::default(),
        }];

        let mut state = FunctionParser {
            alloc: unit_state.alloc,
            blocks,
            builder,
            class_state,
            cur_loc,
            flags: Default::default(),
        };

        while !tokenizer.next_is_identifier("}")? {
            let next = tokenizer.expect_any_identifier()?;
            let tok = next.identifier();

            let res = match tok {
                id if is_block(id) => state.parse_label(tokenizer, &next),
                ".async" => state.parse_flags(&next),
                ".attr" => state.parse_attr(tokenizer),
                ".catch_id" => state.parse_catch_id(tokenizer),
                ".closure_body" => state.parse_flags(&next),
                ".coeffects_cc_param" => state.parse_coeffects_cc_param(tokenizer),
                ".coeffects_cc_reified" => state.parse_coeffects_cc_reified(tokenizer),
                ".coeffects_cc_this" => state.parse_coeffects_cc_this(tokenizer),
                ".coeffects_closure_parent_scope" => {
                    state.builder.func.coeffects.closure_parent_scope = true;
                    Ok(())
                }
                ".coeffects_fun_param" => state.parse_coeffects_fun_param(tokenizer),
                ".coeffects_static" => state.parse_coeffects_static(tokenizer),
                ".coeffects_caller" => {
                    state.builder.func.coeffects.caller = true;
                    Ok(())
                }
                ".const" => state.parse_const(tokenizer),
                ".doc" => state.parse_doc(tokenizer),
                ".ex_frame" => state.parse_ex_frame(tokenizer),
                ".generator" => state.parse_flags(&next),
                ".is_memoize_wrapper" => {
                    state.builder.func.is_memoize_wrapper = true;
                    Ok(())
                }
                ".is_memoize_wrapper_lsb" => {
                    state.builder.func.is_memoize_wrapper_lsb = true;
                    Ok(())
                }
                ".memoize_impl" => state.parse_flags(&next),
                ".num_iters" => state.parse_num_iters(tokenizer),
                ".pair_generator" => state.parse_flags(&next),
                ".srcloc" => state.parse_srcloc_decl(tokenizer),
                ".try_id" => state.parse_try_id(tokenizer),
                _ => state.parse_instr(tokenizer, &next),
            };
            res.with_context(|| format!("Parsing {}", next))?;
            tokenizer.expect_eol()?;
        }

        state.latch_blocks();

        Ok(Function {
            flags: state.flags,
            name,
            func: state.builder.finish(),
        })
    }

    fn latch_blocks(&mut self) {
        assert!(self.builder.func.instrs.is_empty());

        // Sort the blocks.  BlockId::NONE (where the params all live) will end
        // up at the end.
        self.blocks.sort_by_key(|block| block.bid);

        let func_blocks = &mut self.builder.func.blocks;

        let count_instrs: usize = self.blocks.iter().map(|bi| bi.instrs.len()).sum();
        self.builder
            .func
            .instrs
            .resize(count_instrs, Instr::tombstone());

        let mut unused_iids: InstrIdSet = (0..count_instrs).map(InstrId::from_usize).collect();

        let func_instrs = &mut self.builder.func.instrs;

        // Place the Instrs with known InstrIds.
        for bi in &mut self.blocks {
            for ii in &mut bi.instrs {
                if ii.iid != InstrId::NONE {
                    unused_iids.remove(&ii.iid);
                    while ii.iid.as_usize() >= func_instrs.len() {
                        func_instrs.resize(ii.iid.as_usize() + 1, Instr::tombstone());
                    }
                    std::mem::swap(&mut func_instrs[ii.iid], &mut ii.instr);
                }
            }
        }

        // Figure out the (ordered) unused slots.
        let mut unused_iids = unused_iids.into_iter().sorted();

        // Place the remaining Instrs and fill in the Func blocks.
        for bi in &mut self.blocks {
            for ii in &mut bi.instrs {
                if ii.iid == InstrId::NONE {
                    let next_iid = unused_iids.next().unwrap();
                    std::mem::swap(&mut func_instrs[next_iid], &mut ii.instr);
                    ii.iid = next_iid;
                }
            }

            if bi.bid != BlockId::NONE {
                func_blocks[bi.bid].iids = bi.instrs.iter().map(|ii| ii.iid).collect();
            }
        }
    }
}

impl FunctionParser<'_, '_> {
    fn iid(&self, tokenizer: &mut Tokenizer<'_>) -> Result<InstrId> {
        match tokenizer.expect_any_token()? {
            Token::Identifier(s, _) if s.starts_with('%') => {
                let i = s[1..].parse()?;
                Ok(InstrId::from_usize(i))
            }
            t => Err(t.bail(format!("Expected InstrId, not '{t}'"))),
        }
    }

    fn keyvalue(&self, tokenizer: &mut Tokenizer<'_>) -> Result<(Option<LocalId>, LocalId)> {
        let a = self.lid(tokenizer)?;
        Ok(if tokenizer.next_is_identifier("=>")? {
            (Some(a), self.lid(tokenizer)?)
        } else {
            (None, a)
        })
    }

    fn lid(&self, tokenizer: &mut Tokenizer<'_>) -> Result<LocalId> {
        let (id, loc) = parse_user_id(tokenizer)?;
        if id.is_empty() {
            return Err(loc.bail("'$' expected, not ''".to_string()));
        }

        match id[0] {
            b'$' => {
                let id = self.builder.strings.intern_bytes(id);
                Ok(LocalId::Named(id))
            }
            b'@' => {
                let id = std::str::from_utf8(&id[1..])?.parse()?;
                Ok(LocalId::Unnamed(UnnamedLocalId::from_usize(id)))
            }
            _ => Err(loc.bail(format!(
                "'$' expected, not '{}'",
                String::from_utf8_lossy(&id)
            ))),
        }
    }

    fn vid(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<ValueId> {
        match tokenizer.peek_token()? {
            Some(Token::Identifier(s, _)) if s.starts_with('#') => {
                Ok(ValueId::from_constant(parse_constant_id(tokenizer)?))
            }
            Some(Token::Identifier(s, _)) if s.starts_with('%') => {
                Ok(ValueId::from_instr(parse_instr_id(tokenizer)?))
            }
            Some(_) => {
                let c = parse_constant(tokenizer, self.alloc)?;
                let id = self.builder.emit_constant(c);
                Ok(id)
            }
            None => Err(anyhow!("Expected token at end")),
        }
    }

    fn vid_opt(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<Option<ValueId>> {
        let t = tokenizer.peek_expect_token()?;
        match t {
            Token::Identifier(s, _) if s.starts_with('#') || s.starts_with('%') => {
                Ok(Some(self.vid(tokenizer)?))
            }
            _ => Ok(None),
        }
    }

    fn vid2(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<[ValueId; 2]> {
        parse!(tokenizer, <p0:self.vid> "," <p1:self.vid>);
        Ok([p0, p1])
    }

    fn vid3(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<[ValueId; 3]> {
        parse!(tokenizer, <p0:self.vid> "," <p1:self.vid> "," <p2:self.vid>);
        Ok([p0, p1, p2])
    }

    fn arg(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<Arg> {
        let mut inout = false;
        let mut readonly = false;

        loop {
            if tokenizer.next_is_identifier("inout")? {
                inout = true;
                continue;
            }
            if tokenizer.next_is_identifier("readonly")? {
                readonly = true;
                continue;
            }
            break;
        }

        let vid = self.vid(tokenizer)?;
        Ok(Arg {
            inout,
            readonly,
            vid,
        })
    }
}

struct Arg {
    inout: bool,
    readonly: bool,
    vid: ValueId,
}

impl FunctionParser<'_, '_> {
    fn parse_call(
        &mut self,
        tokenizer: &mut Tokenizer<'_>,
        is_async: bool,
        loc: LocId,
    ) -> Result<Instr> {
        // call direct var_dump(%1) enforce_mutable_return ""
        let kind = tokenizer.expect_any_identifier()?;
        let mut operands = Vec::new();
        let mut operands_suffix = Vec::new();
        let detail = match kind.identifier() {
            "cls_method" => {
                if let Some(clsref) = parse_special_cls_ref_opt(tokenizer)? {
                    tokenizer.expect_identifier("::")?;
                    if let Some(vid) = self.vid_opt(tokenizer)? {
                        // clsref::vid => FCallClsMethodS
                        operands_suffix.push(vid);
                        CallDetail::FCallClsMethodS { clsref }
                    } else {
                        // clsref::id => FCallClsMethodSD
                        let method = parse_method_id(tokenizer)?;
                        CallDetail::FCallClsMethodSD { clsref, method }
                    }
                } else if let Some(cls_vid) = self.vid_opt(tokenizer)? {
                    operands_suffix.push(cls_vid);
                    tokenizer.expect_identifier("::")?;
                    if let Some(vid) = self.vid_opt(tokenizer)? {
                        // vid::vid dc => FCallClsMethod
                        operands_suffix.push(vid);
                        let log = parse_dynamic_call_op(tokenizer)?;
                        CallDetail::FCallClsMethod { log }
                    } else {
                        // vid::id dc => FCallClsMethodM
                        let method = parse_method_id(tokenizer)?;
                        let log = parse_dynamic_call_op(tokenizer)?;
                        CallDetail::FCallClsMethodM { method, log }
                    }
                } else {
                    // id::id => FCallClsMethodD
                    let clsid = parse_class_id(tokenizer)?;
                    tokenizer.expect_identifier("::")?;
                    let method = parse_method_id(tokenizer)?;
                    CallDetail::FCallClsMethodD { clsid, method }
                }
            }
            "ctor" => {
                let op = self.vid(tokenizer)?;
                operands.push(op);
                CallDetail::FCallCtor
            }
            "func" => {
                if let Some(vid) = self.vid_opt(tokenizer)? {
                    operands_suffix.push(vid);
                    CallDetail::FCallFunc
                } else {
                    let func = parse_func_id(tokenizer)?;
                    CallDetail::FCallFuncD { func }
                }
            }
            "obj_method" => {
                parse!(tokenizer, <obj:self.vid>
                       <flavor:["->": "->" { ObjMethodOp::NullThrows } ;
                                "?->": "?->" { ObjMethodOp::NullSafe }]> );
                operands.push(obj);
                let next = tokenizer.peek_expect_token()?;
                if next
                    .get_identifier()
                    .map_or(false, |n| is_vid(n.as_bytes()))
                {
                    // vid->vid => FCallObjMethod
                    let method = self.vid(tokenizer)?;
                    operands_suffix.push(method);
                    CallDetail::FCallObjMethod { flavor }
                } else {
                    // vid->id => FCallObjMethodD
                    let method = parse_method_id(tokenizer)?;
                    CallDetail::FCallObjMethodD { method, flavor }
                }
            }
            _ => return Err(kind.bail(format!("Unknown call kind '{kind}'"))),
        };

        parse!(tokenizer, "(" <ops:self.arg,*> ")");
        operands.extend(ops.iter().map(|arg| arg.vid));
        operands_suffix.reverse();
        operands.extend(operands_suffix);

        let inouts: Option<Box<[u32]>> = if ops.iter().any(|arg| arg.inout) {
            Some(
                ops.iter()
                    .enumerate()
                    .filter_map(|(i, arg)| if arg.inout { Some(i as u32) } else { None })
                    .collect(),
            )
        } else {
            None
        };

        let num_rets = 1 + inouts.as_ref().map_or(0, |i| i.len() as u32);

        let readonly = if ops.iter().any(|arg| arg.readonly) {
            Some(
                ops.iter()
                    .enumerate()
                    .filter_map(|(i, arg)| if arg.readonly { Some(i as u32) } else { None })
                    .collect(),
            )
        } else {
            None
        };

        fn convert_fcall_args_flags(id: &str) -> Option<FCallArgsFlags> {
            Some(match id {
                "has_unpack" => FCallArgsFlags::HasUnpack,
                "has_generics" => FCallArgsFlags::HasGenerics,
                "lock_while_unwinding" => FCallArgsFlags::LockWhileUnwinding,
                "enforce_mutable_return" => FCallArgsFlags::EnforceMutableReturn,
                "enforce_readonly_this" => FCallArgsFlags::EnforceReadonlyThis,
                "skip_repack" => FCallArgsFlags::SkipRepack,
                "skip_coeffects_check" => FCallArgsFlags::SkipCoeffectsCheck,
                "explicit_context" => FCallArgsFlags::ExplicitContext,
                "has_in_out" => FCallArgsFlags::HasInOut,
                "enforce_in_out" => FCallArgsFlags::EnforceInOut,
                "enforce_readonly" => FCallArgsFlags::EnforceReadonly,
                "has_async_eager_offset" => FCallArgsFlags::HasAsyncEagerOffset,
                "num_args_start" => FCallArgsFlags::NumArgsStart,
                _ => return None,
            })
        }

        let mut flags = FCallArgsFlags::default();
        while let Some(flag) = parse_opt_enum(tokenizer, convert_fcall_args_flags)? {
            flags |= flag;
        }

        let context = parse_string_id(tokenizer)?;

        let call = instr::Call {
            operands: operands.into(),
            context,
            detail,
            flags,
            num_rets,
            inouts,
            readonly,
            loc,
        };

        if is_async {
            parse!(tokenizer, "to" <p0:parse_bid> "eager" <p1:parse_bid>);
            Ok(Instr::Terminator(Terminator::CallAsync(
                Box::new(call),
                [p0, p1],
            )))
        } else {
            Ok(Instr::Call(Box::new(call)))
        }
    }

    fn parse_cmp(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        parse!(tokenizer, <lhs:self.vid> <e:id> <rhs:self.vid>);
        let op = match e.identifier() {
            "<=>" => {
                return Ok(Instr::Hhbc(Hhbc::Cmp([lhs, rhs], loc)));
            }
            "==" => CmpOp::Eq,
            ">" => CmpOp::Gt,
            ">=" => CmpOp::Gte,
            "<" => CmpOp::Lt,
            "<=" => CmpOp::Lte,
            "!==" => CmpOp::NSame,
            "!=" => CmpOp::Neq,
            "===" => CmpOp::Same,
            _ => return Err(e.bail(format!("Expected compare identifier, got '{e}'"))),
        };

        Ok(Instr::Hhbc(Hhbc::CmpOp([lhs, rhs], op, loc)))
    }

    fn parse_col_from_array(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        parse!(tokenizer, <colty:id> <vid:self.vid>);

        fn convert_collection_type(id: &str) -> Option<CollectionType> {
            Some(match id {
                "vector" => CollectionType::Vector,
                "map" => CollectionType::Map,
                "set" => CollectionType::Set,
                "pair" => CollectionType::Pair,
                "imm_vector" => CollectionType::ImmVector,
                "imm_map" => CollectionType::ImmMap,
                "imm_set" => CollectionType::ImmSet,
                _ => return None,
            })
        }

        let colty = convert_collection_type(colty.identifier())
            .ok_or_else(|| colty.bail(format!("Expected CollectionType but got '{colty}'")))?;
        Ok(Instr::Hhbc(Hhbc::ColFromArray(vid, colty, loc)))
    }

    fn parse_incdec_local(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        parse!(tokenizer, <inc_dec_op_pre:parse_inc_dec_op_pre> <lid:self.lid> <inc_dec_op:parse_inc_dec_op_post(inc_dec_op_pre)>);
        Ok(Instr::Hhbc(Hhbc::IncDecL(lid, inc_dec_op, loc)))
    }

    fn parse_incdec_static_prop(
        &mut self,
        tokenizer: &mut Tokenizer<'_>,
        loc: LocId,
    ) -> Result<Instr> {
        parse!(tokenizer, <inc_dec_op_pre:parse_inc_dec_op_pre> <cls:self.vid> "::" <prop:self.vid> <inc_dec_op:parse_inc_dec_op_post(inc_dec_op_pre)>);
        Ok(Instr::Hhbc(Hhbc::IncDecS([cls, prop], inc_dec_op, loc)))
    }

    fn parse_iterator(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        parse!(tokenizer, "^" <iter_id:parse_u32> <op:["free":"free"; "next":"next"; "init":"init"]>);
        let iter_id = IterId { idx: iter_id };
        Ok(match op.identifier() {
            "free" => Instr::Hhbc(Hhbc::IterFree(iter_id, loc)),
            "init" => {
                parse!(tokenizer, "from" <vid:self.vid> "jmp" "to" <target0:parse_bid> "else" <target1:parse_bid> "with" <locals:self.keyvalue>);
                Instr::Terminator(Terminator::IterInit(
                    instr::IteratorArgs::new(iter_id, locals.0, locals.1, target0, target1, loc),
                    vid,
                ))
            }
            "next" => {
                parse!(tokenizer, "jmp" "to" <target0:parse_bid> "else" <target1:parse_bid> "with" <locals:self.keyvalue>);
                Instr::Terminator(Terminator::IterNext(instr::IteratorArgs::new(
                    iter_id, locals.0, locals.1, target0, target1, loc,
                )))
            }
            _ => unreachable!(),
        })
    }

    fn parse_jmp(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        let jmp = if tokenizer.next_is_identifier("if")? {
            parse!(tokenizer,
                   <pred:["nonzero": "nonzero" { Predicate::NonZero } ; "zero": "zero" { Predicate::Zero }]>
                   <cond:self.vid> "to" <true_bid:parse_bid> "else" <false_bid:parse_bid>);
            Terminator::JmpOp {
                cond,
                pred,
                targets: [true_bid, false_bid],
                loc,
            }
        } else {
            parse!(tokenizer, "to" <target:parse_bid>);

            if tokenizer.next_is_identifier("with")? {
                parse!(tokenizer, "(" <params:self.vid,*> ")");
                Terminator::JmpArgs(target, params.into(), loc)
            } else {
                Terminator::Jmp(target, loc)
            }
        };
        Ok(Instr::Terminator(jmp))
    }

    fn parse_memo_get(
        &mut self,
        tokenizer: &mut Tokenizer<'_>,
        mnemonic: &str,
        loc: LocId,
    ) -> Result<Instr> {
        Ok(Instr::Terminator(match mnemonic {
            "memo_get" => {
                parse!(tokenizer, "[" <locals:self.lid,*> "]" "to" <value:parse_bid> "else" <no_value:parse_bid>);
                Terminator::MemoGet(MemoGet::new(value, no_value, &locals, loc))
            }
            "memo_get_eager" => {
                parse!(tokenizer, "[" <locals:self.lid,*> "]" "to" <suspended:parse_bid> "eager" <eager:parse_bid> "else" <no_value:parse_bid>);
                Terminator::MemoGetEager(MemoGetEager::new(
                    no_value, suspended, eager, &locals, loc,
                ))
            }
            _ => unreachable!(),
        }))
    }

    fn parse_new_obj(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        if tokenizer.next_is_identifier("direct")? {
            let clsid = parse_class_id(tokenizer)?;
            Ok(Instr::Hhbc(Hhbc::NewObjD(clsid, loc)))
        } else if tokenizer.next_is_identifier("static")? {
            let clsref = parse_special_cls_ref(tokenizer)?;
            Ok(Instr::Hhbc(Hhbc::NewObjS(clsref, loc)))
        } else {
            Ok(Instr::Hhbc(Hhbc::NewObj(self.vid(tokenizer)?, loc)))
        }
    }

    fn parse_new_struct_dict(
        &mut self,
        tokenizer: &mut Tokenizer<'_>,
        loc: LocId,
    ) -> Result<Instr> {
        tokenizer.expect_identifier("[")?;

        let mut keys: Vec<UnitBytesId> = Vec::new();
        let mut values: Vec<ValueId> = Vec::new();

        loop {
            let name = parse_string_id(tokenizer)?;
            keys.push(name);
            tokenizer.expect_identifier("=>")?;
            values.push(self.vid(tokenizer)?);
            if !tokenizer.next_is_identifier(",")? {
                break;
            }
        }

        tokenizer.expect_identifier("]")?;

        Ok(Instr::Hhbc(Hhbc::NewStructDict(
            keys.into(),
            values.into(),
            loc,
        )))
    }

    fn parse_ret(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        Ok(if tokenizer.next_is_identifier("[")? {
            parse!(tokenizer, <params:self.vid,*> "]");
            Instr::Terminator(Terminator::RetM(params.into(), loc))
        } else {
            Instr::ret(self.vid(tokenizer)?, loc)
        })
    }

    fn parse_sswitch(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        parse!(tokenizer, <cond:self.vid> "[" <cases:self.parse_sswitch_case,*> "]");
        let (cases, targets): (Vec<UnitBytesId>, Vec<BlockId>) = cases.into_iter().unzip();
        Ok(Instr::Terminator(Terminator::SSwitch {
            cond,
            cases: cases.into(),
            targets: targets.into(),
            loc,
        }))
    }

    fn parse_switch(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        parse!(tokenizer,
               <bounded:["bounded": "bounded" { SwitchKind::Bounded } ;
                         "unbounded": "unbounded" { SwitchKind::Unbounded }]>
               <cond:self.vid> <base:parse_i64> "[" <targets:parse_bid,*> "]");
        Ok(Instr::Terminator(Terminator::Switch {
            cond,
            bounded,
            base,
            targets: targets.into(),
            loc,
        }))
    }

    fn parse_sswitch_case(
        &mut self,
        tokenizer: &mut Tokenizer<'_>,
    ) -> Result<(UnitBytesId, BlockId)> {
        parse!(tokenizer, <key:parse_string_id> "=>" <bid:parse_bid>);
        Ok((key, bid))
    }

    fn parse_unset(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        if let Some(t) = tokenizer.peek_if_any_identifier()? {
            if t.identifier().starts_with('@') || t.identifier().starts_with('$') {
                return Ok(Instr::Hhbc(Hhbc::UnsetL(self.lid(tokenizer)?, loc)));
            }
        }
        Ok(Instr::Hhbc(Hhbc::UnsetG(self.vid(tokenizer)?, loc)))
    }
    fn parse_yield(&mut self, tokenizer: &mut Tokenizer<'_>, loc: LocId) -> Result<Instr> {
        let value = self.vid(tokenizer)?;

        if tokenizer.next_is_identifier("=>")? {
            let key = self.vid(tokenizer)?;
            Ok(Instr::Hhbc(Hhbc::YieldK([value, key], loc)))
        } else {
            Ok(Instr::Hhbc(Hhbc::Yield(value, loc)))
        }
    }
}

impl FunctionParser<'_, '_> {
    fn parse_member_op(
        &mut self,
        tokenizer: &mut Tokenizer<'_>,
        mnemonic: &str,
        loc: LocId,
    ) -> Result<Instr> {
        let inc_dec_op = if mnemonic == "incdecm" {
            parse_inc_dec_op_pre(tokenizer)?
        } else {
            None
        };

        let mut operands = Vec::new();
        let mut locals = Vec::new();

        let base_op = {
            let mode = parse_m_op_mode(tokenizer)?;
            let readonly = parse_readonly(tokenizer)?;

            let t = tokenizer.peek_expect_token()?;
            let s = t.unescaped_identifier()?;
            match s.as_ref() {
                b"$this" => {
                    tokenizer.read_token()?;
                    BaseOp::BaseH { loc }
                }
                b"global" => {
                    tokenizer.read_token()?;
                    let vid = self.vid(tokenizer)?;
                    operands.push(vid);
                    BaseOp::BaseGC { mode, loc }
                }
                s if is_vid(s) => {
                    let vid_cls = self.vid(tokenizer)?;

                    if tokenizer.next_is_identifier("::")? {
                        let vid_prop = self.vid(tokenizer)?;
                        operands.push(vid_prop);
                        operands.push(vid_cls);
                        BaseOp::BaseSC {
                            mode,
                            loc,
                            readonly,
                        }
                    } else {
                        operands.push(vid_cls);
                        BaseOp::BaseC { mode, loc }
                    }
                }
                s if is_lid(s) => {
                    let lid = self.lid(tokenizer)?;
                    locals.push(lid);
                    BaseOp::BaseL {
                        mode,
                        readonly,
                        loc,
                    }
                }
                _ => return Err(t.bail(format!("Invalid base, got '{t}'"))),
            }
        };

        let mut intermediate_ops = Vec::new();
        while let Some(op) = self.parse_intermediate_op(tokenizer, &mut operands, &mut locals)? {
            intermediate_ops.push(op);
        }

        let final_op = if mnemonic == "setrangem" {
            parse!(tokenizer, "set" <vids:self.vid3> "size" <sz:parse_u32>
                   <set_range_op:[
                       "forward":"forward" { SetRangeOp::Forward } ;
                       "reverse":"reverse" { SetRangeOp::Reverse }
                   ]>);
            operands.extend(vids);
            FinalOp::SetRangeM {
                sz,
                set_range_op,
                loc,
            }
        } else {
            let IntermediateOp {
                key,
                readonly,
                loc,
                mode: _,
            } = intermediate_ops.pop().unwrap();

            match mnemonic {
                "incdecm" => {
                    let inc_dec_op = parse_inc_dec_op_post(tokenizer, inc_dec_op)?;
                    FinalOp::IncDecM {
                        inc_dec_op,
                        key,
                        readonly,
                        loc,
                    }
                }
                "querym" => {
                    let query_m_op = parse_opt_enum(tokenizer, |id| {
                        Some(match id {
                            "quiet" => QueryMOp::CGetQuiet,
                            "isset" => QueryMOp::Isset,
                            "inout" => QueryMOp::InOut,
                            _ => return None,
                        })
                    })?
                    .unwrap_or(QueryMOp::CGet);

                    FinalOp::QueryM {
                        key,
                        readonly,
                        query_m_op,
                        loc,
                    }
                }
                "setm" => {
                    parse!(tokenizer, "=" <value:self.vid>);
                    operands.push(value);
                    FinalOp::SetM { key, readonly, loc }
                }
                "setopm" => {
                    parse!(tokenizer, <set_op_op:parse_set_op_op> <value:self.vid>);
                    operands.push(value);
                    FinalOp::SetOpM {
                        set_op_op,
                        key,
                        readonly,
                        loc,
                    }
                }
                "unsetm" => FinalOp::UnsetM { key, readonly, loc },
                _ => unreachable!(),
            }
        };

        Ok(Instr::MemberOp(MemberOp {
            base_op,
            final_op,
            intermediate_ops: intermediate_ops.into(),
            locals: locals.into(),
            operands: operands.into(),
        }))
    }

    fn parse_intermediate_op(
        &mut self,
        tokenizer: &mut Tokenizer<'_>,
        operands: &mut Vec<ValueId>,
        locals: &mut Vec<LocalId>,
    ) -> Result<Option<IntermediateOp>> {
        let lead = if let Some(t) = tokenizer.next_if_predicate(|t| {
            t.is_identifier("[") || t.is_identifier("->") || t.is_identifier("?->")
        })? {
            t
        } else {
            return Ok(None);
        };

        if tokenizer.next_is_identifier("<")? {
            parse!(tokenizer, "srcloc" self.parse_src_loc ">");
        }

        let mode = parse_opt_enum(tokenizer, |id| match id {
            "define" => Some(MOpMode::Define),
            "inout" => Some(MOpMode::InOut),
            "unset" => Some(MOpMode::Unset),
            "warn" => Some(MOpMode::Warn),
            _ => None,
        })?
        .unwrap_or(MOpMode::None);

        let readonly = parse_readonly(tokenizer)?;

        let (kind, tloc) = self.read_op(tokenizer, operands, locals)?;
        let key = match lead.identifier() {
            "[" => {
                let key = match kind {
                    OpKind::C => MemberKey::EC,
                    OpKind::I(i) => MemberKey::EI(i),
                    OpKind::L => MemberKey::EL,
                    OpKind::T(id) => MemberKey::ET(id),
                    OpKind::W => MemberKey::W,
                };
                tokenizer.expect_identifier("]")?;
                key
            }
            "->" => match kind {
                OpKind::C => MemberKey::PC,
                OpKind::I(i) => {
                    // This needs to be pushed as a constant!
                    operands.push(self.builder.emit_constant(Constant::Int(i)));
                    MemberKey::PC
                }
                OpKind::L => MemberKey::PL,
                OpKind::T(id) => MemberKey::PT(PropId::new(id)),
                OpKind::W => return Err(tloc.bail("']' not allowed with pointer")),
            },
            "?->" => match kind {
                OpKind::T(id) => MemberKey::QT(PropId::new(id)),
                OpKind::C | OpKind::I(_) | OpKind::L | OpKind::W => {
                    return Err(tloc.bail("']' not allowed with {kind:?}"));
                }
            },
            _ => unreachable!(),
        };

        let loc = self.cur_loc;

        Ok(Some(IntermediateOp {
            key,
            mode,
            readonly,
            loc,
        }))
    }

    fn read_op(
        &mut self,
        tokenizer: &mut Tokenizer<'_>,
        operands: &mut Vec<ValueId>,
        locals: &mut Vec<LocalId>,
    ) -> Result<(OpKind, TokenLoc)> {
        let tloc = tokenizer.peek_loc();
        let t = tokenizer.peek_expect_token()?;
        Ok(match t {
            Token::QuotedString(_, _, _) => {
                let id = parse_string_id(tokenizer)?;
                (OpKind::T(id), tloc)
            }
            Token::Identifier(ident, _) => {
                if ident == "]" {
                    (OpKind::W, tloc)
                } else if is_lid(ident.as_bytes()) {
                    let lid = self.lid(tokenizer)?;
                    locals.push(lid);
                    (OpKind::L, tloc)
                } else if is_int(ident.as_bytes()) {
                    let i = parse_i64(tokenizer)?;
                    (OpKind::I(i), tloc)
                } else {
                    let vid = self.vid(tokenizer)?;
                    operands.push(vid);
                    (OpKind::C, tloc)
                }
            }
            Token::Eol(_) => return Err(t.bail(format!("Expected op but got '{t}'"))),
        })
    }
}

impl FunctionParser<'_, '_> {
    fn parse_attr(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let attr = parse_attribute(tokenizer)?;
        self.builder.func.attributes.push(attr);
        Ok(())
    }

    fn parse_catch_id(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let id = ExFrameId::from_usize(parse_usize(tokenizer)?);
        self.builder.cur_block_mut().tcid = TryCatchId::Catch(id);
        Ok(())
    }

    fn parse_coeffects_cc_param(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <index:parse_u32> <ctx_name:string>);
        let ctx_name = ctx_name.unescaped_string()?;
        self.builder.func.coeffects.cc_param.push(CcParam {
            index,
            ctx_name: ir_core::intern(String::from_utf8(ctx_name)?),
        });
        Ok(())
    }

    fn parse_coeffects_cc_reified(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <is_class:"is_class"?> <index:parse_u32>);
        let is_class = is_class.is_some();

        let mut types = Vec::new();
        loop {
            let (id, _) = parse_user_id(tokenizer)?;
            types.push(ir_core::intern(std::str::from_utf8(&id)?));
            if !tokenizer.next_is_identifier("::")? {
                break;
            }
        }

        self.builder.func.coeffects.cc_reified.push(CcReified {
            is_class,
            index,
            types: types.into(),
        });
        Ok(())
    }

    fn parse_coeffects_cc_this(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <types:string,*>);
        let types = types
            .into_iter()
            .map(|t| {
                Ok(ir_core::intern(std::str::from_utf8(
                    &t.unescaped_string()?,
                )?))
            })
            .collect::<Result<Vec<_>>>()?
            .into();
        self.builder.func.coeffects.cc_this.push(CcThis { types });
        Ok(())
    }

    fn parse_coeffects_fun_param(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <fun_param:parse_u32,+>);
        self.builder.func.coeffects.fun_param = fun_param.into();
        Ok(())
    }

    fn parse_coeffects_static(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let static_coeffects = if tokenizer.next_is_identifier("enforced")? {
            parse!(tokenizer, "(" <static_coeffects:parse_user_id,*> ")");
            static_coeffects
                .into_iter()
                .map(|(ctx, loc)| {
                    let s = String::from_utf8_lossy(&ctx);
                    naming_special_names_rust::coeffects::ctx_str_to_enum(&s).ok_or_else(|| {
                        loc.bail(format!(
                            "Expected coeffect but got '{}'",
                            String::from_utf8_lossy(&ctx)
                        ))
                    })
                })
                .try_collect()?
        } else {
            Vec::new()
        };

        let unenforced_static_coeffects = if tokenizer.next_is_identifier("unenforced")? {
            parse!(tokenizer, "(" <unenforced_static_coeffects:parse_user_id,*> ")");
            unenforced_static_coeffects
                .into_iter()
                .map(|(ctx, _)| Ok(ir_core::intern(std::str::from_utf8(&ctx)?)))
                .collect::<Result<Vec<_>>>()?
        } else {
            Vec::new()
        };

        self.builder.func.coeffects = Coeffects {
            static_coeffects: static_coeffects.into(),
            unenforced_static_coeffects: unenforced_static_coeffects.into(),
            ..Coeffects::default()
        };
        Ok(())
    }

    fn parse_const(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <idx:parse_constant_id> "=" <value:parse_constant(self.alloc)>);

        if self.builder.func.constants.len() <= idx.as_usize() {
            self.builder
                .func
                .constants
                .resize(idx.as_usize() + 1, Constant::Uninit);
        }
        self.builder.constant_lookup.insert(value.clone(), idx);
        self.builder.func.constants[idx] = value;
        Ok(())
    }

    fn parse_doc(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let doc = tokenizer.expect_any_string()?.unescaped_string()?;
        self.builder.func.doc_comment = Some(doc);
        Ok(())
    }

    fn parse_ex_frame(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <num:parse_u32> ":" "catch" "=" <catch_bid:parse_bid>);
        let num = ExFrameId(num);

        let parent = if tokenizer.next_is_identifier(",")? {
            parse!(tokenizer, "parent" "=" <kind:["try": "try"; "catch": "catch"]> "(" <parent:parse_u32> ")");
            let parent = ExFrameId(parent);
            match kind.identifier() {
                "try" => TryCatchId::Try(parent),
                "catch" => TryCatchId::Catch(parent),
                _ => unreachable!(),
            }
        } else {
            TryCatchId::None
        };

        let frame = ExFrame { parent, catch_bid };
        self.builder.func.ex_frames.insert(num, frame);

        Ok(())
    }

    fn parse_flags(&mut self, lead: &Token) -> Result<()> {
        if let Some(class_state) = &mut self.class_state {
            let bit = match lead.identifier() {
                ".async" => MethodFlags::IS_ASYNC,
                ".closure_body" => MethodFlags::IS_CLOSURE_BODY,
                ".generator" => MethodFlags::IS_GENERATOR,
                ".pair_generator" => MethodFlags::IS_PAIR_GENERATOR,
                _ => return Err(lead.bail(format!("Expected MethodFlags, got {lead}"))),
            };
            class_state.flags |= bit;
        } else {
            let bit = match lead.identifier() {
                ".async" => FunctionFlags::ASYNC,
                ".generator" => FunctionFlags::GENERATOR,
                ".memoize_impl" => FunctionFlags::MEMOIZE_IMPL,
                ".pair_generator" => FunctionFlags::PAIR_GENERATOR,
                _ => return Err(lead.bail(format!("Expected FunctionFlags, got {lead}"))),
            };
            self.flags |= bit;
        }
        Ok(())
    }

    fn parse_num_iters(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        self.builder.func.num_iters = parse_usize(tokenizer)?;
        Ok(())
    }

    fn parse_instr(&mut self, tokenizer: &mut Tokenizer<'_>, lead: &Token) -> Result<()> {
        let tmp: Token;
        let mut mnemonic = lead.identifier();

        let iid = if mnemonic.starts_with('%') {
            let iid = InstrId::from_usize(mnemonic[1..].parse::<usize>()?);
            tokenizer.expect_identifier("=")?;
            tmp = tokenizer.expect_any_identifier()?;
            mnemonic = tmp.identifier();
            iid
        } else {
            InstrId::NONE
        };

        use Hhbc as H;
        use Instr as I;
        use Terminator as T;

        let loc = self.cur_loc;
        let tok = tokenizer;
        #[rustfmt::skip]
        let instr = match mnemonic {
            "add" => I::Hhbc(H::Add(self.vid2(tok)?, loc)),
            "add_elem_c" => parse_instr!(tok, I::Hhbc(H::AddElemC([p0, p1, p2], loc)), <p0:self.vid> "[" <p1:self.vid> "]" "=" <p2:self.vid>),
            "add_new_elem_c" => parse_instr!(tok, I::Hhbc(H::AddNewElemC([p0, p1], loc)), <p0:self.vid> "[" "]" "=" <p1:self.vid>),
            "ak_exists" => I::Hhbc(Hhbc::AKExists(self.vid2(tok)?, loc)),
            "array_idx" => parse_instr!(tok, I::Hhbc(H::ArrayIdx([p0, p1, p2], loc)), <p0:self.vid> "[" <p1:self.vid> "]" "or" <p2:self.vid>),
            "array_mark_legacy" => I::Hhbc(H::ArrayMarkLegacy(self.vid2(tok)?, loc)),
            "array_unmark_legacy" => I::Hhbc(H::ArrayUnmarkLegacy(self.vid2(tok)?, loc)),
            "async_call" => self.parse_call(tok, true, loc)?,
            "await" => I::Hhbc(H::Await(self.vid(tok)?, loc)),
            "await_all" => parse_instr!(tok, I::Hhbc(H::AwaitAll(p0.into(), loc)), "[" <p0:self.lid,*> "]"),
            "bare_this" => I::Hhbc(H::BareThis(parse_bare_this_op(tok)?, loc)),
            "bit_and" => I::Hhbc(H::BitAnd(self.vid2(tok)?, loc)),
            "bit_not" => I::Hhbc(H::BitNot(self.vid(tok)?, loc)),
            "bit_or" => I::Hhbc(H::BitOr(self.vid2(tok)?, loc)),
            "bit_xor" => I::Hhbc(H::BitXor(self.vid2(tok)?, loc)),
            "call" => self.parse_call(tok, false, loc)?,
            "cast_bool" => I::Hhbc(H::CastBool(self.vid(tok)?, loc)),
            "cast_dict" => I::Hhbc(H::CastDict(self.vid(tok)?, loc)),
            "cast_double" => I::Hhbc(H::CastDouble(self.vid(tok)?, loc)),
            "cast_int" => I::Hhbc(H::CastInt(self.vid(tok)?, loc)),
            "cast_keyset" => I::Hhbc(H::CastKeyset(self.vid(tok)?, loc)),
            "cast_string" => I::Hhbc(H::CastString(self.vid(tok)?, loc)),
            "cast_vec" => I::Hhbc(H::CastVec(self.vid(tok)?, loc)),
            "chain_faults" => I::Hhbc(H::ChainFaults(self.vid2(tok)?, loc)),
            "check_cls_reified_generic_mismatch" => I::Hhbc(H::CheckClsReifiedGenericMismatch(self.vid(tok)?, loc)),
            "check_cls_rg_soft" => I::Hhbc(H::CheckClsRGSoft(self.vid(tok)?, loc)),
            "check_prop" => I::Hhbc(H::CheckProp(parse_prop_id(tok)?, loc)),
            "check_this" => I::Hhbc(H::CheckThis(loc)),
            "class_get_c" => parse_instr!(tok, I::Hhbc(H::ClassGetC(p1, p0, loc)), <p0:parse_class_get_c_kind> <p1:self.vid>),

            "class_get_ts" => I::Hhbc(H::ClassGetTS(self.vid(tok)?, loc)),
            "class_has_reified_generics" => I::Hhbc(H::ClassHasReifiedGenerics(self.vid(tok)?, loc)),
            "class_name" => I::Hhbc(H::ClassName(self.vid(tok)?, loc)),
            "clone" => I::Hhbc(H::Clone(self.vid(tok)?, loc)),
            "cls_cns" => parse_instr!(tok, I::Hhbc(H::ClsCns(p0, p1, loc)), <p0:self.vid> "::" <p1:parse_const_id>),
            "cls_cns_d" => parse_instr!(tok, I::Hhbc(H::ClsCnsD(p0, p1, loc)), <p1:parse_class_id> "::" <p0:parse_const_id>),
            "cmp" => self.parse_cmp(tok, loc)?,
            "col_from_array" => self.parse_col_from_array(tok, loc)?,
            "combine_and_resolve_type_struct" => I::Hhbc(H::CombineAndResolveTypeStruct(parse_comma_list(tok, false, |tok| self.vid(tok))?.into(), loc)),
            "concat" => I::Hhbc(H::Concat(self.vid2(tok)?, loc)),
            "concatn" => parse_instr!(tok, I::Hhbc(H::ConcatN(p0.into(), loc)), <p0:self.vid,*>),
            "consume_local" => I::Hhbc(H::ConsumeL(self.lid(tok)?, loc)),
            "cont_current" => I::Hhbc(H::ContCurrent(loc)),
            "cont_enter" => I::Hhbc(H::ContEnter(self.vid(tok)?, loc)),
            "cont_get_return" => I::Hhbc(H::ContGetReturn(loc)),
            "cont_key" => I::Hhbc(H::ContKey(loc)),
            "cont_raise" => I::Hhbc(H::ContRaise(self.vid(tok)?, loc)),
            "cont_valid" => I::Hhbc(H::ContValid(loc)),
            "create_class" => parse_instr!(tok, I::Hhbc(H::CreateCl{operands: operands.into(), clsid, loc}), <clsid:parse_class_id> "(" <operands:self.vid,*> ")"),
            "create_cont" => I::Hhbc(H::CreateCont(loc)),
            "create_special_implicit_context" => I::Hhbc(H::CreateSpecialImplicitContext(self.vid2(tok)?, loc)),
            "div" => I::Hhbc(H::Div(self.vid2(tok)?, loc)),
            "enter" => parse_instr!(tok, I::Terminator(T::Enter(p0, loc)), "to" <p0:parse_bid>),
            "eval" => I::Hhbc(H::IncludeEval(IncludeEval { kind: IncludeKind::Eval, vid: self.vid(tok)?, loc })),
            "exit" => I::Terminator(T::Exit(self.vid(tok)?, loc)),
            "fatal" => parse_instr!(tok, I::Terminator(T::Fatal(p0, p1, loc)), <p1:parse_fatal_op> "," <p0:self.vid>),
            "get_class_rg_prop" => I::Hhbc(H::GetClsRGProp(self.vid(tok)?, loc)),
            "get_global" => I::Hhbc(H::CGetG(self.vid(tok)?, loc)),
            "get_local" => I::Hhbc(H::CGetL(self.lid(tok)?, loc)),
            "get_local_quiet" => I::Hhbc(H::CGetQuietL(self.lid(tok)?, loc)),
            "get_local_or_uninit" => I::Hhbc(H::CUGetL(self.lid(tok)?, loc)),
            "get_memo_key" => I::Hhbc(H::GetMemoKeyL(self.lid(tok)?, loc)),
            "get_static" => parse_instr!(tok, I::Hhbc(H::CGetS([p0, p1], p2, loc)), <p1:self.vid> "->" <p0:self.vid> <p2:parse_readonly>),
            "has_reified_parent" => I::Hhbc(H::HasReifiedParent(self.vid(tok)?, loc)),
            "idx" => parse_instr!(tok, I::Hhbc(H::Idx([p0, p1, p2], loc)), <p0:self.vid> "[" <p1:self.vid> "]" "or" <p2:self.vid>),
            "incdec_local" => self.parse_incdec_local(tok, loc)?,
            "incdec_static_prop" => self.parse_incdec_static_prop(tok, loc)?,
            "incdecm" => self.parse_member_op(tok, mnemonic, loc)?,
            "include" => I::Hhbc(H::IncludeEval(IncludeEval { kind: IncludeKind::Include, vid: self.vid(tok)?, loc })),
            "include_once" => I::Hhbc(H::IncludeEval(IncludeEval { kind: IncludeKind::IncludeOnce, vid: self.vid(tok)?, loc })),
            "init_prop" => parse_instr!(tok, I::Hhbc(H::InitProp(p0, p1, p2, loc)), <p1:parse_prop_id> "," <p0:self.vid> "," <p2:parse_init_prop_op>),
            "instance_of_d" => parse_instr!(tok, I::Hhbc(H::InstanceOfD(p0, p1, loc)), <p0:self.vid> "," <p1:parse_class_id>),
            "is_late_bound_cls" => I::Hhbc(H::IsLateBoundCls(self.vid(tok)?, loc)),
            "is_type_c" => parse_instr!(tok, I::Hhbc(H::IsTypeC(p0, p1, loc)), <p0:self.vid> "," <p1:parse_is_type_op>),
            "is_type_l" => parse_instr!(tok, I::Hhbc(H::IsTypeL(p0, p1, loc)), <p0:self.lid> "," <p1:parse_is_type_op>),
            "is_type_struct_c" => parse_instr!(tok, I::Hhbc(H::IsTypeStructC([p0, p1], p2, p3, loc)), <p0:self.vid> <p2:parse_type_struct_resolve_op> <p3:parse_type_struct_enforce_kind> <p1:self.vid>),
            "isset_g" => I::Hhbc(H::IssetG(self.vid(tok)?, loc)),
            "isset_l" => I::Hhbc(H::IssetL(self.lid(tok)?, loc)),
            "isset_s" => parse_instr!(tok, I::Hhbc(H::IssetS([p0, p1], loc)), <p0:self.vid> "::" <p1:self.vid>),
            "iterator" => self.parse_iterator(tok, loc)?,
            "jmp" => self.parse_jmp(tok, loc)?,
            "late_bound_cls" => I::Hhbc(H::LateBoundCls(loc)),
            "lazy_class_from_class" => I::Hhbc(H::LazyClassFromClass(self.vid(tok)?, loc)),
            "lock_obj" => I::Hhbc(H::LockObj(self.vid(tok)?, loc)),
            "memo_get" => self.parse_memo_get(tok, mnemonic, loc)?,
            "memo_get_eager" => self.parse_memo_get(tok, mnemonic, loc)?,
            "memo_set" => parse_instr!(tok, I::Hhbc(H::MemoSet(p0, p1.into(), loc)), "[" <p1:self.lid,*> "]" "," <p0:self.vid>),
            "memo_set_eager" => parse_instr!(tok, I::Hhbc(H::MemoSetEager(p0, p1.into(), loc)), "[" <p1:self.lid,*> "]" "," <p0:self.vid>),
            "mod" => I::Hhbc(H::Modulo(self.vid2(tok)?, loc)),
            "mul" => I::Hhbc(H::Mul(self.vid2(tok)?, loc)),
            "new_dict_array" => I::Hhbc(H::NewDictArray(parse_u32(tok)?, loc)),
            "new_keyset_array" => parse_instr!(tok, I::Hhbc(H::NewKeysetArray(p0.into(), loc)), "[" <p0:self.vid,*> "]"),
            "new_obj" => self.parse_new_obj(tok, loc)?,
            "new_pair" => I::Hhbc(H::NewPair(self.vid2(tok)?, loc)),
            "new_struct_dict" => self.parse_new_struct_dict(tok, loc)?,
            "new_vec" => parse_instr!(tok, I::Hhbc(H::NewVec(p0.into(), loc)), "[" <p0:self.vid,*> "]"),
            "not" => I::Hhbc(H::Not(self.vid(tok)?, loc)),
            "oo_decl_exists" => parse_instr!(tok, I::Hhbc(H::OODeclExists(p0, p1, loc)), <p0:self.vid2> <p1:parse_oo_decl_exists_op>),
            "parent" => I::Hhbc(H::ParentCls(loc)),
            "pow" => I::Hhbc(H::Pow(self.vid2(tok)?, loc)),
            "print" => I::Hhbc(H::Print(self.vid(tok)?, loc)),
            "querym" => self.parse_member_op(tok, mnemonic, loc)?,
            "raise_class_string_conversion_notice" => I::Hhbc(H::RaiseClassStringConversionNotice(loc)),
            "record_reified_generic" => I::Hhbc(H::RecordReifiedGeneric(self.vid(tok)?, loc)),
            "require" => I::Hhbc(H::IncludeEval(IncludeEval { kind: IncludeKind::Require, vid: self.vid(tok)?, loc })),
            "require_once" => I::Hhbc(H::IncludeEval(IncludeEval { kind: IncludeKind::RequireOnce, vid: self.vid(tok)?, loc })),
            "require_once_doc" => I::Hhbc(H::IncludeEval(IncludeEval { kind: IncludeKind::RequireOnceDoc, vid: self.vid(tok)?, loc })),
            "resolve_class" => I::Hhbc(H::ResolveClass(parse_class_id(tok)?, loc)),
            "resolve_cls_method" => parse_instr!(tok, I::Hhbc(H::ResolveClsMethod(p0, p1, loc)), <p0:self.vid> "::" <p1:parse_method_id>),
            "resolve_cls_method_d" => parse_instr!(tok, I::Hhbc(H::ResolveClsMethodD(p0, p1, loc)), <p0:parse_class_id> "::" <p1:parse_method_id>),
            "resolve_cls_method_s" => parse_instr!(tok, I::Hhbc(H::ResolveClsMethodS(p0, p1, loc)), <p0:parse_special_cls_ref> "::" <p1:parse_method_id>),
            "resolve_func" => I::Hhbc(H::ResolveFunc(parse_func_id(tok)?, loc)),
            "resolve_meth_caller" => I::Hhbc(H::ResolveMethCaller(parse_func_id(tok)?, loc)),
            "resolve_r_cls_method" => parse_instr!(tok, I::Hhbc(H::ResolveRClsMethod([p0, p1], p2, loc)), <p0:self.vid> "::" <p2:parse_method_id> "," <p1:self.vid>),
            "resolve_r_cls_method_d" => parse_instr!(tok, I::Hhbc(Hhbc::ResolveRClsMethodD(p0, p1, p2, loc)), <p1:parse_class_id> "::" <p2:parse_method_id> "," <p0:self.vid>),
            "resolve_r_cls_method_s" => parse_instr!(tok, I::Hhbc(H::ResolveRClsMethodS(p0, p1, p2, loc)), <p1:parse_special_cls_ref> "::" <p2:parse_method_id> "," <p0:self.vid>),
            "resolve_r_func" => parse_instr!(tok, I::Hhbc(H::ResolveRFunc(p0, p1, loc)), <p1:parse_func_id> "," <p0:self.vid>),
            "ret" => self.parse_ret(tok, loc)?,
            "ret_c_suspended" => I::Terminator(T::RetCSuspended(self.vid(tok)?, loc)),
            "select" => parse_instr!(tok, I::Special(Special::Select(p0, p1)), <p1:parse_u32> "from" <p0:self.vid>),
            "self" => I::Hhbc(H::SelfCls(loc)),
            "set_global" => I::Hhbc(H::SetG(self.vid2(tok)?, loc)),
            "set_implicit_context_by_value" => I::Hhbc(H::SetImplicitContextByValue(self.vid(tok)?, loc)),
            "set_local" => parse_instr!(tok, I::Hhbc(H::SetL(p0, p1, loc)), <p1:self.lid> "," <p0:self.vid>),
            "set_op_local" => parse_instr!(tok, I::Hhbc(H::SetOpL(p0, p1, p2, loc)), <p1:self.lid> <p2:parse_set_op_op> <p0:self.vid>),
            "set_op_global" => parse_instr!(tok, I::Hhbc(H::SetOpG([p0, p1], p2, loc)), <p0:self.vid> <p2:parse_set_op_op> <p1:self.vid>),
            "set_op_static_property" => parse_instr!(tok, I::Hhbc(H::SetOpS(p0, p1, loc)), <p0:self.vid3> "," <p1:parse_set_op_op>),
            "set_s" => parse_instr!(tok, I::Hhbc(H::SetS([p0, p1, p2], p3, loc)), <p1:self.vid> "->" <p0:self.vid> <p3:parse_readonly> "=" <p2:self.vid>),
            "setm" => self.parse_member_op(tok, mnemonic, loc)?,
            "setopm" => self.parse_member_op(tok, mnemonic, loc)?,
            "setrangem" => self.parse_member_op(tok, mnemonic, loc)?,
            "shl" => I::Hhbc(H::Shl(self.vid2(tok)?, loc)),
            "shr" => I::Hhbc(H::Shr(self.vid2(tok)?, loc)),
            "silence" => parse_instr!(tok, I::Hhbc(Hhbc::Silence(p0, p1, loc)), <p0:self.lid> "," <p1:parse_silence_op>),
            "sswitch" => self.parse_sswitch(tok, loc)?,
            "switch" => self.parse_switch(tok, loc)?,
            "sub" => I::Hhbc(H::Sub(self.vid2(tok)?, loc)),
            "this" => I::Hhbc(H::This(loc)),
            "throw" => I::Terminator(T::Throw(self.vid(tok)?, loc)),
            "throw_as_type_struct_exception" => parse_instr!(tok, I::Terminator(T::ThrowAsTypeStructException([p0, p1], p2, loc)), <p0:self.vid>"," <p1:self.vid>"," <p2:parse_as_type_struct_exception_kind>),
            "throw_nonexhaustive_switch" => I::Hhbc(H::ThrowNonExhaustiveSwitch(loc)),
            "unreachable" => I::Terminator(T::Unreachable),
            "unset" => self.parse_unset(tok, loc)?,
            "unsetm" => self.parse_member_op(tok, mnemonic, loc)?,
            "verify_implicit_context_state" => I::Hhbc(H::VerifyImplicitContextState(loc)),
            "verify_out_type" => parse_instr!(tok, I::Hhbc(Hhbc::VerifyOutType(p0, p1, loc)), <p0:self.vid> "," <p1:self.lid>),
            "verify_param_type" => parse_instr!(tok, I::Hhbc(Hhbc::VerifyParamType(p0, p1, loc)), <p0:self.vid> "," <p1:self.lid>),
            "verify_param_type_ts" => parse_instr!(tok, I::Hhbc(Hhbc::VerifyParamTypeTS(p0, p1, loc)), <p0:self.vid> "," <p1:self.lid>),
            "verify_ret_type_c" => I::Hhbc(H::VerifyRetTypeC(self.vid(tok)?, loc)),
            "verify_ret_type_ts" => I::Hhbc(H::VerifyRetTypeTS(self.vid2(tok)?, loc)),
            "wh_result" => I::Hhbc(H::WHResult(self.vid(tok)?, loc)),
            "yield" => self.parse_yield(tok, loc)?,
            _ => {
                return Err(lead.bail(format!(
                    "Unknown instruction '{mnemonic}' parsing function"
                )));
            }
        };

        self.blocks
            .last_mut()
            .unwrap()
            .instrs
            .push(InstrInfo { iid, instr });
        Ok(())
    }

    fn parse_label(&mut self, tokenizer: &mut Tokenizer<'_>, lead: &Token) -> Result<()> {
        let bid = convert_bid(lead)?;
        self.blocks.push(BlockInfo {
            bid,
            instrs: Default::default(),
        });

        if self.builder.func.blocks.len() <= bid.as_usize() {
            self.builder
                .func
                .blocks
                .resize_with(bid.as_usize() + 1, Block::default);
        }

        self.builder.start_block(bid);

        if tokenizer.next_is_identifier("(")? {
            // b2(%1, %2, %3):
            parse!(tokenizer, <params:self.iid,*> ")");

            debug_assert_eq!(self.blocks[0].bid, BlockId::NONE);
            self.builder.cur_block_mut().params.extend(&params);

            self.blocks[0]
                .instrs
                .extend(params.into_iter().map(|iid| InstrInfo {
                    iid,
                    instr: Instr::param(),
                }));
        }

        tokenizer.expect_identifier(":")?;

        Ok(())
    }

    fn parse_src_loc(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<LocId> {
        let old = if self.cur_loc == LocId::NONE {
            None
        } else {
            self.builder.func.get_loc(self.cur_loc)
        };
        let src_loc = parse_src_loc(tokenizer, old)?;

        self.cur_loc = self.builder.add_loc(src_loc);
        Ok(self.cur_loc)
    }

    fn parse_srcloc_decl(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        self.parse_src_loc(tokenizer)?;
        Ok(())
    }

    fn parse_try_id(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let id = ExFrameId::from_usize(parse_usize(tokenizer)?);
        self.builder.cur_block_mut().tcid = TryCatchId::Try(id);
        Ok(())
    }
}
