// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use derive_more::Display;
use hash::IndexMap;

pub struct File {
    pub defs: IndexMap<TypeName, Def>,
}

impl std::fmt::Display for File {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for (name, def) in self.defs.iter() {
            match def {
                Def::Alias { ty } => writeln!(f, "type {name} = {ty}")?,
                Def::Record { fields } => {
                    writeln!(f, "type {name} = {{")?;
                    for (field_name, ty) in fields {
                        writeln!(f, "  {field_name}: {ty};")?;
                    }
                    writeln!(f, "}}")?;
                }
                Def::Variant { variants } => {
                    writeln!(f, "type {name} =")?;
                    for (variant_name, fields) in variants {
                        write!(f, "  | {variant_name}")?;
                        if let Some(fields) = fields {
                            write!(f, " of {fields}")?;
                        }
                        writeln!(f)?;
                    }
                }
            }
        }
        Ok(())
    }
}

pub enum Def {
    Alias {
        ty: Type,
    },
    Record {
        fields: Vec<(FieldName, Type)>,
    },
    Variant {
        variants: Vec<(VariantName, Option<VariantFields>)>,
    },
}

pub enum VariantFields {
    Unnamed(Vec<Type>),
    Named(Vec<(FieldName, Type)>),
}

impl std::fmt::Display for VariantFields {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Unnamed(fields) => {
                let mut iter = fields.iter();
                let ty = iter.next().expect("empty VariantFields::Unnamed");
                ty.fmt(f)?;
                for ty in iter {
                    write!(f, " * {ty}")?;
                }
                Ok(())
            }
            Self::Named(fields) => {
                writeln!(f, "{{")?;
                for (name, ty) in fields {
                    writeln!(f, "    {name}: {ty};")?;
                }
                write!(f, "}}")
            }
        }
    }
}

pub enum Type {
    Path(TypePath),
    Tuple(TypeTuple),
}

impl std::fmt::Display for Type {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Type::Path(ty) => ty.fmt(f),
            Type::Tuple(ty) => ty.fmt(f),
        }
    }
}

pub struct TypePath {
    pub idents: Vec<String>,
}

impl TypePath {
    pub fn simple(id: impl Into<String>) -> Self {
        Self {
            idents: vec![id.into()],
        }
    }
}

impl std::fmt::Display for TypePath {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.idents.join("."))
    }
}

pub struct TypeTuple {
    pub elems: Vec<Type>,
}

impl std::fmt::Display for TypeTuple {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let mut elems = self.elems.iter();
        let elem = elems.next().expect("empty TypeTuple");
        write!(f, "{elem}")?;
        for elem in elems {
            write!(f, " * {elem}")?;
        }
        Ok(())
    }
}

#[derive(Clone, Hash, PartialEq, Eq, Display)]
pub struct TypeName(pub String);

#[derive(Clone, Hash, PartialEq, Eq, Display)]
pub struct FieldName(pub String);

#[derive(Clone, Hash, PartialEq, Eq, Display)]
pub struct VariantName(pub String);
