// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bumpalo::collections::Vec;

use oxidized::pos::Pos;
pub use oxidized::typing_reason::ArgPosition;
use oxidized::typing_reason::Reason as OxReason;
use oxidized::{aast, ast_defs};

#[derive(Debug)]
pub struct PReason_<'a> {
    pub pos: Option<&'a Pos>,
    pub reason: Reason<'a>,
}

pub type PReason<'a> = &'a PReason_<'a>;

#[derive(Debug)]
pub enum Reason<'a> {
    Rnone,
    Rwitness,
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Ridx(PReason<'a>),
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
    RarithRetFloat(PReason<'a>, ArgPosition),
    /// arg num typing reason, arg position
    RarithRetNum(PReason<'a>, ArgPosition),
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
    RlostInfo(&'a str, PReason<'a>, &'a Pos, bool),
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

#[derive(Debug)]
pub enum ExprDepTypeReason<'a> {
    ERexpr(isize),
    ERstatic,
    ERclass(&'a str),
    ERparent(&'a str),
    ERself(&'a str),
}

impl<'a> From<&'a OxReason> for PReason_<'a> {
    fn from(r: &'a OxReason) -> Self {
        match r {
            OxReason::Rhint(pos) => PReason_ {
                pos: Some(&pos),
                reason: Reason::Rhint,
            },
            OxReason::Rwitness(pos) => PReason_ {
                pos: Some(&pos),
                reason: Reason::Rwitness,
            },
            _ => panic!(
                "Did not expect anything else than Rhint from the decl provider. Got {:?}",
                r
            ),
        }
    }
}

impl<'a> PReason_<'a> {
    pub fn to_oxidized(&self) -> OxReason {
        use OxReason as O;
        use Reason as R;

        let pos = match self.pos {
            Some(pos) => pos.clone(),
            None => Pos::make_none(),
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
            R::RcstrOnGenerics(sid) => O::RcstrOnGenerics(pos, sid.clone()),
            _ => unimplemented!("{:?}", &self.reason),
        }
    }
}
