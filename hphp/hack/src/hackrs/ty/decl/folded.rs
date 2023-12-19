// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::fmt;

use eq_modulo_pos::EqModuloPos;
use hash::IndexMap;
use hash::IndexSet;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
pub use oxidized::ast_defs::Abstraction;
pub use oxidized::ast_defs::ClassishKind;
use pos::Bytes;
use pos::ClassConstName;
use pos::MethodName;
use pos::ModuleName;
use pos::Positioned;
use pos::PropName;
use pos::Symbol;
use pos::TypeConstName;
use pos::TypeName;
use serde::Deserialize;
use serde::Serialize;

pub use crate::decl::subst::Subst;
use crate::decl::ty::ConsistentKind;
use crate::decl::ty::Enforceable;
use crate::decl::ty::XhpEnumValue;
use crate::decl::CeVisibility;
use crate::decl::ClassConstKind;
use crate::decl::ClassConstRef;
use crate::decl::ClassEltFlags;
use crate::decl::EnumType;
use crate::decl::Tparam;
use crate::decl::Ty;
use crate::decl::Typeconst;
use crate::decl::WhereConstraint;
use crate::decl::XhpAttribute;
use crate::decl_error::DeclError;
use crate::reason::Reason;

#[derive(Debug, Clone, Eq, EqModuloPos, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub struct FoldedElement {
    // note(sf, 2022-01-28): c.f. `Decl_defs.element`
    pub flags: ClassEltFlags,
    pub origin: TypeName,
    pub visibility: CeVisibility,

    /// If the element is deprecated, this holds the deprecation message.
    pub deprecated: Option<Bytes>,
    pub sort_text: Option<String>,
}

/// A substitution context contains all the information necessary for changing
/// the type of an inherited class element to the class that is inheriting the
/// class element. It's best illustrated via an example.
/// ```
/// class A<Ta1, Ta2> { public function test(Ta1 $x, Ta2 $y): void {} }
/// class B<Tb> extends A<Tb, int> {}
/// class C extends B<string> {}
/// ```

/// The method `A::test()` has the type `(function(Ta1, Ta2): void)` in the
/// context of class `A`. However in the context of class `B`, it will have type
/// `(function(Tb, int): void)`.
///
/// The substitution that leads to this change is [Ta1 -> Tb, Ta2 -> int], which
/// will produce a new type in the context of class B. It's subst_context would
/// then be:
///
/// ```
/// { subst            = [Ta1 -> Tb, Ta2 -> int];
///   class_context    = 'B';
///   from_req_extends = false;
/// }
/// ```
///
/// The `from_req_extends` field is set to` true` if the context was inherited
/// via a require extends type. This information is relevant when folding
/// `substs` during inheritance. See the `inherit` module.
#[derive(Debug, Clone, Eq, EqModuloPos, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub struct SubstContext<R: Reason> {
    // note(sf, 2022-01-28): c.f. `Decl_defs.subst_context`
    pub subst: Subst<R>,
    pub class_context: TypeName,
    pub from_req_extends: bool,
}

impl<R: Reason> SubstContext<R> {
    pub fn set_from_req_extends(&mut self, p: bool) {
        self.from_req_extends = p;
    }
}

#[derive(Debug, Clone, Eq, EqModuloPos, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub struct TypeConst<R: Reason> {
    // note(sf, 2022-02-08): c.f. `Typing_defs.typeconst_type`
    pub is_synthesized: bool,
    pub name: Positioned<TypeConstName, R::Pos>,
    pub kind: Typeconst<R>, // abstract or concrete
    pub origin: TypeName,
    pub enforceable: Enforceable<R::Pos>,
    pub reifiable: Option<R::Pos>, // When Some, points to __Reifiable attribute
    pub is_concretized: bool,
    pub is_ctx: bool,
}

impl<R: Reason> TypeConst<R> {
    pub fn is_enforceable(&self) -> bool {
        self.enforceable.is_some()
    }
    pub fn is_reifiable(&self) -> bool {
        self.reifiable.is_some()
    }
}

