// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;

use crate::func::SrcLoc;
use crate::Attr;
use crate::Attribute;
use crate::ClassId;
use crate::CtxConstant;
use crate::HackConstant;
use crate::Method;
use crate::PropId;
use crate::TraitReqKind;
use crate::TypeConstant;
use crate::TypeInfo;
use crate::TypedValue;
use crate::Visibility;

/// This represents a Hack class or enum in IR.
#[derive(Debug)]
pub struct Class<'a> {
    /// Class attributes.
    pub attributes: Vec<Attribute>,

    /// Base class.
    pub base: Option<ClassId>,

    /// Class constants.
    pub constants: Vec<HackConstant>,

    // TODO: (doc coeffect constants)
    pub ctx_constants: Vec<CtxConstant<'a>>,

    /// Doc comment for the class.
    pub doc_comment: Option<Str<'a>>,

    /// In an enum this is the enum_type:
    /// ```
    /// enum A: int as int
    ///                ^^^
    /// ```
    pub enum_type: Option<TypeInfo>,
    pub enum_includes: Vec<ClassId>,

    pub flags: Attr,

    /// The implemented interfaces.
    pub implements: Vec<ClassId>,

    /// The methods defined in this class.
    pub methods: Vec<Method<'a>>,

    pub name: ClassId,
    pub properties: Vec<Property<'a>>,
    pub requirements: Vec<Requirement>,
    pub src_loc: SrcLoc,
    pub type_constants: Vec<TypeConstant<'a>>,

    /// For class generics the upper bounds of each generic.
    pub upper_bounds: Vec<(Str<'a>, Vec<TypeInfo>)>,

    pub uses: Vec<ClassId>,
}

impl<'a> Class<'a> {
    pub fn get_prop_by_pid(&self, pid: PropId) -> Option<&Property<'a>> {
        self.properties.iter().find(|prop| prop.name == pid)
    }
}

#[derive(Clone, Debug)]
pub struct Property<'a> {
    pub name: PropId,
    pub flags: Attr,
    pub attributes: Vec<Attribute>,
    pub visibility: Visibility,
    pub initial_value: Option<TypedValue>,
    pub type_info: TypeInfo,
    pub doc_comment: ffi::Maybe<Str<'a>>,
}

#[derive(Debug)]
pub struct Requirement {
    pub name: ClassId,
    pub kind: TraitReqKind,
}
