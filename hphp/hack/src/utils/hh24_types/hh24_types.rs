// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Common types used in the HH24 Hack typechecker rearchitecture.

use hh_hash::hash;
use typing_deps_hash::DepType;

#[derive(thiserror::Error, Debug)]
pub enum HhError {
    #[error("Unexpected: {0:#}")]
    Unexpected(anyhow::Error),

    #[error("Disk changed: {0} - do hh_decl --update then restart the operation. [{1}]")]
    DiskChanged(std::path::PathBuf, String),

    #[error("Decl-store changed its checksum: {0} - restart the operation. [{1}]")]
    ChecksumChanged(Checksum, String),

    #[error("Decl-store stopped - abandon the operation. [{0}]")]
    Stopped(String),
}

pub trait HhErrorContext<T> {
    fn hh_context(self, context: &'static str) -> Result<T, HhError>;
}

impl<T> HhErrorContext<T> for Result<T, HhError> {
    fn hh_context(self, ctx: &'static str) -> Result<T, HhError> {
        match self {
            Ok(r) => Ok(r),
            Err(HhError::Unexpected(err)) => Err(HhError::Unexpected(err.context(ctx))),
            Err(HhError::DiskChanged(path, ctx0)) => {
                Err(HhError::DiskChanged(path, format!("{}\n{}", ctx, ctx0)))
            }
            Err(HhError::ChecksumChanged(checksum, ctx0)) => Err(HhError::ChecksumChanged(
                checksum,
                format!("{}\n{}", ctx, ctx0),
            )),
            Err(HhError::Stopped(ctx0)) => Err(HhError::Stopped(format!("{}\n{}", ctx, ctx0))),
        }
    }
}

impl<T> HhErrorContext<T> for Result<T, std::io::Error> {
    fn hh_context(self, context: &'static str) -> Result<T, HhError> {
        self.map_err(|err| HhError::Unexpected(anyhow::Error::new(err).context(context)))
    }
}

impl<T> HhErrorContext<T> for Result<T, serde_json::error::Error> {
    fn hh_context(self, context: &'static str) -> Result<T, HhError> {
        self.map_err(|err| HhError::Unexpected(anyhow::Error::new(err).context(context)))
    }
}

impl<T> HhErrorContext<T> for Result<T, anyhow::Error> {
    fn hh_context(self, context: &'static str) -> Result<T, HhError> {
        self.map_err(|err| HhError::Unexpected(err.context(context)))
    }
}

#[derive(Copy, Clone, Hash, PartialEq, Eq)]
#[derive(serde::Deserialize, serde::Serialize)]
#[derive(derive_more::UpperHex, derive_more::LowerHex)]
pub struct Checksum(pub u64);

impl Checksum {
    fn xor(&mut self, combined_hash: (ToplevelSymbolHash, DeclHash)) {
        self.0 ^= hash(&combined_hash);
    }

    pub fn addremove(&mut self, symbol_hash: ToplevelSymbolHash, decl_hash: DeclHash) {
        self.xor((symbol_hash, decl_hash));
    }
}

impl std::fmt::Debug for Checksum {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Checksum({:x})", self.0)
    }
}

impl std::fmt::Display for Checksum {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:x}", self.0)
    }
}

impl std::str::FromStr for Checksum {
    type Err = std::num::ParseIntError;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        Ok(Self(u64::from_str_radix(s, 16)?))
    }
}

impl nohash_hasher::IsEnabled for Checksum {}

#[derive(Clone, Debug)]
#[derive(serde::Deserialize, serde::Serialize)]
pub struct RichChecksum {
    pub checksum: Checksum,
    pub timestamp: Timestamp,
    pub example_symbol: String,
}

impl RichChecksum {
    pub fn to_brief_string(&self) -> String {
        format!(
            "RichChecksum({:x}@{}@{})",
            self.checksum,
            self.timestamp.unix_epoch_secs(),
            self.example_symbol
        )
    }
}

