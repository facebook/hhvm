// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Vector;
use hhvm_types_ffi::ffi::Attr;
use serde::Serialize;

use crate::Attribute;
use crate::ClassName;
use crate::Constant;
use crate::CtxConstant;
use crate::MethodImpl;
use crate::PropName;
use crate::Property;
use crate::Span;
use crate::TypeConstant;
use crate::TypeInfo;
use crate::UpperBound;

#[derive(Copy, Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub enum TraitReqKind {
    MustExtend,
    MustImplement,
    MustBeClass,
}

#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Requirement {
    pub name: ClassName,
    pub kind: TraitReqKind,
}

pub type Class = ClassImpl<crate::BcRepr>;

/// This represents a Hack class, trait, interface, or enum
#[derive(Debug, Clone, Serialize)]
#[repr(C)]
pub struct ClassImpl<R> {
    /// Class attributes.
    pub attributes: Vector<Attribute>,

    /// Base class.
    pub base: Maybe<ClassName>,

    /// The implemented interfaces.
    pub implements: Vector<ClassName>,
    pub name: ClassName,
    pub span: Span,
    pub uses: Vector<ClassName>,

    /// In an enum this is the enum_type:
    /// ```
    /// enum A: int as int
    ///                ^^^
    /// ```
    pub enum_type: Maybe<TypeInfo>,
    pub enum_includes: Vector<ClassName>,

    /// The methods defined in this class.
    pub methods: Vector<MethodImpl<R>>,

    /// The properties defined in this class.
    pub properties: Vector<Property>,

    /// Class constants.
    pub constants: Vector<Constant>,
    pub type_constants: Vector<TypeConstant>,
    pub ctx_constants: Vector<CtxConstant>,
    pub requirements: Vector<Requirement>,

    /// For class generics the upper bounds of each generic.
    pub upper_bounds: Vector<UpperBound>,
    /// Names of generic type params declared for this class.
    pub tparams: Vector<ClassName>,

    /// Doc comment for the class.
    pub doc_comment: Maybe<Vector<u8>>,
    pub flags: Attr,
}

impl<R> ClassImpl<R> {
    pub fn is_closure(&self) -> bool {
        self.methods.as_ref().iter().any(|x| x.is_closure_body())
    }

    pub fn get_prop_by_pid(&self, pid: PropName) -> Option<&Property> {
        self.properties.iter().find(|prop| prop.name == pid)
    }

    pub fn is_trait(&self) -> bool {
        (self.flags & Attr::AttrTrait) == Attr::AttrTrait
    }
}
