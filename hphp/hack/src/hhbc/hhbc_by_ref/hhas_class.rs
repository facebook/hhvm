// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;

use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_hhas_coeffects::HhasCtxConstant;
use hhbc_by_ref_hhas_constant::HhasConstant;
use hhbc_by_ref_hhas_method::HhasMethod;
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_property::HhasProperty;
use hhbc_by_ref_hhas_type_const::HhasTypeConstant;
use hhbc_by_ref_hhbc_id::class;
use oxidized::{ast as tast, doc_comment::DocComment};

#[derive(Debug)]
pub enum TraitReqKind {
    MustExtend,
    MustImplement,
}

#[derive(Debug)]
pub struct HhasClass<'a, 'arena> {
    pub attributes: Vec<HhasAttribute<'arena>>,
    pub base: Option<class::Type<'arena>>,
    pub implements: Vec<class::Type<'arena>>,
    pub enum_includes: Vec<class::Type<'arena>>,
    pub name: class::Type<'arena>,
    pub span: Span,
    pub uses: Vec<&'a str>,
    // Deprecated - kill please
    pub use_aliases: Vec<(
        Option<class::Type<'arena>>,
        class::Type<'arena>,
        Option<class::Type<'arena>>,
        &'a Vec<tast::UseAsVisibility>,
    )>,
    // Deprecated - kill please
    pub use_precedences: Vec<(
        class::Type<'arena>,
        class::Type<'arena>,
        Vec<class::Type<'arena>>,
    )>,
    pub enum_type: Option<hhbc_by_ref_hhas_type::Info>,
    pub methods: Vec<HhasMethod<'arena>>,
    pub properties: Vec<HhasProperty<'arena>>,
    pub constants: Vec<HhasConstant<'arena>>,
    pub type_constants: Vec<HhasTypeConstant<'arena>>,
    pub ctx_constants: Vec<HhasCtxConstant>,
    pub requirements: Vec<(class::Type<'arena>, TraitReqKind)>,
    pub upper_bounds: Vec<(String, Vec<hhbc_by_ref_hhas_type::Info>)>,
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

impl<'a, 'arena> HhasClass<'a, 'arena> {
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
}
