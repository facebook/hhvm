// Copyright (c) 2019; Facebook; Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! HackC emitter options.
//!
//! Canonical name for each option is chosen to be JSON key, which often
//! differs from the ones passed as CLI arguments via `-v KEY=VALUE`. The
//! names for arguments (non-flags) are derived via the `serde` framework
//! by taking the name of the field (or the one given via serde(rename=new_name)),
//! and the names for flags (boolean options) are derived by downcasing bitflags names.
//! The options are grouped their common prefix in their canonical names,
//! which is specified via macros `prefix_all` or `prefixed_flags`, respectively.
//! E.g., `prefix_all("hhvm.")`` or `prefixed_flags(..., "hhvm.", ...)` ensure that
//! an argument "emit_meth_caller_func_pointers" or flag LOG_EXTERN_COMPILER_PERF gets the
//! canonical name "hhvm.emit_meth_caller_func_pointers" or "hhvm.log_extern_compiler_perf", respectively.
//!
//! Non-canonical names (used when parsing from CLI) are specified by:
//! - `options_cli::CANON_BY_ALIAS.get("some_alias")`; and
//! - attribute `#[serde(alias = "some_alias")]`, for any non-flag argument.
//! The latter is mainly for convenience, so that JSON can be deserialized
//! even when the caller passes a CLI name (which would be understood for flags,
//! so it is also more consistent), but can also be handy for migration towards
//! consistent names between JSON and CLI.
//!
//! Example:
//! ```
//! let opts: Options = Options::default(); // JSON key
//! opts.doc_root.get();                    // doc_root
//! opts.hhvm.emit_meth_caller_func_pointers.set(42); // hhvm.emit_meth_caller_func_pointers
//! opts.hhvm_flags.contains(
//!     HhvmFlags::ENABLE_IMPLICIT_CONTEXT);          // hhvm.enable_implicit_context
//! opts.hhvm.hack_lang_flags.set(
//!     LangFlags::ENABLE_ENUM_CLASSES);      // hhvm.hack.lang.enable_enum_classes
//! ```

mod options_cli;

use std::cell::RefCell;
use std::collections::BTreeMap;
use std::ffi::OsStr;
use std::os::unix::ffi::OsStrExt;
use std::path::Path;
use std::path::PathBuf;

use bitflags::bitflags;
use bstr::BString;
use bstr::ByteSlice;
use hhbc::IncludePath;
use lru::LruCache;
use options_serde::prefix_all;
use oxidized::relative_path::RelativePath;
use serde_derive::Deserialize;
use serde_derive::Serialize;
use serde_json::json;
use serde_json::value::Value as Json;

/// Provides uniform access to bitflags-generated structs in JSON SerDe
trait PrefixedFlags:
    Sized
    + Copy
    + Default
    + std::fmt::Debug
    + std::ops::BitOrAssign
    + std::ops::BitAndAssign
    + std::ops::Not<Output = Self>
{
    const PREFIX: &'static str;

    // these methods (or equivalents) are implemented by bitflags!
    const EMPTY: Self;
    const ALL: Self;
    fn from_flags(flags: &Flags) -> Option<Self>;
    fn contains(&self, other: Self) -> bool;
    fn bits(&self) -> u64;
    fn to_map() -> BTreeMap<String, Self>;
}

macro_rules! prefixed_flags {
    ($class:ident, $prefix:expr, $($field:ident),*,) => { // require trailing comma

        bitflags! {
            pub struct $class: u64 {
                // TODO(leoo) expand RHS this into 1 << i, using equivalent of C++ index_sequence
                $( const $field = Flags::$field.bits(); )*
            }
        }
        impl PrefixedFlags for $class {
            const PREFIX: &'static str = $prefix;
            const EMPTY: Self = Self::empty();
            const ALL: Self = Self::all();

            // TODO(leoo) use proc_macro_hack and field_to_config_name!($field)
            // to map ("some.prefix", SOME_FIELD) into "some.prefix.some_field"
            // fn by_name(name: &'static str) -> Self {
            //     match name {
            //         $( case field_to_config_name!($prefix, $field) => Flags::$field, )*
            //     }
            // }
            fn to_map() -> BTreeMap<String, Self> {{
                let mut ret: BTreeMap<String, Self> = BTreeMap::new();
                $(
                    ret.insert(stringify!($field).to_lowercase(), Self::$field);
                )*
                ret
            }}

            fn contains(&self, other: Self) -> bool {
                self.contains(other)
            }

            fn bits(&self) -> u64 {
                self.bits()
            }

            fn from_flags(flags: &Flags) -> Option<Self> {
                Self::from_bits(flags.bits())
            }
        }
    }
}

