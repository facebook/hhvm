// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;

use closure_convert_rust as closure_convert;
use hhas_attribute_rust::HhasAttribute;
use hhas_constant_rust::HhasConstant;
use hhas_method_rust::HhasMethod;
use hhas_pos_rust::Span;
use hhas_property_rust::HhasProperty;
use hhas_type_const::HhasTypeConstant;
use hhbc_id_rust::class;
use oxidized::{ast as tast, doc_comment::DocComment};

#[derive(Debug)]
pub enum TraitReqKind {
    MustExtend,
    MustImplement,
}

#[derive(Debug)]
pub struct HhasClass<'a> {
    pub attributes: Vec<HhasAttribute>,
    pub base: Option<class::Type<'a>>,
    pub implements: Vec<class::Type<'a>>,
    pub name: class::Type<'a>,
    pub span: Span,
    pub hoisted: closure_convert::HoistKind,
    pub uses: Vec<class::Type<'a>>,
    // Deprecated - kill please
    pub use_aliases: Vec<(
        Option<class::Type<'a>>,
        class::Type<'a>,
        Option<class::Type<'a>>,
        &'a Vec<tast::UseAsVisibility>,
    )>,
    // Deprecated - kill please
    pub use_precedences: Vec<(class::Type<'a>, class::Type<'a>, Vec<class::Type<'a>>)>,
    pub method_trait_resolutions: Vec<(&'a tast::MethodRedeclaration, class::Type<'a>)>,
    pub enum_type: Option<hhas_type::Info>,
    pub methods: Vec<HhasMethod<'a>>,
    pub properties: Vec<HhasProperty<'a>>,
    pub constants: Vec<HhasConstant<'a>>,
    pub type_constants: Vec<HhasTypeConstant>,
    pub requirements: Vec<(class::Type<'a>, TraitReqKind)>,
    pub upper_bounds: Vec<(String, Vec<hhas_type::Info>)>,
    pub doc_comment: Option<DocComment>,
    pub flags: HhasClassFlags,
}

bitflags! {
    pub struct HhasClassFlags: u16 {
        const IS_FINAL = 1 << 1;
        const IS_SEALED = 1 << 2;
        const IS_ABSTRACT = 1 << 3;
        const IS_INTERFACE = 1 << 4;
        const IS_TRAIT = 1 << 5;
        const IS_XHP = 1 << 6;
        const IS_CONST = 1 << 7;
        const NO_DYNAMIC_PROPS = 1 << 8;
        const NEEDS_NO_REIFIEDINIT = 1 << 9;
    }
}

impl<'a> HhasClass<'a> {
    pub fn is_final(&self) -> bool {
        self.flags.contains(HhasClassFlags::IS_FINAL)
    }
    pub fn is_sealed(&self) -> bool {
        self.flags.contains(HhasClassFlags::IS_SEALED)
    }
    pub fn is_abstract(&self) -> bool {
        self.flags.contains(HhasClassFlags::IS_ABSTRACT)
    }
    pub fn is_interface(&self) -> bool {
        self.flags.contains(HhasClassFlags::IS_INTERFACE)
    }
    pub fn is_trait(&self) -> bool {
        self.flags.contains(HhasClassFlags::IS_TRAIT)
    }
    pub fn is_xhp(&self) -> bool {
        self.flags.contains(HhasClassFlags::IS_XHP)
    }
    pub fn is_const(&self) -> bool {
        self.flags.contains(HhasClassFlags::IS_CONST)
    }
    pub fn no_dynamic_props(&self) -> bool {
        self.flags.contains(HhasClassFlags::NO_DYNAMIC_PROPS)
    }
    pub fn needs_no_reifiedinit(&self) -> bool {
        self.flags.contains(HhasClassFlags::NEEDS_NO_REIFIEDINIT)
    }
    pub fn is_closure(&self) -> bool {
        self.methods.iter().any(|x| x.is_closure_body())
    }
    pub fn is_top(&self) -> bool {
        self.hoisted == closure_convert::HoistKind::TopLevel
    }
}
