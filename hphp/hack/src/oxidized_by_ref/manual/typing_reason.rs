// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_trait::TrivialDrop;
use bumpalo::collections::Vec;

use oxidized::typing_reason::Reason as OxReason;

use crate::pos::Pos;
use crate::{aast, ast_defs, ast_defs::Id};

pub use oxidized::typing_reason::ArgPosition;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct Reason<'a> {
    pub pos: Option<&'a Pos<'a>>,
    pub reason: Reason_<'a>,
}

impl TrivialDrop for Reason<'_> {}

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub enum Reason_<'a> {
    Rnone,
    Rwitness,
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Ridx(&'a Reason<'a>),
    /// Used as an index, in the Vector case
    RidxVector,
    /// Because it is iterated in a foreach loop
    Rforeach,
    /// Because it is iterated "await as" in foreach
    Rasyncforeach,
    Rarith,
    RarithInt,
    RarithRet,
    /// arg float typing reason, arg position
    RarithRetFloat(&'a Reason<'a>, ArgPosition),
    /// arg num typing reason, arg position
    RarithRetNum(&'a Reason<'a>, ArgPosition),
    RarithRetInt,
    RarithDynamic,
    RbitwiseDynamic,
    RincdecDynamic,
    Rcomp,
    RconcatRet,
    RlogicRet,
    Rbitwise,
    RbitwiseRet,
    RnoReturn,
    RnoReturnAsync,
    RretFunKind(ast_defs::FunKind),
    Rhint,
    Rthrow,
    Rplaceholder,
    RretDiv,
    RyieldGen,
    RyieldAsyncgen,
    RyieldAsyncnull,
    RyieldSend,
    /// true if due to lambda
    RlostInfo(&'a str, &'a Reason<'a>, &'a Pos<'a>, bool),
    Rformat(&'a str, &'a Reason<'a>),
    RclassClass(&'a str),
    RunknownClass,
    RdynamicYield(&'a Pos<'a>, &'a str, &'a str),
    RmapAppend,
    RvarParam,
    /// fun def pos, number of args before splat
    RunpackParam(&'a Pos<'a>, isize),
    RinoutParam,
    Rinstantiate(&'a Reason<'a>, &'a str, &'a Reason<'a>),
    RarrayFilter(&'a Reason<'a>),
    Rtypeconst(
        &'a Reason<'a>,
        (&'a Pos<'a>, &'a str),
        &'a str,
        &'a Reason<'a>,
    ),
    RtypeAccess(&'a Reason<'a>, Vec<'a, (&'a Reason<'a>, &'a str)>),
    RexprDepType(&'a Reason<'a>, &'a Pos<'a>, ExprDepTypeReason<'a>),
    /// ?-> operator is used
    RnullsafeOp,
    RtconstNoCstr(&'a aast::Sid<'a>),
    Rpredicated(&'a str),
    Ris,
    Ras,
    RvarrayOrDarrayKey,
    Rusing,
    RdynamicProp,
    RdynamicCall,
    RidxDict,
    RmissingRequiredField(&'a str),
    RmissingOptionalField(&'a str),
    RunsetField(&'a str),
    RcontravariantGeneric(&'a Reason<'a>, &'a str),
    RinvariantGeneric(&'a Reason<'a>, &'a str),
    Rregex,
    RlambdaUse,
    RimplicitUpperBound(&'a str),
    RtypeVariable,
    RtypeVariableGenerics(&'a str, &'a str),
    RsolveFail,
    RcstrOnGenerics(&'a aast::Sid<'a>),
    RlambdaParam(&'a Reason<'a>),
    Rshape(&'a str),
    Renforceable,
    Rdestructure,
    RkeyValueCollectionKey,
    RglobalClassProp,
    RglobalFunParam,
    RglobalFunRet,
}

impl TrivialDrop for Reason_<'_> {}

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub enum ExprDepTypeReason<'a> {
    ERexpr(isize),
    ERstatic,
    ERclass(&'a str),
    ERparent(&'a str),
    ERself(&'a str),
}

const RNONE: &Reason<'_> = &Reason::new(None, Reason_::Rnone);

impl<'a> Reason<'a> {
    pub const fn new(pos: Option<&'a Pos<'a>>, reason: Reason_<'a>) -> Self {
        Self { pos, reason }
    }

    pub const fn none() -> &'static Reason<'static> {
        RNONE
    }

    pub fn pos(&self) -> Option<&'a Pos<'a>> {
        self.pos
    }

    pub fn with_pos(pos: &'a Pos<'a>, reason: Reason_<'a>) -> Self {
        let pos = if pos.is_none() { None } else { Some(pos) };
        Self { pos, reason }
    }

    pub fn hint(pos: &'a Pos<'a>) -> Self {
        Self::with_pos(pos, Reason_::Rhint)
    }

    pub fn witness(pos: &'a Pos<'a>) -> Self {
        Self::with_pos(pos, Reason_::Rwitness)
    }

    pub fn instantiate(r1: &'a Reason<'a>, x: &'a str, r2: &'a Reason<'a>) -> Self {
        Self::new(r2.pos, Reason_::Rinstantiate(r1, x, r2))
    }

    pub fn to_oxidized(&self) -> OxReason {
        use OxReason as O;
        use Reason_ as R;

        let pos = match self.pos {
            Some(pos) => pos.to_owned(),
            None => oxidized::pos::Pos::make_none(),
        };
        match self.reason {
            R::Rnone => O::Rnone,
            R::Rwitness => O::Rwitness(pos),
            R::RnoReturn => O::RnoReturn(pos),
            R::Rhint => O::Rhint(pos),
            R::Rinstantiate(r_orig, generic_name, r_inst) => O::Rinstantiate(
                Box::new(r_orig.to_oxidized()),
                generic_name.to_string(),
                Box::new(r_inst.to_oxidized()),
            ),
            R::RcontravariantGeneric(r_orig, class_name) => {
                O::RcontravariantGeneric(Box::new(r_orig.to_oxidized()), class_name.to_string())
            }
            R::RinvariantGeneric(r_orig, class_name) => {
                O::RinvariantGeneric(Box::new(r_orig.to_oxidized()), class_name.to_string())
            }
            R::RtypeVariable => O::RtypeVariable(pos),
            R::RtypeVariableGenerics(tp_name, s) => {
                O::RtypeVariableGenerics(pos, tp_name.to_string(), s.to_string())
            }
            R::RsolveFail => O::RsolveFail(pos),
            R::RcstrOnGenerics(Id(id_pos, id)) => O::RcstrOnGenerics(
                pos,
                oxidized::ast_defs::Id((*id_pos).to_owned(), id.to_string()),
            ),
            _ => unimplemented!("Reason::to_oxidized for {:?}", &self.reason),
        }
    }
}

impl serde::Serialize for Reason<'_> {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        self.to_oxidized().serialize(serializer)
    }
}

impl ocamlrep::ToOcamlRep for Reason<'_> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, alloc: &'a A) -> ocamlrep::Value<'a> {
        self.to_oxidized().to_ocamlrep(alloc)
    }
}