/// An option of non-boolean type T (i.e., not a flag)
#[derive(Clone, Serialize, Deserialize, Debug, Default, PartialEq)]
pub struct Arg<T> {
    global_value: T,
}
impl<T> Arg<T> {
    pub fn get(&self) -> &T {
        &self.global_value
    }

    pub fn get_mut(&mut self) -> &mut T {
        &mut self.global_value
    }

    pub fn new(global_value: T) -> Arg<T> {
        Arg { global_value }
    }
}

// group options by JSON config prefix to avoid error-prone repetition & boilerplate in SerDe

prefixed_flags!(
    CompilerFlags,
    "hack.compiler.",
    CONSTANT_FOLDING,
    OPTIMIZE_NULL_CHECKS,
    RELABEL,
);
impl Default for CompilerFlags {
    fn default() -> CompilerFlags {
        CompilerFlags::CONSTANT_FOLDING | CompilerFlags::RELABEL
    }
}

prefixed_flags!(
    HhvmFlags,
    "hhvm.",
    ARRAY_PROVENANCE,
    EMIT_CLS_METH_POINTERS,
    EMIT_METH_CALLER_FUNC_POINTERS,
    ENABLE_INTRINSICS_EXTENSION,
    FOLD_LAZY_CLASS_KEYS,
    JIT_ENABLE_RENAME_FUNCTION,
    LOG_EXTERN_COMPILER_PERF,
);
impl Default for HhvmFlags {
    fn default() -> HhvmFlags {
        HhvmFlags::EMIT_CLS_METH_POINTERS
            | HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS
            | HhvmFlags::FOLD_LAZY_CLASS_KEYS
    }
}

mod serde_bstr_str {
    use serde::ser::Error;
    use serde::Deserialize;
    use serde::Serialize;

    use super::*;
    pub(crate) fn serialize<S: Serializer>(s: &Arg<BString>, ser: S) -> Result<S::Ok, S::Error> {
        let s = Arg::new(String::from_utf8(s.get().to_vec()).map_err(Error::custom)?);
        s.serialize(ser)
    }
    pub(crate) fn deserialize<'de, D: Deserializer<'de>>(de: D) -> Result<Arg<BString>, D::Error> {
        let s = <Arg<String>>::deserialize(de)?;
        let s = Arg::new(BString::from(s.global_value.into_bytes()));
        Ok(s)
    }
}

mod serde_bstr_vec {
    use serde::ser::Error;
    use serde::Deserialize;
    use serde::Serialize;

    use super::*;
    pub(crate) fn serialize<S: Serializer>(
        s: &Arg<Vec<BString>>,
        ser: S,
    ) -> Result<S::Ok, S::Error> {
        let s: Arg<Vec<String>> = Arg::new(
            s.get()
                .iter()
                .map(|v| {
                    let v = String::from_utf8(v.to_vec()).map_err(Error::custom)?;
                    Ok(v)
                })
                .collect::<Result<_, _>>()?,
        );
        s.serialize(ser)
    }
    pub(crate) fn deserialize<'de, D: Deserializer<'de>>(
        de: D,
    ) -> Result<Arg<Vec<BString>>, D::Error> {
        let s = <Arg<Vec<String>>>::deserialize(de)?;
        let s: Arg<Vec<BString>> = Arg::new(
            s.global_value
                .into_iter()
                .map(|v| BString::from(v.into_bytes()))
                .collect(),
        );
        Ok(s)
    }
}

mod serde_bstr_btree {
    use serde::ser::Error;
    use serde::Deserialize;
    use serde::Serialize;

    use super::*;
    pub(crate) fn serialize<S: Serializer>(
        s: &Arg<BTreeMap<BString, BString>>,
        ser: S,
    ) -> Result<S::Ok, S::Error> {
        let s: Arg<BTreeMap<String, String>> = Arg::new(
            s.get()
                .iter()
                .map(|(k, v)| {
                    let k = String::from_utf8(k.to_vec()).map_err(Error::custom)?;
                    let v = String::from_utf8(v.to_vec()).map_err(Error::custom)?;
                    Ok((k, v))
                })
                .collect::<Result<_, _>>()?,
        );
        s.serialize(ser)
    }
    pub(crate) fn deserialize<'de, D: Deserializer<'de>>(
        de: D,
    ) -> Result<Arg<BTreeMap<BString, BString>>, D::Error> {
        let s = <Arg<BTreeMap<String, String>>>::deserialize(de)?;
        let s: Arg<BTreeMap<BString, BString>> = Arg::new(
            s.global_value
                .into_iter()
                .map(|(k, v)| {
                    let k = BString::from(k.into_bytes());
                    let v = BString::from(v.into_bytes());
                    (k, v)
                })
                .collect(),
        );
        Ok(s)
    }
}

