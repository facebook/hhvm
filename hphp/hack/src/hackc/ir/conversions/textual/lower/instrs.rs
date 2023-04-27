// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::func_builder::TransformInstr;
use ir::func_builder::TransformState;
use ir::instr::CallDetail;
use ir::instr::CmpOp;
use ir::instr::HasLoc;
use ir::instr::HasLocals;
use ir::instr::HasOperands;
use ir::instr::Hhbc;
use ir::instr::IteratorArgs;
use ir::instr::MemoGet;
use ir::instr::Predicate;
use ir::instr::Special;
use ir::instr::Terminator;
use ir::instr::Textual;
use ir::BareThisOp;
use ir::Call;
use ir::Constant;
use ir::FCallArgsFlags;
use ir::Func;
use ir::FuncBuilder;
use ir::FuncBuilderEx as _;
use ir::GlobalId;
use ir::InitPropOp;
use ir::Instr;
use ir::InstrId;
use ir::IsTypeOp;
use ir::LocId;
use ir::LocalId;
use ir::MemberOpBuilder;
use ir::MethodId;
use ir::ObjMethodOp;
use ir::PropId;
use ir::ReadonlyOp;
use ir::SpecialClsRef;
use ir::TypeStructResolveOp;
use ir::UnitBytesId;
use ir::ValueId;
use itertools::Itertools;

use super::func_builder::FuncBuilderEx as _;
use crate::class::IsStatic;
use crate::func::lookup_constant_string;
use crate::func::FuncInfo;
use crate::hack;

/// Lower individual Instrs in the Func to simpler forms.
pub(crate) fn lower_instrs(builder: &mut FuncBuilder<'_>, func_info: &FuncInfo<'_>) {
    let mut lowerer = LowerInstrs {
        changed: false,
        func_info,
    };

    let mut bid = Func::ENTRY_BID;
    while bid.0 < builder.func.blocks.len() as u32 {
        lowerer.changed = true;
        while lowerer.changed {
            // The lowered instructions may have emitted stuff that needs to be
            // lowered again. This could be optimized by only setting `changed`
            // when we think we need to loop.
            lowerer.changed = false;
            builder.rewrite_block(bid, &mut lowerer);
        }
        bid.0 += 1;
    }
}

struct LowerInstrs<'a> {
    changed: bool,
    func_info: &'a FuncInfo<'a>,
}

