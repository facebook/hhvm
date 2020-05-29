// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::typing_reason::Reason as OxReason;
use std::rc::Rc;

use crate::pos::Pos;
use crate::{aast, ast_defs, ast_defs::Id};

pub use oxidized::typing_reason::ArgPosition;

#[derive(Clone, Debug, Hash)]
pub struct Reason {
    pub pos: Option<Rc<Pos>>,
    pub reason: Reason_,
}

#[derive(Clone, Debug, Hash)]
pub enum Reason_ {
    Rnone,
    Rwitness,
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Ridx(Rc<Reason>),
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
    RarithRetFloat(Rc<Reason>, ArgPosition),
    /// arg num typing reason, arg position
    RarithRetNum(Rc<Reason>, ArgPosition),
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
    RlostInfo(String, Rc<Reason>, Rc<Pos>, bool),
    Rformat(String, Rc<Reason>),
    RclassClass(String),
    RunknownClass,
    RdynamicYield(Rc<Pos>, String, String),
    RmapAppend,
    RvarParam,
    /// fun def pos, number of args before splat
    RunpackParam(Rc<Pos>, isize),
    RinoutParam,
    Rinstantiate(Rc<Reason>, String, Rc<Reason>),
    RarrayFilter(Rc<Reason>),
    Rtypeconst(Rc<Reason>, (Rc<Pos>, String), String, Rc<Reason>),
    RtypeAccess(Rc<Reason>, Vec<(Rc<Reason>, String)>),
    RexprDepType(Rc<Reason>, Rc<Pos>, ExprDepTypeReason),
    /// ?-> operator is used
    RnullsafeOp,
    RtconstNoCstr(aast::Sid),
    Rpredicated(String),
    Ris,
    Ras,
    RvarrayOrDarrayKey,
    Rusing,
    RdynamicProp,
    RdynamicCall,
    RidxDict,
    RmissingRequiredField(String),
    RmissingOptionalField(String),
    RunsetField(String),
    RcontravariantGeneric(Rc<Reason>, String),
    RinvariantGeneric(Rc<Reason>, String),
    Rregex,
    RlambdaUse,
    RimplicitUpperBound(String),
    RtypeVariable,
    RtypeVariableGenerics(String, String),
    RsolveFail,
    RcstrOnGenerics(aast::Sid),
    RlambdaParam(Rc<Reason>),
    Rshape(String),
    Renforceable,
    Rdestructure,
    RkeyValueCollectionKey,
    RglobalClassProp,
    RglobalFunParam,
    RglobalFunRet,
}

#[derive(Clone, Debug, Hash)]
pub enum ExprDepTypeReason {
    ERexpr(isize),
    ERstatic,
    ERclass(String),
    ERparent(String),
    ERself(String),
}

impl Reason {
    pub const fn new(pos: Option<Rc<Pos>>, reason: Reason_) -> Self {
        Self { pos, reason }
    }

    pub const fn none() -> Reason {
        Self {
            pos: None,
            reason: Reason_::Rnone,
        }
    }

    pub fn with_pos(pos: Rc<Pos>, reason: Reason_) -> Self {
        let pos = if pos.is_none() { None } else { Some(pos) };
        Self { pos, reason }
    }

    pub fn hint(pos: Rc<Pos>) -> Self {
        Self::with_pos(pos, Reason_::Rhint)
    }

    pub fn witness(pos: Rc<Pos>) -> Self {
        Self::with_pos(pos, Reason_::Rwitness)
    }

    pub fn instantiate(r1: Rc<Reason>, x: String, r2: Rc<Reason>) -> Self {
        Self::new(r2.pos.clone(), Reason_::Rinstantiate(r1, x, r2))
    }

    pub fn to_oxidized(&self) -> OxReason {
        use OxReason as O;
        use Reason_ as R;

        let pos = match &self.pos {
            Some(pos) => pos.as_ref().clone(),
            None => oxidized::pos::Pos::make_none(),
        };
        match &self.reason {
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
                oxidized::ast_defs::Id(id_pos.as_ref().clone(), id.to_string()),
            ),
            _ => unimplemented!("Reason::to_oxidized for {:?}", &self.reason),
        }
    }
}

impl serde::Serialize for Reason {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        self.to_oxidized().serialize(serializer)
    }
}

impl ocamlrep::ToOcamlRep for Reason {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, alloc: &'a A) -> ocamlrep::Value<'a> {
        self.to_oxidized().to_ocamlrep(alloc)
    }
}
