// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hash::IndexMap;

pub struct File {
    pub defs: IndexMap<String, Def>,
}

impl std::fmt::Display for File {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for (name, def) in self.defs.iter() {
            match def {
                Def::Alias { ty } => write!(f, "type {name} = {ty}\n")?,
                Def::Type => write!(f, "type {name}\n")?,
                Def::Record { fields } => {
                    write!(f, "type {name} = {{\n")?;
                    for (field_name, ty) in fields {
                        write!(f, "  {field_name}: {ty};\n")?;
                    }
                    write!(f, "}}\n")?;
                }
            }
        }
        Ok(())
    }
}

pub enum Def {
    Alias { ty: Type },
    Record { fields: Vec<(String, Type)> },
    Type,
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
