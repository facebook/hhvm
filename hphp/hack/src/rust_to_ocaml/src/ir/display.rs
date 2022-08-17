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
                attrs,
                tparams,
                name,
                ty,
            } => {
                write_toplevel_doc_comment(f, doc)?;
                write!(f, "type ")?;
                write_type_parameters(f, tparams)?;
                write!(f, "{name} = {ty}")?;
                for attr in attrs {
                    write!(f, " [@@{}]", attr)?;
                }
                writeln!(f)?;
            }
            Self::Record {
                doc,
                attrs,
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
                write!(f, "}}")?;
                for attr in attrs {
                    write!(f, " [@@{}]", attr)?;
                }
                writeln!(f)?;
            }
            Self::Variant {
                doc,
                attrs,
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
                for attr in attrs {
                    writeln!(f)?;
                    write!(f, "[@@{}]", attr)?;
                }
                writeln!(f)?;
            }
        }
        writeln!(f)?;
        Ok(())
    }
}

impl Display for ir::Variant {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let Self {
            name,
            fields,
            doc,
            attrs,
        } = self;
        write!(f, "{name}")?;
        if let Some(fields) = fields {
            write!(f, " of {fields}")?;
        }
        for attr in attrs {
            write!(f, " [@{}]", attr)?;
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
        let Self {
            name,
            ty,
            doc,
            attrs,
        } = self;
        write!(f, "{name}: {ty};")?;
        for attr in attrs {
            write!(f, " [@{}]", attr)?;
        }
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
        let name = &self.0;
        let mut first_char = name.chars().next().unwrap(); // Invariant: self.0 is nonempty
        // OCaml modules _must_ start with an uppercase letter (the OCaml parser
        // depends on this). We ensure in `ModuleName`'s constructor that the
        // first character is ASCII, so we can use `make_ascii_uppercase`.
        first_char.make_ascii_uppercase();
        assert!(first_char.is_ascii_uppercase());
        write!(f, "{}", first_char)?;
        write!(f, "{}", &name[1..])
    }
}

fn is_ocaml_keyword(name: &str) -> bool {
    match name {
        "and" | "as" | "assert" | "asr" | "begin" | "class" | "constraint" | "do" | "done"
        | "downto" | "else" | "end" | "exception" | "external" | "false" | "for" | "fun"
        | "function" | "functor" | "if" | "in" | "include" | "inherit" | "initializer" | "land"
        | "lazy" | "let" | "lor" | "lsl" | "lsr" | "lxor" | "match" | "method" | "mod"
        | "module" | "mutable" | "new" | "nonrec" | "object" | "of" | "open" | "or" | "private"
        | "rec" | "sig" | "struct" | "then" | "to" | "true" | "try" | "type" | "val"
        | "virtual" | "when" | "while" | "with" => true,
        _ => false,
    }
}

impl Display for ir::TypeName {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let name = self.0.to_case(Case::Snake);
        if is_ocaml_keyword(name.as_str()) {
            write!(f, "{}_", name)
        } else {
            name.fmt(f)
        }
    }
}

impl Display for ir::FieldName {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let name = self.0.to_case(Case::Snake);
        if is_ocaml_keyword(name.as_str()) {
            write!(f, "{}_", name)
        } else {
            name.fmt(f)
        }
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
    match tparams {
        [] => {}
        [tparam] => write!(f, "'{} ", tparam.to_case(Case::Snake))?,
        [first, rest @ ..] => {
            write!(f, "('{}", first.to_case(Case::Snake))?;
            for tparam in rest {
                write!(f, ", '{} ", tparam.to_case(Case::Snake))?;
            }
            write!(f, ") ")?;
        }
    }
    Ok(())
}
