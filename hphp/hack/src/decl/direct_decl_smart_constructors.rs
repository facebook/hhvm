/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use parser_rust as parser;

use escaper::{extract_unquoted_string, unescape_double, unescape_single};
use flatten_smart_constructors::{FlattenOp, FlattenSmartConstructors};
use hhbc_rust::string_utils::GetName;
use oxidized::pos::Pos;
use parser::{
    indexed_source_text::IndexedSourceText, lexable_token::LexableToken, token_kind::TokenKind,
};

pub use crate::direct_decl_smart_constructors_generated::*;

#[derive(Clone, Debug)]
pub struct State<'a> {
    pub source_text: IndexedSourceText<'a>,
}

#[derive(Clone, Debug)]
pub enum HintValue {
    String,
    Int,
    Float,
    Num,
    ClassType(GetName),
}

#[derive(Clone, Debug)]
pub enum Node {
    List(Vec<Node>),
    Ignored,
    // tokens
    Name(GetName),
    String(GetName),
    XhpName(GetName),
    Hint(HintValue, Pos),
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
    EnumDecl(Box<EnumDeclChildren>),
    TraitUseClause(Box<Node>),
    RequireExtendsClause(Box<Node>),
    RequireImplementsClause(Box<Node>),
    ConstDecl(Box<ConstDeclChildren>),
    Define(Box<Node>),
    TypeAliasDecl(Box<TypeAliasDeclChildren>),
    NamespaceDecl(Box<Node>, Box<Node>),
    EmptyBody,
}

#[derive(Clone, Debug)]
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

#[derive(Clone, Debug)]
pub struct EnumDeclChildren {
    pub name: Node,
    pub attributes: Node,
}

#[derive(Clone, Debug)]
pub struct ConstDeclChildren {
    pub name: Node,
    pub initializer: Node,
    pub hint: Node,
}

#[derive(Clone, Debug)]
pub struct TypeAliasDeclChildren {
    pub name: Node,
    pub attributes: Node,
}

impl<'a> FlattenOp for DirectDeclSmartConstructors<'_> {
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
            Node::Hint(_, _) |
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

impl<'a> FlattenSmartConstructors<'a, State<'a>> for DirectDeclSmartConstructors<'a> {
    fn make_token(&mut self, token: Self::Token) -> Self::R {
        let token_text = || {
            self.state
                .source_text
                .source_text()
                .sub(
                    token.leading_start_offset().unwrap_or(0) + token.leading_width(),
                    token.width(),
                )
                .to_vec()
        };
        let token_pos = || {
            self.state
                .source_text
                .relative_pos(token.start_offset(), token.end_offset() + 1)
        };
        let kind = token.kind();
        match kind {
            TokenKind::Name => Node::Name(GetName::new(token_text(), |string| string)),
            TokenKind::DecimalLiteral => Node::String(GetName::new(token_text(), |string| string)),
            TokenKind::SingleQuotedStringLiteral => {
                Node::String(GetName::new(token_text(), |string| {
                    let tmp = unescape_single(string.as_str()).ok().unwrap();
                    extract_unquoted_string(&tmp, 0, tmp.len()).ok().unwrap()
                }))
            }
            TokenKind::DoubleQuotedStringLiteral => {
                Node::String(GetName::new(token_text(), |string| {
                    let tmp = unescape_double(string.as_str()).ok().unwrap();
                    extract_unquoted_string(&tmp, 0, tmp.len()).ok().unwrap()
                }))
            }
            TokenKind::XHPClassName => Node::XhpName(GetName::new(token_text(), |string| string)),
            TokenKind::String => Node::Hint(HintValue::String, token_pos()),
            TokenKind::Int => Node::Hint(HintValue::Int, token_pos()),
            TokenKind::Float | TokenKind::Double => Node::Hint(HintValue::Float, token_pos()),
            TokenKind::Num => Node::Hint(HintValue::Num, token_pos()),
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
        }
    }

    fn make_missing(&mut self, _: usize) -> Self::R {
        Node::Ignored
    }

    fn make_list(&mut self, items: Vec<Self::R>, _: usize) -> Self::R {
        let result = if !items.is_empty()
            && !items.iter().all(|r| match r {
                Node::Ignored => true,
                _ => false,
            }) {
            Node::List(items)
        } else {
            Node::Ignored
        };
        result
    }

    fn make_qualified_name(&mut self, arg0: Self::R) -> Self::R {
        match arg0 {
            Node::Ignored => Node::Ignored,
            Node::List(nodes) => Node::QualifiedName(nodes),
            node => Node::QualifiedName(vec![node]),
        }
    }

    fn make_simple_type_specifier(&mut self, arg0: Self::R) -> Self::R {
        arg0
    }

    fn make_simple_initializer(&mut self, _arg0: Self::R, arg1: Self::R) -> Self::R {
        arg1
    }

    fn make_literal_expression(&mut self, arg0: Self::R) -> Self::R {
        arg0
    }

    fn make_list_item(&mut self, item: Self::R, sep: Self::R) -> Self::R {
        match (item, sep) {
            (Node::Ignored, Node::Ignored) => Node::Ignored,
            (x, Node::Ignored) | (Node::Ignored, x) => x,
            (x, y) => Node::ListItem(Box::new((x, y))),
        }
    }

    fn make_generic_type_specifier(
        &mut self,
        class_type: Self::R,
        _argument_list: Self::R,
    ) -> Self::R {
        class_type
    }

