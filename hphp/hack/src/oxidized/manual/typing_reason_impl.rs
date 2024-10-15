// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::pos::Pos;
use crate::typing_reason::*;

impl WitnessLocl {
    pub fn pos(&self) -> &Pos {
        match self {
            Self::Witness(pos)
            | Self::IdxVector(pos)
            | Self::Foreach(pos)
            | Self::Asyncforeach(pos)
            | Self::Arith(pos)
            | Self::ArithRet(pos)
            | Self::ArithRetInt(pos)
            | Self::ArithDynamic(pos)
            | Self::BitwiseDynamic(pos)
            | Self::IncdecDynamic(pos)
            | Self::Comp(pos)
            | Self::ConcatRet(pos)
            | Self::LogicRet(pos)
            | Self::Bitwise(pos)
            | Self::BitwiseRet(pos)
            | Self::NoReturn(pos)
            | Self::NoReturnAsync(pos)
            | Self::RetFunKind(pos, _)
            | Self::Throw(pos)
            | Self::Placeholder(pos)
            | Self::RetDiv(pos)
            | Self::YieldGen(pos)
            | Self::YieldAsyncgen(pos)
            | Self::YieldAsyncnull(pos)
            | Self::YieldSend(pos)
            | Self::UnknownClass(pos)
            | Self::VarParam(pos)
            | Self::UnpackParam(pos, _, _)
            | Self::NullsafeOp(pos)
            | Self::Predicated(pos, _)
            | Self::IsRefinement(pos)
            | Self::AsRefinement(pos)
            | Self::Equal(pos)
            | Self::Using(pos)
            | Self::DynamicProp(pos)
            | Self::DynamicCall(pos)
            | Self::DynamicConstruct(pos)
            | Self::IdxDict(pos)
            | Self::IdxSetElement(pos)
            | Self::UnsetField(pos, _)
            | Self::Regex(pos)
            | Self::TypeVariable(pos, _)
            | Self::TypeVariableGenerics(pos, _, _, _)
            | Self::TypeVariableError(pos, _)
            | Self::Shape(pos, _)
            | Self::ShapeLiteral(pos)
            | Self::Destructure(pos)
            | Self::KeyValueCollectionKey(pos)
            | Self::Splice(pos)
            | Self::EtBoolean(pos)
            | Self::ConcatOperand(pos)
            | Self::InterpOperand(pos)
            | Self::MissingClass(pos)
            | Self::CapturedLike(pos)
            | Self::UnsafeCast(pos)
            | Self::Pattern(pos)
            | Self::JoinPoint(pos) => pos,
        }
    }
}

impl WitnessDecl {
    pub fn pos(&self) -> &Pos {
        match self {
            Self::WitnessFromDecl(pos_or_decl)
            | Self::IdxVectorFromDecl(pos_or_decl)
            | Self::RetFunKindFromDecl(pos_or_decl, _)
            | Self::Hint(pos_or_decl)
            | Self::ClassClass(pos_or_decl, _)
            | Self::VarParamFromDecl(pos_or_decl)
            | Self::InoutParam(pos_or_decl)
            | Self::TconstNoCstr((pos_or_decl, _))
            | Self::VarrayOrDarrayKey(pos_or_decl)
            | Self::VecOrDictKey(pos_or_decl)
            | Self::MissingOptionalField(pos_or_decl, _)
            | Self::ImplicitUpperBound(pos_or_decl, _)
            | Self::GlobalTypeVariableGenerics(pos_or_decl, _, _)
            | Self::SolveFail(pos_or_decl)
            | Self::CstrOnGenerics(pos_or_decl, _)
            | Self::Enforceable(pos_or_decl)
            | Self::GlobalClassProp(pos_or_decl)
            | Self::GlobalFunParam(pos_or_decl)
            | Self::GlobalFunRet(pos_or_decl)
            | Self::DefaultCapability(pos_or_decl)
            | Self::SupportDynamicType(pos_or_decl)
            | Self::PessimisedInout(pos_or_decl)
            | Self::PessimisedReturn(pos_or_decl)
            | Self::PessimisedProp(pos_or_decl)
            | Self::PessimisedThis(pos_or_decl)
            | Self::IllegalRecursiveType(pos_or_decl, _)
            | Self::TupleFromSplat(pos_or_decl) => pos_or_decl,
        }
    }
}

impl Reason {
    pub fn pos(&self) -> Option<&Pos> {
        use T_::*;
        match self {
            NoReason => None,
            Invalid => None,
            MissingField => None,
            FromWitnessLocl(witness) => Some(witness.pos()),
            FromWitnessDecl(witness) => Some(witness.pos()),
            Idx(pos, _)
            | ArithRetFloat(pos, _, _)
            | ArithRetNum(pos, _, _)
            | Format(pos, _, _)
            | LambdaParam(pos, _)
            | Typeconst(box NoReason, (pos, _), _, _)
            | RigidTvarEscape(pos, _, _, _) => Some(pos),
            DynamicPartialEnforcement(pos_or_decl, _, _)
            | OpaqueTypeFromModule(pos_or_decl, _, _) => Some(pos_or_decl),
            LostInfo(_, t, _)
            | TypeAccess(t, _)
            | InvariantGeneric(t, _)
            | ContravariantGeneric(t, _)
            | DynamicCoercion(t)
            | ExprDepType(t, _, _)
            | Typeconst(t, _, _, _)
            | Instantiate(_, _, t) => t.pos(),
            LowerBound { .. }
            | Flow { .. }
            | PrjBoth { .. }
            | PrjOne { .. }
            | Axiom { .. }
            | Def(_, _)
            | Solved { .. } => None,
        }
    }
}