impl std::str::FromStr for RichChecksum {
    type Err = ParseRichChecksumError;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let s = s.trim_start_matches("RichChecksum(");
        let s = s.trim_end_matches(')');
        let mut iter = s.split('@');
        match (iter.next(), iter.next(), iter.next(), iter.next()) {
            (Some(checksum), Some(timestamp), Some(example_symbol), None) => Ok(Self {
                checksum: checksum
                    .parse()
                    .map_err(ParseRichChecksumError::InvalidChecksum)?,
                timestamp: timestamp
                    .parse()
                    .map_err(ParseRichChecksumError::InvalidTimestamp)?,
                example_symbol: String::from(example_symbol),
            }),
            _ => Err(ParseRichChecksumError::Invalid),
        }
    }
}

#[derive(thiserror::Error, Debug)]
pub enum ParseRichChecksumError {
    #[error("expected \"RichChecksum(<checksum>@<timestamp>@<example_symbol>)\"")]
    Invalid,
    #[error("{0}")]
    InvalidChecksum(#[source] std::num::ParseIntError),
    #[error("{0}")]
    InvalidTimestamp(#[source] std::num::ParseIntError),
}

/// A measurement of the system clock, useful for talking to external entities
/// like the file system or other processes. Wraps `std::time::SystemTime`, but
/// implements `serde::Serialize` and `serde::Deserialize`.
///
/// Invariant: always represents a time later than the unix epoch.
#[derive(Copy, Clone)]
pub struct Timestamp(std::time::SystemTime);

impl Timestamp {
    /// Returns the system time corresponding to "now".
    pub fn now() -> Self {
        Self(std::time::SystemTime::now())
    }

    /// Returns the system time corresponding to the unix epoch plus the given
    /// number of seconds.
    pub fn from_unix_epoch_secs(secs: u64) -> Self {
        Self(
            std::time::SystemTime::UNIX_EPOCH
                .checked_add(std::time::Duration::from_secs(secs))
                .expect("Seconds since UNIX_EPOCH too large to fit in SystemTime"),
        )
    }

    /// Returns the number of seconds elapsed between the unix epoch and this
    /// `Timestamp`.
    pub fn unix_epoch_secs(&self) -> u64 {
        self.0
            .duration_since(std::time::SystemTime::UNIX_EPOCH)
            .expect("Timestamp before UNIX_EPOCH")
            .as_secs()
    }

    /// Returns the `SystemTime` corresponding to this `Timestamp`.
    pub fn as_system_time(&self) -> std::time::SystemTime {
        self.0
    }
}

impl std::fmt::Debug for Timestamp {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Timestamp({})", self.unix_epoch_secs())
    }
}

impl std::str::FromStr for Timestamp {
    type Err = std::num::ParseIntError;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let s = s.trim_start_matches("Timestamp(");
        let s = s.trim_end_matches(')');
        Ok(Self::from_unix_epoch_secs(s.parse()?))
    }
}

impl serde::Serialize for Timestamp {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.serialize_u64(self.unix_epoch_secs())
    }
}

impl<'de> serde::Deserialize<'de> for Timestamp {
    fn deserialize<D: serde::Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Visitor;
        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = Timestamp;

            fn expecting(&self, formatter: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(formatter, "a u64 for Timestamp")
            }
            fn visit_u64<E: serde::de::Error>(self, value: u64) -> Result<Self::Value, E> {
                Ok(Self::Value::from_unix_epoch_secs(value))
            }
        }
        deserializer.deserialize_u64(Visitor)
    }
}

