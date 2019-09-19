// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{file_info::Mode, relative_path::RelativePath};
use parser_rust::{
    parser::Parser, parser_env::ParserEnv, smart_constructors_wrappers::WithKind,
    source_text::SourceText,
};
use syntax_tree::mode_parser::parse_mode;

use crate::facts::*;
use crate::facts_smart_constructors::*;

pub type FactsParser<'a> = Parser<'a, WithKind<FactsSmartConstructors<'a>>, HasScriptContent<'a>>;

pub struct ExtractAsJsonOpts {
    pub php5_compat_mode: bool,
    pub hhvm_compat_mode: bool,
    pub allow_new_attribute_syntax: bool,
    pub filename: RelativePath,
}

pub fn extract_as_json(text: &str, opts: ExtractAsJsonOpts) -> Option<String> {
    from_text(text, opts).map(|facts| facts.to_json(text))
}

pub fn from_text(text: &str, opts: ExtractAsJsonOpts) -> Option<Facts> {
    let text = SourceText::make(&opts.filename, text.as_bytes());
    let is_experimental = match parse_mode(&text) {
        Some(Mode::Mexperimental) => true,
        _ => false,
    };
    let env = ParserEnv {
        php5_compat_mode: opts.php5_compat_mode,
        hhvm_compat_mode: opts.hhvm_compat_mode,
        is_experimental_mode: is_experimental,
        allow_new_attribute_syntax: opts.allow_new_attribute_syntax,
        ..ParserEnv::default()
    };
    let mut parser = FactsParser::make(&text, env);
    let root = parser.parse_script(None);

    // report errors only if result of parsing is non-empty *)
    if parser.sc_state().0 && !parser.errors().is_empty() {
        None
    } else {
        Some(collect(("".to_owned(), Facts::default()), root).1)
    }
}

pub fn without_xhp_mangling<T>(f: impl FnOnce() -> T) -> T {
    MANGLE_XHP_MODE.with(|cur| {
        let old = cur.replace(false);
        let ret = f();
        cur.set(old); // use old instead of true to support nested calls in the same thread
        ret
    })
}

// implementation details

use std::cell::Cell;
use std::string::String;
use Node::*; // Ensure String doesn't refer to Node::String

thread_local!(static MANGLE_XHP_MODE: Cell<bool> = Cell::new(true));

// TODO(leoo) move to hhbc_utils::Xhp::mangle_id​ once HHBC is ported
fn mangle_xhp_id(mut name: String) -> String {
    fn ignore_id(name: &str) -> bool {
        name.starts_with("class@anonymous") || name.starts_with("Closure$")
    }

    fn is_xhp(name: &str) -> bool {
        name.chars().next().map_or(false, |c| c == ':')
    }

    if !ignore_id(&name) && MANGLE_XHP_MODE.with(|x| x.get()) {
        if is_xhp(&name) {
            name.replace_range(..1, "xhp_")
        }
        name.replace(":", "__").replace("-", "_")
    } else {
        name
    }
}

