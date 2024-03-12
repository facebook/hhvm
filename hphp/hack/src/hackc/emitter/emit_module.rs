// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::emitter::Emitter;
use error::Result;
use ffi::Maybe;
use hhbc::Module;
use hhbc::ModuleName;
use hhbc::Rule;
use hhbc::RuleKind;
use hhbc::Span;
use oxidized::ast;

use crate::emit_attribute;

fn emit_rule(rule: &ast::MdNameKind) -> Rule {
    match rule {
        ast::MdNameKind::MDNameGlobal(_) => Rule {
            kind: RuleKind::Global,
            name: Maybe::Nothing,
        },
        ast::MdNameKind::MDNamePrefix(id) => Rule {
            kind: RuleKind::Prefix,
            name: Maybe::Just(hhbc::intern(&id.1)),
        },
        ast::MdNameKind::MDNameExact(id) => Rule {
            kind: RuleKind::Exact,
            name: Maybe::Just(hhbc::intern(&id.1)),
        },
    }
}

pub fn emit_module<'a, 'd>(
    emitter: &mut Emitter<'d>,
    ast_module: &'a ast::ModuleDef,
) -> Result<Module> {
    let attributes = emit_attribute::from_asts(emitter, &ast_module.user_attributes)?;
    let name = ModuleName::intern(&ast_module.name.1);
    let span = Span::from_pos(&ast_module.span);
    let doc_comment = ast_module.doc_comment.clone();

    Ok(Module {
        attributes: attributes.into(),
        name,
        span,
        doc_comment: doc_comment
            .map(|(_, comment)| comment.into_bytes().into())
            .into(),
        exports: Maybe::from(
            ast_module
                .exports
                .as_ref()
                .map(|v| Vec::from_iter(v.iter().map(emit_rule)).into()),
        ),
        imports: Maybe::from(
            ast_module
                .imports
                .as_ref()
                .map(|v| Vec::from_iter(v.iter().map(emit_rule)).into()),
        ),
    })
}

pub fn emit_modules_from_program<'a, 'd>(
    emitter: &mut Emitter<'d>,
    ast: &'a [ast::Def],
) -> Result<Vec<Module>> {
    ast.iter()
        .filter_map(|def| {
            if let ast::Def::Module(md) = def {
                Some(emit_module(emitter, md))
            } else {
                None
            }
        })
        .collect()
}

pub fn emit_module_use_from_program(prog: &[ast::Def]) -> Maybe<ModuleName> {
    for node in prog.iter() {
        if let ast::Def::SetModule(s) = node {
            return Maybe::Just(ModuleName::intern(&s.1));
        }
    }
    Maybe::Nothing
}