#[prefix_all("hhvm.")]
#[derive(Clone, Serialize, Deserialize, Debug, PartialEq)]
pub struct Hhvm {
    #[serde(default)]
    pub aliased_namespaces: Arg<BTreeMapOrEmptyVec<String, String>>,

    // TODO(leoo) change to HashMap if order doesn't matter
    #[serde(default, with = "serde_bstr_btree")]
    pub include_roots: Arg<BTreeMap<BString, BString>>,

    #[serde(default = "defaults::emit_class_pointers")]
    pub emit_class_pointers: Arg<String>,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub flags: HhvmFlags,

    #[serde(flatten, default)]
    pub hack_lang: HackLang,
}

impl Default for Hhvm {
    fn default() -> Self {
        Self {
            aliased_namespaces: Default::default(),
            include_roots: Default::default(),
            emit_class_pointers: defaults::emit_class_pointers(),
            flags: Default::default(),
            hack_lang: Default::default(),
        }
    }
}

impl Hhvm {
    pub fn aliased_namespaces_cloned(&self) -> impl Iterator<Item = (String, String)> + '_ {
        let x = match self.aliased_namespaces.get() {
            BTreeMapOrEmptyVec::Nonempty(m) => Some(m.iter().map(|(x, y)| (x.clone(), y.clone()))),
            _ => None,
        };
        x.into_iter().flatten()
    }
}

#[prefix_all("hhvm.hack.lang.")]
#[derive(Clone, Serialize, Deserialize, Debug, Default, PartialEq)]
pub struct HackLang {
    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub flags: LangFlags,

    #[serde(default)]
    pub check_int_overflow: Arg<String>,
}

prefixed_flags!(
    LangFlags,
    "hhvm.hack.lang.",
    ABSTRACT_STATIC_PROPS,
    ALLOW_NEW_ATTRIBUTE_SYNTAX,
    ALLOW_UNSTABLE_FEATURES,
    CONST_DEFAULT_FUNC_ARGS,
    CONST_DEFAULT_LAMBDA_ARGS,
    CONST_STATIC_PROPS,
    DISABLE_LEGACY_ATTRIBUTE_SYNTAX,
    DISABLE_LEGACY_SOFT_TYPEHINTS,
    DISABLE_LVAL_AS_AN_EXPRESSION,
    DISABLE_XHP_ELEMENT_MANGLING,
    DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS,
    DISALLOW_INST_METH,
    DISALLOW_FUNC_PTRS_IN_CONSTANTS,
    ENABLE_CLASS_LEVEL_WHERE_CLAUSES,
    ENABLE_ENUM_CLASSES,
    ENABLE_XHP_CLASS_MODIFIER,
    RUST_EMITTER,
);
impl Default for LangFlags {
    fn default() -> LangFlags {
        LangFlags::DISABLE_LEGACY_SOFT_TYPEHINTS | LangFlags::ENABLE_ENUM_CLASSES
    }
}

prefixed_flags!(
    Php7Flags,
    "hhvm.php7.",
    LTR_ASSIGN, //
    UVS,        //
);
impl Default for Php7Flags {
    fn default() -> Php7Flags {
        Php7Flags::empty()
    }
}

prefixed_flags!(
    RepoFlags,
    "hhvm.repo.",
    AUTHORITATIVE, //
);
impl Default for RepoFlags {
    fn default() -> RepoFlags {
        RepoFlags::empty()
    }
}

#[prefix_all("hhvm.server.")]
#[derive(Clone, Serialize, Deserialize, Default, PartialEq, Debug)]
pub struct Server {
    #[serde(default, with = "serde_bstr_vec")]
    pub include_search_paths: Arg<Vec<BString>>,
}

#[derive(Clone, Serialize, Deserialize, PartialEq, Debug)]
pub struct Options {
    #[serde(default, with = "serde_bstr_str")]
    pub doc_root: Arg<BString>,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub hack_compiler_flags: CompilerFlags,

    #[serde(flatten, default)]
    pub hhvm: Hhvm,

    #[serde(default = "defaults::max_array_elem_size_on_the_stack")]
    pub max_array_elem_size_on_the_stack: Arg<isize>,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub php7_flags: Php7Flags,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub repo_flags: RepoFlags,

    #[serde(flatten, default)]
    pub server: Server,
}

impl Options {
    pub fn log_extern_compiler_perf(&self) -> bool {
        self.hhvm
            .flags
            .contains(HhvmFlags::LOG_EXTERN_COMPILER_PERF)
    }
}

impl Default for Options {
    fn default() -> Options {
        Options {
            max_array_elem_size_on_the_stack: defaults::max_array_elem_size_on_the_stack(),
            hack_compiler_flags: CompilerFlags::default(),
            hhvm: Hhvm::default(),
            php7_flags: Php7Flags::default(),
            repo_flags: RepoFlags::default(),
            server: Server::default(),
            // the rest is zeroed out (cannot do ..Default::default() as it'd be recursive)
            doc_root: Arg::new("".into()),
        }
    }
}

