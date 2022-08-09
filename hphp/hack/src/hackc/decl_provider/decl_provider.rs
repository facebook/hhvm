// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bincode::Options;
use oxidized_by_ref::direct_decl_parser::Decls;
use oxidized_by_ref::shallow_decl_defs::ClassDecl;
use oxidized_by_ref::shallow_decl_defs::ConstDecl;
use oxidized_by_ref::shallow_decl_defs::Decl;
use oxidized_by_ref::shallow_decl_defs::FunDecl;
use oxidized_by_ref::shallow_decl_defs::ModuleDecl;
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

/// DeclProvider is an interface for requesting named decl data required
/// for bytecode compilation.
///
/// The provider is supplied by the client requesting a bytecode
/// compilation. This client may wish to cache the compiled bytecode and
/// will record the various symbols observed during compilation for the
/// purpose of generating a cache key.
///
/// Methods are provided for each distinct kind of name (starting with
/// types, for now).
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
///     type MyType = Bar<Biz, Buz>;
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
pub trait DeclProvider: std::fmt::Debug {
    /// Get a decl for the given type name and depth.
    /// * `symbol` - the name of the symbol being requested
    /// * `depth` - a hint to the provider about the number of layers of decl
    ///             request traversed to arrive at this request
    fn type_decl(&self, symbol: &str, depth: u64) -> Result<TypeDecl<'_>>;

    fn func_decl(&self, symbol: &str) -> Result<&'_ FunDecl<'_>>;
    fn const_decl(&self, symbol: &str) -> Result<&'_ ConstDecl<'_>>;
    fn module_decl(&self, symbol: &str) -> Result<&'_ ModuleDecl<'_>>;
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

pub fn find_type_decl<'a>(decls: &Decls<'a>, needle: &str) -> Result<TypeDecl<'a>> {
    decls
        .types()
        .find_map(|(name, decl)| match decl {
            Decl::Class(c) if needle.eq_ignore_ascii_case(name) => Some(TypeDecl::Class(c)),
            Decl::Typedef(c) if needle.eq_ignore_ascii_case(name) => Some(TypeDecl::Typedef(c)),
            Decl::Class(_) | Decl::Typedef(_) => None,
            Decl::Fun(_) | Decl::Const(_) | Decl::Module(_) => unreachable!(),
        })
        .ok_or(Error::NotFound)
}

pub fn find_func_decl<'a>(decls: &Decls<'a>, needle: &str) -> Result<&'a FunDecl<'a>> {
    decls
        .funs()
        .find_map(|(name, decl)| {
            if needle.eq_ignore_ascii_case(name) {
                Some(decl)
            } else {
                None
            }
        })
        .ok_or(Error::NotFound)
}

pub fn find_const_decl<'a>(decls: &Decls<'a>, needle: &str) -> Result<&'a ConstDecl<'a>> {
    decls
        .consts()
        .find_map(|(name, decl)| if needle == name { Some(decl) } else { None })
        .ok_or(Error::NotFound)
}

pub fn find_module_decl<'a>(decls: &Decls<'a>, needle: &str) -> Result<&'a ModuleDecl<'a>> {
    decls
        .modules()
        .find_map(|(name, decl)| if needle == name { Some(decl) } else { None })
        .ok_or(Error::NotFound)
}
