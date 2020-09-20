// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use arena_collections::AssocListMut;
use ocamlrep::rc::RcOc;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::direct_decl_parser::Decls;
use parser_core_types::{parser_env::ParserEnv, source_text::SourceText};

pub fn parse_decls<'a>(filename: RelativePath, text: &'a [u8], arena: &'a Bump) -> Decls<'a> {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, _errors, state) =
        direct_decl_parser::parse_script(&text, ParserEnv::default(), arena, None);
    let decls = state.decls;

    let mut classes = AssocListMut::new_in(arena);
    for (name, decl) in decls.classes {
        classes.insert(*name, decl.clone());
    }
    let mut funs = AssocListMut::new_in(arena);
    for (name, decl) in decls.funs {
        funs.insert(*name, decl.clone());
    }
    let mut typedefs = AssocListMut::new_in(arena);
    for (name, decl) in decls.typedefs {
        typedefs.insert(*name, decl.clone());
    }
    let mut consts = AssocListMut::new_in(arena);
    for &(name, decl) in decls.consts {
        consts.insert(name, decl);
    }

    Decls {
        classes: classes.into(),
        funs: funs.into(),
        typedefs: typedefs.into(),
        consts: consts.into(),
    }
}