#[derive(Debug, Clone, Eq, EqModuloPos, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub struct ClassConst<R: Reason> {
    // note(sf, 2022-02-08): c.f. `Typing_defs.class_const`
    pub is_synthesized: bool,
    pub kind: ClassConstKind,
    pub pos: R::Pos,
    pub ty: Ty<R>,
    pub origin: TypeName, // Identifies the class from which this const originates
    pub refs: Box<[ClassConstRef]>,
}

impl<R: Reason> ClassConst<R> {
    pub fn set_is_synthesized(&mut self, p: bool) {
        self.is_synthesized = p;
    }
}

impl<R: Reason> TypeConst<R> {
    pub fn set_is_synthesized(&mut self, p: bool) {
        self.is_synthesized = p;
    }
}

/// The position is that of the hint in the `use` / `implements` AST node
/// that causes a class to have this requirement applied to it. E.g.
///
/// ```
/// class Foo {}
///
/// interface Bar {
///   require extends Foo; <- position of the decl_phase ty
/// }
///
/// class Baz extends Foo implements Bar { <- position of the `implements`
/// }
/// ```
#[derive(Clone, Eq, EqModuloPos, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub struct Requirement<R: Reason> {
    pub pos: R::Pos,
    pub ty: Ty<R>,
}

#[derive(Clone, Debug, Eq, EqModuloPos, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub struct Constructor {
    pub elt: Option<FoldedElement>,
    pub consistency: ConsistentKind,
}

#[derive(Clone, Eq, EqModuloPos, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub struct FoldedClass<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Decl_defs.decl_class_type`
    pub name: TypeName,
    pub pos: R::Pos,
    pub kind: ClassishKind,
    /// A final type cannot have children. Declared as `e.g. final class C {}`
    pub is_final: bool,
    /// Whether the type has attribute __Const, meaning all properties
    /// are immutable.
    pub is_const: bool,
    /// Whether internal to a module, i.e. declared as e.g. `internal class C {}`
    pub is_internal: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub support_dynamic_type: bool,
    pub enum_type: Option<EnumType<R>>,
    pub module: Option<Positioned<ModuleName, R::Pos>>,
    pub is_module_level_trait: bool,
    pub tparams: Box<[Tparam<R, Ty<R>>]>,
    pub where_constraints: Box<[WhereConstraint<Ty<R>>]>,
    pub substs: IndexMap<TypeName, SubstContext<R>>,
    pub ancestors: IndexMap<TypeName, Ty<R>>,
    pub props: IndexMap<PropName, FoldedElement>,
    pub static_props: IndexMap<PropName, FoldedElement>,
    pub methods: IndexMap<MethodName, FoldedElement>,
    pub static_methods: IndexMap<MethodName, FoldedElement>,
    pub constructor: Constructor,
    pub consts: IndexMap<ClassConstName, ClassConst<R>>,
    pub type_consts: IndexMap<TypeConstName, TypeConst<R>>,
    pub xhp_enum_values: BTreeMap<Symbol, Box<[XhpEnumValue]>>,
    pub xhp_marked_empty: bool,
    pub extends: IndexSet<TypeName>,
    pub xhp_attr_deps: IndexSet<TypeName>,
    pub req_ancestors: Box<[Requirement<R>]>,
    pub req_ancestors_extends: IndexSet<TypeName>,
    /// `req_class_ancestors` gathers all the `require class` requirements
    /// declared in self and ancestors. Note that `require class` requirements
    /// are _not_ stored in `req_ancestors` or `req_ancestors_extends` fields.
    pub req_class_ancestors: Box<[Requirement<R>]>,
    pub sealed_whitelist: Option<IndexSet<TypeName>>,
    pub deferred_init_members: IndexSet<PropName>,
    pub decl_errors: Box<[DeclError<R::Pos>]>,
    pub docs_url: Option<String>,
    /// Whether has attribute <<__UNSAFE_AllowMultipleInstantiations>>
    pub allow_multiple_instantiations: bool,
}

