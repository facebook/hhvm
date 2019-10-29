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
use hhbc_string_utils_rust::GetName;
use oxidized::{
    aast,
    ast_defs::{FunKind, Id, Variance},
    direct_decl_parser::Decls,
    pos::Pos,
    s_map::SMap,
    typing_defs::{
        DeclTy, FunArity, FunElt, FunParam, FunParams, FunTparamsKind, FunType, ParamMode,
        PossiblyEnforcedTy, Reactivity, Tparam, Ty, Ty_, TypedefType,
    },
    typing_reason::Reason,
};
use parser::{
    indexed_source_text::IndexedSourceText, lexable_token::LexableToken, token_kind::TokenKind,
};
use std::collections::HashSet;

pub use crate::direct_decl_smart_constructors_generated::*;

pub fn empty_decls() -> Decls {
    Decls {
        classes: SMap::new(),
        funs: SMap::new(),
        typedefs: SMap::new(),
        consts: SMap::new(),
    }
}

fn try_collect<T, E>(vec: Vec<Result<T, E>>) -> Result<Vec<T>, E> {
    vec.into_iter().try_fold(Vec::new(), |mut acc, elem| {
        acc.push(elem?);
        Ok(acc)
    })
}

fn mangle_xhp_id(mut name: String) -> String {
    fn ignore_id(name: &str) -> bool {
        name.starts_with("class@anonymous") || name.starts_with("Closure$")
    }

    fn is_xhp(name: &str) -> bool {
        name.chars().next().map_or(false, |c| c == ':')
    }

    if !ignore_id(&name) {
        if is_xhp(&name) {
            name.replace_range(..1, "xhp_")
        }
        name.replace(":", "__").replace("-", "_")
    } else {
        name
    }
}

pub fn get_name(namespace: &str, name: &Node_) -> Result<(String, Pos), String> {
    fn qualified_name_from_parts(
        namespace: &str,
        parts: &Vec<Node_>,
        pos: &Pos,
    ) -> Result<(String, Pos), String> {
        let mut qualified_name = String::new();
        for part in parts {
            match part {
                Node_::Name(name, _pos) => {
                    qualified_name.push_str(&String::from_utf8_lossy(name.get().as_slice()))
                }
                Node_::Backslash(_) => qualified_name.push('\\'),
                Node_::ListItem(listitem) => {
                    if let (Node_::Name(name, _), Node_::Backslash(_)) = &**listitem {
                        qualified_name.push_str(&String::from_utf8_lossy(name.get().as_slice()));
                        qualified_name.push_str("\\");
                    } else {
                        return Err(format!(
                            "Expected a name or backslash, but got {:?}",
                            listitem
                        ));
                    }
                }
                n => {
                    return Err(format!(
                        "Expected a name, backslash, or list item, but got {:?}",
                        n
                    ))
                }
            }
        }
        let name = if namespace.is_empty() {
            qualified_name // globally qualified name
        } else {
            let namespace = namespace.to_owned();
            let namespace = if namespace.ends_with("\\") {
                namespace
            } else {
                namespace + "\\"
            };
            namespace + &qualified_name
        };
        let pos = pos.clone();
        Ok((name, pos))
    }

    match name {
        Node_::Name(name, pos) => {
            // always a simple name
            let name = name.to_string();
            let name = if namespace.is_empty() {
                name
            } else {
                namespace.to_owned() + "\\" + &name
            };
            let pos = pos.clone();
            Ok((name, pos))
        }
        Node_::XhpName(name, pos) => {
            // xhp names are always unqualified
            let name = name.to_string();
            Ok((mangle_xhp_id(name), pos.clone()))
        }
        Node_::QualifiedName(parts, pos) => qualified_name_from_parts(namespace, &parts, pos),
        n => {
            return Err(format!(
                "Expected a name, XHP name, or qualified name, but got {:?}",
                n
            ))
        }
    }
}

#[derive(Clone, Debug)]
enum NamespaceType {
    Simple(String),
    Delimited(Vec<String>),
}

#[derive(Clone, Debug)]
struct NamespaceBuilder {
    namespace: NamespaceType,
    in_progress_namespace: String,
    is_building_namespace: bool,
}

impl NamespaceBuilder {
    fn new() -> NamespaceBuilder {
        NamespaceBuilder {
            namespace: NamespaceType::Delimited(Vec::new()),
            in_progress_namespace: String::new(),
            is_building_namespace: false,
        }
    }

