// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Provide impls of `Serialize` and `Deserialize` which delegate to the impls
/// of `std::fmt::Display` and `std::str::FromStr` respectively.
macro_rules! serde_from_display {
    ($name:ident, $expecting:expr) => {
        impl ::serde::Serialize for $name {
            fn serialize<S: ::serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
                serializer.serialize_str(&self.to_string())
            }
        }
        impl<'de> ::serde::Deserialize<'de> for $name {
            fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
            where
                D: ::serde::Deserializer<'de>,
            {
                struct Visitor;
                impl<'de> ::serde::de::Visitor<'de> for Visitor {
                    type Value = $name;
                    fn expecting(
                        &self,
                        formatter: &mut ::std::fmt::Formatter<'_>,
                    ) -> ::std::fmt::Result {
                        formatter.write_str($expecting)
                    }
                    fn visit_str<E>(self, value: &str) -> Result<Self::Value, E>
                    where
                        E: ::serde::de::Error,
                    {
                        value.parse().map_err(|e| {
                            E::invalid_value(::serde::de::Unexpected::Other(&format!("{e}")), &self)
                        })
                    }
                }
                deserializer.deserialize_str(Visitor)
            }
        }
    };
}