impl LowerInstrs<'_> {
    fn c_get_g(&self, builder: &mut FuncBuilder<'_>, vid: ValueId, _loc: LocId) -> Option<Instr> {
        // A lot of times the name for the global comes from a static
        // string name - see if we can dig it up and turn this into a
        // Textual::LoadGlobal.
        vid.constant()
            .and_then(|cid| match builder.func.constant(cid) {
                Constant::String(s) => {
                    let id = ir::GlobalId::new(*s);
                    Some(Instr::Special(Special::Textual(Textual::LoadGlobal {
                        id,
                        is_const: false,
                    })))
                }
                _ => None,
            })
    }

    fn handle_hhbc_with_builtin(&self, hhbc: &Hhbc) -> Option<hack::Hhbc> {
        let builtin = match hhbc {
            Hhbc::Add(..) => hack::Hhbc::Add,
            Hhbc::AddElemC(..) => hack::Hhbc::AddElemC,
            Hhbc::AddNewElemC(..) => hack::Hhbc::AddNewElemC,
            Hhbc::AwaitAll(..) => hack::Hhbc::AwaitAll,
            Hhbc::CastKeyset(..) => hack::Hhbc::CastKeyset,
            Hhbc::CastVec(..) => hack::Hhbc::CastVec,
            Hhbc::ClassGetC(..) => hack::Hhbc::ClassGetC,
            Hhbc::ClassHasReifiedGenerics(..) => hack::Hhbc::ClassHasReifiedGenerics,
            Hhbc::CheckClsRGSoft(..) => hack::Hhbc::CheckClsRGSoft,
            Hhbc::CmpOp(_, CmpOp::Eq, _) => hack::Hhbc::CmpEq,
            Hhbc::CmpOp(_, CmpOp::Gt, _) => hack::Hhbc::CmpGt,
            Hhbc::CmpOp(_, CmpOp::Gte, _) => hack::Hhbc::CmpGte,
            Hhbc::CmpOp(_, CmpOp::Lt, _) => hack::Hhbc::CmpLt,
            Hhbc::CmpOp(_, CmpOp::Lte, _) => hack::Hhbc::CmpLte,
            Hhbc::CmpOp(_, CmpOp::NSame, _) => hack::Hhbc::CmpNSame,
            Hhbc::CmpOp(_, CmpOp::Neq, _) => hack::Hhbc::CmpNeq,
            Hhbc::CmpOp(_, CmpOp::Same, _) => hack::Hhbc::CmpSame,
            Hhbc::CombineAndResolveTypeStruct(..) => hack::Hhbc::CombineAndResolveTypeStruct,
            Hhbc::Concat(..) => hack::Hhbc::Concat,
            Hhbc::ConcatN(..) => hack::Hhbc::ConcatN,
            Hhbc::Div(..) => hack::Hhbc::Div,
            Hhbc::GetClsRGProp(..) => hack::Hhbc::GetClsRGProp,
            Hhbc::GetMemoKeyL(..) => hack::Hhbc::GetMemoKeyL,
            Hhbc::HasReifiedParent(..) => hack::Hhbc::HasReifiedParent,
            Hhbc::IsLateBoundCls(..) => hack::Hhbc::IsLateBoundCls,
            Hhbc::IsTypeC(_, IsTypeOp::ArrLike, _) => hack::Hhbc::IsTypeArrLike,
            Hhbc::IsTypeC(_, IsTypeOp::Bool, _) => hack::Hhbc::IsTypeBool,
            Hhbc::IsTypeC(_, IsTypeOp::Class, _) => hack::Hhbc::IsTypeClass,
            Hhbc::IsTypeC(_, IsTypeOp::ClsMeth, _) => hack::Hhbc::IsTypeClsMeth,
            Hhbc::IsTypeC(_, IsTypeOp::Dbl, _) => hack::Hhbc::IsTypeDbl,
            Hhbc::IsTypeC(_, IsTypeOp::Dict, _) => hack::Hhbc::IsTypeDict,
            Hhbc::IsTypeC(_, IsTypeOp::Func, _) => hack::Hhbc::IsTypeFunc,
            Hhbc::IsTypeC(_, IsTypeOp::Int, _) => hack::Hhbc::IsTypeInt,
            Hhbc::IsTypeC(_, IsTypeOp::Keyset, _) => hack::Hhbc::IsTypeKeyset,
            Hhbc::IsTypeC(_, IsTypeOp::LegacyArrLike, _) => hack::Hhbc::IsTypeLegacyArrLike,
            Hhbc::IsTypeC(_, IsTypeOp::Null, _) => hack::Hhbc::IsTypeNull,
            Hhbc::IsTypeC(_, IsTypeOp::Obj, _) => hack::Hhbc::IsTypeObj,
            Hhbc::IsTypeC(_, IsTypeOp::Res, _) => hack::Hhbc::IsTypeRes,
            Hhbc::IsTypeC(_, IsTypeOp::Scalar, _) => hack::Hhbc::IsTypeScalar,
            Hhbc::IsTypeC(_, IsTypeOp::Str, _) => hack::Hhbc::IsTypeStr,
            Hhbc::IsTypeC(_, IsTypeOp::Vec, _) => hack::Hhbc::IsTypeVec,
            Hhbc::Modulo(..) => hack::Hhbc::Modulo,
            Hhbc::Mul(..) => hack::Hhbc::Mul,
            Hhbc::NewDictArray(..) => hack::Hhbc::NewDictArray,
            Hhbc::NewKeysetArray(..) => hack::Hhbc::NewKeysetArray,
            Hhbc::NewVec(..) => hack::Hhbc::NewVec,
            Hhbc::Not(..) => hack::Hhbc::Not,
            Hhbc::Print(..) => hack::Hhbc::Print,
            Hhbc::RecordReifiedGeneric(..) => hack::Hhbc::RecordReifiedGeneric,
            Hhbc::Sub(..) => hack::Hhbc::Sub,
            Hhbc::ThrowNonExhaustiveSwitch(..) => hack::Hhbc::ThrowNonExhaustiveSwitch,
            Hhbc::WHResult(..) => hack::Hhbc::WHResult,
            _ => return None,
        };
        Some(builtin)
    }

    fn handle_terminator_with_builtin(&self, term: &Terminator) -> Option<hack::Hhbc> {
        let builtin = match term {
            Terminator::Exit(..) => hack::Hhbc::Exit,
            Terminator::Throw(..) => hack::Hhbc::Throw,
            Terminator::ThrowAsTypeStructException(..) => hack::Hhbc::ThrowAsTypeStructException,
            _ => return None,
        };
        Some(builtin)
    }

    fn handle_with_builtin(&self, builder: &mut FuncBuilder<'_>, instr: &Instr) -> Option<Instr> {
        let loc = instr.loc_id();
        match instr {
            Instr::Hhbc(hhbc) => {
                let hhbc = self.handle_hhbc_with_builtin(hhbc)?;
                Some(
                    match (instr.locals().is_empty(), instr.operands().is_empty()) {
                        (true, true) => builder.hhbc_builtin(hhbc, &[], loc),
                        (true, false) => builder.hhbc_builtin(hhbc, instr.operands(), loc),
                        (false, true) => {
                            let ops = Self::get_locals(builder, instr.locals(), loc);
                            builder.hhbc_builtin(hhbc, &ops, loc)
                        }
                        (false, false) => panic!(
                            "Unable to handle mixed instruction (locals and operands) as a built-in: {hhbc:?}"
                        ),
                    },
                )
            }
            Instr::Terminator(term) => {
                let hhbc = self.handle_terminator_with_builtin(term)?;
                builder.emit_hhbc_builtin(hhbc, instr.operands(), loc);
                Some(Instr::unreachable())
            }
            _ => None,
        }
    }

    fn check_prop(&self, builder: &mut FuncBuilder<'_>, _pid: PropId, _loc: LocId) -> Instr {
        // CheckProp checks to see if the prop has already been initialized - we'll just always say "no".
        Instr::copy(builder.emit_constant(Constant::Bool(false)))
    }

    fn init_prop(
        &self,
        builder: &mut FuncBuilder<'_>,
        vid: ValueId,
        pid: PropId,
        _op: InitPropOp,
        loc: LocId,
    ) -> Instr {
        // $this->pid = vid
        MemberOpBuilder::base_h(loc).emit_set_m_pt(builder, pid, vid);
        Instr::tombstone()
    }

    fn iter_init(
        &self,
        builder: &mut FuncBuilder<'_>,
        args: IteratorArgs,
        container: ValueId,
    ) -> Instr {
        // iterator ^0 init from %0 jmp to b2 else b1 with $index
        // ->
        // %n = hack::iter_init(&iter0, /* key */ null, &$index, %0)
        // if %n jmp b1 else b2

        let loc = args.loc;
        let iter_lid = iter_var_name(args.iter_id, &builder.strings);

        let iter_var = builder.emit(Textual::deref(iter_lid));

        let value_var = builder.emit(Textual::deref(args.value_lid()));

        let key_var = if let Some(key_lid) = args.key_lid() {
            builder.emit(Textual::deref(key_lid))
        } else {
            builder.emit_constant(Constant::Null)
        };

        let pred = builder.emit_hhbc_builtin(
            hack::Hhbc::IterInit,
            &[iter_var, key_var, value_var, container],
            loc,
        );

        Instr::jmp_op(
            pred,
            Predicate::NonZero,
            args.next_bid(),
            args.done_bid(),
            loc,
        )
    }

    fn iter_next(&self, builder: &mut FuncBuilder<'_>, args: IteratorArgs) -> Instr {
        // iterator ^0 next jmp to b2 else b1 with $index
        // ->
        // %n = hack::iter_next(&iter0, /* key */ null, &$index)
        // if %n jmp b1 else b2

        let loc = args.loc;
        let iter_lid = iter_var_name(args.iter_id, &builder.strings);

        let value_var = builder.emit(Textual::deref(args.value_lid()));

        let key_var = if let Some(key_lid) = args.key_lid() {
            builder.emit(Textual::deref(key_lid))
        } else {
            builder.emit_constant(Constant::Null)
        };

        let iter_var = builder.emit(Instr::Hhbc(Hhbc::CGetL(iter_lid, loc)));
        let pred =
            builder.emit_hhbc_builtin(hack::Hhbc::IterNext, &[iter_var, key_var, value_var], loc);

        Instr::jmp_op(
            pred,
            Predicate::NonZero,
            args.next_bid(),
            args.done_bid(),
            loc,
        )
    }

    fn get_locals<'s>(
        builder: &mut FuncBuilder<'s>,
        locals: &[LocalId],
        loc: LocId,
    ) -> Vec<ValueId> {
        locals
            .iter()
            .map(|lid| builder.emit(Instr::Hhbc(Hhbc::CGetL(*lid, loc))))
            .collect()
    }

    fn compute_memo_ops(
        &self,
        builder: &mut FuncBuilder<'_>,
        locals: &[LocalId],
        loc: LocId,
    ) -> Vec<ValueId> {
        let name = match *self.func_info {
            FuncInfo::Method(ref mi) => {
                crate::mangle::FunctionName::method(mi.class.name, mi.is_static, mi.name)
            }
            FuncInfo::Function(ref fi) => crate::mangle::FunctionName::Function(fi.name),
        };

        let mut ops = Vec::new();

        let name = format!("{}", name.display(&builder.strings));
        let name = GlobalId::new(builder.strings.intern_str(format!(
            "memocache::{}",
            crate::util::escaped_ident(name.into())
        )));
        // NOTE: This is called 'LocalId' but actually represents a GlobalId
        // (because we don't have a DerefGlobal instr - but it would work the
        // same with just a different type).
        let global = builder.emit(Textual::deref(LocalId::Named(name.id)));
        ops.push(global);

        match *self.func_info {
            FuncInfo::Method(_) => {
                let this = builder.strings.intern_str("$this");
                let lid = LocalId::Named(this);
                ops.push(builder.emit(Instr::Hhbc(Hhbc::CGetL(lid, loc))));
            }
            FuncInfo::Function(_) => {}
        };

        ops.extend(Self::get_locals(builder, locals, loc).into_iter());
        ops
    }

    fn memo_get(&self, builder: &mut FuncBuilder<'_>, memo_get: MemoGet) -> Instr {
        // memo_get([$a, $b], b1, b2)
        // ->
        // global NAME
        // if memo_isset($this, $a, $b) {
        //   tmp = memo_get($this, $a, $b)
        //   goto b1(tmp);
        // } else {
        //   goto b2;
        // }
        let loc = memo_get.loc;

        let ops = self.compute_memo_ops(builder, &memo_get.locals, loc);

        let pred = builder.emit_hack_builtin(hack::Builtin::MemoIsset, &ops, loc);
        builder.emit_if_then_else(
            pred,
            loc,
            |builder| {
                let value = builder.emit_hack_builtin(hack::Builtin::MemoGet, &ops, loc);
                Instr::jmp_args(memo_get.edges[0], &[value], loc)
            },
            |_| Instr::jmp(memo_get.edges[1], loc),
        );

        Instr::tombstone()
    }

    fn memo_set(
        &self,
        builder: &mut FuncBuilder<'_>,
        vid: ValueId,
        locals: &[LocalId],
        loc: LocId,
    ) -> Instr {
        let mut ops = self.compute_memo_ops(builder, locals, loc);
        ops.push(vid);
        builder.hhbc_builtin(hack::Hhbc::MemoSet, &ops, loc)
    }

    fn verify_out_type(
        &self,
        builder: &mut FuncBuilder<'_>,
        obj: ValueId,
        lid: LocalId,
        loc: LocId,
    ) -> Instr {
        let param = builder
            .func
            .get_param_by_lid(lid)
            .expect("Unknown parameter in verify_out_type()");
        let param_type = param.ty.enforced.clone();
        let pred = builder.emit_is(obj, &param_type, loc);
        builder.emit_hack_builtin(hack::Builtin::VerifyTypePred, &[obj, pred], loc);
        Instr::copy(obj)
    }

    fn verify_param_type_ts(
        &self,
        builder: &mut FuncBuilder<'_>,
        lid: LocalId,
        ts: ValueId,
        loc: LocId,
    ) -> Instr {
        let obj = builder.emit(Instr::Hhbc(Hhbc::CGetL(lid, loc)));
        let builtin = hack::Hhbc::VerifyParamTypeTS;
        builder.emit_hhbc_builtin(builtin, &[obj, ts], loc);
        Instr::tombstone()
    }

    fn verify_ret_type_c(&self, builder: &mut FuncBuilder<'_>, obj: ValueId, loc: LocId) -> Instr {
        let return_type = builder.func.return_type.enforced.clone();
        let pred = builder.emit_is(obj, &return_type, loc);
        builder.emit_hack_builtin(hack::Builtin::VerifyTypePred, &[obj, pred], loc);
        Instr::copy(obj)
    }

    fn verify_ret_type_ts(
        &self,
        builder: &mut FuncBuilder<'_>,
        obj: ValueId,
        ts: ValueId,
        loc: LocId,
    ) -> Instr {
        let builtin = hack::Hhbc::VerifyParamTypeTS;
        builder.emit_hhbc_builtin(builtin, &[obj, ts], loc);
        Instr::copy(obj)
    }

    fn emit_special_cls_ref(
        &mut self,
        builder: &mut FuncBuilder<'_>,
        clsref: SpecialClsRef,
        loc: LocId,
    ) -> ValueId {
        match clsref {
            SpecialClsRef::SelfCls => builder.emit(Instr::Hhbc(Hhbc::SelfCls(loc))),
            SpecialClsRef::LateBoundCls => builder.emit(Instr::Hhbc(Hhbc::LateBoundCls(loc))),
            SpecialClsRef::ParentCls => builder.emit(Instr::Hhbc(Hhbc::ParentCls(loc))),
            _ => unreachable!(),
        }
    }

    fn set_s(
        &mut self,
        builder: &mut FuncBuilder<'_>,
        field: ValueId,
        class: ValueId,
        vid: ValueId,
        _readonly: ReadonlyOp,
        loc: LocId,
    ) -> Option<Instr> {
        // Some magic - if the class points to a ClassGetC with a
        // constant parameter and the field is a constant then replace
        // this with a Textual::SetPropD.
        if let Some(propname) = lookup_constant_string(&builder.func, field) {
            if let Some(class) = class.instr() {
                let class_instr = builder.func.instr(class);
                // The ClassGetC will have already been converted to a
                // textual builtin.
                if let Instr::Special(Special::Textual(Textual::HackBuiltin {
                    target,
                    values: box [cid],
                    loc: _,
                })) = class_instr
                {
                    if target == hack::Hhbc::ClassGetC.as_str() {
                        if let Some(classname) = lookup_constant_string(&builder.func, *cid) {
                            let cid = builder.emit_constant(Constant::String(classname));
                            let pid = builder.emit_constant(Constant::String(propname));
                            return Some(builder.hack_builtin(
                                hack::Builtin::SetStaticProp,
                                &[cid, pid, vid],
                                loc,
                            ));
                        }
                    }
                }
            }
        }

        None
    }
}

