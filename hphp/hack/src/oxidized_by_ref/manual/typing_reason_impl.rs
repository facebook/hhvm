// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::pos::Pos;
use crate::typing_reason::*;

const RNONE: &Reason<'_> = &Reason::NoReason;

impl<'a> WitnessLocl<'a> {
    pub fn pos(&self) -> &'a Pos<'a> {
        match self {
            WitnessLocl::Witness(p)
            | WitnessLocl::IdxVector(p)
            | WitnessLocl::Foreach(p)
            | WitnessLocl::Asyncforeach(p)
            | WitnessLocl::Arith(p)
            | WitnessLocl::ArithRet(p)
            | WitnessLocl::ArithDynamic(p)
            | WitnessLocl::Comp(p)
            | WitnessLocl::ConcatRet(p)
            | WitnessLocl::LogicRet(p)
            | WitnessLocl::Bitwise(p)
            | WitnessLocl::BitwiseRet(p)
            | WitnessLocl::NoReturn(p)
            | WitnessLocl::NoReturnAsync(p)
            | WitnessLocl::RetFunKind((p, _))
            | WitnessLocl::Throw(p)
            | WitnessLocl::Placeholder(p)
            | WitnessLocl::RetDiv(p)
            | WitnessLocl::YieldGen(p)
            | WitnessLocl::YieldAsyncgen(p)
            | WitnessLocl::YieldAsyncnull(p)
            | WitnessLocl::YieldSend(p)
            | WitnessLocl::UnknownClass(p)
            | WitnessLocl::VarParam(p)
            | WitnessLocl::UnpackParam((p, _, _))
            | WitnessLocl::NullsafeOp(p)
            | WitnessLocl::Predicated((p, _))
            | WitnessLocl::IsRefinement(p)
            | WitnessLocl::AsRefinement(p)
            | WitnessLocl::Equal(p)
            | WitnessLocl::Using(p)
            | WitnessLocl::DynamicProp(p)
            | WitnessLocl::DynamicCall(p)
            | WitnessLocl::DynamicConstruct(p)
            | WitnessLocl::IdxDict(p)
            | WitnessLocl::IdxSetElement(p)
            | WitnessLocl::UnsetField((p, _))
            | WitnessLocl::Regex(p)
            | WitnessLocl::ArithRetInt(p)
            | WitnessLocl::BitwiseDynamic(p)
            | WitnessLocl::IncdecDynamic(p)
            | WitnessLocl::TypeVariable((p, _))
            | WitnessLocl::TypeVariableGenerics((p, _, _, _))
            | WitnessLocl::TypeVariableError((p, _))
            | WitnessLocl::Shape((p, _))
            | WitnessLocl::ShapeLiteral(p)
            | WitnessLocl::Destructure(p)
            | WitnessLocl::KeyValueCollectionKey(p)
            | WitnessLocl::Splice(p)
            | WitnessLocl::EtBoolean(p)
            | WitnessLocl::ConcatOperand(p)
            | WitnessLocl::InterpOperand(p)
            | WitnessLocl::MissingClass(p)
            | WitnessLocl::CapturedLike(p)
            | WitnessLocl::UnsafeCast(p)
            | WitnessLocl::Pattern(p)
            | WitnessLocl::JoinPoint(p)
            | WitnessLocl::StaticPropertyAccess(p)
            | WitnessLocl::ClassConstantAccess(p) => p,
        }
    }
}

impl<'a> WitnessDecl<'a> {
    pub fn pos(&self) -> &'a Pos<'a> {
        match self {
            WitnessDecl::WitnessFromDecl(p)
            | WitnessDecl::IdxVectorFromDecl(p)
            | WitnessDecl::RetFunKindFromDecl((p, _))
            | WitnessDecl::Hint(p)
            | WitnessDecl::ClassClass((p, _))
            | WitnessDecl::VarParamFromDecl(p)
            | WitnessDecl::TupleFromSplat(p)
            | WitnessDecl::InoutParam(p)
            | WitnessDecl::TconstNoCstr((p, _))
            | WitnessDecl::VarrayOrDarrayKey(p)
            | WitnessDecl::VecOrDictKey(p)
            | WitnessDecl::MissingOptionalField((p, _))
            | WitnessDecl::ImplicitUpperBound((p, _))
            | WitnessDecl::GlobalTypeVariableGenerics((p, _, _))
            | WitnessDecl::SolveFail(p)
            | WitnessDecl::CstrOnGenerics((p, _))
            | WitnessDecl::Enforceable(p)
            | WitnessDecl::GlobalClassProp(p)
            | WitnessDecl::GlobalFunParam(p)
            | WitnessDecl::GlobalFunRet(p)
            | WitnessDecl::DefaultCapability(p)
            | WitnessDecl::SupportDynamicType(p)
            | WitnessDecl::PessimisedInout(p)
            | WitnessDecl::PessimisedReturn((p, _))
            | WitnessDecl::PessimisedProp((p, _))
            | WitnessDecl::IllegalRecursiveType((p, _))
            | WitnessDecl::SupportDynamicTypeAssume(p)
            | WitnessDecl::PessimisedThis(p) => p,
        }
    }
}

