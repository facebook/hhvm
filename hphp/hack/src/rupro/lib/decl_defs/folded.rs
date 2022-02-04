// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::decl_defs::{CeVisibility, DeclTy};
use crate::reason::Reason;
use obr::typing_defs_flags::ClassEltFlags;
use oxidized_by_ref as obr;
use pos::{SymbolMap, TypeName, TypeNameMap};

#[derive(Debug, Clone)]
pub struct FoldedElement<R: Reason> {
    // note(sf, 2022-01-28): c.f. `Decl_defs.element`
    pub flags: oxidized_by_ref::typing_defs_flags::ClassEltFlags,
    pub origin: TypeName,
    pub visibility: CeVisibility<R::Pos>,

    /// If the element is deprecated, this holds the deprecation message.
    pub deprecated: Option<intern::string::BytesId>,
}

#[derive(Debug, Clone)]
pub struct SubstContext<R: Reason> {
    // note(sf, 2022-01-28): c.f. `Decl_defs.subst_context`
    pub subst: TypeNameMap<DeclTy<R>>,
    pub class_context: TypeName,
}

#[derive(Debug, Clone)]
pub struct FoldedClass<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Decl_defs.decl_class_type`
    pub name: TypeName,
    pub pos: R::Pos,
    pub substs: TypeNameMap<SubstContext<R>>,
    pub ancestors: TypeNameMap<DeclTy<R>>,
    pub props: SymbolMap<FoldedElement<R>>,
    pub static_props: SymbolMap<FoldedElement<R>>,
    pub methods: SymbolMap<FoldedElement<R>>,
    pub static_methods: SymbolMap<FoldedElement<R>>,
    pub constructor: Option<FoldedElement<R>>,
}

impl<R: Reason> FoldedElement<R> {
    pub fn is_abstract(&self) -> bool {
        self.flags.contains(ClassEltFlags::ABSTRACT)
    }

    pub fn is_final(&self) -> bool {
        self.flags.contains(ClassEltFlags::FINAL)
    }

    pub fn is_superfluous_override(&self) -> bool {
        self.flags.contains(ClassEltFlags::SUPERFLUOUS_OVERRIDE)
    }

    pub fn is_lsb(&self) -> bool {
        self.flags.contains(ClassEltFlags::LSB)
    }

    pub fn is_synthesized(&self) -> bool {
        self.flags.contains(ClassEltFlags::SYNTHESIZED)
    }

    pub fn is_const(&self) -> bool {
        self.flags.contains(ClassEltFlags::CONST)
    }

    pub fn is_lateinit(&self) -> bool {
        self.flags.contains(ClassEltFlags::LATEINIT)
    }

    pub fn is_dynamicallycallable(&self) -> bool {
        self.flags.contains(ClassEltFlags::DYNAMICALLYCALLABLE)
    }

    pub fn supports_dynamic_type(&self) -> bool {
        self.flags.contains(ClassEltFlags::SUPPORT_DYNAMIC_TYPE)
    }

    pub fn is_readonly_prop(&self) -> bool {
        self.flags.contains(ClassEltFlags::READONLY_PROP)
    }
    pub fn needs_init(&self) -> bool {
        self.flags.contains(ClassEltFlags::NEEDS_INIT)
    }
}