impl<R: Reason> FoldedClass<R> {
    // c.f. `Decl_folded_class.class_is_abstract`
    pub fn is_abstract(&self) -> bool {
        match self.kind {
            ClassishKind::Cclass(abstraction) | ClassishKind::CenumClass(abstraction) => {
                abstraction.is_abstract()
            }
            ClassishKind::Cinterface | ClassishKind::Ctrait | ClassishKind::Cenum => true,
        }
    }

    // c.f. `Decl_defs.dc_need_init`, via `has_concrete_cstr` in `Decl_folded_class.class_decl`
    pub fn has_concrete_constructor(&self) -> bool {
        match &self.constructor.elt {
            Some(elt) => elt.is_concrete(),
            None => false,
        }
    }
}

impl FoldedElement {
    pub fn is_concrete(&self) -> bool {
        !self.is_abstract()
    }

    pub fn is_abstract(&self) -> bool {
        self.flags.contains(ClassEltFlags::ABSTRACT)
    }

    pub fn set_is_abstract(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::ABSTRACT, p)
    }

    pub fn is_final(&self) -> bool {
        self.flags.contains(ClassEltFlags::FINAL)
    }

    pub fn set_is_final(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::FINAL, p)
    }

    pub fn is_superfluous_override(&self) -> bool {
        self.flags.contains(ClassEltFlags::SUPERFLUOUS_OVERRIDE)
    }

    pub fn set_is_superfluous_override(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::SUPERFLUOUS_OVERRIDE, p)
    }

    pub fn is_lsb(&self) -> bool {
        self.flags.contains(ClassEltFlags::LSB)
    }

    pub fn set_is_lsb(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::LSB, p)
    }

    pub fn is_synthesized(&self) -> bool {
        self.flags.contains(ClassEltFlags::SYNTHESIZED)
    }

    pub fn set_is_synthesized(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::SYNTHESIZED, p)
    }

    pub fn is_const(&self) -> bool {
        self.flags.contains(ClassEltFlags::CONST)
    }

    pub fn set_is_const(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::CONST, p)
    }

    pub fn is_lateinit(&self) -> bool {
        self.flags.contains(ClassEltFlags::LATEINIT)
    }

    pub fn set_is_lateinit(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::LATEINIT, p)
    }

    pub fn is_dynamicallycallable(&self) -> bool {
        self.flags.contains(ClassEltFlags::DYNAMICALLYCALLABLE)
    }

    pub fn set_is_dynamicallycallable(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::DYNAMICALLYCALLABLE, p)
    }

    pub fn supports_dynamic_type(&self) -> bool {
        self.flags.contains(ClassEltFlags::SUPPORT_DYNAMIC_TYPE)
    }

    pub fn set_supports_dynamic_type(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::SUPPORT_DYNAMIC_TYPE, p)
    }

    pub fn is_readonly_prop(&self) -> bool {
        self.flags.contains(ClassEltFlags::READONLY_PROP)
    }

    pub fn set_is_readonly_prop(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::READONLY_PROP, p)
    }

    pub fn needs_init(&self) -> bool {
        self.flags.contains(ClassEltFlags::NEEDS_INIT)
    }

    pub fn set_needs_init(&mut self, p: bool) {
        self.flags.set(ClassEltFlags::NEEDS_INIT, p)
    }

    pub fn get_xhp_attr(&self) -> Option<XhpAttribute> {
        self.flags.get_xhp_attr()
    }
}

impl<R: Reason> Requirement<R> {
    pub fn new(pos: R::Pos, ty: Ty<R>) -> Self {
        Self { pos, ty }
    }
}

impl<R: Reason> fmt::Debug for Requirement<R> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_tuple("Requirement")
            .field(&self.pos)
            .field(&self.ty)
            .finish()
    }
}

impl Constructor {
    pub fn new(elt: Option<FoldedElement>, consistency: ConsistentKind) -> Self {
        Self { elt, consistency }
    }
}