impl TransformInstr for LowerInstrs<'_> {
    fn apply(
        &mut self,
        iid: InstrId,
        instr: Instr,
        builder: &mut FuncBuilder<'_>,
        state: &mut TransformState,
    ) -> Instr {
        use hack::Builtin;

        if let Some(instr) = self.handle_with_builtin(builder, &instr) {
            self.changed = true;
            return instr;
        }

        let instr = match instr {
            // NOTE: Be careful trying to lower most Instr::Call - the details
            // matter and it's hard to rewrite them to preserve those details
            // properly.
            Instr::Call(
                box mut call @ Call {
                    detail: CallDetail::FCallCtor,
                    ..
                },
            ) => {
                let flavor = ObjMethodOp::NullThrows;
                let method = MethodId::constructor(&builder.strings);
                call.detail = CallDetail::FCallObjMethodD { flavor, method };
                Instr::Call(Box::new(call))
            }
            Instr::Call(
                box call @ Call {
                    detail:
                        CallDetail::FCallObjMethod {
                            flavor: ObjMethodOp::NullSafe,
                            ..
                        }
                        | CallDetail::FCallObjMethodD {
                            flavor: ObjMethodOp::NullSafe,
                            ..
                        },
                    ..
                },
            ) => rewrite_nullsafe_call(iid, builder, call, state),
            Instr::Hhbc(Hhbc::BareThis(_op, loc)) => {
                let this = builder.strings.intern_str("$this");
                let lid = LocalId::Named(this);
                Instr::Hhbc(Hhbc::CGetL(lid, loc))
            }
            Instr::Hhbc(Hhbc::SetS([field, class, vid], readonly, loc)) => {
                if let Some(replace) = self.set_s(builder, field, class, vid, readonly, loc) {
                    replace
                } else {
                    return instr;
                }
            }
            Instr::Hhbc(Hhbc::CheckProp(pid, loc)) => self.check_prop(builder, pid, loc),
            Instr::Hhbc(Hhbc::CheckThis(loc)) => {
                let builtin = hack::Hhbc::CheckThis;
                let op = BareThisOp::NoNotice;
                let this = builder.emit(Instr::Hhbc(Hhbc::BareThis(op, loc)));
                builder.hhbc_builtin(builtin, &[this], loc)
            }
            Instr::Hhbc(Hhbc::CGetG(name, loc)) => {
                if let Some(instr) = self.c_get_g(builder, name, loc) {
                    instr
                } else {
                    return Instr::Hhbc(Hhbc::CGetG(name, loc));
                }
            }
            Instr::Hhbc(Hhbc::ClsCns(cls, const_id, loc)) => {
                let builtin = hack::Hhbc::ClsCns;
                let const_id = builder.emit_constant(Constant::String(const_id.id));
                builder.hhbc_builtin(builtin, &[cls, const_id], loc)
            }
            Instr::Hhbc(Hhbc::ColFromArray(vid, col_type, loc)) => {
                use ir::CollectionType;
                let builtin = match col_type {
                    CollectionType::ImmMap => hack::Hhbc::ColFromArrayImmMap,
                    CollectionType::ImmSet => hack::Hhbc::ColFromArrayImmSet,
                    CollectionType::ImmVector => hack::Hhbc::ColFromArrayImmVector,
                    CollectionType::Map => hack::Hhbc::ColFromArrayMap,
                    CollectionType::Pair => hack::Hhbc::ColFromArrayPair,
                    CollectionType::Set => hack::Hhbc::ColFromArraySet,
                    CollectionType::Vector => hack::Hhbc::ColFromArrayVector,
                    other => panic!("Unknown collection type: {:?}", other),
                };

                builder.hhbc_builtin(builtin, &[vid], loc)
            }
            Instr::Hhbc(Hhbc::InitProp(vid, pid, op, loc)) => {
                self.init_prop(builder, vid, pid, op, loc)
            }
            Instr::Hhbc(Hhbc::InstanceOfD(vid, cid, loc)) => {
                let cid = builder.emit_constant(Constant::String(cid.id));
                builder.hack_builtin(Builtin::IsType, &[vid, cid], loc)
            }
            Instr::Hhbc(Hhbc::IsTypeStructC([obj, ts], op, loc)) => {
                let builtin = hack::Hhbc::IsTypeStructC;
                let op = match op {
                    TypeStructResolveOp::DontResolve => 0,
                    TypeStructResolveOp::Resolve => 1,
                    _ => unreachable!(),
                };
                let op = builder.emit_constant(Constant::Int(op));
                builder.hhbc_builtin(builtin, &[obj, ts, op], loc)
            }
            Instr::Hhbc(Hhbc::LateBoundCls(loc)) => match *self.func_info {
                FuncInfo::Method(ref mi) => match mi.is_static {
                    IsStatic::Static => {
                        let this = builder.emit(Instr::Hhbc(Hhbc::This(loc)));
                        builder.hack_builtin(Builtin::GetClass, &[this], loc)
                    }
                    IsStatic::NonStatic => {
                        let this = builder.emit(Instr::Hhbc(Hhbc::This(loc)));
                        builder.hack_builtin(Builtin::GetStaticClass, &[this], loc)
                    }
                },
                FuncInfo::Function(_) => unreachable!(),
            },
            Instr::Hhbc(Hhbc::LazyClass(clsid, _)) => {
                // Treat a lazy class as a simple string.
                let c = builder.emit_constant(Constant::String(clsid.id));
                Instr::copy(c)
            }
            Instr::Hhbc(Hhbc::LockObj(obj, loc)) => {
                builder.emit_hhbc_builtin(hack::Hhbc::LockObj, &[obj], loc);
                Instr::copy(obj)
            }
            Instr::Hhbc(Hhbc::MemoSet(vid, locals, loc)) => {
                self.memo_set(builder, vid, &locals, loc)
            }
            Instr::Hhbc(Hhbc::NewStructDict(names, values, loc)) => {
                let args = names
                    .iter()
                    .zip(values.iter().copied())
                    .flat_map(|(name, value)| {
                        [builder.emit_constant(Constant::String(*name)), value]
                    })
                    .collect_vec();
                builder.hack_builtin(Builtin::NewDict, &args, loc)
            }
            Instr::Hhbc(Hhbc::NewObj(cls, loc)) => {
                let method = MethodId::factory(&builder.strings);
                let operands = vec![cls].into_boxed_slice();
                let context = UnitBytesId::NONE;
                let flavor = ObjMethodOp::NullThrows;
                let detail = CallDetail::FCallObjMethodD { flavor, method };
                let flags = FCallArgsFlags::default();
                Instr::Call(Box::new(Call {
                    operands,
                    context,
                    detail,
                    flags,
                    num_rets: 1,
                    inouts: None,
                    readonly: None,
                    loc,
                }))
            }
            Instr::Hhbc(Hhbc::NewObjS(clsref, loc)) => {
                let cls = self.emit_special_cls_ref(builder, clsref, loc);
                Instr::Hhbc(Hhbc::NewObj(cls, loc))
            }
            Instr::Hhbc(Hhbc::VerifyOutType(vid, lid, loc)) => {
                self.verify_out_type(builder, vid, lid, loc)
            }
            Instr::Hhbc(Hhbc::VerifyParamTypeTS(ts, lid, loc)) => {
                self.verify_param_type_ts(builder, lid, ts, loc)
            }
            Instr::Hhbc(Hhbc::VerifyRetTypeC(vid, loc)) => {
                self.verify_ret_type_c(builder, vid, loc)
            }
            Instr::Hhbc(Hhbc::VerifyRetTypeTS([obj, ts], loc)) => {
                self.verify_ret_type_ts(builder, obj, ts, loc)
            }
            Instr::Hhbc(Hhbc::VerifyImplicitContextState(_)) => {
                // no-op
                Instr::tombstone()
            }
            Instr::Hhbc(Hhbc::IterFree(id, loc)) => {
                let lid = iter_var_name(id, &builder.strings);
                let value = builder.emit(Instr::Hhbc(Hhbc::CGetL(lid, loc)));
                builder.hhbc_builtin(hack::Hhbc::IterFree, &[value], loc)
            }
            Instr::Terminator(Terminator::IterInit(args, value)) => {
                self.iter_init(builder, args, value)
            }
            Instr::Terminator(Terminator::IterNext(args)) => self.iter_next(builder, args),
            Instr::Terminator(Terminator::MemoGet(memo)) => self.memo_get(builder, memo),
            Instr::Terminator(Terminator::RetM(ops, loc)) => {
                // ret a, b;
                // =>
                // ret vec[a, b];
                let builtin = hack::Hhbc::NewVec;
                let vec = builder.emit_hhbc_builtin(builtin, &ops, loc);
                Instr::ret(vec, loc)
            }
            Instr::Terminator(Terminator::SSwitch {
                cond,
                cases,
                targets,
                loc,
            }) => {
                // if (StrEq(cond, case_0)) jmp targets[0];
                // else if (StrEq(cond, case_1)) jmp targets[1];
                // ...
                // else jmp targets.last();

                // Last case MUST be 'default'.
                let iter = cases.iter().take(cases.len() - 1).zip(targets.iter());

                for (case, target) in iter {
                    let string = builder.emit_constant(Constant::String(*case));
                    let pred = builder.emit_hhbc_builtin(hack::Hhbc::CmpSame, &[string, cond], loc);
                    let false_bid = builder.alloc_bid();
                    builder.emit(Instr::jmp_op(
                        pred,
                        Predicate::NonZero,
                        *target,
                        false_bid,
                        loc,
                    ));
                    builder.start_block(false_bid);
                }

                Instr::jmp(*targets.last().unwrap(), loc)
            }
            instr => {
                return instr;
            }
        };
        self.changed = true;
        instr
    }
}

