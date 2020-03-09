// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bumpalo::collections::Vec;

use oxidized::pos::Pos;
pub use oxidized::typing_reason::ArgPosition;
use oxidized::{aast, ast_defs};

pub struct PReason_<'a> {
    pub pos: Option<&'a Pos>,
    pub reason: Reason<'a>,
}

pub type PReason<'a> = &'a PReason_<'a>;

pub enum Reason<'a> {
    Rnone,
    Rwitness,
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Ridx(PReason<'a>),
    /// Used as an index, in the Vector case
    RidxVector,
    /// Used to append element to an array
    Rappend,
    /// Array accessed with a static string index
    Rfield,
    /// Because it is iterated in a foreach loop
    Rforeach,
    /// Because it is iterated "await as" in foreach
    Rasyncforeach,
    Raccess,
    Rarith,
    RarithInt,
    RarithRet,
    /// arg float typing reason, arg position
    RarithRetFloat(PReason<'a>, ArgPosition),
    /// arg num typing reason, arg position
    RarithRetNum(PReason<'a>, ArgPosition),
    RarithRetInt,
    RarithDynamic,
    RbitwiseDynamic,
    RincdecDynamic,
    Rstring2,
    Rcomp,
    Rconcat,
    RconcatRet,
    Rlogic,
    RlogicRet,
    Rbitwise,
    RbitwiseRet,
    Rstmt,
    RnoReturn,
    RnoReturnAsync,
    RretFunKind(ast_defs::FunKind),
    Rhint,
    RnullCheck,
    RnotInCstr,
    Rthrow,
    Rplaceholder,
    Rattr,
    Rxhp,
    RretDiv,
    RyieldGen,
    RyieldAsyncgen,
    RyieldAsyncnull,
    RyieldSend,
    /// true if due to lambda
    RlostInfo(&'a str, PReason<'a>, &'a Pos, bool),
    Rcoerced(PReason<'a>, &'a Pos, &'a str),
    Rformat(&'a str, PReason<'a>),
    RclassClass(&'a str),
    RunknownClass,
    RdynamicYield(&'a Pos, &'a str, &'a str),
    RmapAppend,
    RvarParam,
    /// fun def pos, number of args before splat
    RunpackParam(&'a Pos, isize),
    RinoutParam,
    Rinstantiate(PReason<'a>, &'a str, PReason<'a>),
    RarrayFilter(PReason<'a>),
    Rtypeconst(PReason<'a>, (&'a Pos, &'a str), &'a str, PReason<'a>),
    RtypeAccess(PReason<'a>, Vec<'a, (PReason<'a>, &'a str)>),
    RexprDepType(PReason<'a>, &'a Pos, ExprDepTypeReason<'a>),
    /// ?-> operator is used
    RnullsafeOp,
    RtconstNoCstr(&'a aast::Sid),
    RusedAsMap,
    RusedAsShape,
    Rpredicated(&'a str),
    Ris,
    Ras,
    RfinalProperty,
    RvarrayOrDarrayKey,
    Rusing,
    RdynamicProp,
    RdynamicCall,
    RidxDict,
    RmissingRequiredField(&'a str),
    RmissingOptionalField(&'a str),
    RunsetField(&'a str),
    RcontravariantGeneric(PReason<'a>, &'a str),
    RinvariantGeneric(PReason<'a>, &'a str),
    Rregex,
    RlambdaUse,
    RimplicitUpperBound(&'a str),
    RtypeVariable,
    RtypeVariableGenerics(&'a str, &'a str),
    RsolveFail,
    RcstrOnGenerics(&'a aast::Sid),
    RlambdaParam(PReason<'a>),
    Rshape(&'a str),
    Renforceable,
    Rdestructure,
    RkeyValueCollectionKey,
    RglobalClassProp,
    RglobalFunParam,
    RglobalFunRet,
}

pub enum ExprDepTypeReason<'a> {
    ERexpr(isize),
    ERstatic,
    ERclass(&'a str),
    ERparent(&'a str),
    ERself(&'a str),
}
