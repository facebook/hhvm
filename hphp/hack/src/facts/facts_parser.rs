// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_rust as parser;

use parser::parser::Parser;
use parser::parser_env::ParserEnv;
use parser::smart_constructors_wrappers::WithKind;
use parser::source_text::SourceText;

use crate::facts::*;
use crate::facts_smart_constructors::*;

pub type FactsParser<'a> = Parser<'a, WithKind<FactsSmartConstructors>, HasScriptContent<'a>>;

pub struct ExtractAsJsonOpts {
    pub php5_compat_mode: bool,
    pub hhvm_compat_mode: bool,
    pub filename: String, // TODO(leoo,kasper) should eventually be Relative_path
}

pub fn extract_as_json(text: &str, opts: ExtractAsJsonOpts) -> Option<String> {
    from_text(text, opts).map(|facts| facts.to_json(text))
}

// implementation details

use std::string::String;
use Node::*; // Ensure String doesn't refer to Node::String

fn qualified_name(namespace: &str, name: Node) -> Option<String> {
    fn qualified_name_from_parts(namespace: &str, parts: Vec<Node>) -> Option<String> {
        let mut qualified_name = String::new();
        let mut leading_backslash = false;
        for (index, part) in parts.into_iter().enumerate() {
            match part {
                Name(name) => qualified_name.push_str(&String::from_utf8_lossy(name.as_slice())),
                Backslash if index == 0 => leading_backslash = true,
                ListItem(box (Name(name), Backslash)) => {
                    qualified_name.push_str(&String::from_utf8_lossy(name.as_slice()));
                    qualified_name.push_str("\\");
                }
                _ => return None,
            }
        }
        Some(if leading_backslash || namespace.is_empty() {
            qualified_name // globally qualified name
        } else {
            namespace.to_owned() + "\\" + &qualified_name
        })
    }

    match name {
        Name(name) => {
            // always a simple name
            let name = String::from_utf8_lossy(&name).to_string();
            Some(if namespace.is_empty() {
                name
            } else {
                namespace.to_owned() + "\\" + &name
            })
        }
        XhpName(name) => {
            // xhp names are always unqualified
            let name = String::from_utf8_lossy(&name).to_string();
            Some(name) // TODO(leoo): need to port Hhbc_utils.Xhp.mangle_id
        }
        Node::QualifiedName(parts) => qualified_name_from_parts(namespace, parts),
        _ => None,
    }
}

fn modifiers_to_flags(modifiers: &Node) -> Flags {
    let mut flags = Flag::default();
    if let List(modifiers) = modifiers {
        for modifier in modifiers {
            flags = match modifier {
                Node::Abstract => Flag::Abstract.set(flags),
                Node::Final => Flag::Final.set(flags),
                Node::Static => Flag::Final.set(Flag::Abstract.set(flags)),
                _ => flags,
            };
        }
    }
    flags
}

fn typenames_from_list(list: Node, namespace: &str, names: &mut StringSet) {
    match list {
        Node::List(nodes) => nodes.into_iter().for_each(|name| {
            if let Some(name) = qualified_name(namespace, name) {
                names.insert(name);
            }
        }),
        _ => (),
    };
}

fn define_name(name: &[u8]) -> String {
    let name = &String::from_utf8_lossy(name);
    name[1..name.len() - 2].to_owned() // strip quotes
}

fn defines_from_method_body(constants: Vec<String>, body: Node) -> Vec<String> {
    fn aux(mut acc: Vec<String>, list: Node) -> Vec<String> {
        match list {
            Node::List(nodes) => nodes.into_iter().fold(acc, aux),
            Node::Define(box Node::Name(name)) => {
                acc.push(define_name(&name));
                acc
            }
            _ => acc,
        }
    }
    aux(constants, body)
}

fn type_info_from_class_body(
    namespace: &str,
    check_require: bool,
    body: Node,
    facts: &mut Facts,
    type_facts: &mut TypeFacts,
) {
    let aux = |mut constants: Vec<String>, node| {
        match node {
            RequireExtendsClause(box name) if check_require => {
                if let Some(name) = qualified_name(namespace, name) {
                    type_facts.require_extends.insert(name);
                }
            }
            RequireImplementsClause(box name) if check_require => {
                if let Some(name) = qualified_name(namespace, name) {
                    type_facts.require_implements.insert(name);
                }
            }
            TraitUseClause(box uses) => {
                typenames_from_list(uses, namespace, &mut type_facts.base_types);
            }
            MethodDecl(box body) if namespace.is_empty() => {
                // in methods we collect only defines
                constants = defines_from_method_body(constants, body);
            }
            _ => (),
        };
        constants
    };
    if let List(nodes) = body {
        let facts_constants = std::mem::replace(&mut facts.constants, vec![]);
        facts.constants = nodes.into_iter().fold(facts_constants, aux);
    }
}