// Common impls for types which wrap a hash value represented by u64.
macro_rules! u64_hash_wrapper_impls {
    ($name:ident) => {
        impl $name {
            #[inline]
            pub fn from_u64(hash: u64) -> Self {
                Self(hash)
            }
            #[inline]
            pub fn as_u64(self) -> u64 {
                self.0
            }
        }

        impl std::fmt::Debug for $name {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(f, concat!(stringify!($name), "({:x})"), self.0)
            }
        }

        impl std::str::FromStr for $name {
            type Err = std::num::ParseIntError;
            fn from_str(s: &str) -> Result<Self, Self::Err> {
                Ok(Self(u64::from_str_radix(s, 16)?))
            }
        }

        impl nohash_hasher::IsEnabled for $name {}

        impl rusqlite::ToSql for $name {
            fn to_sql(&self) -> rusqlite::Result<rusqlite::types::ToSqlOutput<'_>> {
                Ok(rusqlite::types::ToSqlOutput::from(self.0 as i64))
            }
        }

        impl rusqlite::types::FromSql for $name {
            fn column_result(
                value: rusqlite::types::ValueRef<'_>,
            ) -> rusqlite::types::FromSqlResult<Self> {
                Ok(Self(value.as_i64()? as u64))
            }
        }
    };
}

/// The hash of a toplevel symbol name, as it appears in the 64bit dependency graph.
#[derive(Copy, Clone, PartialEq, Eq, Hash, PartialOrd, Ord)]
#[derive(serde::Deserialize, serde::Serialize)]
#[derive(derive_more::UpperHex, derive_more::LowerHex)]
pub struct ToplevelSymbolHash(u64);
u64_hash_wrapper_impls! { ToplevelSymbolHash }

impl ToplevelSymbolHash {
    // TODO: Should be called `new`.
    pub fn from(dep_type: typing_deps_hash::DepType, symbol: &str) -> Self {
        assert!(dep_type.is_toplevel_symbol());
        Self(typing_deps_hash::hash1(dep_type, symbol.as_bytes()))
    }

    pub fn from_type(symbol: &str) -> Self {
        Self::from(typing_deps_hash::DepType::Type, symbol)
    }

    pub fn from_fun(symbol: &str) -> Self {
        Self::from(typing_deps_hash::DepType::Fun, symbol)
    }

    pub fn from_const(symbol: &str) -> Self {
        Self::from(typing_deps_hash::DepType::GConst, symbol)
    }

    pub fn from_module(symbol: &str) -> Self {
        Self::from(typing_deps_hash::DepType::Module, symbol)
    }

    #[inline(always)]
    pub fn to_bytes(self) -> [u8; 8] {
        self.0.to_be_bytes()
    }

    #[inline(always)]
    pub fn from_bytes(bs: [u8; 8]) -> Self {
        Self(u64::from_be_bytes(bs))
    }

    #[inline(always)]
    pub fn to_dependency_hash(self) -> DependencyHash {
        DependencyHash(self.0)
    }
}

impl From<ToplevelSymbolHash> for depgraph::dep::Dep {
    fn from(symbol_hash: ToplevelSymbolHash) -> depgraph::dep::Dep {
        depgraph::dep::Dep::new(symbol_hash.0)
    }
}

/// The "canon hash" of a toplevel symbol name (i.e., the hash of the symbol
/// name after ASCII characters in the name have been converted to lowercase),
/// as it appears in the naming table.
#[derive(Copy, Clone, PartialEq, Eq, Hash, PartialOrd, Ord)]
#[derive(serde::Deserialize, serde::Serialize)]
#[derive(derive_more::UpperHex, derive_more::LowerHex)]
pub struct ToplevelCanonSymbolHash(u64);
u64_hash_wrapper_impls! { ToplevelCanonSymbolHash }

impl ToplevelCanonSymbolHash {
    // TODO: Should be called `new`.
    pub fn from(dep_type: typing_deps_hash::DepType, mut symbol: String) -> Self {
        symbol.make_ascii_lowercase();
        Self(typing_deps_hash::hash1(dep_type, symbol.as_bytes()))
    }

    pub fn from_type(symbol: String) -> Self {
        Self::from(typing_deps_hash::DepType::Type, symbol)
    }

    pub fn from_fun(symbol: String) -> Self {
        Self::from(typing_deps_hash::DepType::Fun, symbol)
    }
}

