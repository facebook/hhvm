// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub mod external;

use bincode::Options;
use oxidized_by_ref::{
    direct_decl_parser::Decls,
    file_info::NameType,
    shallow_decl_defs::{ClassDecl, Decl, TypedefDecl},
};
use thiserror::Error;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(Debug, Error)]
pub enum Error {
    #[error("Decl not found")]
    NotFound,

    #[error(transparent)]
    Bincode(#[from] bincode::Error),
}

pub enum TypeDecl<'a> {
    Class(&'a ClassDecl<'a>),
    Typedef(&'a TypedefDecl<'a>),
}

pub trait DeclProvider<'a>: std::fmt::Debug {
    fn decl(&self, kind: NameType, symbol: &str) -> Result<Decl<'a>>;

    fn type_decl(&self, symbol: &str) -> Result<TypeDecl<'a>> {
        match self.decl(NameType::Class, symbol)? {
            Decl::Class(c) => Ok(TypeDecl::Class(c)),
            Decl::Typedef(a) => Ok(TypeDecl::Typedef(a)),
            x @ Decl::Fun(_) | x @ Decl::Const(_) | x @ Decl::Module(_) => {
                // We asked for a NameType::Class and got something weird back.
                // This must be a bug in decl() because types/funcs/constants
                // are different kinds of names.
                panic!(
                    "Unexpected Decl kind from DeclProvider::decl(NameType::Class): {:?}",
                    x
                )
            }
        }
    }
}

pub fn serialize_decls(decls: &Decls<'_>) -> Result<Vec<u8>, bincode::Error> {
    bincode::options().with_native_endian().serialize(decls)
}

pub fn deserialize_decls<'a>(
    arena: &'a bumpalo::Bump,
    data: &[u8],
) -> Result<Decls<'a>, bincode::Error> {
    use arena_deserializer::serde::Deserialize;
    let op = bincode::options().with_native_endian();
    let mut de = bincode::de::Deserializer::from_slice(data, op);
    let de = arena_deserializer::ArenaDeserializer::new(arena, &mut de);
    Decls::deserialize(de)
}
