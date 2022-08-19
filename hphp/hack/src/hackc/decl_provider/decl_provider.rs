// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod memo_provider;

use std::io::BufReader;
use std::io::BufWriter;
use std::io::Write;

use arena_deserializer::serde::Deserialize;
use arena_deserializer::ArenaDeserializer;
use bincode::Options;
pub use memo_provider::MemoProvider;
use oxidized_by_ref::direct_decl_parser::Decls;
use oxidized_by_ref::shallow_decl_defs::ClassDecl;
use oxidized_by_ref::shallow_decl_defs::ConstDecl;
use oxidized_by_ref::shallow_decl_defs::Decl;
use oxidized_by_ref::shallow_decl_defs::FunDecl;
use oxidized_by_ref::shallow_decl_defs::ModuleDecl;
use oxidized_by_ref::shallow_decl_defs::TypedefDecl;
use sha1::Digest;
use sha1::Sha1;
use thiserror::Error;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(Debug, Error)]
pub enum Error {
    #[error("Decl not found")]
    NotFound,

    #[error(transparent)]
    Bincode(#[from] bincode::Error),
}

#[derive(Clone, Copy)]
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

/// Serialize decls into an opaque blob suffixed with a Sha1 content hash.
pub fn serialize_decls(decls: &Decls<'_>) -> Result<Vec<u8>, bincode::Error> {
    const ZSTD_LEVEL: i32 = 0;
    let mut blob = Vec::new();
    let w = zstd::stream::write::Encoder::new(&mut blob, ZSTD_LEVEL)?.auto_finish();
    let w = BufWriter::new(w);
    bincode::options()
        .with_native_endian()
        .serialize_into(w, decls)?;
    let mut digest = Sha1::new();
    digest.update(&blob);
    blob.write_all(&digest.finalize())?;
    Ok(blob)
}

/// Deserialize decls. Panic in cfg(debug) if the content hash is wrong.
pub fn deserialize_decls<'a>(
    arena: &'a bumpalo::Bump,
    data: &[u8],
) -> Result<Decls<'a>, bincode::Error> {
    let (data, hash) = split_serialized_decls(data);
    debug_assert!({
        let mut digest = Sha1::new();
        digest.update(data);
        digest.finalize().to_vec() == hash
    });
    let r = zstd::stream::read::Decoder::new(data)?;
    let r = BufReader::new(r);
    let op = bincode::options().with_native_endian();
    let mut de = bincode::de::Deserializer::with_reader(r, op);
    let de = ArenaDeserializer::new(arena, &mut de);
    Decls::deserialize(de)
}

fn split_serialized_decls(data: &[u8]) -> (&[u8], &[u8]) {
    assert!(data.len() >= Sha1::output_size());
    let split = data.len() - Sha1::output_size();
    (&data[0..split], &data[split..])
}

/// Recover the content hash that was appended to serialized decls.
pub fn decls_content_hash(data: &[u8]) -> &[u8] {
    split_serialized_decls(data).1
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
