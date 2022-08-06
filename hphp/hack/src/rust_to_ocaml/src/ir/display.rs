// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Display;
use std::fmt::Formatter;
use std::fmt::Result;

use convert_case::Case;
use convert_case::Casing;

use crate::ir;

impl Display for ir::File {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        self.root.fmt(f)
    }
}

impl Display for ir::Module {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        for def in self.defs.iter() {
            def.fmt(f)?
        }
        Ok(())
    }
}

impl Display for ir::Def {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        match self {
            Self::Module(module) => {
                writeln!(f, "module {} = struct", module.name)?;
                module.fmt(f)?;
                writeln!(f, "end")?
            }
            Self::Alias {
                doc,
                tparams,
                name,
                ty,
            } => {
                write_toplevel_doc_comment(f, doc)?;
                write!(f, "type ")?;
                write_type_parameters(f, tparams)?;
                writeln!(f, "{name} = {ty}")?
            }
            Self::Record {
                doc,
                tparams,
                name,
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
            Self::Variant {
                doc,
                tparams,
                name,
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
        writeln!(f)?;
        Ok(())
    }
}

impl Display for ir::Variant {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let Self { name, fields, doc } = self;
        write!(f, "{name}")?;
        if let Some(fields) = fields {
            write!(f, " of {fields}")?;
        }
        write_field_or_variant_doc_comment(f, doc)?;
        Ok(())
    }
}

impl Display for ir::VariantFields {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
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

impl Display for ir::Field {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let Self { name, ty, doc } = self;
        write!(f, "{name}: {ty};")?;
        write_field_or_variant_doc_comment(f, doc)?;
        Ok(())
    }
}

impl Display for ir::Type {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        match self {
            Self::Path(ty) => ty.fmt(f),
            Self::Tuple(ty) => ty.fmt(f),
        }
    }
}

impl Display for ir::TypePath {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        match self.targs.as_slice() {
            [] => {}
            [targ] => write!(f, "{} ", targ)?,
            [first, rest @ ..] => {
                write!(f, "({}", first)?;
                for targ in rest {
                    write!(f, ", {}", targ)?;
                }
                write!(f, ") ")?;
            }
        }
        for module in self.modules.iter() {
            write!(f, "{}.", module)?;
        }
        write!(f, "{}", self.ty)
    }
}

impl Display for ir::TypeTuple {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
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

impl Display for ir::ModuleName {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        self.0.to_case(Case::UpperCamel).fmt(f)
    }
}

impl Display for ir::TypeName {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        self.0.to_case(Case::Snake).fmt(f)
    }
}

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
    writeln!(f, "*)")?;
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
    writeln!(f, "*)")?;
    Ok(())
}

fn write_type_parameters(f: &mut std::fmt::Formatter<'_>, tparams: &[String]) -> std::fmt::Result {
    for tparam in tparams.iter() {
        write!(f, "'{} ", tparam.to_case(Case::Snake))?;
    }
    Ok(())
}
