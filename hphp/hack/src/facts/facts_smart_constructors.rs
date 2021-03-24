/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
mod facts_smart_constructors_generated;

use escaper::{extract_unquoted_string, unescape_double, unescape_single};
use flatten_smart_constructors::{FlattenOp, FlattenSmartConstructors};
use parser_core_types::{
    lexable_token::LexableToken, positioned_token::PositionedToken, source_text::SourceText,
    syntax_kind::SyntaxKind, token_factory::SimpleTokenFactoryImpl, token_kind::TokenKind,
};

pub use crate::facts_smart_constructors_generated::*;

pub type HasScriptContent<'a> = (bool, SourceText<'a>);

impl<'src> FactsSmartConstructors<'src> {
    pub fn new(src: &SourceText<'src>) -> Self {
        Self {
            state: (false, src.clone()),
            token_factory: SimpleTokenFactoryImpl::new(),
        }
    }
}

// TODO(leoo) consider avoiding always materializing substrings using something like (hard):
// type GetName<'a> = Box<Fn() -> &'a [u8]>;  // would require lifetime 'a param everywhere
pub struct GetName {
    string: Vec<u8>,
    unescape: fn(String) -> String,
}

impl GetName {
    pub fn get(&self) -> &Vec<u8> {
        &self.string
    }
    pub fn to_string(&self) -> String {
        String::from_utf8_lossy(&self.string).to_string()
    }
    pub fn to_unescaped_string(&self) -> String {
        let unescape = self.unescape;
        unescape(self.to_string())
    }
}

impl std::fmt::Debug for GetName {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "GetName {{ string: {}, unescape:? }}", self.to_string())
    }
}

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
    FileAttributeSpecification(Box<Node>),
    // declarations
    ClassDecl(Box<ClassDeclChildren>),
    FunctionDecl(Box<Node>),
    MethodDecl(Box<Node>, Box<Node>, Box<Node>),
    EnumUseClause(Box<Node>),
    EnumDecl(Box<EnumDeclChildren>),
    EnumClassDecl(Box<EnumClassDeclChildren>),
    TraitUseClause(Box<Node>),
    RequireExtendsClause(Box<Node>),
    RequireImplementsClause(Box<Node>),
    ConstDecl(Box<Node>),
    Define(Box<Node>),
    TypeAliasDecl(Box<TypeAliasDeclChildren>),
    NamespaceDecl(Box<Node>, Box<Node>),
    EmptyBody,
}

#[derive(Debug)]
pub struct ClassDeclChildren {
    pub modifiers: Node,
    pub xhp: Node,
    pub kind: Node,
    pub name: Node,
    pub attributes: Node,
    pub extends: Node,
    pub implements: Node,
    pub constrs: Node,
    pub body: Node,
}

#[derive(Debug)]
pub struct EnumDeclChildren {
    pub name: Node,
    pub attributes: Node,
    pub use_clauses: Node,
}

#[derive(Debug)]
pub struct EnumClassDeclChildren {
    pub name: Node,
    pub attributes: Node,
    pub extends: Node,
}

#[derive(Debug)]
pub struct TypeAliasDeclChildren {
    pub name: Node,
    pub attributes: Node,
}

impl<'a> FlattenOp for FactsSmartConstructors<'_> {
    type S = Node;