impl<'a> Reason<'a> {
    pub const fn none() -> &'static Reason<'static> {
        RNONE
    }

    pub fn instantiate(args: &'a (Reason<'a>, &'a str, Reason<'a>)) -> Self {
        Reason::Instantiate(args)
    }

    pub fn pos(&self) -> Option<&'a Pos<'a>> {
        use T_::*;
        match self {
            NoReason => None,
            Invalid => None,
            MissingField => None,
            FromWitnessLocl(&witness) => Some(witness.pos()),
            FromWitnessDecl(&witness) => Some(witness.pos()),
            Idx((p, _))
            | ArithRetFloat((p, _, _))
            | ArithRetNum((p, _, _))
            | Format((p, _, _))
            | LambdaParam((p, _))
            | RigidTvarEscape((p, _, _, _)) => Some(p),

            DynamicPartialEnforcement((p, _, _))
            | SDTCall((p, _))
            | LikeCall((p, _))
            | OpaqueTypeFromModule((p, _, _)) => Some(p),
            LostInfo((_, r, _)) | TypeAccess((r, _)) | InvariantGeneric((r, _)) => r.pos(),

            DynamicCoercion(r) => r.pos(),
            ContravariantGeneric((r, _))
            | ExprDepType((r, _, _))
            | Typeconst((r, _, _, _))
            | Instantiate((_, _, r)) => r.pos(),
            _ => None,
        }
    }
}

impl<'a> std::fmt::Debug for T_<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use T_::*;
        match self {
            NoReason => f.debug_tuple("Rnone").finish(),
            MissingField => f.debug_tuple("RmissingField").finish(),
            FromWitnessLocl(witness) => witness.fmt(f),
            FromWitnessDecl(witness) => witness.fmt(f),
            Idx((p, t)) => f.debug_tuple("Ridx").field(p).field(t).finish(),
            ArithRetFloat((p, t, ap)) => f
                .debug_tuple("RarithRetFloat")
                .field(p)
                .field(t)
                .field(ap)
                .finish(),
            ArithRetNum((p, t, ap)) => f
                .debug_tuple("RarithRetNum")
                .field(p)
                .field(t)
                .field(ap)
                .finish(),
            LostInfo(p) => f.debug_tuple("RlostInfo").field(p).finish(),
            Format(p) => f.debug_tuple("Rformat").field(p).finish(),
            Instantiate(p) => f.debug_tuple("Rinstantiate").field(p).finish(),
            Typeconst(p) => f.debug_tuple("Rtypeconst").field(p).finish(),
            TypeAccess(p) => f.debug_tuple("RtypeAccess").field(p).finish(),
            ExprDepType(p) => f.debug_tuple("RexprDepType").field(p).finish(),
            ContravariantGeneric(p) => f.debug_tuple("RcontravariantGeneric").field(p).finish(),
            InvariantGeneric(p) => f.debug_tuple("RinvariantGeneric").field(p).finish(),
            LambdaParam(p) => f.debug_tuple("RlambdaParam").field(p).finish(),
            DynamicPartialEnforcement((p, s, t)) => f
                .debug_tuple("RdynamicPartialEnforcement")
                .field(p)
                .field(s)
                .field(t)
                .finish(),
            RigidTvarEscape((p, s1, s2, t)) => f
                .debug_tuple("RigidTvarEscape")
                .field(p)
                .field(s1)
                .field(s2)
                .field(t)
                .finish(),
            OpaqueTypeFromModule((p, s, t)) => f
                .debug_tuple("RopaqueTypeFromModule")
                .field(p)
                .field(s)
                .field(t)
                .finish(),
            SDTCall((p, t)) => f.debug_tuple("RSDTCall").field(p).field(t).finish(),
            LikeCall((p, t)) => f.debug_tuple("RlikeCall").field(p).field(t).finish(),
            DynamicCoercion(p) => f.debug_tuple("RdynamicCoercion").field(p).finish(),
            Invalid => f.debug_tuple("Rinvalid").finish(),
            LowerBound { .. }
            | Flow { .. }
            | PrjBoth { .. }
            | PrjOne { .. }
            | Axiom { .. }
            | Solved { .. }
            | Def(_) => Ok(()),
        }
    }
}
