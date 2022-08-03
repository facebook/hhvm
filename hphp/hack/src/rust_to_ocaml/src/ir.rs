// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use convert_case::Case;
use convert_case::Casing;
use derive_more::Display;
use hash::IndexMap;

pub struct File {
    pub defs: IndexMap<TypeName, Def>,
}

impl std::fmt::Display for File {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for (name, def) in self.defs.iter() {
            match def {
                Def::Alias { doc, tparams, ty } => {
                    write_toplevel_doc_comment(f, doc)?;
                    write!(f, "type ")?;
                    write_type_parameters(f, tparams)?;
                    writeln!(f, "{name} = {ty}")?
                }
                Def::Record {
                    doc,
                    tparams,
                    fields,
                } => {
                    write_toplevel_doc_comment(f, doc)?;
                    write!(f, "type ")?;
                    write_type_parameters(f, tparams)?;
                    writeln!(f, "{name} = {{")?;
                    for field in fields {
                        writeln!(f, "  {field}")?;
                    }
                    writeln!(f, "}}")?;
                }
                Def::Variant {
                    doc,
                    tparams,
                    variants,
                } => {
                    write_toplevel_doc_comment(f, doc)?;
                    write!(f, "type ")?;
                    write_type_parameters(f, tparams)?;
                    writeln!(f, "{name} =")?;
                    for variant in variants {
                        writeln!(f, "  | {variant}")?;
                    }
                }
            }
            write!(f, "\n")?;
        }
        Ok(())
    }
}

pub enum Def {
    Alias {
        doc: Vec<String>,
        tparams: Vec<String>,
        ty: Type,
    },
    Record {
        doc: Vec<String>,
        tparams: Vec<String>,
        fields: Vec<Field>,
    },
    Variant {
        doc: Vec<String>,
        tparams: Vec<String>,
        variants: Vec<Variant>,
    },
}

pub struct Variant {
    pub name: VariantName,
    pub fields: Option<VariantFields>,
    pub doc: Vec<String>,
}

impl std::fmt::Display for Variant {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let Self { name, fields, doc } = self;
        write!(f, "{name}")?;
        if let Some(fields) = fields {
            write!(f, " of {fields}")?;
        }
        write_field_or_variant_doc_comment(f, doc)?;
        Ok(())
    }
}

pub enum VariantFields {
    Unnamed(Vec<Type>),
    Named(Vec<Field>),
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
                for field in fields {
                    writeln!(f, "    {field}")?;
                }
                write!(f, "}}")
            }
        }
    }
}

pub struct Field {
    pub name: FieldName,
    pub ty: Type,
    pub doc: Vec<String>,
}

impl std::fmt::Display for Field {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let Self { name, ty, doc } = self;
        write!(f, "{name}: {ty};")?;
        write_field_or_variant_doc_comment(f, doc)?;
        Ok(())
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
        write!(f, "(")?;
        let mut elems = self.elems.iter();
        let elem = elems.next().expect("empty TypeTuple");
        write!(f, "{elem}")?;
        for elem in elems {
            write!(f, " * {elem}")?;
        }
        write!(f, ")")
    }
}

#[derive(Clone, Hash, PartialEq, Eq, Display)]
pub struct TypeName(pub String);

#[derive(Clone, Hash, PartialEq, Eq, Display)]
pub struct FieldName(pub String);

#[derive(Clone, Hash, PartialEq, Eq, Display)]
pub struct VariantName(pub String);

fn write_toplevel_doc_comment(
    f: &mut std::fmt::Formatter<'_>,
    doc: &Vec<String>,
) -> std::fmt::Result {
    if doc.is_empty() {
        return Ok(());
    }
    write!(f, "(**{}", doc.join("\n *"))?;
    if doc.len() == 1 {
        if !doc[0].contains('\n') {
            write!(f, " ")?;
        }
    } else {
        write!(f, "\n ")?;
    }
    write!(f, "*)\n")?;
    Ok(())
}

fn write_field_or_variant_doc_comment(
    f: &mut std::fmt::Formatter<'_>,
    doc: &Vec<String>,
) -> std::fmt::Result {
    if doc.is_empty() {
        return Ok(());
    }
    let joined = doc.join("\n       *");
    write!(f, "(**{}", joined)?;
    if !joined.ends_with(' ') {
        write!(f, " ")?;
    }
    write!(f, "*)\n")?;
    Ok(())
}

fn write_type_parameters(f: &mut std::fmt::Formatter<'_>, tparams: &[String]) -> std::fmt::Result {
    for tparam in tparams.iter() {
        write!(f, "'{} ", tparam.to_case(Case::Snake))?;
    }
    Ok(())
}