    fn flatten(&self, _kind: SyntaxKind, lst: Vec<Self::S>) -> Self::S {
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

    fn zero(_kind: SyntaxKind) -> Self::S {
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

impl<'a> FlattenSmartConstructors<'a, HasScriptContent<'a>> for FactsSmartConstructors<'a> {
    fn make_token(&mut self, token: PositionedToken) -> Self::R {
        let token_text = || {
            self.state
                .1
                .sub(
                    token.leading_start_offset().unwrap_or(0) + token.leading_width(),
                    token.width(),
                )
                .to_vec()
        };
        let kind = token.kind();
        let result = match kind {
            TokenKind::Name => Node::Name(GetName {
                string: token_text(),
                unescape: |string| string,
            }),
            TokenKind::DecimalLiteral => Node::String(GetName {
                string: token_text(),
                unescape: |string| string,
            }),
            TokenKind::SingleQuotedStringLiteral => Node::String(GetName {
                string: token_text(),
                unescape: |string| {
                    let tmp = unescape_single(string.as_str()).ok().unwrap();
                    extract_unquoted_string(&tmp, 0, tmp.len()).ok().unwrap()
                },
            }),
            TokenKind::DoubleQuotedStringLiteral => Node::String(GetName {
                string: token_text(),
                unescape: |string| {
                    let tmp = unescape_double(string.as_str()).ok().unwrap();
                    // FIXME: This is NOT SAFE--`unescape_double` may return
                    // invalid UTF-8, and constructing a String containing
                    // invalid UTF-8 can trigger undefined behavior. We should
                    // use `String::from_utf8_lossy` or consistently represent
                    // unescaped string literals with `bstr::BString` instead.
                    let tmp = unsafe { String::from_utf8_unchecked(tmp.into()) };
                    extract_unquoted_string(&tmp, 0, tmp.len()).ok().unwrap()
                },
            }),
            TokenKind::XHPClassName => Node::XhpName(GetName {
                string: token_text(),
                unescape: |string| string,
            }),
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
        // assume file has script content if it has any tokens besides !# or EOF
        self.state.0 |= match kind {
            TokenKind::EndOfFile | TokenKind::Hashbang => false,
            _ => true,
        };
        result
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

    fn make_enum_use(&mut self, _keyword: Self::R, names: Self::R, _semicolon: Self::R) -> Self::R {
        match names {
            Node::Ignored => Node::Ignored,
            _ => Node::EnumUseClause(Box::new(names)),
        }
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
        use_clauses: Self::R,
        _enumerators: Self::R,
        _right_brace: Self::R,
    ) -> Self::R {
        match name {
            Node::Ignored => Node::Ignored,
            _ => Node::EnumDecl(Box::new(EnumDeclChildren {
                name,
                attributes,
                use_clauses,
            })),
        }
    }

    fn make_enum_class_declaration(
        &mut self,
        attributes: Self::R,
        _enum_keyword: Self::R,
        _class_keyword: Self::R,
        name: Self::R,
        _colon: Self::R,
        _base: Self::R,
        _extends_keyword: Self::R,
        extends_list: Self::R,
        _left_brace: Self::R,
        _elements: Self::R,
        _right_brace: Self::R,
    ) -> Self::R {
        match name {
            Node::Ignored => Node::Ignored,
            _ => Node::EnumClassDecl(Box::new(EnumClassDeclChildren {
                name,
                attributes,
                extends: extends_list,
            })),
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
            _ => {}
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
        _capability: Self::R,
        _colon: Self::R,
        _readonly: Self::R,
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

    fn make_constant_declarator(&mut self, name: Self::R, _initializer: Self::R) -> Self::R {
        match name {
            Node::Ignored => Node::Ignored,
            _ => Node::ConstDecl(Box::new(name)),
        }
    }

    fn make_namespace_declaration(&mut self, header: Self::R, body: Self::R) -> Self::R {
        match body {
            Node::Ignored => Node::Ignored,
            _ => Node::NamespaceDecl(Box::new(header), Box::new(body)),
        }
    }

    fn make_namespace_declaration_header(&mut self, _keyword: Self::R, name: Self::R) -> Self::R {
        name
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
        attributes: Self::R,
        function_decl_header: Self::R,
        body: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        Node::MethodDecl(
            Box::new(attributes),
            Box::new(function_decl_header),
            Box::new(body),
        )
    }

    fn make_classish_declaration(
        &mut self,
        attributes: Self::R,
        modifiers: Self::R,
        xhp: Self::R,
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
                xhp,
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

    fn make_file_attribute_specification(
        &mut self,
        _left_double_angle: Self::R,
        _keyword: Self::R,
        _colon: Self::R,
        attributes: Self::R,
        _right_double_angle: Self::R,
    ) -> Self::R {
        Node::FileAttributeSpecification(Box::new(attributes))
    }
}
