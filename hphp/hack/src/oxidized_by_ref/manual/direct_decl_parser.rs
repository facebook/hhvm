// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_collections::List;
use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use oxidized::file_info::NameType;
use serde::Deserialize;
use serde::Serialize;
pub use shallow_decl_defs::Decl;

use crate::file_info;
use crate::shallow_decl_defs;
use crate::typing_defs;

/// WARNING: This type has not undergone the processing that you often expect in the typechecker,
/// e.g. deregistering PHPStdLib symbols, or putting into forward lexical order, or removing
/// duplicates. It's often a good idea to use ParsedFileWithHashes instead (or rather, one of
/// its wrappers).
/// NB: Must keep in sync with OCaml type Direct_decl_parser.parsed_file
#[derive(Debug, Copy, Clone, Eq, Hash, Ord, PartialEq, PartialOrd)]
#[derive(Deserialize, FromOcamlRepIn, NoPosHash, Serialize, ToOcamlRep)]
pub struct ParsedFile<'a> {
    /// Was the file Hack or HHI?
    pub mode: Option<file_info::Mode>,

    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a typing_defs::UserAttribute<'a>],

    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub decls: Decls<'a>,

    /// How did the parser handle xhp elements? This is a copy of
    /// the setting from DeclParserOptions, for use when converting
    /// Decls to Facts.
    pub disable_xhp_element_mangling: bool,

    /// True if the FFP detected parse errors while parsing. Other parse errors
    /// are detected in a second pass over the CST, and this field does not
    /// indicate whether errors would be detected in that second pass.
    pub has_first_pass_parse_errors: bool,
}

arena_deserializer::impl_deserialize_in_arena!(ParsedFile<'arena>);

impl<'a> IntoIterator for ParsedFile<'a> {
    type Item = (&'a str, shallow_decl_defs::Decl<'a>);
    type IntoIter = std::iter::Rev<std::vec::IntoIter<Self::Item>>;

    /// This iterates the decls in forward lexical order
    /// (Use into_iter().rev() if you want reverse order)
    fn into_iter(self) -> Self::IntoIter {
        // Note that our `self.decls` are stored in reverse order, so we have to reverse now.
        self.decls.into_iter().collect::<Vec<_>>().into_iter().rev()
    }
}

/// This is a store of decls. It allows iteration of decls in forward lexical order,
/// and you can .rev() to iterate in reverse lexical order.
/// By construction, you can safely trust that whoever constructed this object has explicitly
/// passed `deregister_php_stdlib`, which we hope and trust they got from .hhconfig.
#[derive(Clone, Debug)]
pub struct ParsedFileWithHashes<'a> {
    pub mode: Option<file_info::Mode>,

    /// `file_decls_hash` is a position *insensitive* decl hash, computed before php stdlib decls
    /// and duplicates are removed, and is computed over the decls in reverse lexical order
    /// (the hash is order-sensitive)
    pub file_decls_hash: hh24_types::FileDeclsHash,

    /// Decls along with a position *sensitive* hash. Internally they're stored in reverse
    /// lexical order. The choice of what to go into this list (remove dupes? remove php_stdlib
    /// in hhi files? transform class decls by removing php_stdlib members?) is determined
    /// by how it was constructed. The field is private: the only way to access it
    /// are through accessors .iter() and .into_iter(), which give the illusion of it being
    /// in forward lexical order.
    decls: Vec<(&'a str, Decl<'a>, hh24_types::DeclHash)>,
}

