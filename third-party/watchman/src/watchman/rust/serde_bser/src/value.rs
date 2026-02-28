/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::collections::HashMap;
use std::path::PathBuf;

use serde::Serialize;
use serde::de::Deserialize;
use serde::de::MapAccess;
use serde::de::SeqAccess;
use serde::de::Visitor;

use crate::bytestring::ByteString;

/// The Value type is used in cases where the schema is not known statically.
/// As used in Watchman's protocol, this allows encoding arbitrary metadata
/// that can be passed through the system by eg: the `state-enter` command,
/// or returned from a saved state storage engine.
/// The values are conceptually equivalent to json values, with the notable
/// difference that BSER can represent a binary byte string value.
#[derive(Debug, Clone, PartialEq)]
pub enum Value {
    Array(Vec<Value>),
    Object(HashMap<String, Value>),
    ByteString(ByteString),
    Integer(i64),
    Real(f64),
    Bool(bool),
    Null,
    Utf8String(String),
}

impl From<Vec<Value>> for Value {
    fn from(v: Vec<Value>) -> Self {
        Self::Array(v)
    }
}

impl From<HashMap<String, Value>> for Value {
    fn from(v: HashMap<String, Value>) -> Self {
        Self::Object(v)
    }
}

impl From<bool> for Value {
    fn from(v: bool) -> Self {
        Self::Bool(v)
    }
}

impl From<&str> for Value {
    fn from(s: &str) -> Self {
        Self::Utf8String(s.to_string())
    }
}

impl From<String> for Value {
    fn from(s: String) -> Self {
        Self::Utf8String(s)
    }
}

impl TryInto<Value> for PathBuf {
    type Error = &'static str;

    fn try_into(self) -> Result<Value, Self::Error> {
        let s: ByteString = self.try_into()?;
        Ok(Value::ByteString(s))
    }
}

impl From<i64> for Value {
    fn from(v: i64) -> Self {
        Self::Integer(v)
    }
}

impl TryInto<Value> for usize {
    type Error = &'static str;

    fn try_into(self) -> Result<Value, Self::Error> {
        if self > i64::MAX as usize {
            Err("value is too large to represent as i64")
        } else {
            Ok(Value::Integer(self as i64))
        }
    }
}

impl<'de> Deserialize<'de> for Value {
    #[inline]
    fn deserialize<D>(deserializer: D) -> Result<Value, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        struct ValueVisitor;

        impl<'de> Visitor<'de> for ValueVisitor {
            type Value = Value;

            fn expecting(&self, formatter: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                formatter.write_str("any valid BSER value")
            }

            #[inline]
            fn visit_bool<E>(self, value: bool) -> Result<Value, E> {
                Ok(Value::Bool(value))
            }

            #[inline]
            fn visit_i64<E>(self, value: i64) -> Result<Value, E> {
                Ok(Value::Integer(value))
            }

            /*
            #[inline]
            fn visit_u64<E>(self, v: u64) -> Result<Value, E> {
                // maybe_put_int! doesn't work for u64 because it converts to i64
                // internally.
                if v > (i64::MAX as u64) {
                    Err(serde::de::Error::custom(format!(
                        "value {} is too large to represent as a BSER integer",
                        v
                    )))
                } else {
                    Ok(Value::Integer(v as i64))
                }
            }
            */

            #[inline]
            fn visit_f64<E>(self, value: f64) -> Result<Value, E> {
                Ok(Value::Real(value))
            }

            #[inline]
            fn visit_bytes<E>(self, value: &[u8]) -> Result<Self::Value, E> {
                Ok(Value::ByteString(value.to_vec().into()))
            }

            #[inline]
            fn visit_byte_buf<E>(self, value: Vec<u8>) -> Result<Self::Value, E> {
                Ok(Value::ByteString(value.into()))
            }

            #[inline]
            fn visit_str<E>(self, value: &str) -> Result<Value, E>
            where
                E: serde::de::Error,
            {
                self.visit_string(String::from(value))
            }

            #[inline]
            fn visit_string<E>(self, value: String) -> Result<Value, E> {
                Ok(Value::Utf8String(value))
            }

            #[inline]
            fn visit_some<D>(self, deserializer: D) -> Result<Value, D::Error>
            where
                D: serde::Deserializer<'de>,
            {
                Deserialize::deserialize(deserializer)
            }

            #[inline]
            fn visit_none<E>(self) -> Result<Value, E> {
                Ok(Value::Null)
            }

            #[inline]
            fn visit_unit<E>(self) -> Result<Value, E> {
                Ok(Value::Null)
            }

            #[inline]
            fn visit_seq<V>(self, mut visitor: V) -> Result<Value, V::Error>
            where
                V: SeqAccess<'de>,
            {
                let mut vec = Vec::new();

                while let Some(elem) = visitor.next_element()? {
                    vec.push(elem);
                }

                Ok(Value::Array(vec))
            }

            fn visit_map<V>(self, mut visitor: V) -> Result<Value, V::Error>
            where
                V: MapAccess<'de>,
            {
                match visitor.next_key()? {
                    Some(Value::ByteString(key)) => {
                        let mut values = HashMap::new();

                        values.insert(
                            key.try_into().map_err(serde::de::Error::custom)?,
                            visitor.next_value()?,
                        );
                        while let Some((key, value)) = visitor.next_entry()? {
                            values.insert(key, value);
                        }

                        Ok(Value::Object(values))
                    }
                    Some(Value::Utf8String(key)) => {
                        let mut values = HashMap::new();

                        values.insert(key, visitor.next_value()?);
                        while let Some((key, value)) = visitor.next_entry()? {
                            values.insert(key, value);
                        }

                        Ok(Value::Object(values))
                    }
                    Some(value) => Err(serde::de::Error::custom(format!(
                        "value {:?} is illegal as a key in a BSER map",
                        value
                    ))),
                    None => Ok(Value::Object(HashMap::new())),
                }
            }
        }

        deserializer.deserialize_any(ValueVisitor)
    }
}

impl Serialize for Value {
    #[inline]
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: ::serde::Serializer,
    {
        match *self {
            Value::Null => serializer.serialize_unit(),
            Value::Bool(b) => serializer.serialize_bool(b),
            Value::Integer(n) => serializer.serialize_i64(n),
            Value::Real(n) => serializer.serialize_f64(n),
            Value::Utf8String(ref s) => serializer.serialize_str(s),
            Value::ByteString(ref b) => serializer.serialize_bytes(b.as_bytes()),
            Value::Array(ref v) => v.serialize(serializer),
            Value::Object(ref m) => {
                use serde::ser::SerializeMap;
                let mut map = serializer.serialize_map(Some(m.len()))?;
                for (k, v) in m {
                    map.serialize_key(k)?;
                    map.serialize_value(v)?;
                }
                map.end()
            }
        }
    }
}
