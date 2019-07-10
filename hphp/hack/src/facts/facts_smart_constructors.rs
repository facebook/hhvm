/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use parser_rust as parser;

use parser::flatten_smart_constructors::{FlattenOp, FlattenSmartConstructors};
use parser::lexable_token::LexableToken;
use parser::source_text::SourceText;
use parser::token_kind::TokenKind;

pub use crate::facts_smart_constructors_generated::*;

pub type HasScriptContent<'a> = (bool, &'a SourceText<'a>);

// TODO(leoo) consider avoiding always materializing substrings using something like (hard):
// type GetName<'a> = Box<Fn() -> &'a [u8]>;  // would require lifetime 'a param everywhere
type GetName = Vec<u8>;

#[derive(Debug)]
pub enum Node {
    List(Vec<Node>),
    Ignored,
    // tokens
    Name(GetName),
    String(GetName),
    XhpName(GetName),
    Backslash,
    ListItem(Box<(Node, Node)>),
    Class,
    Interface,
    Trait,
    Extends,
    Implements,
    Abstract,
    Final,
    Static,
    QualifiedName(Vec<Node>),
    ScopeResolutionExpression(Box<(Node, Node)>),
    // declarations
    ClassDecl(Box<ClassDeclChildren>),
    FunctionDecl(Box<Node>),
    MethodDecl(Box<Node>),
    EnumDecl(Box<Node>),
    TraitUseClause(Box<Node>),
    RequireExtendsClause(Box<Node>),
    RequireImplementsClause(Box<Node>),
    ConstDecl(Box<Node>),
    Define(Box<Node>),
    TypeAliasDecl(Box<Node>),
    NamespaceDecl(Box<Node>, Box<Node>),
    EmptyBody,
}

#[derive(Debug)]
pub struct ClassDeclChildren {
    pub modifiers: Node,
    pub kind: Node,
    pub name: Node,
    pub attributes: Node,
    pub extends: Node,
    pub implements: Node,
    pub constrs: Node,
    pub body: Node,
}

impl FlattenOp for FactsSmartConstructors {
    type S = Node;

    fn flatten(lst: Vec<Self::S>) -> Self::S {
        let mut r = lst
            .into_iter()
            .map(|s| match s {
                Node::List(children) => children,
                x => {
                    if Self::is_zero(&x) {
                        vec![]
                    } else {
                        vec![x]
                    }
                }
            })
            .flatten()
            .collect::<Vec<Self::S>>();
        match r.as_slice() {
            [] => Node::Ignored,
            [_] => r.pop().unwrap(),
            _ => Node::List(r),
        }
    }

    fn zero() -> Self::S {
        Node::Ignored
    }

    fn is_zero(s: &Self::S) -> bool {
        match s {
            Node::Ignored |
            // tokens
            Node::Name(_) |
            Node::String(_) |
            Node::XhpName(_) |
            Node::Backslash |
            Node::ListItem(_) |
            Node::Class |
            Node::Interface |
            Node::Trait |
            Node::Extends |
            Node::Implements |
            Node::Abstract |
            Node::Final |
            Node::Static |
            Node::QualifiedName(_) => true,
            _ => false,
        }
    }
}

