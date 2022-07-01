// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl::ty::{
    ClassConstKind, ClassConstRef, EnumType, FunElt, ModuleDefType, Tparam, Ty, Typeconst,
    TypedefType, UserAttribute, WhereConstraint, XhpAttribute, XhpEnumValue,
};
use crate::reason::Reason;
use eq_modulo_pos::EqModuloPos;
use pos::{
    Bytes, ClassConstName, ConstName, FunName, MethodName, ModuleName, Positioned, PropName,
    Symbol, TypeConstName, TypeName,
};
use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;

pub use crate::decl::ty::ConstDecl;
pub use oxidized::ast_defs::Visibility;
pub use oxidized_by_ref::{method_flags::MethodFlags, prop_flags::PropFlags};

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub struct ShallowClassConst<R: Reason> {
    pub kind: ClassConstKind,
    pub name: Positioned<ClassConstName, R::Pos>,

    /// This field is used for two different meanings in two different places:
    ///
    ///   enum class A:arraykey { int X = "a"; }
    ///
    /// In an enum class, X.ty = \HH\MemberOf<A,int>.
    ///
    ///   enum B:int as arraykey { X = "a"; Y = 1; Z = B::X; }
    ///
    /// In a legacy enum, X.ty = string, Y.ty = int, and Z.ty = TAny, and ty is
    /// just a simple syntactic attempt to retrieve the type from the initializer.
    pub ty: Ty<R>,

    /// This is a list of all scope-resolution operators "A::B" that are mentioned
    /// in the const initializer, for members of regular-enums and enum-class-enums
    /// to detect circularity of initializers. We don't yet have a similar mechanism
    /// for top-level const initializers.
    pub refs: Box<[ClassConstRef]>,
}

walkable!(ShallowClassConst<R> => [ty]);

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub struct ShallowTypeconst<R: Reason> {
    pub name: Positioned<TypeConstName, R::Pos>,
    pub kind: Typeconst<R>,
    pub enforceable: Option<R::Pos>, // When Some, points to __Enforceable attribute
    pub reifiable: Option<R::Pos>,   // When Some, points to __Reifiable attribute
    pub is_ctx: bool,
}

walkable!(ShallowTypeconst<R> => [kind]);

impl<R: Reason> ShallowTypeconst<R> {
    pub fn is_enforceable(&self) -> bool {
        self.enforceable.is_some()
    }
    pub fn is_reifiable(&self) -> bool {
        self.reifiable.is_some()
    }
}

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub struct ShallowProp<R: Reason> {
    pub name: Positioned<PropName, R::Pos>,
    pub xhp_attr: Option<XhpAttribute>,
    pub ty: Option<Ty<R>>,
    pub visibility: Visibility,
    pub flags: PropFlags,
}

walkable!(ShallowProp<R> => [ty]);

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub struct ShallowMethod<R: Reason> {
    // note(sf, 2022-01-27):
    //   - c.f.
    //     - `Shallow_decl_defs.shallow_method`
    //     - `oxidized_by_ref::shallow_decl_defs::ShallowMethod<'_>`
    pub name: Positioned<MethodName, R::Pos>,
    pub ty: Ty<R>,
    pub visibility: Visibility,
    pub deprecated: Option<Bytes>, // e.g. "The method foo is deprecated: ..."
    pub flags: MethodFlags,
    pub attributes: Box<[UserAttribute<R::Pos>]>,
}

walkable!(ShallowMethod<R> => [ty]);

#[derive(Clone, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub struct ShallowClass<R: Reason> {
    // note(sf, 2022-01-27):
    //  - c.f.
    //    - `Shallow_decl_defs.shallow_class`
    //    - `oxidized_by_ref::shallow_decl_defs::ShallowClass<'_>`
    pub mode: oxidized::file_info::Mode,
    pub is_final: bool,
    pub is_abstract: bool,
    pub is_internal: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassishKind,
    pub module: Option<Positioned<ModuleName, R::Pos>>,
    pub name: Positioned<TypeName, R::Pos>,
    pub tparams: Box<[Tparam<R, Ty<R>>]>,
    pub where_constraints: Box<[WhereConstraint<Ty<R>>]>,
    pub extends: Box<[Ty<R>]>,
    pub uses: Box<[Ty<R>]>,
    pub xhp_attr_uses: Box<[Ty<R>]>,
    pub xhp_enum_values: BTreeMap<Symbol, Box<[XhpEnumValue]>>,
    pub req_extends: Box<[Ty<R>]>,
    pub req_implements: Box<[Ty<R>]>,
    pub req_class: Box<[Ty<R>]>,
    pub implements: Box<[Ty<R>]>,
    pub support_dynamic_type: bool,
    pub consts: Box<[ShallowClassConst<R>]>,
    pub typeconsts: Box<[ShallowTypeconst<R>]>,
    pub props: Box<[ShallowProp<R>]>,
    pub static_props: Box<[ShallowProp<R>]>,
    pub constructor: Option<ShallowMethod<R>>,
    pub static_methods: Box<[ShallowMethod<R>]>,
    pub methods: Box<[ShallowMethod<R>]>,
    pub user_attributes: Box<[UserAttribute<R::Pos>]>,
    pub enum_type: Option<EnumType<R>>,
}

walkable!(ShallowClass<R> => [
    tparams, where_constraints, extends, uses, xhp_attr_uses, req_extends,
    req_implements, implements, consts, typeconsts, props, static_props,
    constructor, static_methods, methods, enum_type
]);

pub type FunDecl<R> = FunElt<R>;

pub type ClassDecl<R> = ShallowClass<R>;

pub type TypedefDecl<R> = TypedefType<R>;

pub type ModuleDecl<R> = ModuleDefType<R>;

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub enum Decl<R: Reason> {
    Class(TypeName, ClassDecl<R>),
    Fun(FunName, FunDecl<R>),
    Typedef(TypeName, TypedefDecl<R>),
    Const(ConstName, ConstDecl<R>),
    Module(ModuleName, ModuleDecl<R>),
}

walkable!(Decl<R> => {
    Decl::Class(_, x) => [x],
    Decl::Fun(_, x) => [x],
    Decl::Typedef(_, x) => [x],
    Decl::Const(_, x) => [x],
    Decl::Module(_, x) =>  [x],
});

impl<R: Reason> Decl<R> {
    pub fn name(&self) -> Symbol {
        match self {
            Decl::Class(name, _) => name.as_symbol(),
            Decl::Fun(name, _) => name.as_symbol(),
            Decl::Typedef(name, _) => name.as_symbol(),
            Decl::Const(name, _) => name.as_symbol(),
            Decl::Module(name, _) => name.as_symbol(),
        }
    }

    pub fn name_kind(&self) -> oxidized::naming_types::NameKind {
        use oxidized::naming_types::{KindOfType, NameKind};
        match self {
            Decl::Class(..) => NameKind::TypeKind(KindOfType::TClass),
            Decl::Typedef(..) => NameKind::TypeKind(KindOfType::TTypedef),
            Decl::Fun(..) => NameKind::FunKind,
            Decl::Const(..) => NameKind::ConstKind,
            Decl::Module(..) => NameKind::ModuleKind,
        }
    }
}
