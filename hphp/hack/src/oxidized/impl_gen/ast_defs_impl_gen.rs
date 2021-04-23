// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<f52f8164f7f5efcb0299276148c33ae7>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use crate::ast_defs::*;
impl ShapeFieldName {
    pub fn mk_sflit_int(p0: Pstring) -> Self {
        ShapeFieldName::SFlitInt(p0)
    }
    pub fn mk_sflit_str(p0: PositionedByteString) -> Self {
        ShapeFieldName::SFlitStr(p0)
    }
    pub fn mk_sfclass_const(p0: Id, p1: Pstring) -> Self {
        ShapeFieldName::SFclassConst(p0, p1)
    }
    pub fn is_sflit_int(&self) -> bool {
        match self {
            ShapeFieldName::SFlitInt(..) => true,
            _ => false,
        }
    }
    pub fn is_sflit_str(&self) -> bool {
        match self {
            ShapeFieldName::SFlitStr(..) => true,
            _ => false,
        }
    }
    pub fn is_sfclass_const(&self) -> bool {
        match self {
            ShapeFieldName::SFclassConst(..) => true,
            _ => false,
        }
    }
    pub fn as_sflit_int(&self) -> Option<&Pstring> {
        match self {
            ShapeFieldName::SFlitInt(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_sflit_str(&self) -> Option<&PositionedByteString> {
        match self {
            ShapeFieldName::SFlitStr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_sfclass_const(&self) -> Option<(&Id, &Pstring)> {
        match self {
            ShapeFieldName::SFclassConst(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_sflit_int_mut(&mut self) -> Option<&mut Pstring> {
        match self {
            ShapeFieldName::SFlitInt(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_sflit_str_mut(&mut self) -> Option<&mut PositionedByteString> {
        match self {
            ShapeFieldName::SFlitStr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_sfclass_const_mut(&mut self) -> Option<(&mut Id, &mut Pstring)> {
        match self {
            ShapeFieldName::SFclassConst(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_sflit_int_into(self) -> Option<Pstring> {
        match self {
            ShapeFieldName::SFlitInt(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_sflit_str_into(self) -> Option<PositionedByteString> {
        match self {
            ShapeFieldName::SFlitStr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_sfclass_const_into(self) -> Option<(Id, Pstring)> {
        match self {
            ShapeFieldName::SFclassConst(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
}
impl Variance {
    pub fn mk_covariant() -> Self {
        Variance::Covariant
    }
    pub fn mk_contravariant() -> Self {
        Variance::Contravariant
    }
    pub fn mk_invariant() -> Self {
        Variance::Invariant
    }
    pub fn is_covariant(&self) -> bool {
        match self {
            Variance::Covariant => true,
            _ => false,
        }
    }
    pub fn is_contravariant(&self) -> bool {
        match self {
            Variance::Contravariant => true,
            _ => false,
        }
    }
    pub fn is_invariant(&self) -> bool {
        match self {
            Variance::Invariant => true,
            _ => false,
        }
    }
}
impl ConstraintKind {
    pub fn mk_constraint_as() -> Self {
        ConstraintKind::ConstraintAs
    }
    pub fn mk_constraint_eq() -> Self {
        ConstraintKind::ConstraintEq
    }
    pub fn mk_constraint_super() -> Self {
        ConstraintKind::ConstraintSuper
    }
    pub fn is_constraint_as(&self) -> bool {
        match self {
            ConstraintKind::ConstraintAs => true,
            _ => false,
        }
    }
    pub fn is_constraint_eq(&self) -> bool {
        match self {
            ConstraintKind::ConstraintEq => true,
            _ => false,
        }
    }
    pub fn is_constraint_super(&self) -> bool {
        match self {
            ConstraintKind::ConstraintSuper => true,
            _ => false,
        }
    }
}
impl ClassKind {
    pub fn mk_cabstract() -> Self {
        ClassKind::Cabstract
    }
    pub fn mk_cnormal() -> Self {
        ClassKind::Cnormal
    }
    pub fn mk_cinterface() -> Self {
        ClassKind::Cinterface
    }
    pub fn mk_ctrait() -> Self {
        ClassKind::Ctrait
    }
    pub fn mk_cenum() -> Self {
        ClassKind::Cenum
    }
    pub fn is_cabstract(&self) -> bool {
        match self {
            ClassKind::Cabstract => true,
            _ => false,
        }
    }
    pub fn is_cnormal(&self) -> bool {
        match self {
            ClassKind::Cnormal => true,
            _ => false,
        }
    }
    pub fn is_cinterface(&self) -> bool {
        match self {
            ClassKind::Cinterface => true,
            _ => false,
        }
    }
    pub fn is_ctrait(&self) -> bool {
        match self {
            ClassKind::Ctrait => true,
            _ => false,
        }
    }
    pub fn is_cenum(&self) -> bool {
        match self {
            ClassKind::Cenum => true,
            _ => false,
        }
    }
}
impl ParamKind {
    pub fn mk_pinout() -> Self {
        ParamKind::Pinout
    }
    pub fn is_pinout(&self) -> bool {
        true
    }
}
impl ReadonlyKind {
    pub fn mk_readonly() -> Self {
        ReadonlyKind::Readonly
    }
    pub fn is_readonly(&self) -> bool {
        true
    }
}
impl OgNullFlavor {
    pub fn mk_ognullthrows() -> Self {
        OgNullFlavor::OGNullthrows
    }
    pub fn mk_ognullsafe() -> Self {
        OgNullFlavor::OGNullsafe
    }
    pub fn is_ognullthrows(&self) -> bool {
        match self {
            OgNullFlavor::OGNullthrows => true,
            _ => false,
        }
    }
    pub fn is_ognullsafe(&self) -> bool {
        match self {
            OgNullFlavor::OGNullsafe => true,
            _ => false,
        }
    }
}
impl FunKind {
    pub fn mk_fsync() -> Self {
        FunKind::FSync
    }
    pub fn mk_fasync() -> Self {
        FunKind::FAsync
    }
    pub fn mk_fgenerator() -> Self {
        FunKind::FGenerator
    }
    pub fn mk_fasync_generator() -> Self {
        FunKind::FAsyncGenerator
    }
    pub fn is_fsync(&self) -> bool {
        match self {
            FunKind::FSync => true,
            _ => false,
        }
    }
    pub fn is_fasync(&self) -> bool {
        match self {
            FunKind::FAsync => true,
            _ => false,
        }
    }
    pub fn is_fgenerator(&self) -> bool {
        match self {
            FunKind::FGenerator => true,
            _ => false,
        }
    }
    pub fn is_fasync_generator(&self) -> bool {
        match self {
            FunKind::FAsyncGenerator => true,
            _ => false,
        }
    }
}
impl Bop {
    pub fn mk_plus() -> Self {
        Bop::Plus
    }
    pub fn mk_minus() -> Self {
        Bop::Minus
    }
    pub fn mk_star() -> Self {
        Bop::Star
    }
    pub fn mk_slash() -> Self {
        Bop::Slash
    }
    pub fn mk_eqeq() -> Self {
        Bop::Eqeq
    }
    pub fn mk_eqeqeq() -> Self {
        Bop::Eqeqeq
    }
    pub fn mk_starstar() -> Self {
        Bop::Starstar
    }
    pub fn mk_diff() -> Self {
        Bop::Diff
    }
    pub fn mk_diff2() -> Self {
        Bop::Diff2
    }
    pub fn mk_ampamp() -> Self {
        Bop::Ampamp
    }
    pub fn mk_barbar() -> Self {
        Bop::Barbar
    }
    pub fn mk_lt() -> Self {
        Bop::Lt
    }
    pub fn mk_lte() -> Self {
        Bop::Lte
    }
    pub fn mk_gt() -> Self {
        Bop::Gt
    }
    pub fn mk_gte() -> Self {
        Bop::Gte
    }
    pub fn mk_dot() -> Self {
        Bop::Dot
    }
    pub fn mk_amp() -> Self {
        Bop::Amp
    }
    pub fn mk_bar() -> Self {
        Bop::Bar
    }
    pub fn mk_ltlt() -> Self {
        Bop::Ltlt
    }
    pub fn mk_gtgt() -> Self {
        Bop::Gtgt
    }
    pub fn mk_percent() -> Self {
        Bop::Percent
    }
    pub fn mk_xor() -> Self {
        Bop::Xor
    }
    pub fn mk_cmp() -> Self {
        Bop::Cmp
    }
    pub fn mk_question_question() -> Self {
        Bop::QuestionQuestion
    }
    pub fn mk_eq(p0: Option<Box<Bop>>) -> Self {
        Bop::Eq(p0)
    }
    pub fn is_plus(&self) -> bool {
        match self {
            Bop::Plus => true,
            _ => false,
        }
    }
    pub fn is_minus(&self) -> bool {
        match self {
            Bop::Minus => true,
            _ => false,
        }
    }
    pub fn is_star(&self) -> bool {
        match self {
            Bop::Star => true,
            _ => false,
        }
    }
    pub fn is_slash(&self) -> bool {
        match self {
            Bop::Slash => true,
            _ => false,
        }
    }
    pub fn is_eqeq(&self) -> bool {
        match self {
            Bop::Eqeq => true,
            _ => false,
        }
    }
    pub fn is_eqeqeq(&self) -> bool {
        match self {
            Bop::Eqeqeq => true,
            _ => false,
        }
    }
    pub fn is_starstar(&self) -> bool {
        match self {
            Bop::Starstar => true,
            _ => false,
        }
    }
    pub fn is_diff(&self) -> bool {
        match self {
            Bop::Diff => true,
            _ => false,
        }
    }
    pub fn is_diff2(&self) -> bool {
        match self {
            Bop::Diff2 => true,
            _ => false,
        }
    }
    pub fn is_ampamp(&self) -> bool {
        match self {
            Bop::Ampamp => true,
            _ => false,
        }
    }
    pub fn is_barbar(&self) -> bool {
        match self {
            Bop::Barbar => true,
            _ => false,
        }
    }
    pub fn is_lt(&self) -> bool {
        match self {
            Bop::Lt => true,
            _ => false,
        }
    }
    pub fn is_lte(&self) -> bool {
        match self {
            Bop::Lte => true,
            _ => false,
        }
    }
    pub fn is_gt(&self) -> bool {
        match self {
            Bop::Gt => true,
            _ => false,
        }
    }
    pub fn is_gte(&self) -> bool {
        match self {
            Bop::Gte => true,
            _ => false,
        }
    }
    pub fn is_dot(&self) -> bool {
        match self {
            Bop::Dot => true,
            _ => false,
        }
    }
    pub fn is_amp(&self) -> bool {
        match self {
            Bop::Amp => true,
            _ => false,
        }
    }
    pub fn is_bar(&self) -> bool {
        match self {
            Bop::Bar => true,
            _ => false,
        }
    }
    pub fn is_ltlt(&self) -> bool {
        match self {
            Bop::Ltlt => true,
            _ => false,
        }
    }
    pub fn is_gtgt(&self) -> bool {
        match self {
            Bop::Gtgt => true,
            _ => false,
        }
    }
    pub fn is_percent(&self) -> bool {
        match self {
            Bop::Percent => true,
            _ => false,
        }
    }
    pub fn is_xor(&self) -> bool {
        match self {
            Bop::Xor => true,
            _ => false,
        }
    }
    pub fn is_cmp(&self) -> bool {
        match self {
            Bop::Cmp => true,
            _ => false,
        }
    }
    pub fn is_question_question(&self) -> bool {
        match self {
            Bop::QuestionQuestion => true,
            _ => false,
        }
    }
    pub fn is_eq(&self) -> bool {
        match self {
            Bop::Eq(..) => true,
            _ => false,
        }
    }
    pub fn as_eq(&self) -> Option<&Option<Box<Bop>>> {
        match self {
            Bop::Eq(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_eq_mut(&mut self) -> Option<&mut Option<Box<Bop>>> {
        match self {
            Bop::Eq(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_eq_into(self) -> Option<Option<Box<Bop>>> {
        match self {
            Bop::Eq(p0) => Some(p0),
            _ => None,
        }
    }
}
impl Uop {
    pub fn mk_utild() -> Self {
        Uop::Utild
    }
    pub fn mk_unot() -> Self {
        Uop::Unot
    }
    pub fn mk_uplus() -> Self {
        Uop::Uplus
    }
    pub fn mk_uminus() -> Self {
        Uop::Uminus
    }
    pub fn mk_uincr() -> Self {
        Uop::Uincr
    }
    pub fn mk_udecr() -> Self {
        Uop::Udecr
    }
    pub fn mk_upincr() -> Self {
        Uop::Upincr
    }
    pub fn mk_updecr() -> Self {
        Uop::Updecr
    }
    pub fn mk_usilence() -> Self {
        Uop::Usilence
    }
    pub fn is_utild(&self) -> bool {
        match self {
            Uop::Utild => true,
            _ => false,
        }
    }
    pub fn is_unot(&self) -> bool {
        match self {
            Uop::Unot => true,
            _ => false,
        }
    }
    pub fn is_uplus(&self) -> bool {
        match self {
            Uop::Uplus => true,
            _ => false,
        }
    }
    pub fn is_uminus(&self) -> bool {
        match self {
            Uop::Uminus => true,
            _ => false,
        }
    }
    pub fn is_uincr(&self) -> bool {
        match self {
            Uop::Uincr => true,
            _ => false,
        }
    }
    pub fn is_udecr(&self) -> bool {
        match self {
            Uop::Udecr => true,
            _ => false,
        }
    }
    pub fn is_upincr(&self) -> bool {
        match self {
            Uop::Upincr => true,
            _ => false,
        }
    }
    pub fn is_updecr(&self) -> bool {
        match self {
            Uop::Updecr => true,
            _ => false,
        }
    }
    pub fn is_usilence(&self) -> bool {
        match self {
            Uop::Usilence => true,
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
impl XhpEnumValue {
    pub fn mk_xevint(p0: isize) -> Self {
        XhpEnumValue::XEVInt(p0)
    }
    pub fn mk_xevstring(p0: String) -> Self {
        XhpEnumValue::XEVString(p0)
    }
    pub fn is_xevint(&self) -> bool {
        match self {
            XhpEnumValue::XEVInt(..) => true,
            _ => false,
        }
    }
    pub fn is_xevstring(&self) -> bool {
        match self {
            XhpEnumValue::XEVString(..) => true,
            _ => false,
        }
    }
    pub fn as_xevint(&self) -> Option<&isize> {
        match self {
            XhpEnumValue::XEVInt(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xevstring(&self) -> Option<&String> {
        match self {
            XhpEnumValue::XEVString(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xevint_mut(&mut self) -> Option<&mut isize> {
        match self {
            XhpEnumValue::XEVInt(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xevstring_mut(&mut self) -> Option<&mut String> {
        match self {
            XhpEnumValue::XEVString(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xevint_into(self) -> Option<isize> {
        match self {
            XhpEnumValue::XEVInt(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xevstring_into(self) -> Option<String> {
        match self {
            XhpEnumValue::XEVString(p0) => Some(p0),
            _ => None,
        }
    }
}