impl bytecode_printer::IncludeProcessor for Options {
    fn convert_include<'a>(
        &'a self,
        include_path: IncludePath<'a>,
        cur_path: Option<&'a RelativePath>,
    ) -> Option<PathBuf> {
        let alloc = bumpalo::Bump::new();
        let include_roots = self.hhvm.include_roots.get();
        let search_paths = self.server.include_search_paths.get();
        let doc_root = self.doc_root.get().as_bstr();
        match include_path.into_doc_root_relative(&alloc, include_roots) {
            IncludePath::Absolute(p) => {
                let path = Path::new(OsStr::from_bytes(&p));
                if path.exists() {
                    Some(path.to_owned())
                } else {
                    None
                }
            }
            IncludePath::SearchPathRelative(p) => {
                let path_from_cur_dirname = cur_path
                    .and_then(|p| p.path().parent())
                    .unwrap_or_else(|| Path::new(""))
                    .join(OsStr::from_bytes(&p));
                if path_from_cur_dirname.exists() {
                    Some(path_from_cur_dirname)
                } else {
                    for prefix in search_paths.iter() {
                        let path = Path::new(OsStr::from_bytes(prefix)).join(OsStr::from_bytes(&p));
                        if path.exists() {
                            return Some(path);
                        }
                    }
                    None
                }
            }
            IncludePath::IncludeRootRelative(v, p) => {
                if !p.is_empty() {
                    if let Some(ir) = include_roots.get(v.as_bstr()) {
                        let resolved = Path::new(OsStr::from_bytes(doc_root))
                            .join(OsStr::from_bytes(ir))
                            .join(OsStr::from_bytes(&p));
                        if resolved.exists() {
                            return Some(resolved);
                        }
                    }
                }
                None
            }
            IncludePath::DocRootRelative(p) => {
                let resolved = Path::new(OsStr::from_bytes(doc_root)).join(OsStr::from_bytes(&p));
                if resolved.exists() {
                    Some(resolved)
                } else {
                    None
                }
            }
        }
    }
}

/// Non-zero argument defaults for use in both Default::default & SerDe framework
mod defaults {
    use super::*;

    pub fn max_array_elem_size_on_the_stack() -> Arg<isize> {
        Arg::new(64)
    }

    pub fn emit_class_pointers() -> Arg<String> {
        Arg::new("0".into())
    }
}

thread_local! {
    static CACHE: RefCell<Cache> = {
        let hasher = fnv::FnvBuildHasher::default();
        RefCell::new(LruCache::with_hasher(100, hasher))
    }
}

pub(crate) type Cache = LruCache<Vec<String>, Options, fnv::FnvBuildHasher>;

impl Options {
    /// Merges src JSON into dst JSON, recursively adding or overwriting existing entries.
    /// This method cleverly avoids the need to represent each option as Option<Type>,
    /// since only the ones that are specified by JSON will be actually overridden.
    fn merge(dst: &mut Json, src: &Json) {
        match (dst, src) {
            (&mut Json::Object(ref mut dst), &Json::Object(ref src)) => {
                for (k, v) in src {
                    Self::merge(dst.entry(k.clone()).or_insert(Json::Null), v);
                }
            }
            (dst, src) => {
                *dst = src.clone();
            }
        }
    }

    pub fn from_configs(jsons: &[&str]) -> Result<Self, String> {
        CACHE.with(|cache| {
            let key: Vec<String> = jsons.iter().map(|&x| x.to_string()).collect();
            let mut cache = cache.borrow_mut();
            if let Some(o) = cache.get_mut(&key) {
                Ok(o.clone())
            } else {
                let o = Options::from_configs_(&key)?;
                cache.put(key, o.clone());
                Ok(o)
            }
        })
    }

    fn from_configs_<S: AsRef<str>>(jsons: &[S]) -> Result<Self, String> {
        let mut merged = json!({});
        for json in jsons {
            let json: &str = json.as_ref();
            if json.is_empty() {
                continue;
            }
            Self::merge(
                &mut merged,
                &serde_json::from_str(json).map_err(|e| e.to_string())?,
            );
        }
        let opts: serde_json::Result<Self> = serde_json::value::from_value(merged);
        opts.map_err(|e| e.to_string())
    }

    pub fn array_provenance(&self) -> bool {
        self.hhvm.flags.contains(HhvmFlags::ARRAY_PROVENANCE)
    }

    pub fn check_int_overflow(&self) -> bool {
        self.hhvm
            .hack_lang
            .check_int_overflow
            .get()
            .parse::<i32>()
            .map_or(false, |x| x.is_positive())
    }

