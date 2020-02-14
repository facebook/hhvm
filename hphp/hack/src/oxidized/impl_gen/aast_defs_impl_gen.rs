// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8457ea731588a0b82c7b93a66b609cd5>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use crate::aast_defs::*;
impl PuLoc {
    pub fn mk_unknown() -> Self {
        PuLoc::Unknown
    }
    pub fn mk_type_parameter() -> Self {
        PuLoc::TypeParameter
    }
    pub fn mk_atom() -> Self {
        PuLoc::Atom
    }
    pub fn is_unknown(&self) -> bool {
        match self {
            PuLoc::Unknown => true,
            _ => false,
        }
    }
    pub fn is_type_parameter(&self) -> bool {
        match self {
            PuLoc::TypeParameter => true,
            _ => false,
        }
    }
    pub fn is_atom(&self) -> bool {
        match self {
            PuLoc::Atom => true,
            _ => false,
        }
    }
}
impl CallType {
    pub fn mk_cnormal() -> Self {
        CallType::Cnormal
    }
    pub fn mk_cuser_func() -> Self {
        CallType::CuserFunc
    }
    pub fn is_cnormal(&self) -> bool {
        match self {
            CallType::Cnormal => true,
            _ => false,
        }
    }
    pub fn is_cuser_func(&self) -> bool {
        match self {
            CallType::CuserFunc => true,
            _ => false,
        }
    }
}
impl FuncReactive {
    pub fn mk_freactive() -> Self {
        FuncReactive::FReactive
    }
    pub fn mk_flocal() -> Self {
        FuncReactive::FLocal
    }
    pub fn mk_fshallow() -> Self {
        FuncReactive::FShallow
    }
    pub fn mk_fnonreactive() -> Self {
        FuncReactive::FNonreactive
    }
    pub fn is_freactive(&self) -> bool {
        match self {
            FuncReactive::FReactive => true,
            _ => false,
        }
    }
    pub fn is_flocal(&self) -> bool {
        match self {
            FuncReactive::FLocal => true,
            _ => false,
        }
    }
    pub fn is_fshallow(&self) -> bool {
        match self {
            FuncReactive::FShallow => true,
            _ => false,
        }
    }
    pub fn is_fnonreactive(&self) -> bool {
        match self {
            FuncReactive::FNonreactive => true,
            _ => false,
        }
    }
}
impl ParamMutability {
    pub fn mk_pmutable() -> Self {
        ParamMutability::PMutable
    }
    pub fn mk_powned_mutable() -> Self {
        ParamMutability::POwnedMutable
    }
    pub fn mk_pmaybe_mutable() -> Self {
        ParamMutability::PMaybeMutable
    }
    pub fn is_pmutable(&self) -> bool {
        match self {
            ParamMutability::PMutable => true,
            _ => false,
        }
    }
    pub fn is_powned_mutable(&self) -> bool {
        match self {
            ParamMutability::POwnedMutable => true,
            _ => false,
        }
    }
    pub fn is_pmaybe_mutable(&self) -> bool {
        match self {
            ParamMutability::PMaybeMutable => true,
            _ => false,
        }
    }
}
impl ImportFlavor {
    pub fn mk_include() -> Self {
        ImportFlavor::Include
    }
    pub fn mk_require() -> Self {
        ImportFlavor::Require
    }
    pub fn mk_include_once() -> Self {
        ImportFlavor::IncludeOnce
    }
    pub fn mk_require_once() -> Self {
        ImportFlavor::RequireOnce
    }
    pub fn is_include(&self) -> bool {
        match self {
            ImportFlavor::Include => true,
            _ => false,
        }
    }
    pub fn is_require(&self) -> bool {
        match self {
            ImportFlavor::Require => true,
            _ => false,
        }
    }
    pub fn is_include_once(&self) -> bool {
        match self {
            ImportFlavor::IncludeOnce => true,
            _ => false,
        }
    }
    pub fn is_require_once(&self) -> bool {
        match self {
            ImportFlavor::RequireOnce => true,
            _ => false,
        }
    }
}
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
    pub fn is_child_name(&self) -> bool {
        match self {
            XhpChild::ChildName(..) => true,
            _ => false,
        }
    }
    pub fn is_child_list(&self) -> bool {
        match self {
            XhpChild::ChildList(..) => true,
            _ => false,
        }
    }
    pub fn is_child_unary(&self) -> bool {
        match self {
            XhpChild::ChildUnary(..) => true,
            _ => false,
        }
    }
    pub fn is_child_binary(&self) -> bool {
        match self {
            XhpChild::ChildBinary(..) => true,
            _ => false,
        }
    }
    pub fn as_child_name(&self) -> Option<&Sid> {
        match self {
            XhpChild::ChildName(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_list(&self) -> Option<&Vec<XhpChild>> {
        match self {
            XhpChild::ChildList(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_unary(&self) -> Option<(&XhpChild, &XhpChildOp)> {
        match self {
            XhpChild::ChildUnary(p0, p1) => Some((&p0, p1)),
            _ => None,
        }
    }
    pub fn as_child_binary(&self) -> Option<(&XhpChild, &XhpChild)> {
        match self {
            XhpChild::ChildBinary(p0, p1) => Some((&p0, &p1)),
            _ => None,
        }
    }
    pub fn as_child_name_mut(&mut self) -> Option<&mut Sid> {
        match self {
            XhpChild::ChildName(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_list_mut(&mut self) -> Option<&mut Vec<XhpChild>> {
        match self {
            XhpChild::ChildList(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_unary_mut(&mut self) -> Option<(&mut XhpChild, &mut XhpChildOp)> {
        match self {
            XhpChild::ChildUnary(p0, p1) => Some((p0.as_mut(), p1)),
            _ => None,
        }
    }
    pub fn as_child_binary_mut(&mut self) -> Option<(&mut XhpChild, &mut XhpChild)> {
        match self {
            XhpChild::ChildBinary(p0, p1) => Some((p0.as_mut(), p1.as_mut())),
            _ => None,
        }
    }
    pub fn as_child_name_into(self) -> Option<Sid> {
        match self {
            XhpChild::ChildName(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_list_into(self) -> Option<Vec<XhpChild>> {
        match self {
            XhpChild::ChildList(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_unary_into(self) -> Option<(XhpChild, XhpChildOp)> {
        match self {
            XhpChild::ChildUnary(p0, p1) => Some((*p0, p1)),
            _ => None,
        }
    }
    pub fn as_child_binary_into(self) -> Option<(XhpChild, XhpChild)> {
        match self {
            XhpChild::ChildBinary(p0, p1) => Some((*p0, *p1)),
            _ => None,
        }
    }
}
impl XhpChildOp {
    pub fn mk_child_star() -> Self {
        XhpChildOp::ChildStar
    }
    pub fn mk_child_plus() -> Self {
        XhpChildOp::ChildPlus
    }
    pub fn mk_child_question() -> Self {
        XhpChildOp::ChildQuestion
    }
    pub fn is_child_star(&self) -> bool {
        match self {
            XhpChildOp::ChildStar => true,
            _ => false,
        }
    }
    pub fn is_child_plus(&self) -> bool {
        match self {
            XhpChildOp::ChildPlus => true,
            _ => false,
        }
    }
    pub fn is_child_question(&self) -> bool {
        match self {
            XhpChildOp::ChildQuestion => true,
            _ => false,
        }
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
    pub fn mk_hpu_access(p0: Hint, p1: Sid, p2: PuLoc) -> Self {
        Hint_::HpuAccess(p0, p1, p2)
    }
    pub fn mk_hunion(p0: Vec<Hint>) -> Self {
        Hint_::Hunion(p0)
    }
    pub fn mk_hintersection(p0: Vec<Hint>) -> Self {
        Hint_::Hintersection(p0)
    }
    pub fn is_hoption(&self) -> bool {
        match self {
            Hint_::Hoption(..) => true,
            _ => false,
        }
    }
    pub fn is_hlike(&self) -> bool {
        match self {
            Hint_::Hlike(..) => true,
            _ => false,
        }
    }
    pub fn is_hfun(&self) -> bool {
        match self {
            Hint_::Hfun(..) => true,
            _ => false,
        }
    }
    pub fn is_htuple(&self) -> bool {
        match self {
            Hint_::Htuple(..) => true,
            _ => false,
        }
    }
    pub fn is_happly(&self) -> bool {
        match self {
            Hint_::Happly(..) => true,
            _ => false,
        }
    }
    pub fn is_hshape(&self) -> bool {
        match self {
            Hint_::Hshape(..) => true,
            _ => false,
        }
    }
    pub fn is_haccess(&self) -> bool {
        match self {
            Hint_::Haccess(..) => true,
            _ => false,
        }
    }
    pub fn is_hsoft(&self) -> bool {
        match self {
            Hint_::Hsoft(..) => true,
            _ => false,
        }
    }
    pub fn is_hany(&self) -> bool {
        match self {
            Hint_::Hany => true,
            _ => false,
        }
    }
    pub fn is_herr(&self) -> bool {
        match self {
            Hint_::Herr => true,
            _ => false,
        }
    }
    pub fn is_hmixed(&self) -> bool {
        match self {
            Hint_::Hmixed => true,
            _ => false,
        }
    }
    pub fn is_hnonnull(&self) -> bool {
        match self {
            Hint_::Hnonnull => true,
            _ => false,
        }
    }
    pub fn is_habstr(&self) -> bool {
        match self {
            Hint_::Habstr(..) => true,
            _ => false,
        }
    }
    pub fn is_harray(&self) -> bool {
        match self {
            Hint_::Harray(..) => true,
            _ => false,
        }
    }
    pub fn is_hdarray(&self) -> bool {
        match self {
            Hint_::Hdarray(..) => true,
            _ => false,
        }
    }
    pub fn is_hvarray(&self) -> bool {
        match self {
            Hint_::Hvarray(..) => true,
            _ => false,
        }
    }
    pub fn is_hvarray_or_darray(&self) -> bool {
        match self {
            Hint_::HvarrayOrDarray(..) => true,
            _ => false,
        }
    }
    pub fn is_hprim(&self) -> bool {
        match self {
            Hint_::Hprim(..) => true,
            _ => false,
        }
    }
    pub fn is_hthis(&self) -> bool {
        match self {
            Hint_::Hthis => true,
            _ => false,
        }
    }
    pub fn is_hdynamic(&self) -> bool {
        match self {
            Hint_::Hdynamic => true,
            _ => false,
        }
    }
    pub fn is_hnothing(&self) -> bool {
        match self {
            Hint_::Hnothing => true,
            _ => false,
        }
    }
    pub fn is_hpu_access(&self) -> bool {
        match self {
            Hint_::HpuAccess(..) => true,
            _ => false,
        }
    }
    pub fn is_hunion(&self) -> bool {
        match self {
            Hint_::Hunion(..) => true,
            _ => false,
        }
    }
    pub fn is_hintersection(&self) -> bool {
        match self {
            Hint_::Hintersection(..) => true,
            _ => false,
        }
    }
    pub fn as_hoption(&self) -> Option<&Hint> {
        match self {
            Hint_::Hoption(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hlike(&self) -> Option<&Hint> {
        match self {
            Hint_::Hlike(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hfun(&self) -> Option<&HintFun> {
        match self {
            Hint_::Hfun(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_htuple(&self) -> Option<&Vec<Hint>> {
        match self {
            Hint_::Htuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_happly(&self) -> Option<(&Sid, &Vec<Hint>)> {
        match self {
            Hint_::Happly(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hshape(&self) -> Option<&NastShapeInfo> {
        match self {
            Hint_::Hshape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_haccess(&self) -> Option<(&Hint, &Vec<Sid>)> {
        match self {
            Hint_::Haccess(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hsoft(&self) -> Option<&Hint> {
        match self {
            Hint_::Hsoft(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_habstr(&self) -> Option<&String> {
        match self {
            Hint_::Habstr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_harray(&self) -> Option<(&Option<Hint>, &Option<Hint>)> {
        match self {
            Hint_::Harray(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hdarray(&self) -> Option<(&Hint, &Hint)> {
        match self {
            Hint_::Hdarray(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hvarray(&self) -> Option<&Hint> {
        match self {
            Hint_::Hvarray(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hvarray_or_darray(&self) -> Option<(&Option<Hint>, &Hint)> {
        match self {
            Hint_::HvarrayOrDarray(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hprim(&self) -> Option<&Tprim> {
        match self {
            Hint_::Hprim(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hpu_access(&self) -> Option<(&Hint, &Sid, &PuLoc)> {
        match self {
            Hint_::HpuAccess(p0, p1, p2) => Some((p0, p1, p2)),
            _ => None,
        }
    }
    pub fn as_hunion(&self) -> Option<&Vec<Hint>> {
        match self {
            Hint_::Hunion(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hintersection(&self) -> Option<&Vec<Hint>> {
        match self {
            Hint_::Hintersection(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hoption_mut(&mut self) -> Option<&mut Hint> {
        match self {
            Hint_::Hoption(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hlike_mut(&mut self) -> Option<&mut Hint> {
        match self {
            Hint_::Hlike(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hfun_mut(&mut self) -> Option<&mut HintFun> {
        match self {
            Hint_::Hfun(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_htuple_mut(&mut self) -> Option<&mut Vec<Hint>> {
        match self {
            Hint_::Htuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_happly_mut(&mut self) -> Option<(&mut Sid, &mut Vec<Hint>)> {
        match self {
            Hint_::Happly(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hshape_mut(&mut self) -> Option<&mut NastShapeInfo> {
        match self {
            Hint_::Hshape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_haccess_mut(&mut self) -> Option<(&mut Hint, &mut Vec<Sid>)> {
        match self {
            Hint_::Haccess(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hsoft_mut(&mut self) -> Option<&mut Hint> {
        match self {
            Hint_::Hsoft(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_habstr_mut(&mut self) -> Option<&mut String> {
        match self {
            Hint_::Habstr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_harray_mut(&mut self) -> Option<(&mut Option<Hint>, &mut Option<Hint>)> {
        match self {
            Hint_::Harray(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hdarray_mut(&mut self) -> Option<(&mut Hint, &mut Hint)> {
        match self {
            Hint_::Hdarray(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hvarray_mut(&mut self) -> Option<&mut Hint> {
        match self {
            Hint_::Hvarray(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hvarray_or_darray_mut(&mut self) -> Option<(&mut Option<Hint>, &mut Hint)> {
        match self {
            Hint_::HvarrayOrDarray(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hprim_mut(&mut self) -> Option<&mut Tprim> {
        match self {
            Hint_::Hprim(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hpu_access_mut(&mut self) -> Option<(&mut Hint, &mut Sid, &mut PuLoc)> {
        match self {
            Hint_::HpuAccess(p0, p1, p2) => Some((p0, p1, p2)),
            _ => None,
        }
    }
    pub fn as_hunion_mut(&mut self) -> Option<&mut Vec<Hint>> {
        match self {
            Hint_::Hunion(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hintersection_mut(&mut self) -> Option<&mut Vec<Hint>> {
        match self {
            Hint_::Hintersection(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hoption_into(self) -> Option<Hint> {
        match self {
            Hint_::Hoption(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hlike_into(self) -> Option<Hint> {
        match self {
            Hint_::Hlike(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hfun_into(self) -> Option<HintFun> {
        match self {
            Hint_::Hfun(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_htuple_into(self) -> Option<Vec<Hint>> {
        match self {
            Hint_::Htuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_happly_into(self) -> Option<(Sid, Vec<Hint>)> {
        match self {
            Hint_::Happly(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hshape_into(self) -> Option<NastShapeInfo> {
        match self {
            Hint_::Hshape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_haccess_into(self) -> Option<(Hint, Vec<Sid>)> {
        match self {
            Hint_::Haccess(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hsoft_into(self) -> Option<Hint> {
        match self {
            Hint_::Hsoft(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_habstr_into(self) -> Option<String> {
        match self {
            Hint_::Habstr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_harray_into(self) -> Option<(Option<Hint>, Option<Hint>)> {
        match self {
            Hint_::Harray(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hdarray_into(self) -> Option<(Hint, Hint)> {
        match self {
            Hint_::Hdarray(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hvarray_into(self) -> Option<Hint> {
        match self {
            Hint_::Hvarray(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hvarray_or_darray_into(self) -> Option<(Option<Hint>, Hint)> {
        match self {
            Hint_::HvarrayOrDarray(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hprim_into(self) -> Option<Tprim> {
        match self {
            Hint_::Hprim(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hpu_access_into(self) -> Option<(Hint, Sid, PuLoc)> {
        match self {
            Hint_::HpuAccess(p0, p1, p2) => Some((p0, p1, p2)),
            _ => None,
        }
    }
    pub fn as_hunion_into(self) -> Option<Vec<Hint>> {
        match self {
            Hint_::Hunion(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hintersection_into(self) -> Option<Vec<Hint>> {
        match self {
            Hint_::Hintersection(p0) => Some(p0),
            _ => None,
        }
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
    pub fn is_tnull(&self) -> bool {
        match self {
            Tprim::Tnull => true,
            _ => false,
        }
    }
    pub fn is_tvoid(&self) -> bool {
        match self {
            Tprim::Tvoid => true,
            _ => false,
        }
    }
    pub fn is_tint(&self) -> bool {
        match self {
            Tprim::Tint => true,
            _ => false,
        }
    }
    pub fn is_tbool(&self) -> bool {
        match self {
            Tprim::Tbool => true,
            _ => false,
        }
    }
    pub fn is_tfloat(&self) -> bool {
        match self {
            Tprim::Tfloat => true,
            _ => false,
        }
    }
    pub fn is_tstring(&self) -> bool {
        match self {
            Tprim::Tstring => true,
            _ => false,
        }
    }
    pub fn is_tresource(&self) -> bool {
        match self {
            Tprim::Tresource => true,
            _ => false,
        }
    }
    pub fn is_tnum(&self) -> bool {
        match self {
            Tprim::Tnum => true,
            _ => false,
        }
    }
    pub fn is_tarraykey(&self) -> bool {
        match self {
            Tprim::Tarraykey => true,
            _ => false,
        }
    }
    pub fn is_tnoreturn(&self) -> bool {
        match self {
            Tprim::Tnoreturn => true,
            _ => false,
        }
    }
    pub fn is_tatom(&self) -> bool {
        match self {
            Tprim::Tatom(..) => true,
            _ => false,
        }
    }
    pub fn as_tatom(&self) -> Option<&String> {
        match self {
            Tprim::Tatom(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_tatom_mut(&mut self) -> Option<&mut String> {
        match self {
            Tprim::Tatom(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_tatom_into(self) -> Option<String> {
        match self {
            Tprim::Tatom(p0) => Some(p0),
            _ => None,
        }
    }
}
impl KvcKind {
    pub fn mk_map() -> Self {
        KvcKind::Map
    }
    pub fn mk_imm_map() -> Self {
        KvcKind::ImmMap
    }
    pub fn mk_dict() -> Self {
        KvcKind::Dict
    }
    pub fn is_map(&self) -> bool {
        match self {
            KvcKind::Map => true,
            _ => false,
        }
    }
    pub fn is_imm_map(&self) -> bool {
        match self {
            KvcKind::ImmMap => true,
            _ => false,
        }
    }
    pub fn is_dict(&self) -> bool {
        match self {
            KvcKind::Dict => true,
            _ => false,
        }
    }
}
impl VcKind {
    pub fn mk_vector() -> Self {
        VcKind::Vector
    }
    pub fn mk_imm_vector() -> Self {
        VcKind::ImmVector
    }
    pub fn mk_vec() -> Self {
        VcKind::Vec
    }
    pub fn mk_set() -> Self {
        VcKind::Set
    }
    pub fn mk_imm_set() -> Self {
        VcKind::ImmSet
    }
    pub fn mk_pair_() -> Self {
        VcKind::Pair_
    }
    pub fn mk_keyset() -> Self {
        VcKind::Keyset
    }
    pub fn is_vector(&self) -> bool {
        match self {
            VcKind::Vector => true,
            _ => false,
        }
    }
    pub fn is_imm_vector(&self) -> bool {
        match self {
            VcKind::ImmVector => true,
            _ => false,
        }
    }
    pub fn is_vec(&self) -> bool {
        match self {
            VcKind::Vec => true,
            _ => false,
        }
    }
    pub fn is_set(&self) -> bool {
        match self {
            VcKind::Set => true,
            _ => false,
        }
    }
    pub fn is_imm_set(&self) -> bool {
        match self {
            VcKind::ImmSet => true,
            _ => false,
        }
    }
    pub fn is_pair_(&self) -> bool {
        match self {
            VcKind::Pair_ => true,
            _ => false,
        }
    }
    pub fn is_keyset(&self) -> bool {
        match self {
            VcKind::Keyset => true,
            _ => false,
        }
    }
}
impl Visibility {
    pub fn mk_private() -> Self {
        Visibility::Private
    }
    pub fn mk_public() -> Self {
        Visibility::Public
    }
    pub fn mk_protected() -> Self {
        Visibility::Protected
    }
    pub fn is_private(&self) -> bool {
        match self {
            Visibility::Private => true,
            _ => false,
        }
    }
    pub fn is_public(&self) -> bool {
        match self {
            Visibility::Public => true,
            _ => false,
        }
    }
    pub fn is_protected(&self) -> bool {
        match self {
            Visibility::Protected => true,
            _ => false,
        }
    }
}
impl UseAsVisibility {
    pub fn mk_use_as_public() -> Self {
        UseAsVisibility::UseAsPublic
    }
    pub fn mk_use_as_private() -> Self {
        UseAsVisibility::UseAsPrivate
    }
    pub fn mk_use_as_protected() -> Self {
        UseAsVisibility::UseAsProtected
    }
    pub fn mk_use_as_final() -> Self {
        UseAsVisibility::UseAsFinal
    }
    pub fn is_use_as_public(&self) -> bool {
        match self {
            UseAsVisibility::UseAsPublic => true,
            _ => false,
        }
    }
    pub fn is_use_as_private(&self) -> bool {
        match self {
            UseAsVisibility::UseAsPrivate => true,
            _ => false,
        }
    }
    pub fn is_use_as_protected(&self) -> bool {
        match self {
            UseAsVisibility::UseAsProtected => true,
            _ => false,
        }
    }
    pub fn is_use_as_final(&self) -> bool {
        match self {
            UseAsVisibility::UseAsFinal => true,
            _ => false,
        }
    }
}
impl TypedefVisibility {
    pub fn mk_transparent() -> Self {
        TypedefVisibility::Transparent
    }
    pub fn mk_opaque() -> Self {
        TypedefVisibility::Opaque
    }
    pub fn is_transparent(&self) -> bool {
        match self {
            TypedefVisibility::Transparent => true,
            _ => false,
        }
    }
    pub fn is_opaque(&self) -> bool {
        match self {
            TypedefVisibility::Opaque => true,
            _ => false,
        }
    }
}
