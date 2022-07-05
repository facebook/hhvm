// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub mod external;

use bincode::Options;
use oxidized_by_ref::direct_decl_parser::Decls;
use oxidized_by_ref::file_info::NameType;
use oxidized_by_ref::shallow_decl_defs::ClassDecl;
use oxidized_by_ref::shallow_decl_defs::Decl;
use oxidized_by_ref::shallow_decl_defs::TypedefDecl;
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
    /// Requests shallow decl data for a named symbol required for bytecode
    /// compilation.
    ///
    /// The provider is supplied by the client requesting a bytecode
    /// compilation. This client may wish to cache the compiled bytecode and
    /// will record the various symbols observed during compilation for the
    /// purpose of generating a cache key.
    ///
    /// A special depth parameter is supplied to the client indicating how many
    /// levels of get requests were traversed to arrive at the current
    /// request. It may be used in an implementation specified manner to improve
    /// caching.
    ///
    /// As an example consider these source files:
    ///
    /// a.php:
    ///
    ///   <?hh
    ///
    ///   function foo(MyType $bar): void { ... }
    ///
    /// b.php:
    ///
    ///     <?hh
    ///
    ///     newtype MyType = Bar<Biz, Buz>;
    ///
    /// If while compiling `a.php` a request is made for `MyType` the depth
    /// will be zero as the symbol is referenced directly from `a.php`. The
    /// shallow decl returned will be for a type alias to a `Bar`.
    ///
    /// Should the compiler now request `Bar` the depth should be one, as the
    /// lookup was an indirect reference. Likewise `Biz` and `Buz` would be
    /// requested with a depth of one.
    ///
    /// Further traversal into the type of Bar should it too be a type alias
    /// would be at a depth of two.
    ///
    /// # Arguments
    ///
    /// * `kind` - the type of symbol being requested
    /// * `symbol` - the name of the symbol being requested
    /// * `depth` - a hint to the provider about the number of layers of decl
    ///             request traversed to arrive at this request
    fn decl(&self, kind: NameType, symbol: &str, depth: u64) -> Result<Decl<'a>>;

    fn type_decl(&self, symbol: &str, depth: u64) -> Result<TypeDecl<'a>> {
        match self.decl(NameType::Class, symbol, depth)? {
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