    pub fn emit_class_pointers(&self) -> i32 {
        self.hhvm.emit_class_pointers.get().parse::<i32>().unwrap()
    }
}

use serde::de;
use serde::de::Deserializer;
use serde::de::MapAccess;
use serde::de::Visitor;
use serde::ser::SerializeMap;
use serde::Serializer;

fn serialize_flags<S: Serializer, P: PrefixedFlags>(flags: &P, s: S) -> Result<S::Ok, S::Error> {
    // TODO(leoo) iterate over each set bit: flags.bits() & ~(flags.bits() + 1)
    let mut map = s.serialize_map(None)?;
    for (key, value) in P::to_map().into_iter() {
        let bool_val = flags.contains(value);
        map.serialize_entry(&format!("{}{}", &P::PREFIX, key), &Arg::new(bool_val))?;
    }
    map.end()
}

/// Expected JSON layouts for each field that is a valid Hack option
#[derive(Deserialize)]
#[serde(untagged)]
enum GlobalValue {
    String(String),
    Bool(bool),
    Int(isize),
    VecStr(Vec<String>),
    MapStr(BTreeMap<String, String>),
    Json(Json), // support HHVM options with arbitrary layout
                // (A poorer alternative that risks silent HHVM breakage
                // would be to explicitly enumerate all possible layouts.)
}

fn deserialize_flags<'de, D: Deserializer<'de>, P: PrefixedFlags>(
    deserializer: D,
) -> Result<P, D::Error> {
    use std::fmt;
    use std::marker::PhantomData;
    struct Phantom<P>(PhantomData<P>);

    impl<'de, P: PrefixedFlags> Visitor<'de> for Phantom<P> {
        type Value = P;

        fn expecting(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
            formatter.write_str("flag with string global_value")
        }

        fn visit_map<M: MapAccess<'de>>(self, mut map: M) -> Result<Self::Value, M::Error> {
            // TODO(leoo) proc macro to traverse Flags struct & iter over assoc. constants
            let mut flags = P::default();
            let by_name = P::to_map();
            let prefix_len = P::PREFIX.len();
            let from_str = |v: &str| match v {
                "true" => Ok(true),
                "false" => Ok(false),
                num => num
                    .parse::<i32>()
                    .map(|v| v == 1)
                    .map_err(de::Error::custom),
            };
            while let Some((ref k, ref v)) = map.next_entry::<String, Arg<GlobalValue>>()? {
                let mut found = None;
                let k: &str = k;
                let k: &str = options_cli::CANON_BY_ALIAS.get(k).unwrap_or(&k);
                if k.starts_with(P::PREFIX) {
                    found = by_name.get(&k[prefix_len..]).cloned();
                }
                if let Some(flag) = found {
                    let truish = match v.get() {
                        GlobalValue::String(s) => {
                            if s.is_empty() {
                                false
                            } else {
                                from_str(s)?
                            }
                        }
                        GlobalValue::Bool(b) => *b,
                        GlobalValue::Int(n) => *n == 1,
                        _ => continue, // types such as VecStr aren't parsable as flags
                    };
                    if truish {
                        flags |= flag
                    } else {
                        flags &= !flag
                    }
                }
            }
            Ok(flags)
        }
    }

    deserializer.deserialize_map(Phantom(PhantomData::<P>))
}

/// Wrapper that serves as a workaround for a bug in the HHVM's serialization
/// empty JSON objects ({}) are silently converted to [] (darray vs varray):
/// - (de)serialize [] as Empty (instead of {});
/// - (de)serialize missing values as [].
#[derive(Serialize, Deserialize, PartialEq, Eq, Debug, Clone)]
#[serde(untagged)]
pub enum BTreeMapOrEmptyVec<K: Ord, V> {
    Nonempty(BTreeMap<K, V>),
    Empty(Vec<V>),
}

impl<K: Ord, V> BTreeMapOrEmptyVec<K, V> {
    pub fn as_map(&self) -> Option<&BTreeMap<K, V>> {
        match self {
            BTreeMapOrEmptyVec::Nonempty(m) => Some(m),
            _ => None,
        }
    }
}

impl<K: Ord, V> From<BTreeMapOrEmptyVec<K, V>> for BTreeMap<K, V> {
    fn from(m: BTreeMapOrEmptyVec<K, V>) -> Self {
        match m {
            BTreeMapOrEmptyVec::Nonempty(m) => m,
            _ => BTreeMap::new(),
        }
    }
}

impl<K: Ord, V> From<BTreeMap<K, V>> for BTreeMapOrEmptyVec<K, V> {
    fn from(m: BTreeMap<K, V>) -> Self {
        if m.is_empty() {
            BTreeMapOrEmptyVec::Empty(vec![])
        } else {
            BTreeMapOrEmptyVec::Nonempty(m)
        }
    }
}

