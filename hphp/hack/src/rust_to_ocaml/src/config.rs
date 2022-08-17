// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::str::FromStr;

use indexmap::indexmap;
use indexmap::indexset;
use indexmap::IndexMap;
use indexmap::IndexSet;
use serde::Deserialize;
use serde::Serialize;

use crate::ir;
use crate::ir::ModuleName;
use crate::ir::TypeName;

#[derive(Debug, Default, Serialize, Deserialize)]
pub struct Config {
    types: TypesConfig,
}

#[derive(Debug, Serialize, Deserialize)]
struct TypesConfig {
    transparent: IndexSet<RustTypePath>,
    #[serde(with = "indexmap::serde_seq")]
    rename: IndexMap<RustTypePath, OcamlTypePath>,
}

#[derive(Clone, PartialEq, Eq, Hash)]
pub struct RustTypePath {
    pub modules: Vec<ModuleName>,
    pub ty: TypeName,
}

#[derive(Clone, PartialEq, Eq, Hash)]
pub struct OcamlTypePath {
    pub modules: Vec<ModuleName>,
    pub ty: TypeName,
}

impl Config {
    pub fn is_transparent_type(&self, path: &ir::TypePath) -> bool {
        let rust_path = RustTypePath::from(path);
        self.types.transparent.contains(&rust_path)
    }
    pub fn get_renamed_type(&self, path: &ir::TypePath) -> Option<OcamlTypePath> {
        let rust_path = RustTypePath::from(path);
        self.types.rename.get(&rust_path).cloned()
    }
}

impl Default for TypesConfig {
    fn default() -> Self {
        let r = |s| RustTypePath::from_str(s).unwrap();
        let o = |s| OcamlTypePath::from_str(s).unwrap();
        Self {
            transparent: indexset! {
                r("Box"),
                r("std::boxed::Box"),
                r("Rc"),
                r("std::rc::Rc"),
                r("Arc"),
                r("std::sync::Arc"),
            },
            rename: indexmap! {
                r("Vec") => o("list"),
                r("std::vec::Vec") => o("list"),
            },
        }
    }
}

impl From<&ir::TypePath> for RustTypePath {
    fn from(path: &ir::TypePath) -> Self {
        Self {
            modules: path.modules.clone(),
            ty: path.ty.clone(),
        }
    }
}

impl std::fmt::Display for RustTypePath {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for m in self.modules.iter() {
            write!(f, "{}", m.as_str())?;
            write!(f, "::")?;
        }
        write!(f, "{}", self.ty.as_str())
    }
}
impl std::fmt::Display for OcamlTypePath {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for m in self.modules.iter() {
            write!(f, "{}", m.as_str())?;
            write!(f, ".")?;
        }
        write!(f, "{}", self.ty.as_str())
    }
}

impl std::fmt::Debug for RustTypePath {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        format!("{self}").fmt(f)
    }
}
impl std::fmt::Debug for OcamlTypePath {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        format!("{self}").fmt(f)
    }
}

impl FromStr for RustTypePath {
    type Err = anyhow::Error;
    fn from_str(s: &str) -> anyhow::Result<Self> {
        let (modules, ty) = parse_type_path(s, "::")?;
        Ok(Self { modules, ty })
    }
}
impl FromStr for OcamlTypePath {
    type Err = anyhow::Error;
    fn from_str(s: &str) -> anyhow::Result<Self> {
        let (modules, ty) = parse_type_path(s, ".")?;
        Ok(Self { modules, ty })
    }
}

fn parse_type_path(s: &str, sep: &str) -> anyhow::Result<(Vec<ModuleName>, TypeName)> {
    let mut split = s.rsplit(sep);
    let ty = match split.next() {
        None | Some("") => anyhow::bail!("Invalid type name: {:?}", s),
        Some(ty) => TypeName(ty.to_owned()),
    };
    let mut modules = split
        .map(|m| {
            anyhow::ensure!(!m.is_empty(), "Invalid module name: {:?}", m);
            Ok(ModuleName(m.to_owned()))
        })
        .collect::<Result<Vec<_>, _>>()?;
    modules.reverse();
    Ok((modules, ty))
}

impl Serialize for RustTypePath {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.serialize_str(&self.to_string())
    }
}
impl Serialize for OcamlTypePath {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.serialize_str(&self.to_string())
    }
}

impl<'de> Deserialize<'de> for RustTypePath {
    fn deserialize<D: serde::Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Visitor;
        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = RustTypePath;
            fn expecting(&self, formatter: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(formatter, "a valid Rust type path string")
            }
            fn visit_str<E: serde::de::Error>(self, value: &str) -> Result<Self::Value, E> {
                value.parse().map_err(|e| {
                    E::invalid_value(serde::de::Unexpected::Other(&format!("{e}")), &self)
                })
            }
        }
        deserializer.deserialize_str(Visitor)
    }
}
impl<'de> Deserialize<'de> for OcamlTypePath {
    fn deserialize<D: serde::Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Visitor;
        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = OcamlTypePath;
            fn expecting(&self, formatter: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(formatter, "a valid OCaml type path string")
            }
            fn visit_str<E: serde::de::Error>(self, value: &str) -> Result<Self::Value, E> {
                value.parse().map_err(|e| {
                    E::invalid_value(serde::de::Unexpected::Other(&format!("{e}")), &self)
                })
            }
        }
        deserializer.deserialize_str(Visitor)
    }
}
