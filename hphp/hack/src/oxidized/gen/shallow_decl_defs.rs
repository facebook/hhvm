// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a444e6fbef797706636a4c5193b730a0>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::ConstDecl;
pub use typing_defs::*;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "scc_")]
#[repr(C)]
pub struct ShallowClassConst {
    pub abstract_: typing_defs::ClassConstKind,
    pub name: typing_defs::PosId,
    /// This field is used for two different meanings in two different places...
    /// enum class A:arraykey {int X="a";} -- here X.scc_type=\HH\MemberOf<A,int>
    /// enum B:int as arraykey {X="a"; Y=1; Z=B::X;} -- here X.scc_type=string, Y.scc_type=int, Z.scc_type=TAny
    /// In the later case, the scc_type is just a simple syntactic attempt to retrieve the type from the initializer.
    pub type_: Ty,
    /// This is a list of all scope-resolution operators "A::B" that are mentioned in the const initializer,
    /// for members of regular-enums and enum-class-enums to detect circularity of initializers.
    /// We don't yet have a similar mechanism for top-level const initializers.
    pub refs: Vec<typing_defs::ClassConstRef>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "stc_")]
#[repr(C)]
pub struct ShallowTypeconst {
    pub name: typing_defs::PosId,
    pub kind: typing_defs::Typeconst,
    pub enforceable: (pos_or_decl::PosOrDecl, bool),
    pub reifiable: Option<pos_or_decl::PosOrDecl>,
    pub is_ctx: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "sp_")]
#[repr(C)]
pub struct ShallowProp {
    pub name: typing_defs::PosId,
    pub xhp_attr: Option<XhpAttr>,
    pub type_: Ty,
    pub visibility: ast_defs::Visibility,
    pub flags: prop_flags::PropFlags,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "sm_")]
#[repr(C)]
pub struct ShallowMethod {
    pub name: typing_defs::PosId,
    pub type_: Ty,
    pub visibility: ast_defs::Visibility,
    pub deprecated: Option<String>,
    pub flags: method_flags::MethodFlags,
    pub attributes: Vec<UserAttribute>,
    pub sort_text: Option<String>,
}

#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type XhpEnumValues = s_map::SMap<Vec<ast_defs::XhpEnumValue>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "sc_")]
#[repr(C)]
pub struct ShallowClass {
    pub mode: file_info::Mode,
    pub final_: bool,
    pub abstract_: bool,
    pub is_xhp: bool,
    pub internal: bool,
    pub has_xhp_keyword: bool,
    pub kind: ast_defs::ClassishKind,
    pub module: Option<ast_defs::Id>,
    pub name: typing_defs::PosId,
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    pub extends: Vec<Ty>,
    pub uses: Vec<Ty>,
    pub xhp_attr_uses: Vec<Ty>,
    pub xhp_enum_values: s_map::SMap<Vec<ast_defs::XhpEnumValue>>,
    pub xhp_marked_empty: bool,
    pub req_extends: Vec<Ty>,
    pub req_implements: Vec<Ty>,
    pub req_class: Vec<Ty>,
    pub implements: Vec<Ty>,
    pub support_dynamic_type: bool,
    pub consts: Vec<ShallowClassConst>,
    pub typeconsts: Vec<ShallowTypeconst>,
    pub props: Vec<ShallowProp>,
    pub sprops: Vec<ShallowProp>,
    pub constructor: Option<ShallowMethod>,
    pub static_methods: Vec<ShallowMethod>,
    pub methods: Vec<ShallowMethod>,
    pub user_attributes: Vec<UserAttribute>,
    pub enum_type: Option<EnumType>,
    pub docs_url: Option<String>,
}

#[rust_to_ocaml(attr = "deriving show")]
pub type FunDecl = FunElt;

#[rust_to_ocaml(attr = "deriving show")]
pub type ClassDecl = ShallowClass;

#[rust_to_ocaml(attr = "deriving show")]
pub type TypedefDecl = TypedefType;

#[rust_to_ocaml(attr = "deriving show")]
pub type ModuleDecl = typing_defs::ModuleDefType;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum Decl {
    Class(ClassDecl),
    Fun(FunDecl),
    Typedef(TypedefDecl),
    Const(ConstDecl),
    Module(ModuleDecl),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum NamedDecl {
    NClass(String, ClassDecl),
    NFun(String, FunDecl),
    NTypedef(String, TypedefDecl),
    NConst(String, ConstDecl),
    NModule(String, ModuleDecl),
}
