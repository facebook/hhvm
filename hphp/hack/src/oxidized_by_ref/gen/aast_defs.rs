// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<53eb0fd7f1006b8201134d0081e11d44>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
pub use ast_defs::OgNullFlavor;
pub use ast_defs::Pos;
pub use ast_defs::PositionedByteString;
pub use ast_defs::PropOrMethod;
pub use ast_defs::Pstring;
use eq_modulo_pos::EqModuloPos;
use eq_modulo_pos::EqModuloPosAndReason;
pub use local_id::LocalId;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
pub use oxidized::aast_defs::Visibility;
use serde::Deserialize;
use serde::Serialize;
pub use shape_map::ShapeMap;

pub use crate::ast_defs::shape_map;
#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct Lid<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a LocalId<'a>,
);
impl<'a> TrivialDrop for Lid<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Lid<'arena>);

pub type Sid<'a> = ast_defs::Id<'a>;

pub type ClassName<'a> = Sid<'a>;

pub use oxidized::aast_defs::ImportFlavor;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum XhpChild<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ChildName(&'a Sid<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ChildList(&'a [XhpChild<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    ChildUnary(&'a (XhpChild<'a>, oxidized::aast_defs::XhpChildOp)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    ChildBinary(&'a (XhpChild<'a>, XhpChild<'a>)),
}
impl<'a> TrivialDrop for XhpChild<'a> {}
arena_deserializer::impl_deserialize_in_arena!(XhpChild<'arena>);

pub use oxidized::aast_defs::XhpChildOp;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct Hint<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint_<'a>,
);
impl<'a> TrivialDrop for Hint<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Hint<'arena>);

pub type VariadicHint<'a> = Option<&'a Hint<'a>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct Contexts<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a [&'a Hint<'a>],
);
impl<'a> TrivialDrop for Contexts<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Contexts<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(prefix = "hfparam_")]
#[repr(C)]
pub struct HfParamInfo<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: ast_defs::ParamKind<'a>,
    pub readonlyness: Option<oxidized::ast_defs::ReadonlyKind>,
}
impl<'a> TrivialDrop for HfParamInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(HfParamInfo<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(prefix = "hf_")]
#[repr(C)]
pub struct HintFun<'a> {
    pub is_readonly: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub param_tys: &'a [&'a Hint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub param_info: &'a [Option<&'a HfParamInfo<'a>>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub variadic_ty: &'a VariadicHint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub return_ty: &'a Hint<'a>,
    pub is_readonly_return: Option<oxidized::ast_defs::ReadonlyKind>,
}
impl<'a> TrivialDrop for HintFun<'a> {}
arena_deserializer::impl_deserialize_in_arena!(HintFun<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum Hint_<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hoption(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hlike(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hfun(&'a HintFun<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Htuple(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Happly(&'a (&'a ClassName<'a>, &'a [&'a Hint<'a>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hshape(&'a NastShapeInfo<'a>),
    /// This represents the use of a type const. Type consts are accessed like
    /// regular consts in Hack, i.e.
    ///
    /// [$x | self | static | Class]::TypeConst
    ///
    /// Class  => Happly "Class"
    /// self   => Happly of the class of definition
    /// static => Habstr ("static",
    ///           Habstr ("this", (Constraint_as, Happly of class of definition)))
    /// $x     => Hvar "$x"
    ///
    /// Type const access can be chained such as
    ///
    /// Class::TC1::TC2::TC3
    ///
    /// We resolve the root of the type access chain as a type as follows.
    ///
    /// This will result in the following representation
    ///
    /// Haccess (Happly "Class", ["TC1", "TC2", "TC3"])
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Haccess(&'a (&'a Hint<'a>, &'a [Sid<'a>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hsoft(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Hrefinement(&'a (&'a Hint<'a>, &'a [Refinement<'a>])),
    Hany,
    Herr,
    Hmixed,
    Hnonnull,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Habstr(&'a (&'a str, &'a [&'a Hint<'a>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Hvec_or_dict")]
    #[rust_to_ocaml(inline_tuple)]
    HvecOrDict(&'a (Option<&'a Hint<'a>>, &'a Hint<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hprim(&'a oxidized::aast_defs::Tprim),
    Hthis,
    Hdynamic,
    Hnothing,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hunion(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hintersection(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Hfun_context")]
    HfunContext(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hvar(&'a str),
}
impl<'a> TrivialDrop for Hint_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Hint_<'arena>);

pub use oxidized::aast_defs::Tprim;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum Refinement<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Rctx(&'a (Sid<'a>, CtxRefinement<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Rtype(&'a (Sid<'a>, TypeRefinement<'a>)),
}
impl<'a> TrivialDrop for Refinement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Refinement<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum TypeRefinement<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TRexact(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TRloose(&'a TypeRefinementBounds<'a>),
}
impl<'a> TrivialDrop for TypeRefinement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypeRefinement<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(prefix = "tr_")]
#[repr(C)]
pub struct TypeRefinementBounds<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub lower: &'a [&'a Hint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub upper: &'a [&'a Hint<'a>],
}
impl<'a> TrivialDrop for TypeRefinementBounds<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypeRefinementBounds<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum CtxRefinement<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CRexact(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CRloose(&'a CtxRefinementBounds<'a>),
}
impl<'a> TrivialDrop for CtxRefinement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CtxRefinement<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(prefix = "cr_")]
#[repr(C)]
pub struct CtxRefinementBounds<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub lower: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub upper: Option<&'a Hint<'a>>,
}
impl<'a> TrivialDrop for CtxRefinementBounds<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CtxRefinementBounds<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(prefix = "sfi_")]
#[repr(C)]
pub struct ShapeFieldInfo<'a> {
    pub optional: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub hint: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: ast_defs::ShapeFieldName<'a>,
}
impl<'a> TrivialDrop for ShapeFieldInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeFieldInfo<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(prefix = "nsi_")]
#[repr(C)]
pub struct NastShapeInfo<'a> {
    pub allows_unknown_fields: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub field_map: &'a [&'a ShapeFieldInfo<'a>],
}
impl<'a> TrivialDrop for NastShapeInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(NastShapeInfo<'arena>);

pub use oxidized::aast_defs::KvcKind;
pub use oxidized::aast_defs::TypedefVisibility;
pub use oxidized::aast_defs::VcKind;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(prefix = "e_")]
#[repr(C)]
pub struct Enum_<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub base: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub includes: &'a [&'a Hint<'a>],
}
impl<'a> TrivialDrop for Enum_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Enum_<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct WhereConstraintHint<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint<'a>,
    pub oxidized::ast_defs::ConstraintKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint<'a>,
);
impl<'a> TrivialDrop for WhereConstraintHint<'a> {}
arena_deserializer::impl_deserialize_in_arena!(WhereConstraintHint<'arena>);

pub use oxidized::aast_defs::ReifyKind;
