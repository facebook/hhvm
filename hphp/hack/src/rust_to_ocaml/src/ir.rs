// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod display;

use derive_more::Display;

#[derive(Debug)]
pub struct File {
    pub root: Module,
}

#[derive(Debug)]
pub struct Module {
    pub name: ModuleName,
    pub defs: Vec<Def>,
}

#[derive(Debug)]
pub enum Def {
    Module(Module),
    Alias {
        doc: Vec<String>,
        attrs: Vec<String>,
        tparams: Vec<String>,
        name: TypeName,
        ty: Type,
    },
    Record {
        doc: Vec<String>,
        attrs: Vec<String>,
        tparams: Vec<String>,
        name: TypeName,
        fields: Vec<Field>,
    },
    Variant {
        doc: Vec<String>,
        attrs: Vec<String>,
        tparams: Vec<String>,
        name: TypeName,
        variants: Vec<Variant>,
    },
}

#[derive(Debug)]
pub struct Variant {
    pub name: VariantName,
    pub fields: Option<VariantFields>,
    pub doc: Vec<String>,
    pub attrs: Vec<String>,
}

#[derive(Debug)]
pub enum VariantFields {
    Unnamed(Vec<Type>),
    Named(Vec<Field>),
}

#[derive(Debug)]
pub struct Field {
    pub name: FieldName,
    pub ty: Type,
    pub doc: Vec<String>,
    pub attrs: Vec<String>,
}

#[derive(Debug)]
pub enum Type {
    Path(TypePath),
    Tuple(TypeTuple),
}

#[derive(Debug)]
pub struct TypePath {
    pub targs: Vec<Type>,
    pub modules: Vec<ModuleName>,
    pub ty: TypeName,
}

impl TypePath {
    pub fn simple(id: impl Into<String>) -> Self {
        Self {
            modules: vec![],
            ty: TypeName(id.into()),
            targs: vec![],
        }
    }
}

#[derive(Debug)]
pub struct TypeTuple {
    pub elems: Vec<Type>,
}

#[derive(Clone, Hash, PartialEq, Eq)]
pub struct ModuleName(String);

impl ModuleName {
    pub fn new(name: impl Into<String>) -> anyhow::Result<Self> {
        let name: String = name.into();
        anyhow::ensure!(!name.is_empty(), "Module names must not be empty");
        let first_char = name.chars().next().unwrap();
        anyhow::ensure!(
            first_char.is_ascii(),
            "Module names must start with an ASCII character: {}",
            name
        );
        anyhow::ensure!(
            first_char.to_ascii_uppercase().is_ascii_uppercase(),
            "Module names must start with a character which can be converted to uppercase: {}",
            name
        );
        Ok(Self(name))
    }

    pub fn as_str(&self) -> &str {
        self.0.as_str()
    }
}

impl std::str::FromStr for ModuleName {
    type Err = anyhow::Error;
    fn from_str(s: &str) -> anyhow::Result<Self> {
        Self::new(s)
    }
}

serde_from_display!(ModuleName, "a valid Rust or OCaml module name");

#[derive(Clone, Hash, PartialEq, Eq)]
pub struct TypeName(pub String);

impl TypeName {
    pub fn as_str(&self) -> &str {
        self.0.as_str()
    }
}

#[derive(Clone, Hash, PartialEq, Eq)]
pub struct FieldName(pub String);

#[derive(Clone, Hash, PartialEq, Eq, Display)]
pub struct VariantName(pub String);

impl std::fmt::Debug for ModuleName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}
impl std::fmt::Debug for TypeName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}
impl std::fmt::Debug for FieldName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}
impl std::fmt::Debug for VariantName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}
