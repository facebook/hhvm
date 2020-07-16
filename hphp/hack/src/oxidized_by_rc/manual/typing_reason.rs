// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use crate::pos::Pos;
use crate::{aast, ast_defs};

pub use oxidized_by_ref::typing_reason::ArgPosition;

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

impl serde::Serialize for Reason {
    fn serialize<S: serde::Serializer>(&self, _serializer: S) -> Result<S::Ok, S::Error> {
        unimplemented!()
    }
}

impl ocamlrep::ToOcamlRep for Reason {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, _alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        unimplemented!()
    }
}