    fn make_enum_declaration(
        &mut self,
        attributes: Self::R,
        _keyword: Self::R,
        name: Self::R,
        _colon: Self::R,
        _base: Self::R,
        _type: Self::R,
        _left_brace: Self::R,
        _enumerators: Self::R,
        _right_brace: Self::R,
    ) -> Self::R {
        match name {
            Node::Ignored => Node::Ignored,
            _ => Node::EnumDecl(Box::new(EnumDeclChildren { name, attributes })),
        }
    }

    fn make_alias_declaration(
        &mut self,
        attributes: Self::R,
        _keyword: Self::R,
        name: Self::R,
        _generic_params: Self::R,
        _constraint: Self::R,
        _equal: Self::R,
        _type: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        match name {
            Node::Ignored => Node::Ignored,
            _ => Node::TypeAliasDecl(Box::new(TypeAliasDeclChildren { name, attributes })),
        }
    }

    fn make_define_expression(
        &mut self,
        _keyword: Self::R,
        _left_paren: Self::R,
        args: Self::R,
        _right_paren: Self::R,
    ) -> Self::R {
        match args {
            Node::List(mut nodes) => {
                if let Some(_snd) = nodes.pop() {
                    if let Some(fst @ Node::String(_)) = nodes.pop() {
                        if nodes.is_empty() {
                            return Node::Define(Box::new(fst));
                        }
                    }
                }
            }
            _ => (),
        };
        Node::Ignored
    }

    fn make_function_declaration(
        &mut self,
        _attributes: Self::R,
        header: Self::R,
        body: Self::R,
    ) -> Self::R {
        match (header, body) {
            (Node::Ignored, Node::Ignored) => Node::Ignored,
            (v, Node::Ignored) | (Node::Ignored, v) => v,
            (v1, v2) => Node::List(vec![v1, v2]),
        }
    }

    fn make_function_declaration_header(
        &mut self,
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
    ) -> Self::R {
        match name {
            Node::Ignored => Node::Ignored,
            _ => Node::FunctionDecl(Box::new(name)),
        }
    }

    fn make_trait_use(
        &mut self,
        _keyword: Self::R,
        names: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        match names {
            Node::Ignored => Node::Ignored,
            _ => Node::TraitUseClause(Box::new(names)),
        }
    }

    fn make_require_clause(
        &mut self,
        _keyword: Self::R,
        kind: Self::R,
        name: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        match name {
            Node::Ignored => Node::Ignored,
            _ => match kind {
                Node::Extends => Node::RequireExtendsClause(Box::new(name)),
                Node::Implements => Node::RequireImplementsClause(Box::new(name)),
                _ => Node::Ignored,
            },
        }
    }

    fn make_const_declaration(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        hint: Self::R,
        decls: Self::R,
        _arg4: Self::R,
    ) -> Self::R {
        // None of the Node::Ignoreds should happen in a well-formed file, but they could happen in
        // a malformed one.
        match decls {
            Node::List(nodes) => match nodes.as_slice() {
                [Node::List(nodes)] => match nodes.as_slice() {
                    [name, initializer] => Node::ConstDecl(Box::new(ConstDeclChildren {
                        name: name.clone(),
                        initializer: initializer.clone(),
                        hint,
                    })),
                    _ => Node::Ignored,
                },
                _ => Node::Ignored,
            },
            _ => Node::Ignored,
        }
    }

    fn make_constant_declarator(&mut self, name: Self::R, initializer: Self::R) -> Self::R {
        match name {
            Node::Ignored => Node::Ignored,
            _ => Node::List(vec![name, initializer]),
        }
    }

    fn make_namespace_declaration(
        &mut self,
        _keyword: Self::R,
        name: Self::R,
        body: Self::R,
    ) -> Self::R {
        match body {
            Node::Ignored => Node::Ignored,
            _ => Node::NamespaceDecl(Box::new(name), Box::new(body)),
        }
    }

    fn make_namespace_body(
        &mut self,
        _left_brace: Self::R,
        decls: Self::R,
        _right_brace: Self::R,
    ) -> Self::R {
        decls
    }

    fn make_namespace_empty_body(&mut self, _semicolon: Self::R) -> Self::R {
        Node::EmptyBody
    }

    fn make_methodish_declaration(
        &mut self,
        _attributes: Self::R,
        _function_decl_header: Self::R,
        body: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        match body {
            Node::Ignored => Node::Ignored,
            _ => Node::MethodDecl(Box::new(body)),
        }
    }

    fn make_classish_declaration(
        &mut self,
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
    ) -> Self::R {
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
        }
    }

    fn make_classish_body(
        &mut self,
        _left_brace: Self::R,
        elements: Self::R,
        _right_brace: Self::R,
    ) -> Self::R {
        elements
    }

    fn make_old_attribute_specification(
        &mut self,
        _left_double_angle: Self::R,
        attributes: Self::R,
        _right_double_angle: Self::R,
    ) -> Self::R {
        attributes
    }

    fn make_attribute_specification(&mut self, attributes: Self::R) -> Self::R {
        attributes
    }

    fn make_attribute(&mut self, _at: Self::R, attibute: Self::R) -> Self::R {
        attibute
    }

    fn make_constructor_call(
        &mut self,
        class_type: Self::R,
        _left_paren: Self::R,
        argument_list: Self::R,
        _right_paren: Self::R,
    ) -> Self::R {
        Node::ListItem(Box::new((class_type, argument_list)))
    }

    fn make_decorated_expression(&mut self, _decorator: Self::R, expression: Self::R) -> Self::R {
        expression
    }

    fn make_scope_resolution_expression(
        &mut self,
        qualifier: Self::R,
        _operator: Self::R,
        name: Self::R,
    ) -> Self::R {
        Node::ScopeResolutionExpression(Box::new((qualifier, name)))
    }
}
