// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use bumpalo::Bump;

use ocamlrep::rc::RcOc;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::direct_decl_parser::Decls;
use parser_core_types::{parser_env::ParserEnv, source_text::SourceText};

pub fn parse_decls<'a>(filename: RelativePath, text: &'a [u8], arena: &'a Bump) -> Decls<'a> {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, _errors, state) =
        direct_decl_parser::parse_script(&text, ParserEnv::default(), arena, None);
    let decls = Rc::try_unwrap(state.decls).unwrap();
    Decls {
        classes: decls.classes.into(),
        funs: decls.funs.into(),
        typedefs: decls.typedefs.into(),
        consts: decls.consts.into(),
    }
}