impl<'a> ParsedFileWithHashes<'a> {
    pub fn new(
        parsed_file: ParsedFile<'a>,
        deregister_php_stdlib_if_hhi: bool,
        prefix: relative_path::Prefix,
        arena: &'a bumpalo::Bump,
    ) -> Self {
        let file_decls_hash = hh24_types::FileDeclsHash::from_u64(
            hh_hash::position_insensitive_hash(&parsed_file.decls),
        );
        // Note: parsed_file.decls is in reverse lexical order, and our self.decls must also be in reverse
        // lexical order, so that as `OcamlReverseParsedFileWithHashes` we ffi correctly to ocaml.
        let decls = parsed_file
            .decls
            .into_iter()
            .filter_map(|(name, mut decl)| {
                if deregister_php_stdlib_if_hhi && prefix == relative_path::Prefix::Hhi {
                    match filter_php_stdlib_decls(arena, decl) {
                        Some(altered_decl) => decl = altered_decl,
                        None => return None,
                    }
                }
                // following is a position-sensitive hash
                let hash = hh24_types::DeclHash::from_u64(hh_hash::hash(&decl));
                Some((name, decl, hash))
            })
            .collect();
        Self {
            mode: parsed_file.mode,
            file_decls_hash,
            decls,
        }
    }

    /// This method is a code smell. It shows that the caller didn't take into account
    /// the .hhconfig flag `deregister_php_stdlib`.
    pub fn new_without_deregistering_do_not_use(parsed_file: ParsedFile<'a>) -> Self {
        let file_decls_hash = hh24_types::FileDeclsHash::from_u64(
            hh_hash::position_insensitive_hash(&parsed_file.decls),
        );
        let decls = parsed_file
            .decls
            .into_iter()
            .map(|(name, decl)| {
                let hash = hh24_types::DeclHash::from_u64(hh_hash::hash(&decl));
                (name, decl, hash)
            })
            .collect();
        Self {
            mode: parsed_file.mode,
            file_decls_hash,
            decls,
        }
    }

    /// This iterates the decls in forward lexical order
    /// (Use iter().rev() if you want reverse order)
    pub fn iter(
        &self,
    ) -> impl DoubleEndedIterator<Item = &(&'a str, shallow_decl_defs::Decl<'a>, hh24_types::DeclHash)>
    {
        // Note that our `self.decls` are stored in reverse order, so we have to reverse now.
        self.decls.iter().rev()
    }
}

impl<'a> IntoIterator for ParsedFileWithHashes<'a> {
    type Item = (&'a str, shallow_decl_defs::Decl<'a>, hh24_types::DeclHash);
    type IntoIter = std::iter::Rev<std::vec::IntoIter<Self::Item>>;

    /// This iterates the decls in forward lexical order
    /// (Use into_iter().rev() if you want reverse order)
    fn into_iter(self) -> Self::IntoIter {
        // Note that our `self.decls` are stored in reverse order, so we have to reverse now.
        self.decls.into_iter().rev()
    }
}

// NB: Must keep in sync with OCaml type Direct_decl_parser.decls
#[derive(Copy, Clone, Eq, Hash, Ord, PartialEq, PartialOrd)]
#[derive(Deserialize, FromOcamlRepIn, NoPosHash, Serialize, ToOcamlRep)]
pub struct Decls<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  List<'a, (&'a str, Decl<'a>)>,
);