/// The hash of a toplevel symbol name, or the hash of a class member name, or
/// an Extends or AllMembers hash for a class name.
/// See `Typing_deps.Dep.(dependency variant)`.
#[derive(
    Copy,
    Clone,
    PartialEq,
    Eq,
    PartialOrd,
    Ord,
    serde::Deserialize,
    serde::Serialize
)]
#[derive(derive_more::UpperHex, derive_more::LowerHex)]
pub struct DependencyHash(pub u64);

impl DependencyHash {
    pub fn of_member(
        dep_type: DepType,
        class_symbol: ToplevelSymbolHash,
        member_name: &str,
    ) -> DependencyHash {
        let type_hash = typing_deps_hash::hash1(DepType::Type, &class_symbol.to_bytes());
        Self(typing_deps_hash::hash2(
            dep_type,
            type_hash,
            member_name.as_bytes(),
        ))
    }

    pub fn of_class(dep_type: DepType, class_symbol: ToplevelSymbolHash) -> DependencyHash {
        Self(typing_deps_hash::hash1(dep_type, &class_symbol.to_bytes()))
    }
}

impl std::fmt::Debug for DependencyHash {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "DependencyHash({:x})", self.0)
    }
}

impl std::str::FromStr for DependencyHash {
    type Err = std::num::ParseIntError;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        Ok(Self(u64::from_str_radix(s, 16)?))
    }
}

// A `ToplevelSymbolHash` is a valid `DependencyHash`, but not all
// `DependencyHash` values represent a toplevel symbol.
impl From<ToplevelSymbolHash> for DependencyHash {
    fn from(hash: ToplevelSymbolHash) -> Self {
        Self(hash.0)
    }
}

impl From<DependencyHash> for ToplevelSymbolHash {
    fn from(hash: DependencyHash) -> Self {
        Self(hash.0)
    }
}

impl DependencyHash {
    #[inline(always)]
    pub fn to_bytes(self) -> [u8; 8] {
        self.0.to_be_bytes()
    }

    #[inline(always)]
    pub fn from_bytes(bs: [u8; 8]) -> Self {
        Self(u64::from_be_bytes(bs))
    }
}

#[derive(
    Copy,
    Clone,
    Debug,
    PartialEq,
    Eq,
    PartialOrd,
    Ord,
    serde::Deserialize,
    serde::Serialize
)]
pub struct DepgraphEdge {
    pub dependency: DependencyHash,
    pub dependent: ToplevelSymbolHash,
}

impl std::str::FromStr for DepgraphEdge {
    type Err = ParseDepgraphEdgeError;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let mut iter = s.split(':');
        match (iter.next(), iter.next(), iter.next()) {
            (Some(dependency), Some(dependent), None) => Ok(Self {
                dependency: dependency.parse()?,
                dependent: dependent.parse()?,
            }),
            _ => Err(ParseDepgraphEdgeError::Invalid),
        }
    }
}

#[derive(thiserror::Error, Debug)]
pub enum ParseDepgraphEdgeError {
    #[error("expected dependency_hash:dependent_hash")]
    Invalid,
    #[error("{0}")]
    FromInt(#[from] std::num::ParseIntError),
}

/// The position-insensitive hash of the `Decls` produced by running the direct
/// decl parser on a file. Used in the NAMING_FILE_INFO table.
#[derive(Copy, Clone, PartialEq, Eq, Hash, PartialOrd, Ord)]
#[derive(serde::Deserialize, serde::Serialize)]
#[derive(derive_more::UpperHex, derive_more::LowerHex)]
pub struct FileDeclsHash(u64);
u64_hash_wrapper_impls! { FileDeclsHash }

impl From<oxidized_by_ref::direct_decl_parser::Decls<'_>> for FileDeclsHash {
    fn from(decls: oxidized_by_ref::direct_decl_parser::Decls<'_>) -> Self {
        Self(hash(&decls))
    }
}