fn class_decl_into_facts(decl: ClassDeclChildren, namespace: &str, mut facts: &mut Facts) {
    if let Some(name) = qualified_name(namespace, decl.name) {
        let (kind, flags) = match decl.kind {
            Node::Class => (TypeKind::Class, modifiers_to_flags(&decl.modifiers)),
            Node::Interface => (TypeKind::Interface, Flag::Abstract.as_flags()),
            Node::Trait => (TypeKind::Trait, Flag::Abstract.as_flags()),
            _ => (TypeKind::Unknown, Flag::default()),
        };
        let check_require = match kind {
            TypeKind::Interface | TypeKind::Trait => true,
            _ => false,
        };
        let mut decl_facts = TypeFacts {
            kind,
            flags,
            base_types: StringSet::new(),
            require_extends: StringSet::new(),
            require_implements: StringSet::new(),
        };
        type_info_from_class_body(
            namespace,
            check_require,
            decl.body,
            &mut facts,
            &mut decl_facts,
        );
        // trait uses are already added to base_types, so just add extends & implements
        typenames_from_list(decl.extends, namespace, &mut decl_facts.base_types);
        typenames_from_list(decl.implements, namespace, &mut decl_facts.base_types);
        add_or_update_classish_decl(name, decl_facts, &mut facts.types);
    }
}

fn add_or_update_classish_decl(name: String, mut delta: TypeFacts, types: &mut TypeFactsByName) {
    types
        .entry(name)
        .and_modify(|tf| {
            if tf.kind != delta.kind {
                tf.kind = TypeKind::Mixed;
            }
            tf.flags = Flag::MultipleDeclarations.set(tf.flags);
            tf.flags = Flag::combine(tf.flags, delta.flags);
            tf.base_types.append(&mut delta.base_types);
            tf.require_extends.append(&mut delta.require_extends);
            tf.require_implements.append(&mut delta.require_implements);
        })
        .or_insert_with(|| {
            if let TypeKind::Enum = delta.kind {
                delta.base_types.insert("HH\\BuiltinEnum".into());
            }
            delta
        });
}

type CollectAcc = (String, Facts);
fn collect(mut acc: CollectAcc, node: Node) -> CollectAcc {
    match node {
        List(nodes) => acc = nodes.into_iter().fold(acc, collect),
        ClassDecl(box decl) => {
            class_decl_into_facts(decl, &acc.0, &mut acc.1);
        }
        EnumDecl(box name) => {
            if let Some(name) = qualified_name(&acc.0, name) {
                let enum_facts = TypeFacts {
                    flags: Flag::Final as isize,
                    kind: TypeKind::Enum,
                    base_types: StringSet::new(),
                    require_extends: StringSet::new(),
                    require_implements: StringSet::new(),
                };
                add_or_update_classish_decl(name, enum_facts, &mut acc.1.types);
            }
        }
        FunctionDecl(box name) => {
            if let Some(name) = qualified_name(&acc.0, name) {
                acc.1.functions.push(name);
            }
        }
        ConstDecl(box name) => {
            if let Some(name) = qualified_name(&acc.0, name) {
                acc.1.constants.push(name);
            }
        }
        TypeAliasDecl(box name) => {
            if let Some(name) = qualified_name(&acc.0, name) {
                acc.1.type_aliases.push(name);
            }
        }
        Define(box Node::String(ref name)) if acc.0.is_empty() => {
            acc.1.constants.push(define_name(name));
        }
        NamespaceDecl(box name, box Node::EmptyBody) => {
            if let Some(name) = qualified_name("", name) {
                acc.0 = name;
            }
        }
        NamespaceDecl(box name, box body) => {
            let name = if let Ignored = name {
                Some(acc.0.clone())
            } else {
                qualified_name(&acc.0, name)
            };
            if let Some(name) = name {
                acc.1 = collect((name, acc.1), body).1;
            }
        }
        _ => (),
    };
    acc
}

fn from_text(text: &str, opts: ExtractAsJsonOpts) -> Option<Facts> {
    let env = ParserEnv {
        php5_compat_mode: opts.php5_compat_mode,
        hhvm_compat_mode: opts.hhvm_compat_mode,
        ..ParserEnv::default()
    };
    let text = SourceText::make(&opts.filename, text.as_bytes());
    let mut parser = FactsParser::make(&text, env);
    let root = parser.parse_script(None);

    // report errors only if result of parsing is non-empty *)
    if parser.sc_state().0 && !parser.errors().is_empty() {
        None
    } else {
        Some(collect(("".to_owned(), Facts::default()), root).1)
    }
}
