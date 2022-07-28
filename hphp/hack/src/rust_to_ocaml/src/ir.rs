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
            }
        }
        Ok(())
    }
}

pub enum Def {
    Alias { ty: TypeRef },
    Type,
}

pub struct TypeRef {
    pub idents: Vec<String>,
}

impl TypeRef {
    pub fn simple(id: impl Into<String>) -> Self {
        Self {
            idents: vec![id.into()],
        }
    }
}

impl std::fmt::Display for TypeRef {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.idents.join("."))
    }
}
