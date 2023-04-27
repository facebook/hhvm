// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

pub use hhvm_types_ffi::ffi::TypeStructureKind;

use crate::ArrayKey;
use crate::ClassId;
use crate::StringInterner;
use crate::TypedValue;

#[derive(Debug, Clone, Hash, Eq, PartialEq)]
pub enum TypeStruct {
    Unresolved(ClassId),
}

impl TypeStruct {
    pub fn into_typed_value(self, strings: &StringInterner) -> TypedValue {
        match self {
            TypeStruct::Unresolved(cid) => {
                let kind_key = ArrayKey::String(strings.intern_str("kind"));
                let kind = TypedValue::Int(TypeStructureKind::T_unresolved.repr as i64);
                let classname_key = ArrayKey::String(strings.intern_str("classname"));
                let name = TypedValue::String(cid.id);
                TypedValue::Dict(
                    [(kind_key, kind), (classname_key, name)]
                        .into_iter()
                        .collect(),
                )
            }
        }
    }
}