fn rewrite_nullsafe_call(
    original_iid: InstrId,
    builder: &mut FuncBuilder<'_>,
    mut call: Call,
    state: &mut TransformState,
) -> Instr {
    // rewrite a call like `$a?->b()` into `$a ? $a->b() : null`
    call.detail = match call.detail {
        CallDetail::FCallObjMethod { .. } => CallDetail::FCallObjMethod {
            flavor: ObjMethodOp::NullThrows,
        },
        CallDetail::FCallObjMethodD { method, .. } => CallDetail::FCallObjMethodD {
            flavor: ObjMethodOp::NullThrows,
            method,
        },
        _ => unreachable!(),
    };

    // We need to be careful about multi-return!
    //   (ret, a, b) = obj?->call(inout a, inout b)
    // ->
    //   if (obj) {
    //     (ret, a, b)
    //   } else {
    //     (null, a, b)
    //   }

    let loc = call.loc;
    let obj = call.obj();
    let num_rets = call.num_rets;
    let pred = builder.emit_hack_builtin(hack::Builtin::Hhbc(hack::Hhbc::IsTypeNull), &[obj], loc);

    // Need to customize the if/then/else because of multi-return.
    let join_bid = builder.alloc_bid();
    let null_bid = builder.alloc_bid();
    let nonnull_bid = builder.alloc_bid();
    builder.emit(Instr::jmp_op(
        pred,
        Predicate::NonZero,
        null_bid,
        nonnull_bid,
        loc,
    ));

    // Null case - main value should be null. Inouts should be passed through.
    builder.start_block(null_bid);
    let mut args = Vec::with_capacity(num_rets as usize);
    args.push(builder.emit_constant(Constant::Null));
    if let Some(inouts) = call.inouts.as_ref() {
        for inout_idx in inouts.iter() {
            let op = call.operands[*inout_idx as usize];
            args.push(op);
        }
    }
    builder.emit(Instr::jmp_args(join_bid, &args, loc));

    // Nonnull case - make call and pass Select values.
    builder.start_block(nonnull_bid);
    args.clear();
    let iid = builder.emit(Instr::Call(Box::new(call)));
    if num_rets > 1 {
        for idx in 0..num_rets {
            args.push(builder.emit(Instr::Special(Special::Select(iid, idx))));
        }
    } else {
        args.push(iid);
    }
    builder.emit(Instr::jmp_args(join_bid, &args, loc));

    // Join
    builder.start_block(join_bid);
    args.clear();
    if num_rets > 1 {
        // we need to swap out the original Select statements.
        for idx in 0..num_rets {
            let param = builder.alloc_param();
            args.push(param);
            state.rewrite_select_idx(original_iid, idx as usize, param);
        }
        Instr::tombstone()
    } else {
        let param = builder.alloc_param();
        args.push(param);
        Instr::copy(args[0])
    }
}

fn iter_var_name(id: ir::IterId, strings: &ir::StringInterner) -> LocalId {
    let name = strings.intern_str(format!("iter{}", id.idx));
    LocalId::Named(name)
}