impl<K: Ord, V> Default for BTreeMapOrEmptyVec<K, V> {
    fn default() -> Self {
        BTreeMapOrEmptyVec::Empty(vec![])
    }
}

#[cfg(test)]
mod tests {
    use pretty_assertions::assert_eq;

    use super::*; // make assert_eq print huge diffs more human-readable

    const HHVM_1: &str = r#"{
  "hhvm.aliased_namespaces": {
    "global_value": {
      "bar": "baz",
      "foo": "bar"
    }
  },
  "hhvm.array_provenance": {
    "global_value": false
  },
  "hhvm.emit_class_pointers": {
    "global_value": "0"
  },
  "hhvm.emit_cls_meth_pointers": {
    "global_value": false
  },
  "hhvm.emit_meth_caller_func_pointers": {
    "global_value": true
  },
  "hhvm.enable_intrinsics_extension": {
    "global_value": false
  },
  "hhvm.fold_lazy_class_keys": {
    "global_value": true
  },
  "hhvm.hack.lang.abstract_static_props": {
    "global_value": false
  },
  "hhvm.hack.lang.allow_new_attribute_syntax": {
    "global_value": false
  },
  "hhvm.hack.lang.allow_unstable_features": {
    "global_value": false
  },
  "hhvm.hack.lang.check_int_overflow": {
    "global_value": ""
  },
  "hhvm.hack.lang.const_default_func_args": {
    "global_value": false
  },
  "hhvm.hack.lang.const_default_lambda_args": {
    "global_value": false
  },
  "hhvm.hack.lang.const_static_props": {
    "global_value": false
  },
  "hhvm.hack.lang.disable_legacy_attribute_syntax": {
    "global_value": false
  },
  "hhvm.hack.lang.disable_legacy_soft_typehints": {
    "global_value": true
  },
  "hhvm.hack.lang.disable_lval_as_an_expression": {
    "global_value": false
  },
  "hhvm.hack.lang.disable_xhp_element_mangling": {
    "global_value": false
  },
  "hhvm.hack.lang.disallow_fun_and_cls_meth_pseudo_funcs": {
    "global_value": false
  },
  "hhvm.hack.lang.disallow_func_ptrs_in_constants": {
    "global_value": false
  },
  "hhvm.hack.lang.disallow_inst_meth": {
    "global_value": false
  },
  "hhvm.hack.lang.enable_class_level_where_clauses": {
    "global_value": false
  },
  "hhvm.hack.lang.enable_enum_classes": {
    "global_value": true
  },
  "hhvm.hack.lang.enable_xhp_class_modifier": {
    "global_value": false
  },
  "hhvm.hack.lang.rust_emitter": {
    "global_value": false
  },
  "hhvm.include_roots": {
    "global_value": {}
  },
  "hhvm.jit_enable_rename_function": {
    "global_value": false
  },
  "hhvm.log_extern_compiler_perf": {
    "global_value": false
  }
}"#;

    #[test]
    fn test_hhvm_json_ser() {
        let hhvm = json!(Hhvm {
            aliased_namespaces: Arg::new({
                let mut m = BTreeMap::new();
                m.insert("foo".to_owned(), "bar".to_owned());
                m.insert("bar".to_owned(), "baz".to_owned());
                m.into()
            }),
            flags: HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS | HhvmFlags::FOLD_LAZY_CLASS_KEYS,
            ..Default::default()
        });
        assert_eq!(HHVM_1, serde_json::to_string_pretty(&hhvm).unwrap(),);
    }

    #[test]
    fn test_hhvm_json_de() {
        let j = serde_json::from_str(
            r#"{
            "hhvm.aliased_namespaces": { "global_value": {"foo": "bar"} },
            "hhvm.emit_meth_caller_func_pointers": { "global_value": "true" },
            "hhvm.jit_enable_rename_function": { "global_value": 1 },
            "hhvm.log_extern_compiler_perf": { "global_value": false },
            "hhvm.array_provenance": { "global_value": "1" }
            }"#,
        )
        .unwrap();
        let hhvm: Hhvm = serde_json::from_value(j).unwrap();
        assert!(hhvm.flags.contains(
            HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS
                | HhvmFlags::JIT_ENABLE_RENAME_FUNCTION
                | HhvmFlags::ARRAY_PROVENANCE
        ));
        assert!(!hhvm.flags.contains(HhvmFlags::LOG_EXTERN_COMPILER_PERF));
    }

    #[test]
    fn test_hhvm_json_de_defaults_overridable() {
        let hhvm: Hhvm = serde_json::value::from_value(json!({})).unwrap();
        assert_eq!(hhvm.flags, HhvmFlags::default());
        assert!(
            hhvm.flags
                .contains(HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS)
        );

        // now override a true-by-default option with a false value
        let hhvm: Hhvm = serde_json::value::from_value(json!({
            "hhvm.emit_meth_caller_func_pointers": { "global_value": "false" },
        }))
        .unwrap();
        assert!(
            !hhvm
                .flags
                .contains(HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS)
        );
    }

    #[test]
    fn test_hhvm_flags_alias_json_de() {
        // sanity check for defaults (otherwise this test doesn't do much!)
        assert!(!HhvmFlags::default().contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION));
        assert!(HhvmFlags::default().contains(HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS));

        let hhvm: Hhvm = serde_json::from_str(
            r#"{ "eval.jitenablerenamefunction": { "global_value": "true" },
                 "hhvm.emit_meth_caller_func_pointers": { "global_value": "false" } }"#,
        )
        .unwrap();
        assert!(
            hhvm // verify a false-by-default flag was parsed as true
                .flags
                .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION)
        );

        assert!(
            !hhvm // verify a true-by-default flag was parsed as false
                .flags
                .contains(HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS)
        );
    }

    #[test]
    fn test_empty_flag_treated_as_false_json_de() {
        // verify a true-by-default flag was parsed as false if ""
        let hhvm: Hhvm = serde_json::from_str(
            r#"{ "hhvm.emit_meth_caller_func_pointers": { "global_value": "" } }"#,
        )
        .unwrap();
        assert!(
            !hhvm
                .flags
                .contains(HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS)
        );
    }

    #[test]
    fn test_options_flat_arg_alias_json_de() {
        let act: Options = serde_json::value::from_value(json!({
            "eval.jitenablerenamefunction": {
                "global_value": "true",
            },
        }))
        .expect("failed to deserialize");
        assert!(
            act.hhvm
                .flags
                .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION)
        );
    }

    #[test]
    fn test_options_nonzero_defaults_json_de() {
        let act: Options = serde_json::value::from_value(json!({})).unwrap();
        assert_eq!(act, Options::default());
    }

    #[test]
    fn test_options_map_str_str_json_de() {
        let act: Options = serde_json::value::from_value(json!({
            "hhvm.aliased_namespaces": { "global_value": {"ns1": "ns2"} }
        }))
        .unwrap();
        assert_eq!(act.hhvm.aliased_namespaces.get().as_map().unwrap(), &{
            let mut m = BTreeMap::new();
            m.insert("ns1".to_owned(), "ns2".to_owned());
            m
        },);
    }

    #[test]
    fn test_options_map_str_str_as_empty_array_json_de() {
        let act: Options = serde_json::value::from_value(json!({
            "hhvm.aliased_namespaces": { "global_value": [] }
        }))
        .unwrap();
        assert_eq!(act.hhvm.aliased_namespaces.get().as_map(), None);
    }

    #[test]
    fn test_options_merge() {
        let mut dst = json!({
            "uniqueAtDst": "DST",
            "person" : { "firstName": "John", "lastName": "Doe" },
            "flat": [ "will", "be", "overridden" ],
        });
        let src = json!({
            "uniqueAtSrc": "SRC",
            "person" : { "firstName" : "Jane (not John)" },
            "flat": "overrides dst's field",
        });
        Options::merge(&mut dst, &src);
        assert_eq!(
            dst,
            json!({
                "flat": "overrides dst's field",
                "person": {
                    "firstName": "Jane (not John)",
                    "lastName": "Doe"
                },
                "uniqueAtDst": "DST",
                "uniqueAtSrc": "SRC",
            })
        );
    }

    #[test]
    fn test_options_de_multiple_jsons() {
        let jsons: [String; 2] = [
            json!({
                // override an options from 1 to 0 in first JSON,
                "hhvm.hack.lang.enable_enum_classes": { "global_value": false },
            })
            .to_string(),
            json!({
                // override another option from 0 to 1 in second JSON for the first time
                "hhvm.hack.lang.disable_xhp_element_mangling": { "global_value": true },
            })
            .to_string(),
        ];
        let act = Options::from_configs_(&jsons).unwrap();
        assert!(
            act.hhvm
                .hack_lang
                .flags
                .contains(LangFlags::DISABLE_XHP_ELEMENT_MANGLING)
        );
        assert!(
            !act.hhvm
                .hack_lang
                .flags
                .contains(LangFlags::ENABLE_ENUM_CLASSES)
        );
    }

    #[test]
    fn test_options_de_regression_boolish_parse_on_unrelated_opt() {
        // Note: this fails if bool-looking options are too eagerly parsed
        // (i.e., before they're are looked by JSON key/name and match a flag)
        let _: Options = serde_json::value::from_value(json!({
            "hhvm.only.opt1": { "global_value": "12345678901234567890" },
             "hhvm.only.opt2": { "global_value": "" },
        }))
        .expect("boolish-parsing logic wrongly triggered");
    }

    #[test]
    fn test_options_de_untyped_global_value_no_crash() {
        let res: Result<Options, String> = Options::from_configs(
            // try parsing an HHVM option whose layout is none of the
            // typed variants of GlobalValue, i.e., a string-to-int map
            &[r#"{
            "hhvm.semr_thread_overrides": {
              "global_value": {
                "17":160,
                "14":300,
                "0":320
              },
              "local_value": { "4":300 },
                "access":4
              }
                 }"#],
        );
        assert_eq!(res.err(), None);
    }

    #[test]
    fn test_options_de_empty_configs_skipped_no_crash() {
        let res: Result<Options, String> = Options::from_configs_(
            // a subset (only 30/5K lines) of the real config passed by HHVM
            &[
                "", // this should be skipped (it's an invalid JSON)
                r#"
          {
            "hhvm.trusted_db_path": {
              "access": 4,
              "local_value": "",
              "global_value": ""
            },
            "hhvm.query": {
              "access": 4,
              "local_value": "",
              "global_value": ""
            },
            "hhvm.php7.ltr_assign": {
              "access": 4,
              "local_value": "0",
              "global_value": "0"
            },
            "hhvm.aliased_namespaces": {
              "access": 4,
              "local_value": {
                "C": "HH\\Lib\\C"
              },
              "global_value": {
                "Vec": "HH\\Lib\\Vec"
              }
            }
          }
          "#,
                r#"
          {
            "hhvm.include_roots": {
              "global_value": {}
            }
          }"#,
            ],
        );
        assert_eq!(res.err(), None);
    }
}

