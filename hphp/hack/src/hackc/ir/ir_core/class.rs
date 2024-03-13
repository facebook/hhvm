// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::Attr;
use crate::Attribute;
use crate::ClassName;
use crate::CtxConstant;
use crate::HackConstant;
use crate::Method;
use crate::PropName;
use crate::Requirement;
use crate::SrcLoc;
use crate::StringId;
use crate::TypeConstant;
use crate::TypeInfo;
use crate::TypedValue;
use crate::Visibility;

/// This represents a Hack class or enum in IR.
#[derive(Debug)]
pub struct Class {
    /// Class attributes.
    pub attributes: Vec<Attribute>,

    /// Base class.
    pub base: Option<ClassName>,

    /// Class constants.
    pub constants: Vec<HackConstant>,

    // TODO: (doc coeffect constants)
    pub ctx_constants: Vec<CtxConstant>,

    /// Doc comment for the class.
    pub doc_comment: Option<Vec<u8>>,

    /// In an enum this is the enum_type:
    /// ```
    /// enum A: int as int
    ///                ^^^
    /// ```
    pub enum_type: Option<TypeInfo>,
    pub enum_includes: Vec<ClassName>,

    pub flags: Attr,

    /// The implemented interfaces.
    pub implements: Vec<ClassName>,

    /// The methods defined in this class.
    pub methods: Vec<Method>,

    pub name: ClassName,
    pub properties: Vec<Property>,
    pub requirements: Vec<Requirement>,
    pub src_loc: SrcLoc,
    pub type_constants: Vec<TypeConstant>,

    /// For class generics the upper bounds of each generic.
    pub upper_bounds: Vec<(StringId, Vec<TypeInfo>)>,

    pub uses: Vec<ClassName>,
}

impl Class {
    pub fn get_prop_by_pid(&self, pid: PropName) -> Option<&Property> {
        self.properties.iter().find(|prop| prop.name == pid)
    }

    pub fn is_trait(&self) -> bool {
        (self.flags & Attr::AttrTrait) == Attr::AttrTrait
    }
}

#[derive(Clone, Debug)]
pub struct Property {
    pub name: PropName,
    pub flags: Attr,
    pub attributes: Vec<Attribute>,
    pub visibility: Visibility,
    pub initial_value: Option<TypedValue>,
    pub type_info: TypeInfo,
    pub doc_comment: ffi::Maybe<Vec<u8>>,
}
