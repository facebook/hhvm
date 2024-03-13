// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

pub use hhvm_types_ffi::ffi::TypeStructureKind;
use intern::bytes_id;

use crate::BytesId;
use crate::ClassName;
use crate::DictEntry;
use crate::TypedValue;

#[derive(Debug, Clone, Hash, Eq, PartialEq)]
pub enum TypeStruct {
    Unresolved(ClassName),
    Null,
    Nonnull,
}

impl TypeStruct {
    pub fn into_typed_value(self) -> TypedValue {
        let kind_key = TypedValue::String(bytes_id!(b"kind"));

        match self {
            TypeStruct::Unresolved(cid) => {
                let kind = TypedValue::Int(TypeStructureKind::T_unresolved.repr as i64);
                let classname_key = TypedValue::String(bytes_id!(b"classname"));
                let name = TypedValue::String(cid.as_bytes_id());
                TypedValue::Dict(
                    vec![
                        DictEntry {
                            key: kind_key,
                            value: kind,
                        },
                        DictEntry {
                            key: classname_key,
                            value: name,
                        },
                    ]
                    .into(),
                )
            }
            TypeStruct::Null => {
                let kind = TypedValue::Int(TypeStructureKind::T_null.repr as i64);
                TypedValue::Dict(
                    vec![DictEntry {
                        key: kind_key,
                        value: kind,
                    }]
                    .into(),
                )
            }
            TypeStruct::Nonnull => {
                let kind = TypedValue::Int(TypeStructureKind::T_nonnull.repr as i64);
                TypedValue::Dict(
                    vec![DictEntry {
                        key: kind_key,
                        value: kind,
                    }]
                    .into(),
                )
            }
        }
    }

    pub fn try_from_typed_value(tv: &TypedValue) -> Option<TypeStruct> {
        let dv = tv.get_dict()?;
        let kind_key = TypedValue::String(bytes_id!(b"kind"));
        let kind = hhbc::dict_get(dv, &kind_key)?.get_int()?;
        if kind == i64::from(TypeStructureKind::T_null) {
            Some(TypeStruct::Null)
        } else if kind == i64::from(TypeStructureKind::T_nonnull) {
            Some(TypeStruct::Nonnull)
        } else if kind == i64::from(TypeStructureKind::T_unresolved) {
            let classname_key = TypedValue::String(bytes_id!(b"classname"));
            let classname = hhbc::dict_get(dv, &classname_key)?.get_string()?;
            if classname == BytesId::EMPTY {
                None
            } else {
                let cid = ClassName::from_bytes(classname).ok()?;
                Some(TypeStruct::Unresolved(cid))
            }
        } else {
            None
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test1() {
        assert_eq!(
            TypeStruct::try_from_typed_value(&TypeStruct::Null.into_typed_value()),
            Some(TypeStruct::Null)
        );

        assert_eq!(
            TypeStruct::try_from_typed_value(&TypeStruct::Nonnull.into_typed_value()),
            Some(TypeStruct::Nonnull)
        );

        let class_ts = TypeStruct::Unresolved(ClassName::intern("ExampleClass"));
        assert_eq!(
            TypeStruct::try_from_typed_value(&class_ts.clone().into_typed_value()),
            Some(class_ts)
        );
    }
}
