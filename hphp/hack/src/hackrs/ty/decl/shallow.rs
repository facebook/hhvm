// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use eq_modulo_pos::EqModuloPos;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
pub use oxidized::ast_defs::Visibility;
pub use oxidized_by_ref::method_flags::MethodFlags;
pub use oxidized_by_ref::prop_flags::PropFlags;
use pos::Bytes;
use pos::ClassConstName;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::ModuleName;
use pos::Positioned;
use pos::PropName;
use pos::Symbol;
use pos::TypeConstName;
use pos::TypeName;
use serde::Deserialize;
use serde::Serialize;

use crate::decl::ty::ClassConstKind;
use crate::decl::ty::ClassConstRef;
pub use crate::decl::ty::ConstDecl;
use crate::decl::ty::Enforceable;
use crate::decl::ty::EnumType;
use crate::decl::ty::FunElt;
use crate::decl::ty::ModuleDefType;
use crate::decl::ty::Tag;
use crate::decl::ty::Tparam;
use crate::decl::ty::Ty;
use crate::decl::ty::Typeconst;
use crate::decl::ty::TypedefType;
use crate::decl::ty::UserAttribute;
use crate::decl::ty::WhereConstraint;
use crate::decl::ty::XhpAttribute;
use crate::decl::ty::XhpEnumValue;
use crate::reason::Reason;

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
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

    /// If DeclParserConfig option include_assignment_values is true,
    /// The string value for the constant
    pub value: Option<String>,
}

walkable!(ShallowClassConst<R> => [ty]);

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub struct ShallowTypeconst<R: Reason> {
    pub name: Positioned<TypeConstName, R::Pos>,
    pub kind: Typeconst<R>,
    pub enforceable: Enforceable<R::Pos>,
    pub reifiable: Option<R::Pos>, // When Some, points to __Reifiable attribute
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
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub struct ShallowProp<R: Reason> {
    pub name: Positioned<PropName, R::Pos>,
    pub xhp_attr: Option<XhpAttribute>,
    pub ty: Ty<R>,
    pub visibility: Visibility,
    pub flags: PropFlags,
}

walkable!(ShallowProp<R> => [ty]);

impl<R: Reason> ShallowProp<R> {
    pub fn is_required_xhp_attribute(&self) -> bool {
        match self.xhp_attr {
            None => false,
            Some(attr) => match attr.tag {
                None => false,
                Some(tag) => match tag {
                    Tag::Required => true,
                    Tag::LateInit => false,
                },
            },
        }
    }
}

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
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
    pub sort_text: Option<String>,
}

walkable!(ShallowMethod<R> => [ty]);

#[derive(Clone, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub struct ShallowClass<R: Reason> {
    // note(sf, 2022-01-27):
    //  - c.f.
    //    - `Shallow_decl_defs.shallow_class`
    //    - `oxidized_by_ref::shallow_decl_defs::ShallowClass<'_>`
    pub mode: oxidized::file_info::Mode,
    pub is_final: bool,
    pub is_abstract: bool,
    pub is_xhp: bool,
    pub is_internal: bool,
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
    pub xhp_marked_empty: bool,
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
    pub docs_url: Option<String>,
}

walkable!(ShallowClass<R> as visit_shallow_class => [
    mode, is_final, is_abstract, is_xhp, is_internal, has_xhp_keyword, kind,
    module, name, tparams, where_constraints, extends, uses, xhp_attr_uses,
    xhp_enum_values, req_extends, req_implements, req_class, implements,
    support_dynamic_type, consts, typeconsts, props, static_props, constructor,
    static_methods, methods, user_attributes, enum_type, docs_url
]);

pub type FunDecl<R> = FunElt<R>;

pub type ClassDecl<R> = ShallowClass<R>;

pub type TypedefDecl<R> = TypedefType<R>;

pub type ModuleDecl<R> = ModuleDefType<R>;

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub enum NamedDecl<R: Reason> {
    Class(TypeName, ClassDecl<R>),
    Fun(FunName, FunDecl<R>),
    Typedef(TypeName, TypedefDecl<R>),
    Const(ConstName, ConstDecl<R>),
    Module(ModuleName, ModuleDecl<R>),
}

walkable!(NamedDecl<R> as visit_named_decl => {
    Self::Class(_, x) => [x],
    Self::Fun(_, x) => [x],
    Self::Typedef(_, x) => [x],
    Self::Const(_, x) => [x],
    Self::Module(_, x) =>  [x],
});

impl<R: Reason> NamedDecl<R> {
    pub fn name(&self) -> Symbol {
        match self {
            Self::Class(name, _) => name.as_symbol(),
            Self::Fun(name, _) => name.as_symbol(),
            Self::Typedef(name, _) => name.as_symbol(),
            Self::Const(name, _) => name.as_symbol(),
            Self::Module(name, _) => name.as_symbol(),
        }
    }

    pub fn name_kind(&self) -> oxidized::naming_types::NameKind {
        use oxidized::naming_types::KindOfType;
        use oxidized::naming_types::NameKind;
        match self {
            Self::Class(..) => NameKind::TypeKind(KindOfType::TClass),
            Self::Typedef(..) => NameKind::TypeKind(KindOfType::TTypedef),
            Self::Fun(..) => NameKind::FunKind,
            Self::Const(..) => NameKind::ConstKind,
            Self::Module(..) => NameKind::ModuleKind,
        }
    }
}

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub enum Decl<R: Reason> {
    Class(ClassDecl<R>),
    Fun(FunDecl<R>),
    Typedef(TypedefDecl<R>),
    Const(ConstDecl<R>),
    Module(ModuleDecl<R>),
}

walkable!(Decl<R> as visit_decl => {
    Self::Class(x) => [x],
    Self::Fun(x) => [x],
    Self::Typedef(x) => [x],
    Self::Const(x) => [x],
    Self::Module(x) => [x],
});

impl<R: Reason> Decl<R> {
    pub fn name_kind(&self) -> oxidized::naming_types::NameKind {
        use oxidized::naming_types::KindOfType;
        use oxidized::naming_types::NameKind;
        match self {
            Self::Class(..) => NameKind::TypeKind(KindOfType::TClass),
            Self::Typedef(..) => NameKind::TypeKind(KindOfType::TTypedef),
            Self::Fun(..) => NameKind::FunKind,
            Self::Const(..) => NameKind::ConstKind,
            Self::Module(..) => NameKind::ModuleKind,
        }
    }
}