// boilerplate code that could eventually be avoided via procedural macros

bitflags! {
    struct Flags: u64 {
        const CONSTANT_FOLDING = 1 << 0;
        const OPTIMIZE_NULL_CHECKS = 1 << 1;
        // No longer using bit 2.
        const UVS = 1 << 3;
        const LTR_ASSIGN = 1 << 4;
        /// If true, then renumber labels after generating code for a method
        /// body. Semantic diff doesn't care about labels, but for visual diff against
        /// HHVM it's helpful to renumber in order that the labels match more closely
        const RELABEL = 1 << 5;
        // No longer using bit 6.
        // No longer using bit 7.
        // No longer using bit 8.
        const AUTHORITATIVE = 1 << 9;
        const JIT_ENABLE_RENAME_FUNCTION = 1 << 10;
        // No longer using bits 11-13.
        const LOG_EXTERN_COMPILER_PERF = 1 << 14;
        const ENABLE_INTRINSICS_EXTENSION = 1 << 15;
        // No longer using bits 16-22.
        // No longer using bits 23-25.
        const EMIT_CLS_METH_POINTERS = 1 << 26;
        // No longer using bit 27.
        const EMIT_METH_CALLER_FUNC_POINTERS = 1 << 28;
        // No longer using bit 29.
        const DISABLE_LVAL_AS_AN_EXPRESSION = 1 << 30;
        // No longer using bits 31-32.
        const ARRAY_PROVENANCE = 1 << 33;
        // No longer using bit 34.
        const ENABLE_CLASS_LEVEL_WHERE_CLAUSES = 1 << 35;
        const DISABLE_LEGACY_SOFT_TYPEHINTS = 1 << 36;
        const ALLOW_NEW_ATTRIBUTE_SYNTAX = 1 << 37;
        const DISABLE_LEGACY_ATTRIBUTE_SYNTAX = 1 << 38;
        const CONST_DEFAULT_FUNC_ARGS = 1 << 39;
        const CONST_STATIC_PROPS = 1 << 40;
        const ABSTRACT_STATIC_PROPS = 1 << 41;
        // No longer using bit 42
        const DISALLOW_FUNC_PTRS_IN_CONSTANTS = 1 << 43;
        // No longer using bit 44.
        const CONST_DEFAULT_LAMBDA_ARGS = 1 << 45;
        const ENABLE_XHP_CLASS_MODIFIER = 1 << 46;
        // No longer using bit 47.
        const ENABLE_ENUM_CLASSES = 1 << 48;
        const DISABLE_XHP_ELEMENT_MANGLING = 1 << 49;
        // No longer using bit 50.
        const RUST_EMITTER = 1 << 51;
        // No longer using bits 52-54.
        const ALLOW_UNSTABLE_FEATURES = 1 << 55;
        // No longer using bit 56.
        const DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS = 1 << 57;
        const FOLD_LAZY_CLASS_KEYS = 1 << 58;
        // No longer using bit 59.
        const DISALLOW_INST_METH = 1 << 60;
    }
}
