// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

pub use hhvm_types_ffi::ffi::TypeStructureKind;

use crate::ArrayKey;
use crate::ClassName;
use crate::StringInterner;
use crate::TypedValue;

#[derive(Debug, Clone, Hash, Eq, PartialEq)]
pub enum TypeStruct {
    Unresolved(ClassName),
    Null,
    Nonnull,
}

impl TypeStruct {
    pub fn into_typed_value(self, strings: &StringInterner) -> TypedValue {
        let kind_key = ArrayKey::String(strings.intern_str("kind"));

        match self {
            TypeStruct::Unresolved(cid) => {
                let kind = TypedValue::Int(TypeStructureKind::T_unresolved.repr as i64);
                let classname_key = ArrayKey::String(strings.intern_str("classname"));
                let name = TypedValue::String(cid.as_bytes_id());
                TypedValue::Dict(
                    [(kind_key, kind), (classname_key, name)]
                        .into_iter()
                        .collect(),
                )
            }
            TypeStruct::Null => {
                let kind = TypedValue::Int(TypeStructureKind::T_null.repr as i64);
                TypedValue::Dict([(kind_key, kind)].into_iter().collect())
            }
            TypeStruct::Nonnull => {
                let kind = TypedValue::Int(TypeStructureKind::T_nonnull.repr as i64);
                TypedValue::Dict([(kind_key, kind)].into_iter().collect())
            }
        }
    }

    pub fn try_from_typed_value(tv: &TypedValue, strings: &StringInterner) -> Option<TypeStruct> {
        let dv = tv.get_dict()?;
        let kind_key = ArrayKey::String(strings.intern_str("kind"));
        let kind = dv.get(&kind_key)?.get_int()?;
        if kind == i64::from(TypeStructureKind::T_null) {
            Some(TypeStruct::Null)
        } else if kind == i64::from(TypeStructureKind::T_nonnull) {
            Some(TypeStruct::Nonnull)
        } else if kind == i64::from(TypeStructureKind::T_unresolved) {
            let classname_key = ArrayKey::String(strings.intern_str("classname"));
            let classname = dv.get(&classname_key)?.get_string()?;
            let classname = strings.lookup_bytes_or_none(classname)?;
            let cid = ClassName::from_utf8(classname).ok()?;
            Some(TypeStruct::Unresolved(cid))
        } else {
            None
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::StringInterner;

    #[test]
    fn test1() {
        let strings = StringInterner::default();
        assert_eq!(
            TypeStruct::try_from_typed_value(
                &TypeStruct::Null.into_typed_value(&strings),
                &strings
            ),
            Some(TypeStruct::Null)
        );

        assert_eq!(
            TypeStruct::try_from_typed_value(
                &TypeStruct::Nonnull.into_typed_value(&strings),
                &strings
            ),
            Some(TypeStruct::Nonnull)
        );

        let class_ts = TypeStruct::Unresolved(ClassName::intern("ExampleClass"));
        assert_eq!(
            TypeStruct::try_from_typed_value(
                &class_ts.clone().into_typed_value(&strings),
                &strings
            ),
            Some(class_ts)
        );
    }
}
