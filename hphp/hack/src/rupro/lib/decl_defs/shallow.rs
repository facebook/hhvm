// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::ty::*;
use crate::reason::Reason;
use pos::{Positioned, Symbol, TypeName};
use std::collections::BTreeMap;

pub use crate::decl_defs::ty::ConstDecl;
pub use oxidized::ast_defs::Visibility;
pub use oxidized_by_ref::{method_flags::MethodFlags, prop_flags::PropFlags};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ShallowClassConst<R: Reason> {
    pub is_abstract: ClassConstKind,
    pub name: Positioned<Symbol, R::Pos>,

    /// This field is used for two different meanings in two different places...
    /// enum class A:arraykey {int X="a";} --
    ///   here X.scc_type=\HH\MemberOf<A,int>
    /// enum B:int as arraykey {X="a"; Y=1; Z=B::X;} --
    ///   here X.scc_type=string, Y.scc_type=int, Z.scc_type=TAny
    /// In the later case, the scc_type is just a simple syntactic attempt to retrieve
    /// the type from the initializer.
    pub ty: DeclTy<R>,

    /// This is a list of all scope-resolution operators "A::B" that are mentioned
    /// in the const initializer, for members of regular-enums and enum-class-enums
    /// to detect circularity of initializers. We don't yet have a similar mechanism
    /// for top-level const initializers.
    pub refs: Vec<ClassConstRef>,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ShallowTypeconst<R: Reason> {
    pub name: Positioned<Symbol, R::Pos>,
    pub kind: Typeconst<R>,
    pub enforceable: (R::Pos, bool),
    pub reifiable: Option<R::Pos>,
    pub is_ctx: bool,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ShallowProp<R: Reason> {
    pub name: Positioned<Symbol, R::Pos>,
    pub xhp_attr: Option<XhpAttribute>,
    pub ty: Option<DeclTy<R>>,
    pub visibility: Visibility,
    pub flags: PropFlags,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ShallowMethod<R: Reason> {
    // note(sf, 2022-01-27):
    //   - c.f.
    //     - `Shallow_decl_defs.shallow_method`
    //     - `oxidized_by_ref::shallow_decl_defs::ShallowMethod<'_>`
    pub name: Positioned<Symbol, R::Pos>,
    pub ty: DeclTy<R>,
    pub visibility: Visibility,
    pub deprecated: Option<intern::string::BytesId>, // e.g. "The method foo is deprecated: ..."
    pub flags: MethodFlags,
    pub attributes: Vec<UserAttribute<R::Pos>>,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ShallowClass<R: Reason> {
    // note(sf, 2022-01-27):
    //  - c.f.
    //    - `Shallow_decl_defs.shallow_class`
    //    - `oxidized_by_ref::shallow_decl_defs::ShallowClass<'_>`
    pub mode: oxidized::file_info::Mode,
    pub is_final: bool,
    pub is_abstract: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassishKind,
    pub module: Option<Positioned<Symbol, R::Pos>>,
    pub name: Positioned<TypeName, R::Pos>,
    pub tparams: Vec<Tparam<R, DeclTy<R>>>,
    pub where_constraints: Vec<WhereConstraint<DeclTy<R>>>,
    pub extends: Vec<DeclTy<R>>,
    pub uses: Vec<DeclTy<R>>,
    pub xhp_attr_uses: Vec<DeclTy<R>>,
    pub xhp_enum_values: BTreeMap<Symbol, Vec<XhpEnumValue>>,
    pub req_extends: Vec<DeclTy<R>>,
    pub req_implements: Vec<DeclTy<R>>,
    pub implements: Vec<DeclTy<R>>,
    pub support_dynamic_type: bool,
    pub consts: Vec<ShallowClassConst<R>>,
    pub typeconsts: Vec<ShallowTypeconst<R>>,
    pub props: Vec<ShallowProp<R>>,
    pub static_props: Vec<ShallowProp<R>>,
    pub constructor: Option<ShallowMethod<R>>,
    pub static_methods: Vec<ShallowMethod<R>>,
    pub methods: Vec<ShallowMethod<R>>,
    pub user_attributes: Vec<UserAttribute<R::Pos>>,
    pub enum_type: Option<EnumType<R>>,
}

// TODO: Rename to FunDecl for consistency with OCaml and the other definitions
// here. There's no such thing as a folded function decl, so "ShallowFun" is a
// little misleading.
pub type ShallowFun<R> = FunElt<R>;

pub type ClassDecl<R> = ShallowClass<R>;

pub type TypedefDecl<R> = TypedefType<R>;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
#[repr(C)]
pub enum Decl<R: Reason> {
    Class(ClassDecl<R>),
    Fun(ShallowFun<R>),
    Typedef(TypedefDecl<R>),
    Const(ConstDecl<R>),
}
