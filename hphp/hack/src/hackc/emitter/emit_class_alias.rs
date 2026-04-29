// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use error::Result;
use hhbc::ClassAlias;
use hhbc::ClassName;
use oxidized::ast;

pub fn emit_class_aliases_from_program(
    emitter: &mut Emitter,
    prog: &[ast::Def],
) -> Result<Vec<ClassAlias>> {
    prog.iter()
        .filter_map(|def| def.as_class_alias().map(|ca| emit_class_alias(emitter, ca)))
        .collect()
}

fn emit_class_alias(emitter: &mut Emitter, ca: &ast::ClassAlias) -> Result<ClassAlias> {
    let name = ClassName::from_ast_name_and_mangle(&ca.name.1);
    let orig = ClassName::from_ast_name_and_mangle(&ca.original.1);
    emitter.add_class_ref(orig);
    Ok(ClassAlias { name, orig })
}