    fn set_namespace(&mut self) {
        // This clone isn't a perf mistake because we might keep mutating
        // self.in_progress_namespace and we don't want the current namespace to
        // reflect those changes.
        self.namespace = NamespaceType::Simple(self.in_progress_namespace.clone());
    }

    fn push_namespace(&mut self) {
        // If we're currently in a simple namespace, then having a delimited
        // namespace (the kind we'd be in if we're calling this method) is an
        // error. We're not in the business of handling errors in this parser,
        // so just ignore it.
        if let NamespaceType::Delimited(ref mut vec) = self.namespace {
            // This clone isn't a perf mistake because we might keep mutating
            // self.in_progress_namespace and we don't want the namespace stack
            // to reflect those changes.
            vec.push(self.in_progress_namespace.clone());
        }
    }

    fn pop_namespace(&mut self) {
        // If we're currently in a simple namespace, then having a delimited
        // namespace (the kind we'd be in if we're calling this method) is an
        // error. We're not in the business of handling errors in this parser,
        // so just ignore it.
        if let NamespaceType::Delimited(ref mut vec) = self.namespace {
            // This clone isn't a perf mistake because we might keep mutating
            // self.in_progress_namespace and we don't want the namespace stack
            // to reflect those changes.
            // Also, while we normally try to use the Result type to surface
            // errors that we see as part of the file being malformed, the only
            // way we can hit this pop without having hit the push first is
            // through coding error, so we panic instead.
            vec.pop()
                .expect("Attempted to pop from the namespace stack when there were no entries");
            self.in_progress_namespace = self.current_namespace().to_string();
        }
    }

    fn current_namespace(&self) -> &str {
        match &self.namespace {
            NamespaceType::Simple(s) => &s,
            NamespaceType::Delimited(stack) => stack.last().map(|ns| ns.as_str()).unwrap_or(""),
        }
    }
}

#[derive(Clone, Debug)]
pub struct State<'a> {
    pub source_text: IndexedSourceText<'a>,
    pub decls: Decls,
    namespace_builder: NamespaceBuilder,
}

impl<'a> State<'a> {
    pub fn new(source_text: IndexedSourceText) -> State {
        State {
            source_text,
            decls: empty_decls(),
            namespace_builder: NamespaceBuilder::new(),
        }
    }
}

#[derive(Clone, Debug)]
pub enum HintValue {
    Void,
    Int,
    Bool,
    Float,
    String,
    Num,
    ArrayKey,
    NoReturn,
    Apply(GetName),
}

#[derive(Clone, Debug)]
pub struct VariableDecl {
    hint: Node_,
    name: String,
    pos: Pos,
}

#[derive(Clone, Debug)]
pub enum Node_ {
    List(Vec<Node_>),
    Ignored,
    // tokens
    Name(GetName, Pos),
    String(GetName),
    XhpName(GetName, Pos),
    Hint(HintValue, Pos),
    Backslash(Pos), // This needs a pos since it shows up in names.
    ListItem(Box<(Node_, Node_)>),
    Variable(Box<VariableDecl>),
    Class,
    Interface,
    Trait,
    Extends,
    Implements,
    Abstract,
    Final,
    Static,
    QualifiedName(Vec<Node_>, Pos),
    ScopeResolutionExpression(Box<(Node_, Node_)>),
    // declarations
    ClassDecl(Box<ClassDeclChildren>),
    MethodDecl(Box<Node_>),
    EnumDecl(Box<EnumDeclChildren>),
    TraitUseClause(Box<Node_>),
    RequireExtendsClause(Box<Node_>),
    RequireImplementsClause(Box<Node_>),
    Define(Box<Node_>),
    NamespaceDecl(Box<Node_>, Box<Node_>),
    EmptyBody,
}