fn qualified_name(namespace: &str, name: Node) -> Option<String> {
    fn qualified_name_from_parts(namespace: &str, parts: Vec<Node>) -> Option<String> {
        let mut qualified_name = String::new();
        let mut leading_backslash = false;
        for (index, part) in parts.into_iter().enumerate() {
            match part {
                Name(name) => {
                    qualified_name.push_str(&String::from_utf8_lossy(name.get().as_slice()))
                }
                Backslash if index == 0 => leading_backslash = true,
                ListItem(listitem) => {
                    if let (Name(name), Backslash) = *listitem {
                        qualified_name.push_str(&String::from_utf8_lossy(name.get().as_slice()));
                        qualified_name.push_str("\\");
                    }
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
            let name = name.to_string();
            Some(if namespace.is_empty() {
                name
            } else {
                namespace.to_owned() + "\\" + &name
            })
        }
        XhpName(name) => {
            // xhp names are always unqualified
            let name = name.to_string();
            Some(mangle_xhp_id(name))
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
            Node::Define(define) => {
                if let Node::Name(name) = *define {
                    acc.push(define_name(&name.get()));
                }
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
    attributes: Node,
    body: Node,
    facts: &mut Facts,
    type_facts: &mut TypeFacts,
) {
    let aux = |mut constants: Vec<String>, node| {
        if let RequireExtendsClause(name) = node {
            if check_require {
                if let Some(name) = qualified_name(namespace, *name) {
                    type_facts.require_extends.insert(name);
                }
            }
        } else if let RequireImplementsClause(name) = node {
            if check_require {
                if let Some(name) = qualified_name(namespace, *name) {
                    type_facts.require_implements.insert(name);
                }
            }
        } else if let TraitUseClause(uses) = node {
            typenames_from_list(*uses, namespace, &mut type_facts.base_types);
        } else if let MethodDecl(body) = node {
            if namespace.is_empty() {
                // in methods we collect only defines
                constants = defines_from_method_body(constants, *body);
            }
        }
        constants
    };
    if let List(nodes) = body {
        let facts_constants = std::mem::replace(&mut facts.constants, vec![]);
        facts.constants = nodes.into_iter().fold(facts_constants, aux);
    }
    type_facts.attributes = attributes_into_facts(namespace, attributes);
}

fn attributes_into_facts(namespace: &str, attributes: Node) -> Attributes {
    match attributes {
        Node::List(nodes) => nodes
            .into_iter()
            .fold(Attributes::new(), |mut attributes, node| match node {
                Node::ListItem(item) => {
                    let attribute_values_aux = |attribute_node| match attribute_node {
                        Node::Name(name) => {
                            let mut attribute_values = Vec::new();
                            attribute_values.push(name.to_string());
                            attribute_values
                        }
                        Node::String(name) => {
                            let mut attribute_values = Vec::new();
                            attribute_values.push(name.to_unescaped_string());
                            attribute_values
                        }
                        Node::List(nodes) => {
                            nodes
                                .into_iter()
                                .fold(Vec::new(), |mut attribute_values, node| match node {
                                    Node::Name(name) => {
                                        attribute_values.push(name.to_string());
                                        attribute_values
                                    }
                                    Node::String(name) => {
                                        // TODO(T47593892) fold constant
                                        attribute_values.push(name.to_unescaped_string());
                                        attribute_values
                                    }
                                    Node::ScopeResolutionExpression(expr) => {
                                        if let (Node::Name(name), Node::Class) = *expr {
                                            attribute_values.push(if namespace.is_empty() {
                                                name.to_string()
                                            } else {
                                                namespace.to_owned() + "\\" + &name.to_string()
                                            });
                                        }
                                        attribute_values
                                    }
                                    _ => attribute_values,
                                })
                        }
                        _ => Vec::new(),
                    };
                    match &(item.0) {
                        Node::Name(name) => {
                            attributes.insert(name.to_string(), attribute_values_aux(item.1));
                            attributes
                        }
                        Node::String(name) => {
                            attributes
                                .insert(name.to_unescaped_string(), attribute_values_aux(item.1));
                            attributes
                        }
                        _ => attributes,
                    }
                }
                _ => attributes,
            }),
        _ => Attributes::new(),
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
            attributes: Attributes::new(),
            base_types: StringSet::new(),
            require_extends: StringSet::new(),
            require_implements: StringSet::new(),
        };
        type_info_from_class_body(
            namespace,
            check_require,
            decl.attributes,
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
            tf.attributes.append(&mut delta.attributes);
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
        ClassDecl(decl) => {
            class_decl_into_facts(*decl, &acc.0, &mut acc.1);
        }
        EnumDecl(decl) => {
            if let Some(name) = qualified_name(&acc.0, decl.name) {
                let attributes = attributes_into_facts(&acc.0, decl.attributes);
                let enum_facts = TypeFacts {
                    flags: Flag::Final as isize,
                    kind: TypeKind::Enum,
                    attributes,
                    base_types: StringSet::new(),
                    require_extends: StringSet::new(),
                    require_implements: StringSet::new(),
                };
                add_or_update_classish_decl(name, enum_facts, &mut acc.1.types);
            }
        }
        FunctionDecl(name) => {
            if let Some(name) = qualified_name(&acc.0, *name) {
                acc.1.functions.push(name);
            }
        }
        ConstDecl(name) => {
            if let Some(name) = qualified_name(&acc.0, *name) {
                acc.1.constants.push(name);
            }
        }
        TypeAliasDecl(decl) => {
            if let Some(name) = qualified_name(&acc.0, decl.name) {
                let attributes = attributes_into_facts(&acc.0, decl.attributes);
                let type_alias_facts = TypeFacts {
                    flags: Flag::default() as isize,
                    kind: TypeKind::TypeAlias,
                    attributes,
                    base_types: StringSet::new(),
                    require_extends: StringSet::new(),
                    require_implements: StringSet::new(),
                };
                add_or_update_classish_decl(name.clone(), type_alias_facts, &mut acc.1.types);
                acc.1.type_aliases.push(name);
            }
        }
        Define(define) => {
            if acc.0.is_empty() {
                if let Node::String(ref name) = *define {
                    acc.1.constants.push(define_name(&name.get()));
                }
            }
        }
        NamespaceDecl(name, body) => {
            if let Node::EmptyBody = *body {
                if let Some(name) = qualified_name("", *name) {
                    acc.0 = name;
                }
            } else {
                let name = if let Ignored = *name {
                    Some(acc.0.clone())
                } else {
                    qualified_name(&acc.0, *name)
                };
                if let Some(name) = name {
                    acc.1 = collect((name, acc.1), *body).1;
                }
            }
        }
        _ => (),
    };
    acc
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn xhp_mangling() {
        assert_eq!(mangle_xhp_id(":foo".into()), String::from("xhp_foo"));
        assert_eq!(mangle_xhp_id("with-dash".into()), String::from("with_dash"));
        assert_eq!(
            mangle_xhp_id("test:colon".into()),
            String::from("test__colon")
        );
        assert_eq!(
            mangle_xhp_id(":a:all-in-one:example".into()),
            String::from("xhp_a__all_in_one__example")
        );
    }

    #[test]
    fn xhp_mangling_control_allows_nesting() {
        let name = String::from(":no:mangling");
        without_xhp_mangling(|| {
            assert_eq!(mangle_xhp_id(name.clone()), name);
            without_xhp_mangling(|| {
                assert_eq!(mangle_xhp_id(name.clone()), name);
            });
            assert_eq!(mangle_xhp_id(name.clone()), name);
        });
        assert_ne!(mangle_xhp_id(name.clone()), name);
    }
}
