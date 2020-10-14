// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Write;

pub use anyhow::Result;

pub fn to_snake(s: &str) -> String {
    let mut r = String::new();
    let chars: Vec<char> = s.chars().collect();
    for i in 0..chars.len() {
        if chars[i].is_ascii_uppercase() {
            if i != 0
                && chars[i - 1].is_ascii_lowercase()
                && (i + 1 == chars.len() || chars[i + 1].is_ascii_lowercase())
            {
                r.push('_');
            }
            r.push(chars[i].to_ascii_lowercase());
        } else {
            r.push(chars[i])
        }
    }
    r
}

pub fn insert_header(s: &str, command: &str) -> Result<String> {
    let mut content = String::new();
    write!(
        &mut content,
        "
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @{} <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>
//
// To regenerate this file, run:
//   {}

{}
",
        "generated", command, s
    )?;
    Ok(content)
}

pub mod gen_helpers {
    use proc_macro2::TokenStream;
    use quote::{format_ident, quote};

    pub fn gen_module_uses(ms: impl Iterator<Item = impl AsRef<str>>) -> TokenStream {
        let mods = ms.map(|m| format_ident!("{}", m.as_ref()));
        quote! {
            use crate::{#(#mods::*,)*};
        }
    }
}

pub mod syn_helpers {
    use std::collections::HashSet;

    use anyhow::{anyhow, Result};
    use syn::*;

    pub fn get_ty_def_name(i: &Item) -> Result<String> {
        use Item::*;
        match i {
            Enum(ItemEnum { ident, .. })
            | Struct(ItemStruct { ident, .. })
            | Type(ItemType { ident, .. }) => Ok(ident.to_string()),
            _ => Err(anyhow!("Not supported {:?}", i)),
        }
    }

    pub fn get_dep_tys(defined_types: &HashSet<&str>, i: &Item) -> Result<Vec<String>> {
        use Item::*;
        match i {
            Enum(ItemEnum { variants, .. }) => Ok(variants
                .iter()
                .fold(HashSet::<String>::new(), |mut a, v| {
                    for ty in LeafTyCollector::on_fields(Some(defined_types), &v.fields) {
                        a.insert(ty);
                    }
                    a
                })
                .into_iter()
                .collect()),
            Type(ItemType { ty, .. }) => {
                Ok(LeafTyCollector::on_type(Some(defined_types), ty.as_ref()).collect())
            }
            Struct(ItemStruct { fields, .. }) => {
                Ok(LeafTyCollector::on_fields(Some(defined_types), fields).collect())
            }
            _ => Err(anyhow!("Not supported {:?}", i)),
        }
    }

    struct LeafTyCollector {
        discovered_types: HashSet<String>,
    }

    impl LeafTyCollector {
        pub fn new() -> Self {
            Self {
                discovered_types: HashSet::new(),
            }
        }

        pub fn on_type<'a>(
            filter: Option<&'a HashSet<&'a str>>,
            ty: &Type,
        ) -> impl Iterator<Item = String> + 'a {
            let mut collector = Self::new();
            visit::visit_type(&mut collector, ty);
            collector
                .discovered_types
                .into_iter()
                .filter(move |s| filter.map_or(true, |f| f.contains(s.as_str())))
        }

        pub fn on_fields<'a>(
            filter: Option<&'a HashSet<&'a str>>,
            fields: &Fields,
        ) -> impl Iterator<Item = String> + 'a {
            let mut collector = Self::new();
            visit::visit_fields(&mut collector, fields);
            collector
                .discovered_types
                .into_iter()
                .filter(move |s| filter.map_or(true, |f| f.contains(s.as_str())))
        }
    }

    impl<'ast> visit::Visit<'ast> for LeafTyCollector {
        fn visit_path_segment(&mut self, node: &'ast PathSegment) {
            let ty = node.ident.to_string();
            self.discovered_types.insert(ty);
            visit::visit_path_segment(self, node);
        }
    }
}