impl<'a> FlattenSmartConstructors<'a, HasScriptContent<'a>> for FactsSmartConstructors {
    fn make_token(st: HasScriptContent<'a>, token: Self::Token) -> (HasScriptContent<'a>, Self::R) {
        let token_text = || {
            st.1.sub(
                token.leading_start_offset().unwrap_or(0) + token.leading_width(),
                token.width(),
            )
            .to_vec()
        };
        let kind = token.kind();
        let result = match kind {
            TokenKind::Name => Node::Name(token_text()),
            TokenKind::DecimalLiteral => Node::String(token_text()),
            TokenKind::SingleQuotedStringLiteral => Node::String(token_text()),
            TokenKind::DoubleQuotedStringLiteral => Node::String(token_text()),
            TokenKind::XHPClassName => Node::XhpName(token_text()),
            TokenKind::Backslash => Node::Backslash,
            TokenKind::Class => Node::Class,
            TokenKind::Trait => Node::Trait,
            TokenKind::Interface => Node::Interface,
            TokenKind::Extends => Node::Extends,
            TokenKind::Implements => Node::Implements,
            TokenKind::Abstract => Node::Abstract,
            TokenKind::Final => Node::Final,
            TokenKind::Static => Node::Static,
            _ => Node::Ignored,
        };
        // assume file has script content if it has any tokens besides markup or EOF
        let st = (
            st.0 || match kind {
                TokenKind::EndOfFile | TokenKind::Markup => false,
                _ => true,
            },
            st.1,
        );
        (st, result)
    }

    fn make_missing(st: HasScriptContent<'a>, _: usize) -> (HasScriptContent<'a>, Self::R) {
        (st, Node::Ignored)
    }

    fn make_list(
        st: HasScriptContent<'a>,
        items: Box<Vec<Self::R>>,
        _: usize,
    ) -> (HasScriptContent<'a>, Self::R) {
        let items = *items;
        let result = if !items.is_empty()
            && !items.iter().all(|r| match r {
                Node::Ignored => true,
                _ => false,
            }) {
            Node::List(items)
        } else {
            Node::Ignored
        };
        (st, result)
    }

    fn make_qualified_name(
        st: HasScriptContent<'a>,
        arg0: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        match arg0 {
            Node::Ignored => (st, Node::Ignored),
            Node::List(nodes) => (st, Node::QualifiedName(nodes)),
            node => (st, Node::QualifiedName(vec![node])),
        }
    }

    fn make_simple_type_specifier(
        st: HasScriptContent<'a>,
        arg0: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (st, arg0)
    }

    fn make_literal_expression(
        st: HasScriptContent<'a>,
        arg0: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (st, arg0)
    }

    fn make_list_item(
        st: HasScriptContent<'a>,
        item: Self::R,
        sep: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        match (item, sep) {
            (Node::Ignored, Node::Ignored) => (st, Node::Ignored),
            (x, Node::Ignored) | (Node::Ignored, x) => (st, x),
            (x, y) => (st, Node::ListItem(Box::new((x, y)))),
        }
    }

    fn make_generic_type_specifier(
        st: HasScriptContent<'a>,
        class_type: Self::R,
        _argument_list: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (st, class_type)
    }

    fn make_enum_declaration(
        st: HasScriptContent<'a>,
        _attributes: Self::R,
        _keyword: Self::R,
        name: Self::R,
        _colon: Self::R,
        _base: Self::R,
        _type: Self::R,
        _left_brace: Self::R,
        _enumerators: Self::R,
        _right_brace: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match name {
                Node::Ignored => Node::Ignored,
                _ => Node::EnumDecl(Box::new(name)),
            },
        )
    }

    fn make_alias_declaration(
        st: HasScriptContent<'a>,
        _attributes: Self::R,
        _keyword: Self::R,
        name: Self::R,
        _generic_params: Self::R,
        _constraint: Self::R,
        _equal: Self::R,
        _type: Self::R,
        _semicolon: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match name {
                Node::Ignored => Node::Ignored,
                _ => Node::TypeAliasDecl(Box::new(name)),
            },
        )
    }

    fn make_define_expression(
        st: HasScriptContent<'a>,
        _keyword: Self::R,
        _left_paren: Self::R,
        args: Self::R,
        _right_paren: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        match args {
            Node::List(mut nodes) => {
                if let Some(_snd) = nodes.pop() {
                    if let Some(fst @ Node::String(_)) = nodes.pop() {
                        if nodes.is_empty() {
                            return (st, Node::Define(Box::new(fst)));
                        }
                    }
                }
            }
            _ => (),
        };
        (st, Node::Ignored)
    }

    fn make_function_declaration(
        st: HasScriptContent<'a>,
        _attributes: Self::R,
        header: Self::R,
        body: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match (header, body) {
                (Node::Ignored, Node::Ignored) => Node::Ignored,
                (v, Node::Ignored) | (Node::Ignored, v) => v,
                (v1, v2) => Node::List(vec![v1, v2]),
            },
        )
    }

    fn make_function_declaration_header(
        st: HasScriptContent<'a>,
        _modifiers: Self::R,
        _keyword: Self::R,
        name: Self::R,
        _type_params: Self::R,
        _left_parens: Self::R,
        _param_list: Self::R,
        _right_parens: Self::R,
        _colon: Self::R,
        _type: Self::R,
        _where: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match name {
                Node::Ignored => Node::Ignored,
                _ => Node::FunctionDecl(Box::new(name)),
            },
        )
    }

    fn make_trait_use(
        st: HasScriptContent<'a>,
        _keyword: Self::R,
        names: Self::R,
        _semicolon: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match names {
                Node::Ignored => Node::Ignored,
                _ => Node::TraitUseClause(Box::new(names)),
            },
        )
    }

    fn make_require_clause(
        st: HasScriptContent<'a>,
        _keyword: Self::R,
        kind: Self::R,
        name: Self::R,
        _semicolon: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match name {
                Node::Ignored => Node::Ignored,
                _ => match kind {
                    Node::Extends => Node::RequireExtendsClause(Box::new(name)),
                    Node::Implements => Node::RequireImplementsClause(Box::new(name)),
                    _ => Node::Ignored,
                },
            },
        )
    }

    fn make_constant_declarator(
        st: HasScriptContent<'a>,
        name: Self::R,
        _initializer: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match name {
                Node::Ignored => Node::Ignored,
                _ => Node::ConstDecl(Box::new(name)),
            },
        )
    }

    fn make_namespace_declaration(
        st: HasScriptContent<'a>,
        _keyword: Self::R,
        name: Self::R,
        body: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match body {
                Node::Ignored => Node::Ignored,
                _ => Node::NamespaceDecl(Box::new(name), Box::new(body)),
            },
        )
    }

    fn make_namespace_body(
        st: HasScriptContent<'a>,
        _left_brace: Self::R,
        decls: Self::R,
        _right_brace: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (st, decls)
    }

    fn make_namespace_empty_body(
        st: HasScriptContent<'a>,
        _semicolon: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (st, Node::EmptyBody)
    }

    fn make_methodish_declaration(
        st: HasScriptContent<'a>,
        _attributes: Self::R,
        _function_decl_header: Self::R,
        body: Self::R,
        _semicolon: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match body {
                Node::Ignored => Node::Ignored,
                _ => Node::MethodDecl(Box::new(body)),
            },
        )
    }

    fn make_classish_declaration(
        st: HasScriptContent<'a>,
        attributes: Self::R,
        modifiers: Self::R,
        keyword: Self::R,
        name: Self::R,
        _type_params: Self::R,
        _extends_keyword: Self::R,
        extends: Self::R,
        _implements_keyword: Self::R,
        implements: Self::R,
        constrs: Self::R,
        body: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            match name {
                Node::Ignored => Node::Ignored,
                _ => Node::ClassDecl(Box::new(ClassDeclChildren {
                    modifiers,
                    kind: keyword,
                    name,
                    attributes,
                    extends,
                    implements,
                    constrs,
                    body,
                })),
            },
        )
    }

    fn make_classish_body(
        st: HasScriptContent<'a>,
        _left_brace: Self::R,
        elements: Self::R,
        _right_brace: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (st, elements)
    }

    fn make_attribute_specification(
        st: HasScriptContent<'a>,
        _left_double_angle: Self::R,
        attributes: Self::R,
        _right_double_angle: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (st, attributes)
    }

    fn make_constructor_call(
        st: HasScriptContent<'a>,
        class_type: Self::R,
        _left_paren: Self::R,
        argument_list: Self::R,
        _right_paren: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (st, Node::ListItem(Box::new((class_type, argument_list))))
    }

    fn make_decorated_expression(
        st: HasScriptContent<'a>,
        _decorator: Self::R,
        expression: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (st, expression)
    }

    fn make_scope_resolution_expression(
        st: HasScriptContent<'a>,
        qualifier: Self::R,
        _operator: Self::R,
        name: Self::R,
    ) -> (HasScriptContent<'a>, Self::R) {
        (
            st,
            Node::ScopeResolutionExpression(Box::new((qualifier, name))),
        )
    }
}