impl Node_ {
    pub fn get_pos(&self) -> Result<Pos, String> {
        match self {
            Node_::Name(_, pos) => Ok(pos.clone()),
            Node_::Hint(_, pos) => Ok(pos.clone()),
            Node_::Backslash(pos) => Ok(pos.clone()),
            Node_::ListItem(items) => {
                let fst = &items.0;
                let snd = &items.1;
                match (fst.get_pos(), snd.get_pos()) {
                    (Ok(fst_pos), Ok(snd_pos)) => Pos::merge(fst_pos, snd_pos),
                    (Ok(pos), Err(_)) => Ok(pos),
                    (Err(_), Ok(pos)) => Ok(pos),
                    (Err(_), Err(_)) => Err(format!("No pos found for {:?} or {:?}", fst, snd)),
                }
            }
            Node_::List(items) => items.into_iter().fold(
                Err(format!("No pos found for any children under {:?}", self)),
                |acc, elem| match (acc, elem.get_pos()) {
                    (Ok(acc_pos), Ok(elem_pos)) => Pos::merge(acc_pos, elem_pos),
                    (Err(_), Ok(elem_pos)) => Ok(elem_pos),
                    (acc, Err(_)) => acc,
                },
            ),
            _ => Err(format!("No pos found for node {:?}", self)),
        }
    }

    fn into_vec(self) -> Vec<Self> {
        match self {
            Node_::List(items) => items,
            Node_::Ignored => Vec::new(),
            n => vec![n],
        }
    }
}

pub type Node = Result<Node_, String>;

#[derive(Clone, Debug)]
pub struct ClassDeclChildren {
    pub modifiers: Node_,
    pub kind: Node_,
    pub name: Node_,
    pub attributes: Node_,
    pub extends: Node_,
    pub implements: Node_,
    pub constrs: Node_,
    pub body: Node_,
}

#[derive(Clone, Debug)]
pub struct EnumDeclChildren {
    pub name: Node_,
    pub attributes: Node_,
}

impl DirectDeclSmartConstructors<'_> {
    fn node_to_ty(&self, node: &Node_, type_variables: &HashSet<String>) -> Result<Ty, String> {
        match node {
            Node_::Hint(hv, pos) => {
                let reason = Reason::Rhint(pos.clone());
                let ty_ = match hv {
                    HintValue::Void => Ty_::Tprim(aast::Tprim::Tvoid),
                    HintValue::Int => Ty_::Tprim(aast::Tprim::Tint),
                    HintValue::Bool => Ty_::Tprim(aast::Tprim::Tbool),
                    HintValue::Float => Ty_::Tprim(aast::Tprim::Tfloat),
                    HintValue::String => Ty_::Tprim(aast::Tprim::Tstring),
                    HintValue::Num => Ty_::Tprim(aast::Tprim::Tnum),
                    HintValue::ArrayKey => Ty_::Tprim(aast::Tprim::Tarraykey),
                    HintValue::NoReturn => Ty_::Tprim(aast::Tprim::Tnoreturn),
                    HintValue::Apply(gn) => Ty_::Tapply(
                        Id(pos.clone(), "\\".to_string() + &(gn.to_unescaped_string())),
                        Vec::new(),
                    ),
                };
                Ok(Ty(reason, Box::new(ty_)))
            }
            node => {
                let (name, pos) = get_name("", node)?;
                let reason = Reason::Rhint(pos.clone());
                let ty_ = if type_variables.contains(&name) {
                    Ty_::Tgeneric(name)
                } else {
                    let name = if name.starts_with("\\") {
                        name
                    } else if self.state.namespace_builder.current_namespace().is_empty() {
                        "\\".to_owned() + &name
                    } else {
                        "\\".to_owned()
                            + self.state.namespace_builder.current_namespace()
                            + "\\"
                            + &name
                    };
                    Ty_::Tapply(Id(pos, name), Vec::new())
                };
                Ok(Ty(reason, Box::new(ty_)))
            }
        }
    }

    fn into_variables_list(
        &self,
        list: Node_,
        type_variables: &HashSet<String>,
    ) -> Result<FunParams<DeclTy>, String> {
        match list {
            Node_::List(nodes) => {
                nodes
                    .into_iter()
                    .fold(Ok(Vec::new()), |acc, variable| match (acc, variable) {
                        (Ok(mut variables), Node_::Variable(innards)) => {
                            let VariableDecl { hint, name, pos } = *innards;
                            variables.push(FunParam {
                                pos,
                                name: Some(name),
                                type_: PossiblyEnforcedTy {
                                    enforced: false,
                                    type_: self.node_to_ty(&hint, type_variables)?,
                                },
                                kind: ParamMode::FPnormal,
                                accept_disposable: false,
                                mutability: None,
                                rx_annotation: None,
                            });
                            Ok(variables)
                        }
                        (Ok(_), n) => Err(format!("Expected a variable, but got {:?}", n)),
                        (acc @ Err(_), _) => acc,
                    })
            }
            Node_::Ignored => Ok(Vec::new()),
            n => Err(format!("Expected a list of variables, but got {:?}", n)),
        }
    }
}

