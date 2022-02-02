// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(warnings)]

use std::ops::BitAnd;
use std::ops::BitOr;

#[cxx::bridge(namespace = "HPHP")]
pub mod ffi {
    // This is not a real definition. Cxx just adds static asserts on each of these enum variants
    // that they match the definition in attr.h.
    #[repr(u32)]
    #[derive(Debug, Copy, Clone)]
    enum Attr {
        AttrNone = 0x0,
        AttrForbidDynamicProps = 0x1,
        AttrDeepInit = 0x1,
        AttrPublic = 0x2,
        AttrProtected = 0x4,
        AttrPrivate = 0x8,
        AttrEnum = 0x10,
        AttrSystemInitialValue = 0x20,
        AttrNoImplicitNullable = 0x40,
        AttrStatic = 0x10,
        AttrAbstract = 0x20,
        AttrFinal = 0x40,
        AttrInterface = 0x80,
        AttrLSB = 0x80,
        AttrSupportsAsyncEagerReturn = 0x80,
        AttrTrait = 0x100,
        AttrNoInjection = 0x200,
        AttrInitialSatisfiesTC = 0x200,
        AttrUnique = 0x400,
        AttrNoBadRedeclare = 0x400,
        AttrInterceptable = 0x800,
        AttrSealed = 0x800,
        AttrLateInit = 0x800,
        AttrNoExpandTrait = 0x1000,
        AttrNoOverride = 0x2000,
        AttrIsReadonly = 0x4000,
        AttrReadonlyThis = 0x4000,
        AttrReadonlyReturn = 0x8000,
        AttrPersistent = 0x40000,
        AttrDynamicallyCallable = 0x80000,
        AttrDynamicallyConstructible = 0x80000,
        AttrBuiltin = 0x100000,
        AttrIsConst = 0x200000,
        AttrNoReifiedInit = 0x800000,
        AttrIsMethCaller = 0x1000000,
        AttrIsClosureClass = 0x1000000,
        AttrHasClosureCoeffectsProp = 0x2000000,
        AttrHasCoeffectRules = 0x2000000,
        AttrIsFoldable = 0x4000000,
        AttrNoFCallBuiltin = 0x8000000,
        AttrVariadicParam = 0x10000000,
        AttrProvenanceSkipFrame = 0x20000000,
        AttrEnumClass = 0x40000000,
        AttrUnusedMaxAttr = 0x80000000,
    }

    #[repr(u32)]
    #[derive(Debug, Copy, Clone)]
    enum AttrContext {
        Class = 0x1,
        Func = 0x2,
        Prop = 0x4,
        TraitImport = 0x8,
        Alias = 0x10,
        Parameter = 0x20,
        Constant = 0x40,
    }

    unsafe extern "C++" {
        include!("hphp/hack/src/hackc/hhvm_cxx/hhvm_types/as-attr-ffi.h");
        type Attr;
        type AttrContext;
        fn attrs_to_string_ffi(ctx: AttrContext, attrs: Attr) -> String;
    }
}

use ffi::Attr;

impl From<u32> for Attr {
    fn from(r: u32) -> Self {
        Self { repr: r }
    }
}

impl From<oxidized::ast_defs::Visibility> for Attr {
    fn from(k: oxidized::ast_defs::Visibility) -> Attr {
        use oxidized::ast_defs::Visibility::*;
        match k {
            Private => Attr::AttrPrivate,
            Public => Attr::AttrPublic,
            Protected => Attr::AttrProtected,
            Internal => Attr::AttrPublic,
        }
    }
}

impl From<&oxidized::ast_defs::Visibility> for Attr {
    fn from(k: &oxidized::ast_defs::Visibility) -> Attr {
        use oxidized::ast_defs::Visibility::*;
        match k {
            Private => Attr::AttrPrivate,
            Public => Attr::AttrPublic,
            Protected => Attr::AttrProtected,
            Internal => Attr::AttrNone,
        }
    }
}

impl From<hhbc_ast::Visibility> for Attr {
    fn from(k: hhbc_ast::Visibility) -> Attr {
        use hhbc_ast::Visibility::*;
        match k {
            Private => Attr::AttrPrivate,
            Public => Attr::AttrPublic,
            Protected => Attr::AttrProtected,
            Internal => Attr::AttrNone,
        }
    }
}

impl From<Attr> for u32 {
    fn from(attr: Attr) -> Self {
        attr.repr
    }
}

impl Attr {
    pub fn add(&mut self, attr: Attr) {
        self.repr = *self | attr
    }

    pub fn set(&mut self, attr: Attr, b: bool) {
        if b {
            self.add(attr)
        }
    }
    pub fn is_final(&self) -> bool {
        (*self & Attr::AttrFinal) != 0
    }
    pub fn is_sealed(&self) -> bool {
        (*self & Attr::AttrSealed) != 0
    }
    pub fn is_abstract(&self) -> bool {
        (*self & Attr::AttrAbstract) != 0
    }
    pub fn is_interface(&self) -> bool {
        (*self & Attr::AttrInterface) != 0
    }
    pub fn is_trait(&self) -> bool {
        (*self & Attr::AttrTrait) != 0
    }
    pub fn is_const(&self) -> bool {
        (*self & Attr::AttrIsConst) != 0
    }
    pub fn no_dynamic_props(&self) -> bool {
        (*self & Attr::AttrForbidDynamicProps) != 0
    }
    pub fn needs_no_reifiedinit(&self) -> bool {
        (*self & Attr::AttrNoReifiedInit) != 0
    }
    pub fn is_late_init(&self) -> bool {
        (*self & Attr::AttrLateInit) != 0
    }
    pub fn is_no_bad_redeclare(&self) -> bool {
        (*self & Attr::AttrNoBadRedeclare) != 0
    }
    pub fn initial_satisfies_tc(&self) -> bool {
        (*self & Attr::AttrInitialSatisfiesTC) != 0
    }
    pub fn no_implicit_null(&self) -> bool {
        (*self & Attr::AttrNoImplicitNullable) != 0
    }
    pub fn has_system_initial(&self) -> bool {
        (*self & Attr::AttrSystemInitialValue) != 0
    }
    pub fn is_deep_init(&self) -> bool {
        (*self & Attr::AttrDeepInit) != 0
    }
    pub fn is_lsb(&self) -> bool {
        (*self & Attr::AttrLSB) != 0
    }
    pub fn is_static(&self) -> bool {
        (*self & Attr::AttrStatic) != 0
    }
    pub fn is_readonly(&self) -> bool {
        (*self & Attr::AttrIsReadonly) != 0
    }
    pub fn is_no_injection(&self) -> bool {
        (*self & Attr::AttrNoInjection) != 0
    }
    pub fn is_interceptable(&self) -> bool {
        (*self & Attr::AttrInterceptable) != 0
    }
}

impl BitOr for Attr {
    type Output = u32;

    fn bitor(self, other: Self) -> u32 {
        self.repr | other.repr
    }
}

impl BitOr<Attr> for u32 {
    type Output = u32;

    fn bitor(self, other: Attr) -> u32 {
        self | other.repr
    }
}

impl BitAnd for Attr {
    type Output = u32;

    fn bitand(self, other: Self) -> u32 {
        self.repr & other.repr
    }
}

impl BitAnd<u32> for Attr {
    type Output = u32;

    fn bitand(self, other: u32) -> u32 {
        self.repr & other
    }
}
