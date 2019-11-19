// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<de222e66ab89963081683f3b692de836>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use crate::aast_defs::*;
impl XhpChild {
    pub fn mk_child_name(p0: Sid) -> Self {
        XhpChild::ChildName(p0)
    }
    pub fn mk_child_list(p0: Vec<XhpChild>) -> Self {
        XhpChild::ChildList(p0)
    }
    pub fn mk_child_unary(p0: XhpChild, p1: XhpChildOp) -> Self {
        XhpChild::ChildUnary(Box::new(p0), p1)
    }
    pub fn mk_child_binary(p0: XhpChild, p1: XhpChild) -> Self {
        XhpChild::ChildBinary(Box::new(p0), Box::new(p1))
    }
}
impl Hint_ {
    pub fn mk_hoption(p0: Hint) -> Self {
        Hint_::Hoption(p0)
    }
    pub fn mk_hlike(p0: Hint) -> Self {
        Hint_::Hlike(p0)
    }
    pub fn mk_hfun(p0: HintFun) -> Self {
        Hint_::Hfun(p0)
    }
    pub fn mk_htuple(p0: Vec<Hint>) -> Self {
        Hint_::Htuple(p0)
    }
    pub fn mk_happly(p0: Sid, p1: Vec<Hint>) -> Self {
        Hint_::Happly(p0, p1)
    }
    pub fn mk_hshape(p0: NastShapeInfo) -> Self {
        Hint_::Hshape(p0)
    }
    pub fn mk_haccess(p0: Hint, p1: Vec<Sid>) -> Self {
        Hint_::Haccess(p0, p1)
    }
    pub fn mk_hsoft(p0: Hint) -> Self {
        Hint_::Hsoft(p0)
    }
    pub fn mk_hany() -> Self {
        Hint_::Hany
    }
    pub fn mk_herr() -> Self {
        Hint_::Herr
    }
    pub fn mk_hmixed() -> Self {
        Hint_::Hmixed
    }
    pub fn mk_hnonnull() -> Self {
        Hint_::Hnonnull
    }
    pub fn mk_habstr(p0: String) -> Self {
        Hint_::Habstr(p0)
    }
    pub fn mk_harray(p0: Option<Hint>, p1: Option<Hint>) -> Self {
        Hint_::Harray(p0, p1)
    }
    pub fn mk_hdarray(p0: Hint, p1: Hint) -> Self {
        Hint_::Hdarray(p0, p1)
    }
    pub fn mk_hvarray(p0: Hint) -> Self {
        Hint_::Hvarray(p0)
    }
    pub fn mk_hvarray_or_darray(p0: Option<Hint>, p1: Hint) -> Self {
        Hint_::HvarrayOrDarray(p0, p1)
    }
    pub fn mk_hprim(p0: Tprim) -> Self {
        Hint_::Hprim(p0)
    }
    pub fn mk_hthis() -> Self {
        Hint_::Hthis
    }
    pub fn mk_hdynamic() -> Self {
        Hint_::Hdynamic
    }
    pub fn mk_hnothing() -> Self {
        Hint_::Hnothing
    }
    pub fn mk_hpu_access(p0: Hint, p1: Sid) -> Self {
        Hint_::HpuAccess(p0, p1)
    }
    pub fn mk_hunion(p0: Vec<Hint>) -> Self {
        Hint_::Hunion(p0)
    }
    pub fn mk_hintersection(p0: Vec<Hint>) -> Self {
        Hint_::Hintersection(p0)
    }
}
impl Tprim {
    pub fn mk_tnull() -> Self {
        Tprim::Tnull
    }
    pub fn mk_tvoid() -> Self {
        Tprim::Tvoid
    }
    pub fn mk_tint() -> Self {
        Tprim::Tint
    }
    pub fn mk_tbool() -> Self {
        Tprim::Tbool
    }
    pub fn mk_tfloat() -> Self {
        Tprim::Tfloat
    }
    pub fn mk_tstring() -> Self {
        Tprim::Tstring
    }
    pub fn mk_tresource() -> Self {
        Tprim::Tresource
    }
    pub fn mk_tnum() -> Self {
        Tprim::Tnum
    }
    pub fn mk_tarraykey() -> Self {
        Tprim::Tarraykey
    }
    pub fn mk_tnoreturn() -> Self {
        Tprim::Tnoreturn
    }
    pub fn mk_tatom(p0: String) -> Self {
        Tprim::Tatom(p0)
    }
}