impl<'a> FlattenOp for DirectDeclSmartConstructors<'_> {
    type S = Node;

    fn flatten(lst: Vec<Self::S>) -> Self::S {
        let r = lst
            .into_iter()
            .map(|s| match s {
                Ok(Node_::List(children)) => children.into_iter().map(|x| Ok(x)).collect(),
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
        let mut r = try_collect(r)?;
        Ok(match r.as_slice() {
            [] => Node_::Ignored,
            [_] => r.pop().unwrap(),
            _ => Node_::List(r),
        })
    }

    fn zero() -> Self::S {
        Ok(Node_::Ignored)
    }

    fn is_zero(s: &Self::S) -> bool {
        if let Ok(s) = s {
            match s {
                Node_::Ignored |
                // tokens
                Node_::Name(_, _) |
                Node_::String(_) |
                Node_::XhpName(_, _) |
                Node_::Hint(_, _) |
                Node_::Backslash(_) |
                Node_::ListItem(_) |
                Node_::Class |
                Node_::Interface |
                Node_::Trait |
                Node_::Extends |
                Node_::Implements |
                Node_::Abstract |
                Node_::Final |
                Node_::Static |
                Node_::QualifiedName(_, _) => true,
                _ => false,
            }
        } else {
            false
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

        // So... this part gets pretty disgusting. We're setting a lot of parser
        // state in here, and we're unsetting a lot of it down in the actual
        // make_namespace_XXXX methods. I tried a few other ways of handling
        // namespaces, but ultimately rejected them.
        //
        // The reason we even need this code at all is because we insert entries
        // into our state.decls struct as soon as we see them, but because the
        // FFP calls its make_XXXX functions in inside out order (which is the
        // most reasonable thing for a parser to do), we don't actually know
        // which namespace we're in until we leave it, at which point we've
        // already inserted all of the decls from inside that namespace.
        //
        // One idea I attempted was having both the state.decls struct as well
        // as a new state.in_progress_decls struct, and if we hit a
        // make_namespace_XXXX method, we would rename every decl within that
        // struct to have the correct namespace, then merge
        // state.in_progress_decls into state.decls. The problem that I ran into
        // with this approach had to do with the insides of the decls. While
        // it's easy enough to rename every decl's name, it's much harder to dig
        // into each decl and rename every single Tapply, which we have to do if
        // a decl references a type within the same namespace.
        //
        // The facts parser, meanwhile, solves this problem by keeping track of
        // a micro-AST, and only inserting names into the returned facts struct
        // at the very end. This would be convenient for us, but it would
        // require going back to allocating and returning a micro-AST, then
        // walking over that again, instead of building up the decls struct as
        // we go, which is what we do now. Unfortunately, since decls are much
        // more complex than facts (which are just lists of toplevel symbols),
        // the cost of doing things that way is too high for our purposes.
        //
        // So we can't allocate sub-trees for each namespace, and we can't just
        // wait until we hit make_namespace_XXXX then rewrite all the names, so
        // what can we do? The answer is that we can encode a really gross
        // namespace-tracking state machine inside of our parser, and take
        // advantage of some mildly unspoken quasi-implementation details. We
        // always call the make_XXXX methods after we finish the full parsing of
        // whatever we're making... but since make_token() only depends on a
        // single token, it gets called *before* we enter the namespace!
        //
        // The general idea, then, is that when we see a namespace token we know
        // that we're about to parse a namespace name, so we enter a state where
        // we know we're building up a namespace name (as opposed to the many,
        // many other kinds of names). While we're in that state, every name and
        // backslash token gets appended to the in-progress namespace name. Once
        // we see either an opening curly brace or a semicolon, we know that
        // we've finished building the namespace name, so we exit the "building
        // a namespace" state and push the newly created namespace onto our
        // namespace stack. Then, in all of the various make_namespace_XXXX
        // methods, we pop the namespace (since we know we've just exited it).
        //
        // So there we have it. Fully inline namespace tracking, and all it cost
        // was a little elbow grease and a lot of dignity.
        Ok(match kind {
            TokenKind::Name | TokenKind::Variable => {
                let name = GetName::new(token_text(), |string| string);
                if self.state.namespace_builder.is_building_namespace {
                    self.state
                        .namespace_builder
                        .in_progress_namespace
                        .push_str(&name.to_string());
                    Node_::Ignored
                } else {
                    Node_::Name(name, token_pos())
                }
            }
            TokenKind::DecimalLiteral => Node_::String(GetName::new(token_text(), |string| string)),
            TokenKind::SingleQuotedStringLiteral => {
                Node_::String(GetName::new(token_text(), |string| {
                    let tmp = unescape_single(string.as_str()).ok().unwrap();
                    extract_unquoted_string(&tmp, 0, tmp.len()).ok().unwrap()
                }))
            }
            TokenKind::DoubleQuotedStringLiteral => {
                Node_::String(GetName::new(token_text(), |string| {
                    let tmp = unescape_double(string.as_str()).ok().unwrap();
                    extract_unquoted_string(&tmp, 0, tmp.len()).ok().unwrap()
                }))
            }
            TokenKind::XHPClassName => {
                Node_::XhpName(GetName::new(token_text(), |string| string), token_pos())
            }
            TokenKind::String => Node_::Hint(HintValue::String, token_pos()),
            TokenKind::Int => Node_::Hint(HintValue::Int, token_pos()),
            TokenKind::Float => Node_::Hint(HintValue::Float, token_pos()),
            TokenKind::Double => Node_::Hint(
                HintValue::Apply(GetName::new(token_text(), |string| string)),
                token_pos(),
            ),
            TokenKind::Num => Node_::Hint(HintValue::Num, token_pos()),
            TokenKind::Bool => Node_::Hint(HintValue::Bool, token_pos()),
            TokenKind::Boolean => Node_::Hint(
                HintValue::Apply(GetName::new(token_text(), |string| string)),
                token_pos(),
            ),
            TokenKind::Void => Node_::Hint(HintValue::Void, token_pos()),
            TokenKind::Backslash => {
                if self.state.namespace_builder.is_building_namespace {
                    self.state
                        .namespace_builder
                        .in_progress_namespace
                        .push('\\');
                    Node_::Ignored
                } else {
                    Node_::Backslash(token_pos())
                }
            }
            TokenKind::Class => Node_::Class,
            TokenKind::Trait => Node_::Trait,
            TokenKind::Interface => Node_::Interface,
            TokenKind::Extends => Node_::Extends,
            TokenKind::Implements => Node_::Implements,
            TokenKind::Abstract => Node_::Abstract,
            TokenKind::Final => Node_::Final,
            TokenKind::Static => Node_::Static,
            TokenKind::Namespace => {
                self.state.namespace_builder.is_building_namespace = true;
                if !self.state.namespace_builder.current_namespace().is_empty() {
                    self.state
                        .namespace_builder
                        .in_progress_namespace
                        .push('\\')
                }
                Node_::Ignored
            }
            TokenKind::LeftBrace | TokenKind::Semicolon => {
                if self.state.namespace_builder.is_building_namespace {
                    if kind == TokenKind::LeftBrace {
                        self.state.namespace_builder.push_namespace();
                    } else {
                        self.state.namespace_builder.set_namespace();
                    }
                    self.state.namespace_builder.is_building_namespace = false;
                }
                Node_::Ignored
            }
            _ => Node_::Ignored,
        })
    }

    fn make_missing(&mut self, _: usize) -> Self::R {
        Ok(Node_::Ignored)
    }

    fn make_list(&mut self, items: Vec<Self::R>, _: usize) -> Self::R {
        let result = if !items.is_empty()
            && !items.iter().all(|r| match r {
                Ok(Node_::Ignored) => true,
                _ => false,
            }) {
            let items = try_collect(items)?;
            Node_::List(items)
        } else {
            Node_::Ignored
        };
        Ok(result)
    }

    fn make_qualified_name(&mut self, arg0: Self::R) -> Self::R {
        let arg0 = arg0?;
        let pos = arg0.get_pos();
        Ok(match arg0 {
            Node_::Ignored => Node_::Ignored,
            Node_::List(nodes) => Node_::QualifiedName(nodes, pos?),
            node => Node_::QualifiedName(vec![node], pos?),
        })
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
        Ok(match (item?, sep?) {
            (Node_::Ignored, Node_::Ignored) => Node_::Ignored,
            (x, Node_::Ignored) | (Node_::Ignored, x) => x,
            (x, y) => Node_::ListItem(Box::new((x, y))),
        })
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
        let (name, attributes) = (name?, attributes?);
        Ok(match name {
            Node_::Ignored => Node_::Ignored,
            _ => Node_::EnumDecl(Box::new(EnumDeclChildren { name, attributes })),
        })
    }

    fn make_alias_declaration(
        &mut self,
        _attributes: Self::R,
        _keyword: Self::R,
        name: Self::R,
        _generic_params: Self::R,
        _constraint: Self::R,
        _equal: Self::R,
        aliased_type: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let (name, aliased_type) = (name?, aliased_type?);
        match name {
            Node_::Ignored => (),
            _ => {
                let (name, pos) =
                    get_name(self.state.namespace_builder.current_namespace(), &name)?;
                match self.node_to_ty(&aliased_type, &HashSet::new()) {
                    Ok(ty) => {
                        self.state.decls.typedefs.insert(
                            name,
                            TypedefType {
                                pos,
                                vis: aast::TypedefVisibility::Transparent,
                                tparams: Vec::new(),
                                constraint: None,
                                type_: ty,
                                decl_errors: None,
                            },
                        );
                    }
                    Err(msg) => {
                        return Err(format!(
                            "Expected hint or name for type alias {}, but was {:?} ({})",
                            name, aliased_type, msg
                        ))
                    }
                }
            }
        };
        Ok(Node_::Ignored)
    }

    fn make_define_expression(
        &mut self,
        _keyword: Self::R,
        _left_paren: Self::R,
        args: Self::R,
        _right_paren: Self::R,
    ) -> Self::R {
        match args? {
            Node_::List(mut nodes) => {
                if let Some(_snd) = nodes.pop() {
                    if let Some(fst @ Node_::String(_)) = nodes.pop() {
                        if nodes.is_empty() {
                            return Ok(Node_::Define(Box::new(fst)));
                        }
                    }
                }
            }
            _ => (),
        };
        Ok(Node_::Ignored)
    }

    fn make_type_parameter(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        name: Self::R,
        _arg4: Self::R,
    ) -> Self::R {
        name
    }

    fn make_parameter_declaration(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        hint: Self::R,
        name: Self::R,
        _arg5: Self::R,
    ) -> Self::R {
        let (name, pos) = get_name("", &name?)?;
        let hint = hint?;
        Ok(Node_::Variable(Box::new(VariableDecl { hint, name, pos })))
    }

    fn make_function_declaration(
        &mut self,
        _attributes: Self::R,
        header: Self::R,
        body: Self::R,
    ) -> Self::R {
        Ok(match (header?, body?) {
            (Node_::Ignored, Node_::Ignored) => Node_::Ignored,
            (v, Node_::Ignored) | (Node_::Ignored, v) => v,
            (v1, v2) => Node_::List(vec![v1, v2]),
        })
    }

    fn make_function_declaration_header(
        &mut self,
        _modifiers: Self::R,
        _keyword: Self::R,
        name: Self::R,
        type_params: Self::R,
        _left_parens: Self::R,
        param_list: Self::R,
        _right_parens: Self::R,
        _colon: Self::R,
        ret_hint: Self::R,
        _where: Self::R,
    ) -> Self::R {
        let mut type_variables = HashSet::new();
        let type_params = type_params?
            .into_vec()
            .into_iter()
            .map(|node| {
                let (name, pos) = get_name("", &node)?;
                type_variables.insert(name.clone());
                Ok(Tparam {
                    variance: Variance::Invariant,
                    name: Id(pos, name),
                    constraints: Vec::new(),
                    reified: aast::ReifyKind::Erased,
                    user_attributes: Vec::new(),
                })
            })
            .collect::<Vec<Result<Tparam<DeclTy>, String>>>();
        let type_params = try_collect(type_params)?;
        Ok(match name? {
            Node_::Ignored => Node_::Ignored,
            name => {
                let (name, pos) =
                    get_name(self.state.namespace_builder.current_namespace(), &name)?;
                let params = self.into_variables_list(param_list?, &type_variables)?;
                self.state.decls.funs.insert(
                    name,
                    FunElt {
                        deprecated: None,
                        type_: Ty(
                            Reason::Rwitness(pos.clone()),
                            Box::new(Ty_::Tfun(FunType {
                                is_coroutine: false,
                                arity: FunArity::Fstandard(
                                    params.len() as isize,
                                    params.len() as isize,
                                ),
                                tparams: (type_params, FunTparamsKind::FTKtparams),
                                where_constraints: Vec::new(),
                                params,
                                ret: PossiblyEnforcedTy {
                                    enforced: false,
                                    type_: self.node_to_ty(&ret_hint?, &type_variables)?,
                                },
                                fun_kind: FunKind::FSync,
                                reactive: Reactivity::Nonreactive,
                                return_disposable: false,
                                mutability: None,
                                returns_mutable: false,
                                returns_void_to_rx: false,
                            })),
                        ),
                        decl_errors: None,
                        pos,
                    },
                );
                Node_::Ignored
            }
        })
    }

    fn make_trait_use(
        &mut self,
        _keyword: Self::R,
        names: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        Ok(match names? {
            Node_::Ignored => Node_::Ignored,
            names => Node_::TraitUseClause(Box::new(names)),
        })
    }

    fn make_require_clause(
        &mut self,
        _keyword: Self::R,
        kind: Self::R,
        name: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        Ok(match name? {
            Node_::Ignored => Node_::Ignored,
            name => match kind? {
                Node_::Extends => Node_::RequireExtendsClause(Box::new(name)),
                Node_::Implements => Node_::RequireImplementsClause(Box::new(name)),
                _ => Node_::Ignored,
            },
        })
    }

    fn make_const_declaration(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        hint: Self::R,
        decls: Self::R,
        _arg4: Self::R,
    ) -> Self::R {
        // None of the Node_::Ignoreds should happen in a well-formed file, but they could happen in
        // a malformed one.
        let hint = hint?;
        Ok(match decls? {
            Node_::List(nodes) => match nodes.as_slice() {
                [Node_::List(nodes)] => match nodes.as_slice() {
                    [name, _initializer] => {
                        let (name, _) =
                            get_name(self.state.namespace_builder.current_namespace(), name)?;
                        match self.node_to_ty(&hint, &HashSet::new()) {
                            Ok(ty) => self.state.decls.consts.insert(name, ty),
                            Err(msg) => {
                                return Err(format!(
                                    "Expected hint or name for constant {}, but was {:?} ({})",
                                    name, hint, msg
                                ))
                            }
                        };
                        Node_::Ignored
                    }
                    _ => Node_::Ignored,
                },
                _ => Node_::Ignored,
            },
            _ => Node_::Ignored,
        })
    }

    fn make_constant_declarator(&mut self, name: Self::R, initializer: Self::R) -> Self::R {
        let (name, initializer) = (name?, initializer?);
        Ok(match name {
            Node_::Ignored => Node_::Ignored,
            _ => Node_::List(vec![name, initializer]),
        })
    }

    fn make_namespace_declaration(
        &mut self,
        _keyword: Self::R,
        name: Self::R,
        body: Self::R,
    ) -> Self::R {
        self.state.namespace_builder.pop_namespace();
        let (name, body) = (name?, body?);
        Ok(match body {
            Node_::Ignored => Node_::Ignored,
            _ => Node_::NamespaceDecl(Box::new(name), Box::new(body)),
        })
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
        Ok(Node_::EmptyBody)
    }

    fn make_methodish_declaration(
        &mut self,
        _attributes: Self::R,
        _function_decl_header: Self::R,
        body: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let body = body?;
        Ok(match body {
            Node_::Ignored => Node_::Ignored,
            _ => Node_::MethodDecl(Box::new(body)),
        })
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
        let name = name?;
        Ok(match name {
            Node_::Ignored => Node_::Ignored,
            _ => Node_::ClassDecl(Box::new(ClassDeclChildren {
                modifiers: modifiers?,
                kind: keyword?,
                name,
                attributes: attributes?,
                extends: extends?,
                implements: implements?,
                constrs: constrs?,
                body: body?,
            })),
        })
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
        Ok(Node_::ListItem(Box::new((class_type?, argument_list?))))
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
        Ok(Node_::ScopeResolutionExpression(Box::new((
            qualifier?, name?,
        ))))
    }
}
