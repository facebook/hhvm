// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;
pub use hhbc::hhas_class::TraitReqKind;
pub use hhbc::hhas_pos::HhasSpan;
pub use hhbc::hhas_property::HhasProperty as Property;

use crate::Attr;
use crate::Attribute;
use crate::ClassId;
use crate::CtxConstant;
use crate::HackConstant;
use crate::Method;
use crate::Type;
use crate::TypeConstant;

/// This represents a Hack class or enum in IR.
#[derive(Debug)]
pub struct Class<'a> {
    /// Class attributes.
    pub attributes: Vec<Attribute<'a>>,

    /// Base class.
    pub base: Option<ClassId>,

    /// Class constants.
    pub constants: Vec<HackConstant<'a>>,

    // TODO: (doc coeffect constants)
    pub ctx_constants: Vec<CtxConstant<'a>>,

    /// Doc comment for the class.
    pub doc_comment: Option<Str<'a>>,

    /// In an enum this is the enum_type:
    /// ```
    /// enum A: int as int
    ///                ^^^
    /// ```
    pub enum_type: Option<Type<'a>>,
    pub enum_includes: Vec<ClassId>,

    pub flags: Attr,

    /// The implemented interfaces.
    pub implements: Vec<ClassId>,

    /// The methods defined in this class.
    pub methods: Vec<Method<'a>>,

    pub name: ClassId,
    pub properties: Vec<Property<'a>>,
    pub requirements: Vec<(ClassId, TraitReqKind)>,
    pub span: HhasSpan,
    pub type_constants: Vec<TypeConstant<'a>>,

    /// For class generics the upper bounds of each generic.
    pub upper_bounds: Vec<(Str<'a>, Vec<Type<'a>>)>,

    pub uses: Vec<ClassId>,
}