arena_deserializer::impl_deserialize_in_arena!(Decls<'arena>);

impl<'a> TrivialDrop for Decls<'a> {}

/// WARNING! These decls do not respect the `deregister_php_stdlib` flag in .hhconfig,
/// and are in reverse declaration order, and have not been de-duped. You should
/// probably be consuming ParsedFileWithHashes instead.
impl<'a> Decls<'a> {
    pub const fn empty() -> Self {
        Self(List::empty())
    }

    pub fn get(&self, kind: NameType, symbol: &str) -> Option<&'a Decl<'a>> {
        self.iter().find_map(|(name, decl)| {
            if decl.kind() == kind && *name == symbol {
                Some(decl)
            } else {
                None
            }
        })
    }

    pub fn add<A: arena_trait::Arena>(&mut self, name: &'a str, decl: Decl<'a>, arena: &'a A) {
        self.0.push_front((name, decl), arena)
    }

    pub fn rev<A: arena_trait::Arena>(&mut self, arena: &'a A) {
        self.0 = self.0.rev(arena)
    }

    pub fn iter(&self) -> impl Iterator<Item = &'a (&'a str, Decl<'a>)> {
        self.0.iter()
    }

    pub fn classes(
        &self,
    ) -> impl Iterator<Item = (&'a str, &'a shallow_decl_defs::ShallowClass<'a>)> {
        self.iter().filter_map(|(name, decl)| match *decl {
            Decl::Class(decl) => Some((*name, decl)),
            _ => None,
        })
    }
    pub fn funs(&self) -> impl Iterator<Item = (&'a str, &'a typing_defs::FunElt<'a>)> {
        self.iter().filter_map(|(name, decl)| match *decl {
            Decl::Fun(decl) => Some((*name, decl)),
            _ => None,
        })
    }
    pub fn typedefs(&self) -> impl Iterator<Item = (&'a str, &'a typing_defs::TypedefType<'a>)> {
        self.iter().filter_map(|(name, decl)| match *decl {
            Decl::Typedef(decl) => Some((*name, decl)),
            _ => None,
        })
    }
    pub fn consts(&self) -> impl Iterator<Item = (&'a str, &'a typing_defs::ConstDecl<'a>)> {
        self.iter().filter_map(|(name, decl)| match *decl {
            Decl::Const(decl) => Some((*name, decl)),
            _ => None,
        })
    }
    pub fn types(&self) -> impl Iterator<Item = &'a (&'a str, Decl<'a>)> {
        self.iter().filter(|(_, decl)| match decl {
            Decl::Class(_) | Decl::Typedef(_) => true,
            Decl::Fun(_) | Decl::Const(_) | Decl::Module(_) => false,
        })
    }
    pub fn modules(&self) -> impl Iterator<Item = (&'a str, &'a typing_defs::ModuleDefType<'a>)> {
        self.iter().filter_map(|(name, decl)| match *decl {
            Decl::Module(decl) => Some((*name, decl)),
            _ => None,
        })
    }
}

// c.f. `hphp/hack/src/providers/direct_decl_utils.ml`
fn filter_php_stdlib_decls<'a>(arena: &'a bumpalo::Bump, decl: Decl<'a>) -> Option<Decl<'a>> {
    use crate::method_flags::MethodFlags;
    use crate::prop_flags::PropFlags;
    fn filter<'b, T: Copy>(
        items: &'b [T],
        arena: &'b bumpalo::Bump,
        pred: impl FnMut(&T) -> bool,
    ) -> &'b [T] {
        bumpalo::collections::Vec::from_iter_in(items.iter().copied().filter(pred), arena)
            .into_bump_slice()
    }
    match decl {
        Decl::Fun(f) if f.php_std_lib => None,
        Decl::Class(c) if (c.user_attributes.iter()).any(|ua| ua.name.1 == "__PHPStdLib") => None,
        Decl::Class(c) => {
            let masked = arena.alloc(shallow_decl_defs::ShallowClass {
                props: filter(c.props, arena, |p| {
                    !p.flags.contains(PropFlags::PHP_STD_LIB)
                }),
                sprops: filter(c.sprops, arena, |p| {
                    !p.flags.contains(PropFlags::PHP_STD_LIB)
                }),
                methods: filter(c.methods, arena, |m| {
                    !m.flags.contains(MethodFlags::PHP_STD_LIB)
                }),
                static_methods: filter(c.static_methods, arena, |m| {
                    !m.flags.contains(MethodFlags::PHP_STD_LIB)
                }),
                ..*c
            });
            Some(Decl::Class(masked))
        }
        _ => Some(decl),
    }
}

impl<'a> IntoIterator for Decls<'a> {
    type Item = (&'a str, Decl<'a>);
    type IntoIter = std::iter::Copied<arena_collections::list::Iter<'a, (&'a str, Decl<'a>)>>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.iter().copied()
    }
}

impl std::fmt::Debug for Decls<'_> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.debug_map().entries(self.iter().copied()).finish()
    }
}