/// The position-insensitive hash of a decl (the type signature of a toplevel
/// declaration), as it appears in the naming table. Used in the NAMING_FUNS,
/// NAMING_CONSTS, and NAMING_TYPES tables (in the near future).
#[derive(Copy, Clone, PartialEq, Eq, Hash, PartialOrd, Ord)]
#[derive(serde::Deserialize, serde::Serialize)]
#[derive(derive_more::UpperHex, derive_more::LowerHex)]
pub struct DeclHash(u64);
u64_hash_wrapper_impls! { DeclHash }

use oxidized_by_ref::direct_decl_parser::Decl;
use oxidized_by_ref::shallow_decl_defs;

impl From<Decl<'_>> for DeclHash {
    fn from(decl: Decl<'_>) -> Self {
        match decl {
            Decl::Class(decl) => decl.into(),
            Decl::Fun(decl) => decl.into(),
            Decl::Typedef(decl) => decl.into(),
            Decl::Const(decl) => decl.into(),
            Decl::Module(decl) => decl.into(),
        }
    }
}

impl From<&shallow_decl_defs::ClassDecl<'_>> for DeclHash {
    fn from(decl: &shallow_decl_defs::ClassDecl<'_>) -> Self {
        Self(hash(decl))
    }
}

impl From<&shallow_decl_defs::FunDecl<'_>> for DeclHash {
    fn from(decl: &shallow_decl_defs::FunDecl<'_>) -> Self {
        Self(hash(decl))
    }
}

impl From<&shallow_decl_defs::TypedefDecl<'_>> for DeclHash {
    fn from(decl: &shallow_decl_defs::TypedefDecl<'_>) -> Self {
        Self(hash(decl))
    }
}

impl From<&shallow_decl_defs::ConstDecl<'_>> for DeclHash {
    fn from(decl: &shallow_decl_defs::ConstDecl<'_>) -> Self {
        Self(hash(decl))
    }
}

impl From<&shallow_decl_defs::ModuleDecl<'_>> for DeclHash {
    fn from(decl: &shallow_decl_defs::ModuleDecl<'_>) -> Self {
        Self(hash(decl))
    }
}

/// This type is for serializing an anyhow error. What you get out will print with
/// the same information as the original anyhow, but won't look quite as pretty
/// and doesn't support downcasting.
#[derive(Debug, serde::Serialize, serde::Deserialize)]
pub struct StringifiedError {
    pub chain: Vec<String>, // invariant: this has at least 1 element
    pub backtrace: String,
}

impl StringifiedError {
    pub fn from_anyhow(err: anyhow::Error) -> Self {
        let chain = err.chain().map(|c| format!("{}", c)).rev().collect();
        let backtrace = format!("{:?}", err);
        Self { chain, backtrace }
    }

    pub fn to_anyhow(self) -> anyhow::Error {
        let mut e = anyhow::anyhow!("StringifiedError");
        e = e.context(self.backtrace);
        for cause in self.chain {
            e = e.context(cause);
        }
        e
    }
}

impl std::fmt::Display for StringifiedError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.chain[self.chain.len() - 1])
    }
}

#[cfg(test)]
mod tests {
    use anyhow::Context;

    use super::*; // to bring Result<T,E>.context into scope

    fn stringify_inner() -> anyhow::Result<()> {
        anyhow::bail!("oops");
    }

    fn stringify_middle() -> anyhow::Result<()> {
        stringify_inner().context("ctx_middle")
    }

    fn stringify_outer() -> anyhow::Result<()> {
        stringify_middle().context("ctx_outer")
    }

    #[test]
    fn stringify_without_backtrace() {
        match stringify_outer() {
            Ok(()) => panic!("test wanted to see an error"),
            Err(err1) => {
                let err2 = StringifiedError::from_anyhow(err1);
                let err3 = err2.to_anyhow();
                let display = format!("{}", err3);
                assert_eq!(display, "ctx_outer");
                let debug = format!("{:?}", err3);
                assert!(debug.contains("ctx_outer"));
                assert!(debug.contains("0: ctx_middle"));
                assert!(debug.contains("1: oops"));
            }
        }
    }
}
