// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{desugar_expression_tree::desugar, modifier};
use bstr::{BString, B};
use bumpalo::Bump;
use escaper::*;
use hash::{HashMap, HashSet};
use itertools::{Either, Itertools};
use lint_rust::LintError;
use naming_special_names_rust::{
    classes as special_classes, literal, special_functions, special_idents,
    typehints as special_typehints, user_attributes as special_attrs,
};
use ocaml_helper::{int_of_string_opt, parse_int, ParseIntError};
use ocamlrep::rc::RcOc;
use oxidized::{
    aast,
    aast_visitor::{AstParams, Node, Visitor},
    ast,
    ast::Expr_ as E_,
    doc_comment::DocComment,
    errors::{Error as HHError, Naming, NastCheck},
    file_info,
    global_options::GlobalOptions,
    namespace_env::Env as NamespaceEnv,
    pos::Pos,
};
use parser_core_types::{
    indexed_source_text::IndexedSourceText,
    lexable_token::{LexablePositionedToken, LexableToken},
    source_text::SourceText,
    syntax::SyntaxValueWithKind,
    syntax_by_ref::{
        positioned_token::{PositionedToken, TokenFactory as PositionedTokenFactory},
        positioned_value::PositionedValue,
        syntax::Syntax,
        syntax_variant_generated::{SyntaxVariant::*, *},
    },
    syntax_error, syntax_kind,
    syntax_trait::SyntaxTrait,
    token_factory::TokenMutator,
    token_kind::TokenKind as TK,
};
use regex::bytes::Regex;
use stack_limit::StackLimit;
use std::{
    cell::{Ref, RefCell, RefMut},
    matches, mem,
    rc::Rc,
    slice::Iter,
    str::FromStr,
};

fn unescape_single(s: &str) -> Result<BString, escaper::InvalidString> {
    Ok(escaper::unescape_single(s)?.into())
}

fn unescape_nowdoc(s: &str) -> Result<BString, escaper::InvalidString> {
    Ok(escaper::unescape_nowdoc(s)?.into())
}

#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum LiftedAwaitKind {
    LiftedFromStatement,
    LiftedFromConcurrent,
}

type LiftedAwaitExprs = Vec<(Option<ast::Lid>, ast::Expr)>;

#[derive(Debug, Clone)]
pub struct LiftedAwaits {
    pub awaits: LiftedAwaitExprs,
    lift_kind: LiftedAwaitKind,
}

#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum ExprLocation {
    TopLevel,
    MemberSelect,
    InDoubleQuotedString,
    AsStatement,
    RightOfAssignment,
    RightOfAssignmentInUsingStatement,
    RightOfReturn,
    UsingStatement,
    CallReceiver,
}

#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum SuspensionKind {
    SKSync,
    SKAsync,
}

#[derive(Copy, Clone, Eq, PartialEq)]
pub enum TokenOp {
    Skip,
    Noop,
    LeftTrim(usize),
    RightTrim(usize),
}

#[derive(Debug)]
pub struct FunHdr {
    suspension_kind: SuspensionKind,
    readonly_this: Option<ast::ReadonlyKind>,
    name: ast::Sid,
    constrs: Vec<ast::WhereConstraintHint>,
    type_parameters: Vec<ast::Tparam>,
    parameters: Vec<ast::FunParam>,
    contexts: Option<ast::Contexts>,
    unsafe_contexts: Option<ast::Contexts>,
    readonly_return: Option<ast::ReadonlyKind>,
    return_type: Option<ast::Hint>,
}

impl FunHdr {
    fn make_empty(env: &Env<'_>) -> Self {
        Self {
            suspension_kind: SuspensionKind::SKSync,
            readonly_this: None,
            name: ast::Id(env.mk_none_pos(), String::from("<ANONYMOUS>")),
            constrs: vec![],
            type_parameters: vec![],
            parameters: vec![],
            contexts: None,
            unsafe_contexts: None,
            readonly_return: None,
            return_type: None,
        }
    }
}

#[derive(Debug)]
pub struct State {
    // bool represents reification
    pub cls_generics: HashMap<String, bool>,
    // fn_generics also used for methods; maps are separate due to shadowing
    pub fn_generics: HashMap<String, bool>,
    pub in_static_method: bool,
    pub parent_maybe_reified: bool,
    /// This provides a generic mechanism to delay raising parsing errors;
    /// since we're moving FFP errors away from CST to a stage after lowering
    /// _and_ want to prioritize errors before lowering, the lowering errors
    /// must be merely stored when the lowerer runs (until check for FFP runs (on AST)
    /// and raised _after_ FFP error checking (unless we run the lowerer twice,
    /// which would be expensive).
    pub lowpri_errors: Vec<(Pos, String)>,
    /// hh_errors captures errors after parsing, naming, nast, etc.
    pub hh_errors: Vec<HHError>,
    pub lint_errors: Vec<LintError>,
    pub doc_comments: Vec<Option<DocComment>>,

    pub local_id_counter: isize,

    // TODO(hrust): this check is to avoid crash in Ocaml.
    // Remove it after all Ocaml callers are eliminated.
    pub exp_recursion_depth: usize,
}

const EXP_RECUSION_LIMIT: usize = 30_000;

#[derive(Clone)]
pub struct Env<'a> {
    pub codegen: bool,
    pub keep_errors: bool,
    quick_mode: bool,
    /// Show errors even in quick mode. Does not override keep_errors. Hotfix
    /// until we can properly set up saved states to surface parse errors during
    /// typechecking properly.
    pub show_all_errors: bool,
    fail_open: bool,
    file_mode: file_info::Mode,
    pub top_level_statements: bool, /* Whether we are (still) considering TLSs*/

    // Cache none pos, lazy_static doesn't allow Rc.
    pos_none: Pos,
    pub empty_ns_env: RcOc<NamespaceEnv>,

    pub saw_yield: bool, /* Information flowing back up */
    pub lifted_awaits: Option<LiftedAwaits>,
    pub tmp_var_counter: isize,

    pub indexed_source_text: &'a IndexedSourceText<'a>,
    pub parser_options: &'a GlobalOptions,
    pub stack_limit: Option<&'a StackLimit>,

    pub token_factory: PositionedTokenFactory<'a>,
    pub arena: &'a Bump,

    state: Rc<RefCell<State>>,
}

impl<'a> Env<'a> {
    pub fn make(
        codegen: bool,
        quick_mode: bool,
        keep_errors: bool,
        show_all_errors: bool,
        fail_open: bool,
        mode: file_info::Mode,
        indexed_source_text: &'a IndexedSourceText<'a>,
        parser_options: &'a GlobalOptions,
        namespace_env: RcOc<NamespaceEnv>,
        stack_limit: Option<&'a StackLimit>,
        token_factory: PositionedTokenFactory<'a>,
        arena: &'a Bump,
    ) -> Self {
        Env {
            codegen,
            keep_errors,
            quick_mode,
            show_all_errors,
            fail_open,
            file_mode: mode,
            top_level_statements: true,
            saw_yield: false,
            lifted_awaits: None,
            tmp_var_counter: 1,
            indexed_source_text,
            parser_options,
            pos_none: Pos::make_none(),
            empty_ns_env: namespace_env,
            stack_limit,
            token_factory,
            arena,

            state: Rc::new(RefCell::new(State {
                cls_generics: HashMap::default(),
                fn_generics: HashMap::default(),
                in_static_method: false,
                parent_maybe_reified: false,
                lowpri_errors: vec![],
                doc_comments: vec![],
                local_id_counter: 1,
                hh_errors: vec![],
                lint_errors: vec![],
                exp_recursion_depth: 0,
            })),
        }
    }

    fn file_mode(&self) -> file_info::Mode {
        self.file_mode
    }

    fn should_surface_error(&self) -> bool {
        (!self.quick_mode || self.show_all_errors) && self.keep_errors
    }

    fn is_typechecker(&self) -> bool {
        !self.codegen
    }

    fn codegen(&self) -> bool {
        self.codegen
    }

    fn source_text(&self) -> &SourceText<'a> {
        self.indexed_source_text.source_text()
    }

    fn fail_open(&self) -> bool {
        self.fail_open
    }

    fn cls_generics_mut(&mut self) -> RefMut<'_, HashMap<String, bool>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.cls_generics)
    }

    fn fn_generics_mut(&mut self) -> RefMut<'_, HashMap<String, bool>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.fn_generics)
    }

    pub fn clear_generics(&mut self) {
        let mut s = self.state.borrow_mut();
        s.cls_generics = HashMap::default();
        s.fn_generics = HashMap::default();
    }

    // avoids returning a reference to the env
    pub fn get_reification(&self, id: &str) -> Option<bool> {
        let s = self.state.borrow();
        if let Some(reif) = s.fn_generics.get(id) {
            Some(*reif)
        } else {
            s.cls_generics.get(id).copied()
        }
    }

    fn in_static_method(&mut self) -> RefMut<'_, bool> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.in_static_method)
    }

    fn parent_maybe_reified(&mut self) -> RefMut<'_, bool> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.parent_maybe_reified)
    }

    pub fn lowpri_errors(&mut self) -> RefMut<'_, Vec<(Pos, String)>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.lowpri_errors)
    }

    pub fn hh_errors(&mut self) -> RefMut<'_, Vec<HHError>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.hh_errors)
    }

    pub fn lint_errors(&mut self) -> RefMut<'_, Vec<LintError>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.lint_errors)
    }

    fn top_docblock(&self) -> Ref<'_, Option<DocComment>> {
        Ref::map(self.state.borrow(), |s| {
            s.doc_comments.last().unwrap_or(&None)
        })
    }

    fn exp_recursion_depth(&self) -> RefMut<'_, usize> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.exp_recursion_depth)
    }

    fn next_local_id(&self) -> isize {
        let mut id = RefMut::map(self.state.borrow_mut(), |s| &mut s.local_id_counter);
        *id += 1;
        *id
    }

    fn push_docblock(&mut self, doc_comment: Option<DocComment>) {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.doc_comments).push(doc_comment)
    }

    fn pop_docblock(&mut self) {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.doc_comments).pop();
    }

    fn make_tmp_var_name(&mut self) -> String {
        let name = String::from(special_idents::TMP_VAR_PREFIX) + &self.tmp_var_counter.to_string();
        self.tmp_var_counter += 1;
        name
    }

    fn mk_none_pos(&self) -> Pos {
        self.pos_none.clone()
    }

    fn clone_and_unset_toplevel_if_toplevel<'b, 'c>(
        e: &'b mut Env<'c>,
    ) -> impl AsMut<Env<'c>> + 'b {
        if e.top_level_statements {
            let mut cloned = e.clone();
            cloned.top_level_statements = false;
            Either::Left(cloned)
        } else {
            Either::Right(e)
        }
    }

    fn check_stack_limit(&self) {
        if let Some(limit) = &self.stack_limit {
            limit.panic_if_exceeded()
        }
    }
}

impl<'a> AsMut<Env<'a>> for Env<'a> {
    fn as_mut(&mut self) -> &mut Env<'a> {
        self
    }
}

#[derive(Debug)]
pub enum Error {
    MissingSyntax {
        expecting: String,
        pos: Pos,
        node_name: String,
        kind: syntax_kind::SyntaxKind,
    },
    Failwith(String),
}

type S<'arena> = &'arena Syntax<'arena, PositionedToken<'arena>, PositionedValue<'arena>>;

fn p_pos<'a>(node: S<'a>, env: &Env<'_>) -> Pos {
    node.position_exclusive(env.indexed_source_text)
        .unwrap_or_else(|| env.mk_none_pos())
}

fn raise_parsing_error<'a>(node: S<'a>, env: &mut Env<'a>, msg: &str) {
    raise_parsing_error_(Either::Left(node), env, msg)
}

fn raise_parsing_error_pos<'a>(pos: &Pos, env: &mut Env<'a>, msg: &str) {
    raise_parsing_error_(Either::Right(pos), env, msg)
}

fn raise_parsing_error_<'a>(node_or_pos: Either<S<'a>, &Pos>, env: &mut Env<'a>, msg: &str) {
    if env.should_surface_error() {
        let pos = node_or_pos.either(|node| p_pos(node, env), |pos| pos.clone());
        env.lowpri_errors().push((pos, String::from(msg)))
    } else if env.codegen() {
        let pos = node_or_pos.either(
            |node| {
                node.position_exclusive(env.indexed_source_text)
                    .unwrap_or_else(|| env.mk_none_pos())
            },
            |pos| pos.clone(),
        );
        env.lowpri_errors().push((pos, String::from(msg)))
    }
}

fn raise_hh_error(env: &mut Env<'_>, err: HHError) {
    env.hh_errors().push(err);
}

fn raise_lint_error(env: &mut Env<'_>, err: LintError) {
    env.lint_errors().push(err);
}

fn failwith<N>(msg: impl Into<String>) -> Result<N, Error> {
    Err(Error::Failwith(msg.into()))
}

fn text<'a>(node: S<'a>, env: &Env<'_>) -> String {
    String::from(node.text(env.source_text()))
}

fn text_str<'b, 'a>(node: S<'a>, env: &'b Env<'_>) -> &'b str {
    node.text(env.source_text())
}

fn lowering_error(env: &mut Env<'_>, pos: &Pos, text: &str, syntax_kind: &str) {
    if env.is_typechecker() && env.lowpri_errors().is_empty() {
        raise_parsing_error_pos(
            pos,
            env,
            &syntax_error::lowering_parsing_error(text, syntax_kind),
        )
    }
}

fn missing_syntax_<N>(
    fallback: Option<N>,
    expecting: &str,
    node: S<'_>,
    env: &mut Env<'_>,
) -> Result<N, Error> {
    let pos = p_pos(node, env);
    let text = text(node, env);
    lowering_error(env, &pos, &text, expecting);
    if let Some(x) = fallback {
        if env.fail_open() {
            return Ok(x);
        }
    }
    Err(Error::MissingSyntax {
        expecting: String::from(expecting),
        pos: p_pos(node, env),
        node_name: text,
        kind: node.kind(),
    })
}

fn missing_syntax<'a, N>(expecting: &str, node: S<'a>, env: &mut Env<'a>) -> Result<N, Error> {
    missing_syntax_(None, expecting, node, env)
}

fn mp_optional<'a, F, R>(p: F, node: S<'a>, env: &mut Env<'a>) -> Result<Option<R>, Error>
where
    F: FnOnce(S<'a>, &mut Env<'a>) -> Result<R, Error>,
{
    match &node.children {
        Missing => Ok(None),
        _ => p(node, env).map(Some),
    }
}

fn pos_qualified_name<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Sid, Error> {
    if let QualifiedName(c) = &node.children {
        if let SyntaxList(l) = &c.parts.children {
            let p = p_pos(node, env);
            let mut s = String::with_capacity(node.width());
            for i in l.iter() {
                match &i.children {
                    ListItem(li) => {
                        s += text_str(&li.item, env);
                        s += text_str(&li.separator, env);
                    }
                    _ => s += text_str(i, env),
                }
            }
            return Ok(ast::Id(p, s));
        }
    }
    missing_syntax("qualified name", node, env)
}

fn pos_name<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Sid, Error> {
    pos_name_(node, env, None)
}

fn lid_from_pos_name<'a>(pos: Pos, name: S<'a>, env: &mut Env<'a>) -> Result<ast::Lid, Error> {
    let name = pos_name(name, env)?;
    Ok(ast::Lid::new(pos, name.1))
}

fn lid_from_name<'a>(name: S<'a>, env: &mut Env<'a>) -> Result<ast::Lid, Error> {
    let name = pos_name(name, env)?;
    Ok(ast::Lid::new(name.0, name.1))
}

fn p_pstring<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Pstring, Error> {
    p_pstring_(node, env, None)
}

fn p_pstring_<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
    drop_prefix: Option<char>,
) -> Result<ast::Pstring, Error> {
    let ast::Id(p, id) = pos_name_(node, env, drop_prefix)?;
    Ok((p, id))
}

fn drop_prefix(s: &str, prefix: char) -> &str {
    if !s.is_empty() && s.starts_with(prefix) {
        &s[1..]
    } else {
        s
    }
}

fn pos_name_<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
    drop_prefix_c: Option<char>,
) -> Result<ast::Sid, Error> {
    match &node.children {
        QualifiedName(_) => pos_qualified_name(node, env),
        SimpleTypeSpecifier(c) => pos_name_(&c.specifier, env, drop_prefix_c),
        _ => {
            let mut name = node.text(env.indexed_source_text.source_text());
            if let Some(prefix) = drop_prefix_c {
                name = drop_prefix(name, prefix);
            }
            let p = p_pos(node, env);
            Ok(ast::Id(p, String::from(name)))
        }
    }
}

fn mk_str<'a, F>(node: S<'a>, env: &mut Env<'a>, unescaper: F, mut content: &str) -> BString
where
    F: Fn(&str) -> Result<BString, InvalidString>,
{
    if let Some('b') = content.chars().next() {
        content = content.get(1..).unwrap();
    }

    let len = content.len();
    let no_quotes_result = extract_unquoted_string(content, 0, len);
    match no_quotes_result {
        Ok(no_quotes) => {
            let result = unescaper(&no_quotes);
            match result {
                Ok(s) => s,
                Err(_) => {
                    raise_parsing_error(
                        node,
                        env,
                        &format!("Malformed string literal <<{}>>", &no_quotes),
                    );
                    BString::from("")
                }
            }
        }
        Err(_) => {
            raise_parsing_error(
                node,
                env,
                &format!("Malformed string literal <<{}>>", &content),
            );
            BString::from("")
        }
    }
}

fn unesc_dbl(s: &str) -> Result<BString, InvalidString> {
    let unesc_s = unescape_double(s)?;
    if unesc_s == B("''") || unesc_s == B("\"\"") {
        Ok(BString::from(""))
    } else {
        Ok(unesc_s)
    }
}

// TODO: return Cow<[u8]>
fn unesc_xhp(s: &[u8]) -> Vec<u8> {
    lazy_static! {
        static ref WHITESPACE: Regex = Regex::new("[\x20\t\n\r\x0c]+").unwrap();
    }
    WHITESPACE.replace_all(s, &b" "[..]).into_owned()
}

fn unesc_xhp_attr(s: &[u8]) -> Vec<u8> {
    // TODO: change unesc_dbl to &[u8] -> BString
    let r = get_quoted_content(s);
    let r = unsafe { std::str::from_utf8_unchecked(r) };
    unesc_dbl(r).unwrap().into()
}

fn get_quoted_content(s: &[u8]) -> &[u8] {
    lazy_static! {
        static ref QUOTED: Regex = Regex::new(r#"^[\x20\t\n\r\x0c]*"((?:.|\n)*)""#).unwrap();
    }
    QUOTED
        .captures(s)
        .and_then(|c| c.get(1))
        .map_or(s, |m| m.as_bytes())
}

fn token_kind<'a>(node: S<'a>) -> Option<TK> {
    match &node.children {
        Token(t) => Some(t.kind()),
        _ => None,
    }
}

fn check_valid_reified_hint<'a>(env: &mut Env<'a>, node: S<'a>, hint: &ast::Hint) {
    struct Checker<F: FnMut(&String)>(F);
    impl<'ast, F: FnMut(&String)> Visitor<'ast> for Checker<F> {
        type Params = AstParams<(), ()>;

        fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
            self
        }

        fn visit_hint(&mut self, c: &mut (), h: &ast::Hint) -> Result<(), ()> {
            match h.1.as_ref() {
                ast::Hint_::Happly(id, _) => {
                    self.0(&id.1);
                }
                ast::Hint_::Haccess(_, ids) => {
                    ids.iter().for_each(|id| self.0(&id.1));
                }
                _ => {}
            }
            h.recurse(c, self)
        }
    }

    if *env.in_static_method() {
        let f = |id: &String| {
            fail_if_invalid_reified_generic(node, env, id);
        };
        let mut visitor = Checker(f);
        visitor.visit_hint(&mut (), hint).unwrap();
    }
}

fn p_closure_parameter<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<(ast::Hint, Option<ast::HfParamInfo>), Error> {
    match &node.children {
        ClosureParameterTypeSpecifier(c) => {
            let kind = p_param_kind(&c.call_convention, env)?;
            let readonlyness = mp_optional(p_readonly, &c.readonly, env)?;
            let info = Some(ast::HfParamInfo { kind, readonlyness });
            let hint = p_hint(&c.type_, env)?;
            Ok((hint, info))
        }
        _ => missing_syntax("closure parameter", node, env),
    }
}

fn mp_shape_expression_field<'a, F, R>(
    f: F,
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<(ast::ShapeFieldName, R), Error>
where
    F: Fn(S<'a>, &mut Env<'a>) -> Result<R, Error>,
{
    match &node.children {
        FieldInitializer(c) => {
            let name = p_shape_field_name(&c.name, env)?;
            let value = f(&c.value, env)?;
            Ok((name, value))
        }
        _ => missing_syntax("shape field", node, env),
    }
}

fn p_shape_field_name<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::ShapeFieldName, Error> {
    use ast::ShapeFieldName::*;
    let is_valid_shape_literal = |t: &PositionedToken<'a>| {
        let is_str =
            t.kind() == TK::SingleQuotedStringLiteral || t.kind() == TK::DoubleQuotedStringLiteral;
        let text = t.text(env.source_text());
        let is_empty = text == "\'\'" || text == "\"\"";
        is_str && !is_empty
    };
    if let LiteralExpression(c) = &node.children {
        if let Token(t) = &c.expression.children {
            if is_valid_shape_literal(t) {
                let ast::Id(p, n) = pos_name(node, env)?;
                let unescp = if t.kind() == TK::SingleQuotedStringLiteral {
                    unescape_single
                } else {
                    unesc_dbl
                };
                let str_ = mk_str(node, env, unescp, &n);
                if int_of_string_opt(&str_).is_some() {
                    raise_parsing_error(node, env, &syntax_error::shape_field_int_like_string)
                }
                return Ok(SFlitStr((p, str_)));
            }
        }
    }
    match &node.children {
        ScopeResolutionExpression(c) => Ok(SFclassConst(
            pos_name(&c.qualifier, env)?,
            p_pstring(&c.name, env)?,
        )),
        _ => {
            raise_parsing_error(node, env, &syntax_error::invalid_shape_field_name);
            let ast::Id(p, n) = pos_name(node, env)?;
            Ok(SFlitStr((p, mk_str(node, env, unesc_dbl, &n))))
        }
    }
}

fn p_shape_field<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::ShapeFieldInfo, Error> {
    match &node.children {
        FieldSpecifier(c) => {
            let optional = !c.question.is_missing();
            let name = p_shape_field_name(&c.name, env)?;
            let hint = p_hint(&c.type_, env)?;
            Ok(ast::ShapeFieldInfo {
                optional,
                hint,
                name,
            })
        }
        _ => {
            let (name, hint) = mp_shape_expression_field(p_hint, node, env)?;
            Ok(ast::ShapeFieldInfo {
                optional: false,
                name,
                hint,
            })
        }
    }
}

fn p_targ<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Targ, Error> {
    Ok(ast::Targ((), p_hint(node, env)?))
}

fn p_hint_<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Hint_, Error> {
    use ast::Hint_::*;
    let unary =
        |kw, ty, env: &mut Env<'a>| Ok(Happly(pos_name(kw, env)?, could_map(p_hint, ty, env)?));
    let binary = |kw, key, ty, env: &mut Env<'a>| {
        let kw = pos_name(kw, env)?;
        let key = p_hint(key, env)?;
        let value = p_hint(ty, env)?;
        Ok(Happly(kw, vec![key, value]))
    };

    match &node.children {
        Token(token) if token.kind() == TK::Variable => {
            let ast::Id(_pos, name) = pos_name(node, env)?;
            Ok(Hvar(name))
        }
        /* Dirty hack; CastExpression can have type represented by token */
        Token(_) | SimpleTypeSpecifier(_) | QualifiedName(_) => {
            let ast::Id(pos, name) = pos_name(node, env)?;
            let mut suggest = |name: &str, canonical: &str| {
                raise_parsing_error(
                    node,
                    env,
                    &syntax_error::invalid_typehint_alias(name, canonical),
                );
            };
            if "integer".eq_ignore_ascii_case(&name) {
                suggest(&name, special_typehints::INT);
            } else if "boolean".eq_ignore_ascii_case(&name) {
                suggest(&name, special_typehints::BOOL);
            } else if "double".eq_ignore_ascii_case(&name) || "real".eq_ignore_ascii_case(&name) {
                suggest(&name, special_typehints::FLOAT);
            }

            use naming_special_names_rust::coeffects::{CAPABILITIES, CONTEXTS};
            if env.file_mode() != file_info::Mode::Mhhi && !env.codegen() {
                let sn = strip_ns(&name);
                if sn.starts_with(CONTEXTS) || sn.starts_with(CAPABILITIES) {
                    raise_parsing_error(node, env, &syntax_error::direct_coeffects_reference);
                }
            }
            Ok(Happly(ast::Id(pos, name), vec![]))
        }
        ShapeTypeSpecifier(c) => {
            let allows_unknown_fields = !c.ellipsis.is_missing();
            /* if last element lacks a separator and ellipsis is present, error */
            if allows_unknown_fields {
                if let SyntaxList(items) = &c.fields.children {
                    if let Some(ListItem(item)) = items.last().map(|i| &i.children) {
                        if item.separator.is_missing() {
                            raise_parsing_error(
                                node,
                                env,
                                &syntax_error::shape_type_ellipsis_without_trailing_comma,
                            )
                        }
                    }
                }
            }

            let field_map = could_map(p_shape_field, &c.fields, env)?;
            let mut set = HashSet::default();
            for f in field_map.iter() {
                if !set.insert(f.name.get_name()) {
                    raise_hh_error(env, Naming::fd_name_already_bound(f.name.get_pos().clone()));
                }
            }

            Ok(Hshape(ast::NastShapeInfo {
                allows_unknown_fields,
                field_map,
            }))
        }
        TupleTypeSpecifier(c) => Ok(Htuple(could_map(p_hint, &c.types, env)?)),
        UnionTypeSpecifier(c) => Ok(Hunion(could_map(&p_hint, &c.types, env)?)),
        IntersectionTypeSpecifier(c) => Ok(Hintersection(could_map(&p_hint, &c.types, env)?)),
        KeysetTypeSpecifier(c) => Ok(Happly(
            pos_name(&c.keyword, env)?,
            could_map(p_hint, &c.type_, env)?,
        )),
        VectorTypeSpecifier(c) => unary(&c.keyword, &c.type_, env),
        ClassnameTypeSpecifier(c) => unary(&c.keyword, &c.type_, env),
        TupleTypeExplicitSpecifier(c) => unary(&c.keyword, &c.types, env),
        VarrayTypeSpecifier(c) => unary(&c.keyword, &c.type_, env),
        DarrayTypeSpecifier(c) => binary(&c.keyword, &c.key, &c.value, env),
        DictionaryTypeSpecifier(c) => unary(&c.keyword, &c.members, env),
        GenericTypeSpecifier(c) => {
            let name = pos_name(&c.class_type, env)?;
            let args = &c.argument_list;
            let type_args = match &args.children {
                TypeArguments(c) => could_map(p_hint, &c.types, env)?,
                _ => missing_syntax("generic type arguments", args, env)?,
            };
            Ok(Happly(name, type_args))
        }
        NullableTypeSpecifier(c) => Ok(Hoption(p_hint(&c.type_, env)?)),
        LikeTypeSpecifier(c) => Ok(Hlike(p_hint(&c.type_, env)?)),
        SoftTypeSpecifier(c) => Ok(Hsoft(p_hint(&c.type_, env)?)),
        ClosureTypeSpecifier(c) => {
            let (param_list, variadic_hints): (Vec<S<'a>>, Vec<S<'a>>) = c
                .parameter_list
                .syntax_node_to_list_skip_separator()
                .partition(|n| match &n.children {
                    VariadicParameter(_) => false,
                    _ => true,
                });
            let (type_hints, info) = param_list
                .iter()
                .map(|p| p_closure_parameter(p, env))
                .collect::<Result<Vec<_>, _>>()?
                .into_iter()
                .unzip();
            let variadic_hints = variadic_hints
                .iter()
                .map(|v| match &v.children {
                    VariadicParameter(c) => {
                        if c.type_.is_missing() {
                            raise_parsing_error(v, env, "Cannot use ... without a typehint");
                        }
                        Ok(Some(p_hint(&c.type_, env)?))
                    }
                    _ => panic!("expect variadic parameter"),
                })
                .collect::<Result<Vec<_>, _>>()?;
            if variadic_hints.len() > 1 {
                return failwith(format!(
                    "{} variadic parameters found. There should be no more than one.",
                    variadic_hints.len()
                ));
            }
            let ctxs = p_contexts(&c.contexts, env)?;
            Ok(Hfun(ast::HintFun {
                is_readonly: mp_optional(p_readonly, &c.readonly_keyword, env)?,
                param_tys: type_hints,
                param_info: info,
                variadic_ty: variadic_hints.into_iter().next().unwrap_or(None),
                ctxs,
                return_ty: p_hint(&c.return_type, env)?,
                is_readonly_return: mp_optional(p_readonly, &c.readonly_return, env)?,
            }))
        }
        AttributizedSpecifier(c) => {
            let attrs = p_user_attribute(&c.attribute_spec, env)?;
            let hint = p_hint(&c.type_, env)?;
            if attrs.iter().any(|attr| attr.name.1 != special_attrs::SOFT) {
                raise_parsing_error(node, env, &syntax_error::only_soft_allowed);
            }
            Ok(*soften_hint(&attrs, hint).1)
        }
        FunctionCtxTypeSpecifier(c) => {
            let ast::Id(_p, n) = pos_name(&c.variable, env)?;
            Ok(HfunContext(n))
        }
        TypeConstant(c) => {
            let child = pos_name(&c.right_type, env)?;
            match p_hint_(&c.left_type, env)? {
                Haccess(root, mut cs) => {
                    cs.push(child);
                    Ok(Haccess(root, cs))
                }
                Hvar(n) => {
                    let pos = p_pos(&c.left_type, env);
                    let root = ast::Hint::new(pos, Hvar(n));
                    Ok(Haccess(root, vec![child]))
                }
                Happly(ty, param) => {
                    if param.is_empty() {
                        let root = ast::Hint::new(ty.0.clone(), Happly(ty, param));
                        Ok(Haccess(root, vec![child]))
                    } else {
                        missing_syntax("type constant base", node, env)
                    }
                }
                _ => missing_syntax("type constant base", node, env),
            }
        }
        ReifiedTypeArgument(_) => {
            raise_parsing_error(node, env, &syntax_error::invalid_reified);
            missing_syntax("refied type", node, env)
        }
        _ => missing_syntax("type hint", node, env),
    }
}

fn p_hint<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Hint, Error> {
    let hint_ = p_hint_(node, env)?;
    let pos = p_pos(node, env);
    let hint = ast::Hint::new(pos, hint_);
    check_valid_reified_hint(env, node, &hint);
    Ok(hint)
}

fn p_simple_initializer<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Expr, Error> {
    match &node.children {
        SimpleInitializer(c) => p_expr(&c.value, env),
        _ => missing_syntax("simple initializer", node, env),
    }
}

fn p_member<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<(ast::Expr, ast::Expr), Error> {
    match &node.children {
        ElementInitializer(c) => Ok((p_expr(&c.key, env)?, p_expr(&c.value, env)?)),
        _ => missing_syntax("darray intrinsic expression element", node, env),
    }
}

fn expand_type_args<'a>(ty: S<'a>, env: &mut Env<'a>) -> Result<Vec<ast::Hint>, Error> {
    match &ty.children {
        TypeArguments(c) => could_map(p_hint, &c.types, env),
        _ => Ok(vec![]),
    }
}

fn p_afield<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Afield, Error> {
    match &node.children {
        ElementInitializer(c) => Ok(ast::Afield::AFkvalue(
            p_expr(&c.key, env)?,
            p_expr(&c.value, env)?,
        )),
        _ => Ok(ast::Afield::AFvalue(p_expr(node, env)?)),
    }
}
// We lower readonly lambda declarations as making the inner lambda have readonly_this.
fn process_readonly_expr(mut e: ast::Expr) -> ast::Expr_ {
    use aast::Expr_::*;
    match &mut e {
        ast::Expr(_, _, Efun(ref mut e)) => {
            e.0.readonly_this = Some(ast::ReadonlyKind::Readonly);
        }
        ast::Expr(_, _, Lfun(ref mut l)) => {
            l.0.readonly_this = Some(ast::ReadonlyKind::Readonly);
        }
        _ => {}
    }
    E_::mk_readonly_expr(e)
}

fn check_intrinsic_type_arg_varity<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
    tys: Vec<ast::Hint>,
) -> Option<ast::CollectionTarg> {
    let count = tys.len();
    let mut tys = tys.into_iter();
    match count {
        2 => Some(ast::CollectionTarg::CollectionTKV(
            ast::Targ((), tys.next().unwrap()),
            ast::Targ((), tys.next().unwrap()),
        )),
        1 => Some(ast::CollectionTarg::CollectionTV(ast::Targ(
            (),
            tys.next().unwrap(),
        ))),
        0 => None,
        _ => {
            raise_parsing_error(node, env, &syntax_error::collection_intrinsic_many_typeargs);
            None
        }
    }
}

fn p_import_flavor<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::ImportFlavor, Error> {
    use ast::ImportFlavor::*;
    match token_kind(node) {
        Some(TK::Include) => Ok(Include),
        Some(TK::Require) => Ok(Require),
        Some(TK::Include_once) => Ok(IncludeOnce),
        Some(TK::Require_once) => Ok(RequireOnce),
        _ => missing_syntax("import flavor", node, env),
    }
}

fn p_null_flavor<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::OgNullFlavor, Error> {
    use ast::OgNullFlavor::*;
    match token_kind(node) {
        Some(TK::QuestionMinusGreaterThan) => Ok(OGNullsafe),
        Some(TK::MinusGreaterThan) => Ok(OGNullthrows),
        _ => missing_syntax("null flavor", node, env),
    }
}

fn wrap_unescaper<F>(unescaper: F, s: &str) -> Result<BString, Error>
where
    F: FnOnce(&str) -> Result<BString, InvalidString>,
{
    unescaper(s).map_err(|e| Error::Failwith(e.msg))
}

fn fail_if_invalid_class_creation<'a>(node: S<'a>, env: &mut Env<'a>, id: impl AsRef<str>) {
    let id = id.as_ref();
    let is_in_static_method = *env.in_static_method();
    if is_in_static_method
        && ((id == special_classes::SELF && env.cls_generics_mut().values().any(|reif| *reif))
            || (id == special_classes::PARENT && *env.parent_maybe_reified()))
    {
        raise_parsing_error(node, env, &syntax_error::static_method_reified_obj_creation);
    }
}

fn fail_if_invalid_reified_generic<'a>(node: S<'a>, env: &mut Env<'a>, id: impl AsRef<str>) {
    let is_in_static_method = *env.in_static_method();
    if is_in_static_method && *env.cls_generics_mut().get(id.as_ref()).unwrap_or(&false) {
        raise_parsing_error(
            node,
            env,
            &syntax_error::cls_reified_generic_in_static_method,
        );
    }
}

fn rfind(s: &[u8], mut i: usize, c: u8) -> Result<usize, Error> {
    if i >= s.len() {
        return failwith("index out of range");
    }
    i += 1;
    while i > 0 {
        i -= 1;
        if s[i] == c {
            return Ok(i);
        }
    }
    failwith("char not found")
}

fn prep_string2<'a>(
    nodes: &'a [Syntax<'a, PositionedToken<'a>, PositionedValue<'a>>],
    env: &mut Env<'a>,
) -> Result<(TokenOp, TokenOp), Error> {
    use TokenOp::*;
    let is_qoute = |c| c == b'\"' || c == b'`';
    let start_is_qoute = |s: &[u8]| {
        (!s.is_empty() && is_qoute(s[0])) || (s.len() > 1 && (s[0] == b'b' && s[1] == b'\"'))
    };
    let last_is_qoute = |s: &[u8]| !s.is_empty() && is_qoute(s[s.len() - 1]);
    let is_heredoc = |s: &[u8]| (s.len() > 3 && &s[0..3] == b"<<<");
    let mut nodes = nodes.iter();
    let first = nodes.next();
    match first.map(|n| &n.children) {
        Some(Token(t)) => {
            let raise = |env| {
                raise_parsing_error(first.unwrap(), env, "Malformed String2 SyntaxList");
            };
            let text = t.text_raw(env.source_text());
            if start_is_qoute(text) {
                let first_token_op = match text[0] {
                    b'b' if text.len() > 2 => LeftTrim(2),
                    _ if is_qoute(text[0]) && text.len() > 1 => LeftTrim(1),
                    _ => Skip,
                };
                if let Some(Token(t)) = nodes.last().map(|n| &n.children) {
                    let last_text = t.text_raw(env.source_text());
                    if last_is_qoute(last_text) {
                        let last_taken_op = match last_text.len() {
                            n if n > 1 => RightTrim(1),
                            _ => Skip,
                        };
                        return Ok((first_token_op, last_taken_op));
                    }
                }
                raise(env);
                Ok((first_token_op, Noop))
            } else if is_heredoc(text) {
                let trim_size = text
                    .iter()
                    .position(|c| *c == b'\n')
                    .ok_or_else(|| Error::Failwith(String::from("newline not found")))?
                    + 1;
                let first_token_op = match trim_size {
                    _ if trim_size == text.len() => Skip,
                    _ => LeftTrim(trim_size),
                };
                if let Some(Token(t)) = nodes.last().map(|n| &n.children) {
                    let text = t.text_raw(env.source_text());
                    let len = text.len();
                    if len != 0 {
                        let n = rfind(text, len - 2, b'\n')?;
                        let last_token_op = match n {
                            0 => Skip,
                            _ => RightTrim(len - n),
                        };
                        return Ok((first_token_op, last_token_op));
                    }
                }
                raise(env);
                Ok((first_token_op, Noop))
            } else {
                Ok((Noop, Noop))
            }
        }
        _ => Ok((Noop, Noop)),
    }
}

fn process_token_op<'a>(
    env: &mut Env<'a>,
    op: TokenOp,
    node: S<'a>,
) -> Result<Option<S<'a>>, Error> {
    use TokenOp::*;
    match op {
        LeftTrim(n) => match &node.children {
            Token(t) => {
                let token = env.token_factory.trim_left(t, n);
                let node = env.arena.alloc(Syntax::make_token(token));
                Ok(Some(node))
            }
            _ => failwith("Token expected"),
        },
        RightTrim(n) => match &node.children {
            Token(t) => {
                let token = env.token_factory.trim_right(t, n);
                let node = env.arena.alloc(Syntax::make_token(token));
                Ok(Some(node))
            }
            _ => failwith("Token expected"),
        },
        _ => Ok(None),
    }
}

fn p_string2<'a>(
    nodes: &'a [Syntax<'a, PositionedToken<'a>, PositionedValue<'a>>],
    env: &mut Env<'a>,
) -> Result<Vec<ast::Expr>, Error> {
    use TokenOp::*;
    let (head_op, tail_op) = prep_string2(nodes, env)?;
    let mut result = Vec::with_capacity(nodes.len());
    let mut i = 0;
    let last = nodes.len() - 1;
    while i <= last {
        let op = match i {
            0 => head_op,
            _ if i == last => tail_op,
            _ => Noop,
        };

        if op == Skip {
            i += 1;
            continue;
        }

        let node = process_token_op(env, op, &nodes[i])?;
        let node = node.unwrap_or(&nodes[i]);

        if token_kind(node) == Some(TK::Dollar) && i < last {
            if let EmbeddedBracedExpression(_) = &nodes[i + 1].children {
                raise_parsing_error(&nodes[i + 1], env, &syntax_error::outside_dollar_str_interp);

                result.push(p_expr_with_loc(
                    ExprLocation::InDoubleQuotedString,
                    &nodes[i + 1],
                    env,
                )?);
                i += 2;
                continue;
            }
        }

        result.push(p_expr_with_loc(
            ExprLocation::InDoubleQuotedString,
            node,
            env,
        )?);
        i += 1;
    }
    Ok(result)
}

fn p_expr_l<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<Vec<ast::Expr>, Error> {
    let p_expr = |n: S<'a>, e: &mut Env<'a>| -> Result<ast::Expr, Error> {
        p_expr_with_loc(ExprLocation::TopLevel, n, e)
    };
    could_map(p_expr, node, env)
}

fn p_expr<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Expr, Error> {
    p_expr_with_loc(ExprLocation::TopLevel, node, env)
}

fn p_expr_for_function_call_arguments<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<(ast::ParamKind, ast::Expr), Error> {
    match &node.children {
        DecoratedExpression(DecoratedExpressionChildren {
            decorator,
            expression,
        }) if token_kind(decorator) == Some(TK::Inout) => Ok((
            ast::ParamKind::Pinout(p_pos(decorator, env)),
            p_expr(expression, env)?,
        )),
        _ => Ok((ast::ParamKind::Pnormal, p_expr(node, env)?)),
    }
}

fn p_expr_for_normal_argument<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<(ast::ParamKind, ast::Expr), Error> {
    Ok((ast::ParamKind::Pnormal, p_expr(node, env)?))
}

fn p_expr_with_loc<'a>(
    location: ExprLocation,
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<ast::Expr, Error> {
    p_expr_impl(location, node, env, None)
}

fn p_expr_impl<'a>(
    location: ExprLocation,
    node: S<'a>,
    env: &mut Env<'a>,
    parent_pos: Option<Pos>,
) -> Result<ast::Expr, Error> {
    // We use location=CallReceiver to set PropOrMethod::IsMethod on ObjGet
    // But only if it is the immediate node.
    let location = match (location, &node.children) {
        (
            ExprLocation::CallReceiver,
            MemberSelectionExpression(_)
            | SafeMemberSelectionExpression(_)
            | EmbeddedMemberSelectionExpression(_)
            | ScopeResolutionExpression(_),
        ) => location,
        (ExprLocation::CallReceiver, _) => ExprLocation::TopLevel,
        (_, _) => location,
    };
    match &node.children {
        BracedExpression(c) => {
            // Either a dynamic method lookup on a dynamic value:
            //   $foo->{$meth_name}();
            // or an XHP splice.
            //   <p id={$id}>hello</p>;
            // In both cases, unwrap, consistent with parentheses.
            p_expr_impl(location, &c.expression, env, parent_pos)
        }
        ParenthesizedExpression(c) => p_expr_impl(location, &c.expression, env, parent_pos),
        ETSpliceExpression(c) => {
            let pos = p_pos(node, env);

            let inner_pos = p_pos(&c.expression, env);
            let inner_expr_ = p_expr_impl_(location, &c.expression, env, parent_pos)?;
            let inner_expr = ast::Expr::new((), inner_pos, inner_expr_);
            Ok(ast::Expr::new(
                (),
                pos,
                ast::Expr_::ETSplice(Box::new(inner_expr)),
            ))
        }
        _ => {
            let pos = p_pos(node, env);
            let expr_ = p_expr_impl_(location, node, env, parent_pos)?;
            Ok(ast::Expr::new((), pos, expr_))
        }
    }
}

fn p_expr_lit<'a>(
    location: ExprLocation,
    _parent: S<'a>,
    expr: S<'a>,
    env: &mut Env<'a>,
) -> Result<ast::Expr_, Error> {
    match &expr.children {
        Token(_) => {
            let s = expr.text(env.indexed_source_text.source_text());
            let check_lint_err = |e: &mut Env<'a>, s: &str, expected: &str| {
                if !e.codegen() && s != expected {
                    raise_lint_error(e, LintError::lowercase_constant(p_pos(expr, e), s));
                }
            };
            match (location, token_kind(expr)) {
                (ExprLocation::InDoubleQuotedString, _) if env.codegen() => {
                    Ok(E_::String(mk_str(expr, env, unesc_dbl, s)))
                }
                (_, Some(TK::DecimalLiteral))
                | (_, Some(TK::OctalLiteral))
                | (_, Some(TK::HexadecimalLiteral))
                | (_, Some(TK::BinaryLiteral)) => {
                    let s = s.replace("_", "");
                    match parse_int(&s) {
                        Err(ParseIntError::OutOfRange) => {
                            raise_parsing_error(expr, env, &syntax_error::out_of_int_range(&s));
                        }
                        Err(ParseIntError::InvalidDigit(int_kind)) => {
                            raise_parsing_error(
                                expr,
                                env,
                                &syntax_error::invalid_integer_digit(int_kind),
                            );
                            missing_syntax(&format!("{}", int_kind), expr, env)?;
                        }
                        Err(ParseIntError::Empty) => {
                            failwith("Unexpected int literal error")?;
                        }
                        Ok(_) => {}
                    }
                    Ok(E_::Int(s))
                }
                (_, Some(TK::FloatingLiteral)) => {
                    // f64::from_str accepts more string than Hacklang, invalid Hack float literal
                    // is caught in lexer.
                    if f64::from_str(s).is_err() {
                        raise_parsing_error(expr, env, &syntax_error::out_of_float_range(s))
                    }
                    Ok(E_::Float(s.into()))
                }
                (_, Some(TK::SingleQuotedStringLiteral)) => {
                    Ok(E_::String(mk_str(expr, env, unescape_single, s)))
                }
                (_, Some(TK::DoubleQuotedStringLiteral)) => {
                    Ok(E_::String(mk_str(expr, env, unescape_double, s)))
                }
                (_, Some(TK::HeredocStringLiteral)) => {
                    Ok(E_::String(mk_str(expr, env, unescape_heredoc, s)))
                }
                (_, Some(TK::NowdocStringLiteral)) => {
                    Ok(E_::String(mk_str(expr, env, unescape_nowdoc, s)))
                }
                (_, Some(TK::NullLiteral)) => {
                    check_lint_err(env, s, literal::NULL);
                    Ok(E_::Null)
                }
                (_, Some(TK::BooleanLiteral)) => {
                    if s.eq_ignore_ascii_case(literal::FALSE) {
                        check_lint_err(env, s, literal::FALSE);
                        Ok(E_::False)
                    } else if s.eq_ignore_ascii_case(literal::TRUE) {
                        check_lint_err(env, s, literal::TRUE);
                        Ok(E_::True)
                    } else {
                        missing_syntax(&format!("boolean (not: {})", s), expr, env)
                    }
                }
                _ => missing_syntax("literal", expr, env),
            }
        }
        SyntaxList(ts) => Ok(E_::String2(p_string2(ts, env)?)),
        _ => missing_syntax("literal expressoin", expr, env),
    }
}

fn p_expr_with_loc_<'a>(
    location: ExprLocation,
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<ast::Expr_, Error> {
    p_expr_impl_(location, node, env, None)
}

fn p_expr_impl_<'a>(
    location: ExprLocation,
    node: S<'a>,
    env: &mut Env<'a>,
    parent_pos: Option<Pos>,
) -> Result<ast::Expr_, Error> {
    if *env.exp_recursion_depth() >= EXP_RECUSION_LIMIT {
        Err(Error::Failwith("Expression recursion limit reached".into()))
    } else {
        *env.exp_recursion_depth() += 1;
        let r = p_expr_impl__(location, node, env, parent_pos);
        *env.exp_recursion_depth() -= 1;
        r
    }
}

fn split_args_vararg<'a>(
    arg_list_node: S<'a>,
    e: &mut Env<'a>,
) -> Result<(Vec<(ast::ParamKind, ast::Expr)>, Option<ast::Expr>), Error> {
    let mut arg_list: Vec<_> = arg_list_node.syntax_node_to_list_skip_separator().collect();
    if let Some(last_arg) = arg_list.last() {
        if let DecoratedExpression(c) = &last_arg.children {
            if token_kind(&c.decorator) == Some(TK::DotDotDot) {
                let _ = arg_list.pop();
                let args: Result<Vec<_>, _> = arg_list
                    .iter()
                    .map(|a| p_expr_for_function_call_arguments(a, e))
                    .collect();
                let args = args?;
                let vararg = p_expr(&c.expression, e)?;
                return Ok((args, Some(vararg)));
            }
        }
    }
    Ok((
        could_map(p_expr_for_function_call_arguments, arg_list_node, e)?,
        None,
    ))
}

fn p_expr_impl__<'a>(
    location: ExprLocation,
    node: S<'a>,
    env: &mut Env<'a>,
    parent_pos: Option<Pos>,
) -> Result<ast::Expr_, Error> {
    env.check_stack_limit();
    use ast::Expr as E;
    let mk_lid = |p: Pos, s: String| ast::Lid(p, (0, s));
    let mk_name_lid = |name: S<'a>, env: &mut Env<'a>| {
        let name = pos_name(name, env)?;
        Ok(mk_lid(name.0, name.1))
    };
    let mk_lvar = |name: S<'a>, env: &mut Env<'a>| Ok(E_::mk_lvar(mk_name_lid(name, env)?));
    let mk_id_expr = |name: ast::Sid| E::new((), name.0.clone(), E_::mk_id(name));
    let p_intri_expr = |kw, ty, v, e: &mut Env<'a>| {
        let hints = expand_type_args(ty, e)?;
        let hints = check_intrinsic_type_arg_varity(node, e, hints);
        Ok(E_::mk_collection(
            pos_name(kw, e)?,
            hints,
            could_map(p_afield, v, e)?,
        ))
    };
    let p_special_call = |recv: S<'a>, args: S<'a>, e: &mut Env<'a>| -> Result<ast::Expr_, Error> {
        // Mark expression as CallReceiver so that we can correctly set PropOrMethod field in ObjGet and ClassGet
        let recv = p_expr_with_loc(ExprLocation::CallReceiver, recv, e)?;
        let (args, varargs) = split_args_vararg(args, e)?;
        Ok(E_::mk_call(recv, vec![], args, varargs))
    };
    let p_obj_get =
        |recv: S<'a>, op: S<'a>, name: S<'a>, e: &mut Env<'a>| -> Result<ast::Expr_, Error> {
            if recv.is_object_creation_expression() && !e.codegen() {
                raise_parsing_error(recv, e, &syntax_error::invalid_constructor_method_call);
            }
            let recv = p_expr(recv, e)?;
            let name = p_expr_with_loc(ExprLocation::MemberSelect, name, e)?;
            let op = p_null_flavor(op, e)?;
            Ok(E_::mk_obj_get(
                recv,
                name,
                op,
                match location {
                    ExprLocation::CallReceiver => ast::PropOrMethod::IsMethod,
                    _ => ast::PropOrMethod::IsProp,
                },
            ))
        };
    let pos = match parent_pos {
        None => p_pos(node, env),
        Some(p) => p,
    };
    match &node.children {
        LambdaExpression(c) => {
            let suspension_kind = mk_suspension_kind(&c.async_);
            let (params, (ctxs, unsafe_ctxs), readonly_ret, ret) = match &c.signature.children {
                LambdaSignature(c) => {
                    let params = could_map(p_fun_param, &c.parameters, env)?;
                    let readonly_ret = mp_optional(p_readonly, &c.readonly_return, env)?;
                    let ctxs = p_contexts(&c.contexts, env)?;
                    let unsafe_ctxs = ctxs.clone();
                    if has_polymorphic_context(env, ctxs.as_ref()) {
                        raise_parsing_error(
                            &c.contexts,
                            env,
                            &syntax_error::lambda_effect_polymorphic,
                        );
                    }
                    let ret = mp_optional(p_hint, &c.type_, env)?;
                    (params, (ctxs, unsafe_ctxs), readonly_ret, ret)
                }
                Token(_) => {
                    let ast::Id(p, n) = pos_name(&c.signature, env)?;
                    (
                        vec![ast::FunParam {
                            annotation: (),
                            type_hint: ast::TypeHint((), None),
                            is_variadic: false,
                            pos: p,
                            name: n,
                            expr: None,
                            callconv: ast::ParamKind::Pnormal,
                            readonly: None,
                            user_attributes: vec![],
                            visibility: None,
                        }],
                        (None, None),
                        None,
                        None,
                    )
                }
                _ => missing_syntax("lambda signature", &c.signature, env)?,
            };

            let (body, yield_) = if !c.body.is_compound_statement() {
                mp_yielding(p_function_body, &c.body, env)?
            } else {
                let mut env1 = Env::clone_and_unset_toplevel_if_toplevel(env);
                mp_yielding(&p_function_body, &c.body, env1.as_mut())?
            };
            let external = c.body.is_external();
            let fun = ast::Fun_ {
                span: pos.clone(),
                readonly_this: None, // filled in by mk_unop
                annotation: (),
                readonly_ret,
                ret: ast::TypeHint((), ret),
                name: ast::Id(pos, String::from(";anonymous")),
                tparams: vec![],
                where_constraints: vec![],
                body: ast::FuncBody { fb_ast: body },
                fun_kind: mk_fun_kind(suspension_kind, yield_),
                params,
                ctxs,
                unsafe_ctxs,
                user_attributes: p_user_attributes(&c.attribute_spec, env)?,
                external,
                doc_comment: None,
            };
            Ok(E_::mk_lfun(fun, vec![]))
        }
        BracedExpression(c) => p_expr_with_loc_(location, &c.expression, env),
        EmbeddedBracedExpression(c) => p_expr_impl_(location, &c.expression, env, Some(pos)),
        ParenthesizedExpression(c) => p_expr_with_loc_(location, &c.expression, env),
        DictionaryIntrinsicExpression(c) => {
            p_intri_expr(&c.keyword, &c.explicit_type, &c.members, env)
        }
        KeysetIntrinsicExpression(c) => p_intri_expr(&c.keyword, &c.explicit_type, &c.members, env),
        VectorIntrinsicExpression(c) => p_intri_expr(&c.keyword, &c.explicit_type, &c.members, env),
        CollectionLiteralExpression(c) => {
            let (collection_name, hints) = match &c.name.children {
                SimpleTypeSpecifier(c) => (pos_name(&c.specifier, env)?, None),
                GenericTypeSpecifier(c) => {
                    let hints = expand_type_args(&c.argument_list, env)?;
                    let hints = check_intrinsic_type_arg_varity(node, env, hints);
                    (pos_name(&c.class_type, env)?, hints)
                }
                _ => (pos_name(&c.name, env)?, None),
            };
            Ok(E_::mk_collection(
                collection_name,
                hints,
                could_map(p_afield, &c.initializers, env)?,
            ))
        }
        VarrayIntrinsicExpression(c) => {
            let hints = expand_type_args(&c.explicit_type, env)?;
            let hints = check_intrinsic_type_arg_varity(node, env, hints);
            let targ = match hints {
                Some(ast::CollectionTarg::CollectionTV(ty)) => Some(ty),
                None => None,
                _ => missing_syntax("VarrayIntrinsicExpression type args", node, env)?,
            };
            Ok(E_::mk_varray(targ, could_map(p_expr, &c.members, env)?))
        }
        DarrayIntrinsicExpression(c) => {
            let hints = expand_type_args(&c.explicit_type, env)?;
            let hints = check_intrinsic_type_arg_varity(node, env, hints);
            match hints {
                Some(ast::CollectionTarg::CollectionTKV(tk, tv)) => Ok(E_::mk_darray(
                    Some((tk, tv)),
                    could_map(p_member, &c.members, env)?,
                )),
                None => Ok(E_::mk_darray(None, could_map(p_member, &c.members, env)?)),
                _ => missing_syntax("DarrayIntrinsicExpression type args", node, env),
            }
        }
        ListExpression(c) => {
            /* TODO: Or tie in with other intrinsics and post-process to List */
            let p_binder_or_ignore = |n: S<'a>, e: &mut Env<'a>| -> Result<ast::Expr, Error> {
                match &n.children {
                    Missing => Ok(E::new((), e.mk_none_pos(), E_::Omitted)),
                    _ => p_expr(n, e),
                }
            };
            Ok(E_::List(could_map(&p_binder_or_ignore, &c.members, env)?))
        }
        EvalExpression(c) => p_special_call(&c.keyword, &c.argument, env),
        IssetExpression(c) => p_special_call(&c.keyword, &c.argument_list, env),
        TupleExpression(c) => Ok(E_::mk_tuple(could_map(p_expr, &c.items, env)?)),
        FunctionCallExpression(c) => {
            let recv = &c.receiver;
            let args = &c.argument_list;
            let get_hhas_adata = || {
                if text_str(recv, env) == "__hhas_adata" {
                    if let SyntaxList(l) = &args.children {
                        if let Some(li) = l.first() {
                            if let ListItem(i) = &li.children {
                                if let LiteralExpression(le) = &i.item.children {
                                    let expr = &le.expression;
                                    if token_kind(expr) == Some(TK::NowdocStringLiteral) {
                                        return Some(expr);
                                    }
                                }
                            }
                        }
                    }
                }
                None
            };
            match get_hhas_adata() {
                Some(expr) => {
                    let literal_expression_pos = p_pos(expr, env);
                    let s = extract_unquoted_string(text_str(expr, env), 0, expr.width())
                        .map_err(|e| Error::Failwith(e.msg))?;
                    Ok(E_::mk_call(
                        p_expr(recv, env)?,
                        vec![],
                        vec![(
                            ast::ParamKind::Pnormal,
                            E::new((), literal_expression_pos, E_::String(s.into())),
                        )],
                        None,
                    ))
                }
                None => {
                    let targs = match (&recv.children, &c.type_args.children) {
                        (_, TypeArguments(c)) => could_map(p_targ, &c.types, env)?,
                        /* TODO might not be needed */
                        (GenericTypeSpecifier(c), _) => match &c.argument_list.children {
                            TypeArguments(c) => could_map(p_targ, &c.types, env)?,
                            _ => vec![],
                        },
                        _ => vec![],
                    };

                    // Mark expression as CallReceiver so that we can correctly set PropOrMethod field in ObjGet and ClassGet
                    let recv = p_expr_with_loc(ExprLocation::CallReceiver, recv, env)?;
                    let (mut args, varargs) = split_args_vararg(args, env)?;

                    // If the function has an enum class label expression, that's
                    // the first argument.
                    if let EnumClassLabelExpression(e) = &c.enum_class_label.children {
                        assert!(
                            e.qualifier.is_missing(),
                            "Parser error: function call with enum class labels"
                        );
                        let pos = p_pos(&c.enum_class_label, env);
                        let enum_class_label = ast::Expr::new(
                            (),
                            pos,
                            E_::mk_enum_class_label(None, pos_name(&e.expression, env)?.1),
                        );
                        args.insert(0, (ast::ParamKind::Pnormal, enum_class_label));
                    }

                    Ok(E_::mk_call(recv, targs, args, varargs))
                }
            }
        }
        FunctionPointerExpression(c) => {
            let targs = match &c.type_args.children {
                TypeArguments(c) => could_map(p_targ, &c.types, env)?,
                _ => vec![],
            };

            let recv = p_expr(&c.receiver, env)?;

            match &recv.2 {
                aast::Expr_::Id(id) => Ok(E_::mk_function_pointer(
                    aast::FunctionPtrId::FPId(*(id.to_owned())),
                    targs,
                )),
                aast::Expr_::ClassConst(c) => {
                    if let aast::ClassId_::CIexpr(aast::Expr(_, _, aast::Expr_::Id(_))) = (c.0).2 {
                        Ok(E_::mk_function_pointer(
                            aast::FunctionPtrId::FPClassConst(c.0.to_owned(), c.1.to_owned()),
                            targs,
                        ))
                    } else {
                        raise_parsing_error(node, env, &syntax_error::function_pointer_bad_recv);
                        missing_syntax("function or static method", node, env)
                    }
                }
                _ => {
                    raise_parsing_error(node, env, &syntax_error::function_pointer_bad_recv);
                    missing_syntax("function or static method", node, env)
                }
            }
        }
        QualifiedName(_) => match location {
            ExprLocation::InDoubleQuotedString => {
                let ast::Id(_, n) = pos_qualified_name(node, env)?;
                Ok(E_::String(n.into()))
            }
            _ => Ok(E_::mk_id(pos_qualified_name(node, env)?)),
        },
        VariableExpression(c) => Ok(E_::mk_lvar(lid_from_pos_name(pos, &c.expression, env)?)),
        PipeVariableExpression(_) => Ok(E_::mk_lvar(mk_lid(
            pos,
            special_idents::DOLLAR_DOLLAR.into(),
        ))),
        InclusionExpression(c) => Ok(E_::mk_import(
            p_import_flavor(&c.require, env)?,
            p_expr(&c.filename, env)?,
        )),
        MemberSelectionExpression(c) => p_obj_get(&c.object, &c.operator, &c.name, env),
        SafeMemberSelectionExpression(c) => p_obj_get(&c.object, &c.operator, &c.name, env),
        EmbeddedMemberSelectionExpression(c) => p_obj_get(&c.object, &c.operator, &c.name, env),
        PrefixUnaryExpression(_) | PostfixUnaryExpression(_) | DecoratedExpression(_) => {
            let (operand, op, postfix) = match &node.children {
                PrefixUnaryExpression(c) => (&c.operand, &c.operator, false),
                PostfixUnaryExpression(c) => (&c.operand, &c.operator, true),
                DecoratedExpression(c) => (&c.expression, &c.decorator, false),
                _ => missing_syntax("unary exppr", node, env)?,
            };

            /**
             * FFP does not destinguish between ++$i and $i++ on the level of token
             * kind annotation. Prevent duplication by switching on `postfix` for
             * the two operatores for which AST /does/ differentiate between
             * fixities.
             */
            use ast::Uop::*;
            let mk_unop = |op, e| Ok(E_::mk_unop(op, e));
            let op_kind = token_kind(op);
            if let Some(TK::At) = op_kind {
                if env.parser_options.po_disallow_silence {
                    raise_parsing_error(op, env, &syntax_error::no_silence);
                }
                if env.codegen() {
                    let expr = p_expr(operand, env)?;
                    mk_unop(Usilence, expr)
                } else {
                    let expr = p_expr_impl(ExprLocation::TopLevel, operand, env, Some(pos))?;
                    Ok(expr.2)
                }
            } else {
                let expr = p_expr(operand, env)?;
                match op_kind {
                    Some(TK::PlusPlus) if postfix => mk_unop(Upincr, expr),
                    Some(TK::MinusMinus) if postfix => mk_unop(Updecr, expr),
                    Some(TK::PlusPlus) => mk_unop(Uincr, expr),
                    Some(TK::MinusMinus) => mk_unop(Udecr, expr),
                    Some(TK::Exclamation) => mk_unop(Unot, expr),
                    Some(TK::Tilde) => mk_unop(Utild, expr),
                    Some(TK::Plus) => mk_unop(Uplus, expr),
                    Some(TK::Minus) => mk_unop(Uminus, expr),
                    Some(TK::Await) => lift_await(pos, expr, env, location),
                    Some(TK::Readonly) => Ok(process_readonly_expr(expr)),
                    Some(TK::Clone) => Ok(E_::mk_clone(expr)),
                    Some(TK::Print) => Ok(E_::mk_call(
                        E::new(
                            (),
                            pos.clone(),
                            E_::mk_id(ast::Id(pos, special_functions::ECHO.into())),
                        ),
                        vec![],
                        vec![(ast::ParamKind::Pnormal, expr)],
                        None,
                    )),
                    Some(TK::Dollar) => {
                        raise_parsing_error(op, env, &syntax_error::invalid_variable_name);
                        Ok(E_::Omitted)
                    }
                    _ => missing_syntax("unary operator", node, env),
                }
            }
        }
        BinaryExpression(c) => {
            use ExprLocation::*;
            let rlocation = if token_kind(&c.operator) == Some(TK::Equal) {
                match location {
                    AsStatement => RightOfAssignment,
                    UsingStatement => RightOfAssignmentInUsingStatement,
                    _ => TopLevel,
                }
            } else {
                TopLevel
            };
            let bop_ast_node = p_bop(
                pos,
                &c.operator,
                p_expr(&c.left_operand, env)?,
                p_expr_with_loc(rlocation, &c.right_operand, env)?,
                env,
            )?;
            if let Some((ast::Bop::Eq(_), lhs, _)) = bop_ast_node.as_binop() {
                check_lvalue(lhs, env);
            }
            Ok(bop_ast_node)
        }
        Token(t) => {
            use ExprLocation::*;
            match (location, t.kind()) {
                (MemberSelect, TK::Variable) => mk_lvar(node, env),
                (InDoubleQuotedString, TK::HeredocStringLiteral)
                | (InDoubleQuotedString, TK::HeredocStringLiteralHead)
                | (InDoubleQuotedString, TK::HeredocStringLiteralTail) => Ok(E_::String(
                    wrap_unescaper(unescape_heredoc, text_str(node, env))?,
                )),
                (InDoubleQuotedString, _) => {
                    Ok(E_::String(wrap_unescaper(unesc_dbl, text_str(node, env))?))
                }
                (MemberSelect, _)
                | (TopLevel, _)
                | (AsStatement, _)
                | (UsingStatement, _)
                | (RightOfAssignment, _)
                | (RightOfAssignmentInUsingStatement, _)
                | (RightOfReturn, _)
                | (CallReceiver, _) => Ok(E_::mk_id(pos_name(node, env)?)),
            }
        }
        YieldExpression(c) => {
            use ExprLocation::*;
            env.saw_yield = true;
            if location != AsStatement
                && location != RightOfAssignment
                && location != RightOfAssignmentInUsingStatement
            {
                raise_parsing_error(node, env, &syntax_error::invalid_yield);
            }
            if c.operand.is_missing() {
                Ok(E_::mk_yield(ast::Afield::AFvalue(E::new(
                    (),
                    pos,
                    E_::Null,
                ))))
            } else {
                Ok(E_::mk_yield(p_afield(&c.operand, env)?))
            }
        }
        ScopeResolutionExpression(c) => {
            let qual = p_expr(&c.qualifier, env)?;
            if let E_::Id(id) = &qual.2 {
                fail_if_invalid_reified_generic(node, env, &id.1);
            }
            match &c.name.children {
                Token(token) if token.kind() == TK::Variable => {
                    let ast::Id(p, name) = pos_name(&c.name, env)?;
                    Ok(E_::mk_class_get(
                        ast::ClassId((), pos, ast::ClassId_::CIexpr(qual)),
                        ast::ClassGetExpr::CGstring((p, name)),
                        match location {
                            ExprLocation::CallReceiver => ast::PropOrMethod::IsMethod,
                            _ => ast::PropOrMethod::IsProp,
                        },
                    ))
                }
                _ => {
                    let E(_, p, expr_) = p_expr(&c.name, env)?;
                    match expr_ {
                        E_::String(id) => Ok(E_::mk_class_const(
                            ast::ClassId((), pos, ast::ClassId_::CIexpr(qual)),
                            (
                                p,
                                String::from_utf8(id.into())
                                    .map_err(|e| Error::Failwith(e.to_string()))?,
                            ),
                        )),
                        E_::Id(id) => {
                            let ast::Id(p, n) = *id;
                            Ok(E_::mk_class_const(
                                ast::ClassId((), pos, ast::ClassId_::CIexpr(qual)),
                                (p, n),
                            ))
                        }
                        E_::Lvar(id) => {
                            let ast::Lid(p, (_, n)) = *id;
                            Ok(E_::mk_class_get(
                                ast::ClassId((), pos, ast::ClassId_::CIexpr(qual)),
                                ast::ClassGetExpr::CGstring((p, n)),
                                match location {
                                    ExprLocation::CallReceiver => ast::PropOrMethod::IsMethod,
                                    _ => ast::PropOrMethod::IsProp,
                                },
                            ))
                        }
                        _ => Ok(E_::mk_class_get(
                            ast::ClassId((), pos, ast::ClassId_::CIexpr(qual)),
                            ast::ClassGetExpr::CGexpr(E((), p, expr_)),
                            match location {
                                ExprLocation::CallReceiver => ast::PropOrMethod::IsMethod,
                                _ => ast::PropOrMethod::IsProp,
                            },
                        )),
                    }
                }
            }
        }
        CastExpression(c) => Ok(E_::mk_cast(
            p_hint(&c.type_, env)?,
            p_expr(&c.operand, env)?,
        )),
        PrefixedCodeExpression(c) => {
            let src_expr = p_expr(&c.expression, env)?;

            let hint = p_hint(&c.prefix, env)?;

            let desugar_result = desugar(&hint, src_expr, env);
            for (pos, msg) in desugar_result.errors {
                raise_parsing_error_pos(&pos, env, &msg);
            }

            Ok(desugar_result.expr.2)
        }
        ConditionalExpression(c) => {
            let alter = p_expr(&c.alternative, env)?;
            let consequence = mp_optional(p_expr, &c.consequence, env)?;
            let condition = p_expr(&c.test, env)?;
            Ok(E_::mk_eif(condition, consequence, alter))
        }
        SubscriptExpression(c) => Ok(E_::mk_array_get(
            p_expr(&c.receiver, env)?,
            mp_optional(p_expr, &c.index, env)?,
        )),
        EmbeddedSubscriptExpression(c) => Ok(E_::mk_array_get(
            p_expr(&c.receiver, env)?,
            mp_optional(|n, e| p_expr_with_loc(location, n, e), &c.index, env)?,
        )),
        ShapeExpression(c) => Ok(E_::Shape(could_map(
            |n: S<'a>, e: &mut Env<'a>| mp_shape_expression_field(&p_expr, n, e),
            &c.fields,
            env,
        )?)),
        ObjectCreationExpression(c) => p_expr_impl_(location, &c.object, env, Some(pos)),
        ConstructorCall(c) => {
            let (args, varargs) = split_args_vararg(&c.argument_list, env)?;
            let (e, hl) = match &c.type_.children {
                GenericTypeSpecifier(c) => {
                    let name = pos_name(&c.class_type, env)?;
                    let hints = match &c.argument_list.children {
                        TypeArguments(c) => could_map(p_targ, &c.types, env)?,
                        _ => missing_syntax("generic type arguments", &c.argument_list, env)?,
                    };
                    (mk_id_expr(name), hints)
                }
                SimpleTypeSpecifier(_) => {
                    let name = pos_name(&c.type_, env)?;
                    (mk_id_expr(name), vec![])
                }
                _ => (p_expr(&c.type_, env)?, vec![]),
            };
            if let E_::Id(name) = &e.2 {
                fail_if_invalid_reified_generic(node, env, &name.1);
                fail_if_invalid_class_creation(node, env, &name.1);
            }
            Ok(E_::mk_new(
                ast::ClassId((), pos, ast::ClassId_::CIexpr(e)),
                hl,
                args.into_iter().map(|(_, e)| e).collect(),
                varargs,
                (),
            ))
        }
        GenericTypeSpecifier(c) => {
            if !c.argument_list.is_missing() {
                raise_parsing_error(&c.argument_list, env, &syntax_error::targs_not_allowed)
            }
            Ok(E_::mk_id(pos_name(&c.class_type, env)?))
        }
        LiteralExpression(c) => p_expr_lit(location, node, &c.expression, env),
        PrefixedStringExpression(c) => {
            /* Temporarily allow only`re`- prefixed strings */
            let name_text = text(&c.name, env);
            if name_text != "re" {
                raise_parsing_error(node, env, &syntax_error::non_re_prefix);
            }
            Ok(E_::mk_prefixed_string(name_text, p_expr(&c.str, env)?))
        }
        IsExpression(c) => Ok(E_::mk_is(
            p_expr(&c.left_operand, env)?,
            p_hint(&c.right_operand, env)?,
        )),
        AsExpression(c) => Ok(E_::mk_as(
            p_expr(&c.left_operand, env)?,
            p_hint(&c.right_operand, env)?,
            false,
        )),
        NullableAsExpression(c) => Ok(E_::mk_as(
            p_expr(&c.left_operand, env)?,
            p_hint(&c.right_operand, env)?,
            true,
        )),
        UpcastExpression(c) => Ok(E_::mk_upcast(
            p_expr(&c.left_operand, env)?,
            p_hint(&c.right_operand, env)?,
        )),
        AnonymousFunction(c) => {
            let p_arg = |n: S<'a>, e: &mut Env<'a>| match &n.children {
                Token(_) => mk_name_lid(n, e),
                _ => missing_syntax("use variable", n, e),
            };
            let ctxs = p_contexts(&c.ctx_list, env)?;
            let unsafe_ctxs = ctxs.clone();
            let p_use = |n: S<'a>, e: &mut Env<'a>| match &n.children {
                AnonymousFunctionUseClause(c) => could_map(p_arg, &c.variables, e),
                _ => Ok(vec![]),
            };
            let suspension_kind = mk_suspension_kind(&c.async_keyword);
            let (body, yield_) = {
                let mut env1 = Env::clone_and_unset_toplevel_if_toplevel(env);
                mp_yielding(&p_function_body, &c.body, env1.as_mut())?
            };
            let doc_comment = extract_docblock(node, env).or_else(|| env.top_docblock().clone());
            let user_attributes = p_user_attributes(&c.attribute_spec, env)?;
            let external = c.body.is_external();
            let params = could_map(p_fun_param, &c.parameters, env)?;
            let name_pos = p_fun_pos(node, env);
            let fun = ast::Fun_ {
                span: p_pos(node, env),
                readonly_this: None, // set in process_readonly_expr
                annotation: (),
                readonly_ret: mp_optional(p_readonly, &c.readonly_return, env)?,
                ret: ast::TypeHint((), mp_optional(p_hint, &c.type_, env)?),
                name: ast::Id(name_pos, String::from(";anonymous")),
                tparams: vec![],
                where_constraints: vec![],
                body: ast::FuncBody { fb_ast: body },
                fun_kind: mk_fun_kind(suspension_kind, yield_),
                params,
                ctxs,
                unsafe_ctxs,
                user_attributes,
                external,
                doc_comment,
            };
            let uses = p_use(&c.use_, env).unwrap_or_else(|_| vec![]);
            Ok(E_::mk_efun(fun, uses))
        }
        AwaitableCreationExpression(c) => {
            let suspension_kind = mk_suspension_kind(&c.async_);
            let (blk, yld) = mp_yielding(&p_function_body, &c.compound_statement, env)?;
            let user_attributes = p_user_attributes(&c.attribute_spec, env)?;
            let external = c.compound_statement.is_external();
            let name_pos = p_fun_pos(node, env);
            let body = ast::Fun_ {
                span: pos.clone(),
                annotation: (),
                readonly_this: None, // set in process_readonly_expr
                readonly_ret: None,  // TODO: awaitable creation expression
                ret: ast::TypeHint((), None),
                name: ast::Id(name_pos, String::from(";anonymous")),
                tparams: vec![],
                where_constraints: vec![],
                body: ast::FuncBody {
                    fb_ast: if blk.is_empty() {
                        let pos = p_pos(&c.compound_statement, env);
                        vec![ast::Stmt::noop(pos)]
                    } else {
                        blk
                    },
                },
                fun_kind: mk_fun_kind(suspension_kind, yld),
                params: vec![],
                ctxs: None,        // TODO(T70095684)
                unsafe_ctxs: None, // TODO(T70095684)
                user_attributes,
                external,
                doc_comment: None,
            };
            Ok(E_::mk_call(
                E::new((), pos, E_::mk_lfun(body, vec![])),
                vec![],
                vec![],
                None,
            ))
        }
        XHPExpression(c) if c.open.is_xhp_open() => {
            if let XHPOpen(c1) = &c.open.children {
                let name = pos_name(&c1.name, env)?;
                let attrs = could_map(p_xhp_attr, &c1.attributes, env)?;
                let exprs = aggregate_xhp_tokens(env, &c.body)?
                    .iter()
                    .map(|n| p_xhp_embedded(unesc_xhp, n, env))
                    .collect::<Result<Vec<_>, _>>()?;

                let id = if env.empty_ns_env.disable_xhp_element_mangling {
                    ast::Id(name.0, name.1)
                } else {
                    ast::Id(name.0, String::from(":") + &name.1)
                };

                Ok(E_::mk_xml(
                    // TODO: update pos_name to support prefix
                    id, attrs, exprs,
                ))
            } else {
                failwith("expect xhp open")
            }
        }
        EnumClassLabelExpression(c) => {
            /* Foo#Bar can be the following:
             * - short version: Foo is None/missing and we only have #Bar
             * - Foo is a name -> fully qualified Foo#Bar
             * - Foo is a function call prefix (can happen during auto completion)
             *   $c->foo#Bar or C::foo#Bar
             */
            let ast::Id(label_pos, label_name) = pos_name(&c.expression, env)?;
            if c.qualifier.is_missing() {
                Ok(E_::mk_enum_class_label(None, label_name))
            } else if c.qualifier.is_name() {
                let name = pos_name(&c.qualifier, env)?;
                Ok(E_::mk_enum_class_label(Some(name), label_name))
            } else if label_name.ends_with("AUTO332") {
                // This can happen during parsing in auto-complete mode
                // In such case, the "label_name" must end with AUTO332
                // We want to treat this as a method call even though we haven't yet seen the arguments
                let recv = p_expr_with_loc(ExprLocation::CallReceiver, &c.qualifier, env);
                match recv {
                    Ok(recv) => {
                        let enum_class_label = E_::mk_enum_class_label(None, label_name);
                        let enum_class_label = ast::Expr::new((), label_pos, enum_class_label);
                        Ok(E_::mk_call(
                            recv,
                            vec![],
                            vec![(ast::ParamKind::Pnormal, enum_class_label)],
                            None,
                        ))
                    }
                    Err(err) => Err(err),
                }
            } else {
                missing_syntax_(Some(E_::Null), "method call", node, env)
            }
        }
        _ => missing_syntax_(Some(E_::Null), "expression", node, env),
    }
}

fn check_lvalue<'a>(ast::Expr(_, p, expr_): &ast::Expr, env: &mut Env<'a>) {
    use aast::Expr_::*;
    let mut raise = |s| raise_parsing_error_pos(p, env, s);
    match expr_ {
        ObjGet(og) => {
            if og.as_ref().3 == ast::PropOrMethod::IsMethod {
                raise("Invalid lvalue")
            } else {
                match og.as_ref() {
                    (_, ast::Expr(_, _, Id(_)), ast::OgNullFlavor::OGNullsafe, _) => {
                        raise("?-> syntax is not supported for lvalues")
                    }
                    (_, ast::Expr(_, _, Id(sid)), _, _) if sid.1.as_bytes()[0] == b':' => {
                        raise("->: syntax is not supported for lvalues")
                    }
                    _ => {}
                }
            }
        }
        ArrayGet(ag) => {
            if let ClassConst(_) = (ag.0).2 {
                raise("Array-like class consts are not valid lvalues");
            }
        }
        List(l) => {
            for i in l.iter() {
                check_lvalue(i, env);
            }
        }
        Darray(_) | Varray(_) | Shape(_) | Collection(_) | Null | True | False | Id(_)
        | Clone(_) | ClassConst(_) | Int(_) | Float(_) | PrefixedString(_) | String(_)
        | String2(_) | Yield(_) | Await(_) | Cast(_) | Unop(_) | Binop(_) | Eif(_) | New(_)
        | Efun(_) | Lfun(_) | Xml(_) | Import(_) | Pipe(_) | Is(_) | As(_) | Call(_) => {
            raise("Invalid lvalue")
        }
        _ => {}
    }
}

fn p_xhp_embedded<'a, F>(escaper: F, node: S<'a>, env: &mut Env<'a>) -> Result<ast::Expr, Error>
where
    F: FnOnce(&[u8]) -> Vec<u8>,
{
    if let Some(kind) = token_kind(node) {
        if env.codegen() && TK::XHPStringLiteral == kind {
            let p = p_pos(node, env);
            /* for XHP string literals (attribute values) just extract
            value from quotes and decode HTML entities  */
            let text = html_entities::decode(get_quoted_content(node.full_text(env.source_text())));
            Ok(ast::Expr::new((), p, E_::make_string(text)))
        } else if env.codegen() && TK::XHPBody == kind {
            let p = p_pos(node, env);
            /* for XHP body - only decode HTML entities */
            let text = html_entities::decode(&unesc_xhp(node.full_text(env.source_text())));
            Ok(ast::Expr::new((), p, E_::make_string(text)))
        } else {
            let p = p_pos(node, env);
            let s = escaper(node.full_text(env.source_text()));
            Ok(ast::Expr::new((), p, E_::make_string(s)))
        }
    } else {
        p_expr(node, env)
    }
}

fn p_xhp_attr<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::XhpAttribute, Error> {
    match &node.children {
        XHPSimpleAttribute(c) => {
            let attr_expr = &c.expression;
            let name = p_pstring(&c.name, env)?;
            let expr = if attr_expr.is_braced_expression()
                && env.file_mode() == file_info::Mode::Mhhi
                && !env.codegen()
            {
                ast::Expr::new((), env.mk_none_pos(), E_::Null)
            } else {
                p_xhp_embedded(unesc_xhp_attr, attr_expr, env)?
            };
            let xhp_simple = ast::XhpSimple {
                name,
                type_: (),
                expr,
            };
            Ok(ast::XhpAttribute::XhpSimple(xhp_simple))
        }
        XHPSpreadAttribute(c) => {
            let expr = p_xhp_embedded(unesc_xhp, &c.expression, env)?;
            Ok(ast::XhpAttribute::XhpSpread(expr))
        }
        _ => missing_syntax("XHP attribute", node, env),
    }
}

fn aggregate_xhp_tokens<'a>(env: &mut Env<'a>, nodes: S<'a>) -> Result<Vec<S<'a>>, Error> {
    let nodes = nodes.syntax_node_to_list_skip_separator();
    let mut state = (None, None, vec![]); // (start, end, result)
    let mut combine = |state: &mut (Option<S<'a>>, Option<S<'a>>, Vec<S<'a>>)| {
        match (state.0, state.1) {
            (Some(s), None) => state.2.push(s),
            (Some(s), Some(e)) => {
                let token = env
                    .token_factory
                    .concatenate(s.get_token().unwrap(), e.get_token().unwrap());
                let node = env.arena.alloc(Syntax::make_token(token));
                state.2.push(node)
            }
            _ => {}
        }
        state.0 = None;
        state.1 = None;
        Ok(())
    };
    for n in nodes {
        match &n.children {
            Token(t) if t.kind() == TK::XHPComment => {
                if state.0.is_some() {
                    combine(&mut state)?;
                }
            }
            Token(_) => {
                if state.0.is_none() {
                    state.0 = Some(n)
                } else {
                    state.1 = Some(n)
                }
            }
            _ => {
                combine(&mut state)?;
                state.2.push(n);
            }
        }
    }
    combine(&mut state)?;
    Ok(state.2)
}

fn p_bop<'a>(
    pos: Pos,
    node: S<'a>,
    lhs: ast::Expr,
    rhs: ast::Expr,
    env: &mut Env<'a>,
) -> Result<ast::Expr_, Error> {
    use ast::Bop::*;
    let mk = |op, l, r| Ok(E_::mk_binop(op, l, r));
    let mk_eq = |op, l, r| Ok(E_::mk_binop(Eq(Some(Box::new(op))), l, r));
    match token_kind(node) {
        Some(TK::Equal) => mk(Eq(None), lhs, rhs),
        Some(TK::Bar) => mk(Bar, lhs, rhs),
        Some(TK::Ampersand) => mk(Amp, lhs, rhs),
        Some(TK::Plus) => mk(Plus, lhs, rhs),
        Some(TK::Minus) => mk(Minus, lhs, rhs),
        Some(TK::Star) => mk(Star, lhs, rhs),
        Some(TK::Carat) => mk(Xor, lhs, rhs),
        Some(TK::Slash) => mk(Slash, lhs, rhs),
        Some(TK::Dot) => mk(Dot, lhs, rhs),
        Some(TK::Percent) => mk(Percent, lhs, rhs),
        Some(TK::LessThan) => mk(Lt, lhs, rhs),
        Some(TK::GreaterThan) => mk(Gt, lhs, rhs),
        Some(TK::EqualEqual) => mk(Eqeq, lhs, rhs),
        Some(TK::LessThanEqual) => mk(Lte, lhs, rhs),
        Some(TK::GreaterThanEqual) => mk(Gte, lhs, rhs),
        Some(TK::StarStar) => mk(Starstar, lhs, rhs),
        Some(TK::ExclamationEqual) => mk(Diff, lhs, rhs),
        Some(TK::BarEqual) => mk_eq(Bar, lhs, rhs),
        Some(TK::PlusEqual) => mk_eq(Plus, lhs, rhs),
        Some(TK::MinusEqual) => mk_eq(Minus, lhs, rhs),
        Some(TK::StarEqual) => mk_eq(Star, lhs, rhs),
        Some(TK::StarStarEqual) => mk_eq(Starstar, lhs, rhs),
        Some(TK::SlashEqual) => mk_eq(Slash, lhs, rhs),
        Some(TK::DotEqual) => mk_eq(Dot, lhs, rhs),
        Some(TK::PercentEqual) => mk_eq(Percent, lhs, rhs),
        Some(TK::CaratEqual) => mk_eq(Xor, lhs, rhs),
        Some(TK::AmpersandEqual) => mk_eq(Amp, lhs, rhs),
        Some(TK::BarBar) => mk(Barbar, lhs, rhs),
        Some(TK::AmpersandAmpersand) => mk(Ampamp, lhs, rhs),
        Some(TK::LessThanLessThan) => mk(Ltlt, lhs, rhs),
        Some(TK::GreaterThanGreaterThan) => mk(Gtgt, lhs, rhs),
        Some(TK::EqualEqualEqual) => mk(Eqeqeq, lhs, rhs),
        Some(TK::LessThanLessThanEqual) => mk_eq(Ltlt, lhs, rhs),
        Some(TK::GreaterThanGreaterThanEqual) => mk_eq(Gtgt, lhs, rhs),
        Some(TK::ExclamationEqualEqual) => mk(Diff2, lhs, rhs),
        Some(TK::LessThanEqualGreaterThan) => mk(Cmp, lhs, rhs),
        Some(TK::QuestionQuestion) => mk(QuestionQuestion, lhs, rhs),
        Some(TK::QuestionQuestionEqual) => mk_eq(QuestionQuestion, lhs, rhs),
        /* The ugly duckling; In the FFP, `|>` is parsed as a
         * `BinaryOperator`, whereas the typed AST has separate constructors for
         * Pipe and Binop. This is why we don't just project onto a
         * `bop`, but a `expr -> expr -> expr_`.
         */
        Some(TK::BarGreaterThan) => {
            let lid =
                ast::Lid::from_counter(pos, env.next_local_id(), special_idents::DOLLAR_DOLLAR);
            Ok(E_::mk_pipe(lid, lhs, rhs))
        }
        Some(TK::QuestionColon) => Ok(E_::mk_eif(lhs, None, rhs)),
        _ => missing_syntax("binary operator", node, env),
    }
}

fn p_exprs_with_loc<'a>(n: S<'a>, e: &mut Env<'a>) -> Result<(Pos, Vec<ast::Expr>), Error> {
    let loc = p_pos(n, e);
    let p_expr = |n: S<'a>, e: &mut Env<'a>| -> Result<ast::Expr, Error> {
        p_expr_with_loc(ExprLocation::UsingStatement, n, e)
    };
    Ok((loc, could_map(p_expr, n, e)?))
}

fn p_stmt_list_<'a>(
    pos: &Pos,
    mut nodes: Iter<'_, S<'a>>,
    env: &mut Env<'a>,
) -> Result<Vec<ast::Stmt>, Error> {
    let mut r = vec![];
    loop {
        match nodes.next() {
            Some(n) => match &n.children {
                UsingStatementFunctionScoped(c) => {
                    let body = p_stmt_list_(pos, nodes, env)?;
                    let f = |e: &mut Env<'a>| {
                        Ok(ast::Stmt::new(
                            pos.clone(),
                            ast::Stmt_::mk_using(ast::UsingStmt {
                                is_block_scoped: false,
                                has_await: !c.await_keyword.is_missing(),
                                exprs: p_exprs_with_loc(&c.expression, e)?,
                                block: body,
                            }),
                        ))
                    };
                    let using = lift_awaits_in_statement_(f, Either::Right(pos), env)?;
                    r.push(using);
                    break Ok(r);
                }
                _ => {
                    r.push(p_stmt(n, env)?);
                }
            },
            _ => break Ok(r),
        }
    }
}

fn handle_loop_body<'a>(pos: Pos, node: S<'a>, env: &mut Env<'a>) -> Result<ast::Stmt, Error> {
    let list: Vec<_> = node.syntax_node_to_list_skip_separator().collect();
    let blk: Vec<_> = p_stmt_list_(&pos, list.iter(), env)?
        .into_iter()
        .filter(|stmt| !stmt.1.is_noop())
        .collect();
    let body = if blk.is_empty() {
        vec![mk_noop(env)]
    } else {
        blk
    };
    Ok(ast::Stmt::new(pos, ast::Stmt_::mk_block(body)))
}

fn is_simple_assignment_await_expression<'a>(node: S<'a>) -> bool {
    match &node.children {
        BinaryExpression(c) => {
            token_kind(&c.operator) == Some(TK::Equal)
                && is_simple_await_expression(&c.right_operand)
        }
        _ => false,
    }
}

fn is_simple_await_expression<'a>(node: S<'a>) -> bool {
    match &node.children {
        PrefixUnaryExpression(c) => token_kind(&c.operator) == Some(TK::Await),
        _ => false,
    }
}

fn with_new_nonconcurrent_scope<'a, F, R>(f: F, env: &mut Env<'a>) -> R
where
    F: FnOnce(&mut Env<'a>) -> R,
{
    let saved_lifted_awaits = env.lifted_awaits.take();
    let result = f(env);
    env.lifted_awaits = saved_lifted_awaits;
    result
}

fn with_new_concurrent_scope<'a, F, R>(
    f: F,
    env: &mut Env<'a>,
) -> Result<(LiftedAwaitExprs, R), Error>
where
    F: FnOnce(&mut Env<'a>) -> Result<R, Error>,
{
    let saved_lifted_awaits = env.lifted_awaits.replace(LiftedAwaits {
        awaits: vec![],
        lift_kind: LiftedAwaitKind::LiftedFromConcurrent,
    });
    let result = f(env);
    let lifted_awaits = mem::replace(&mut env.lifted_awaits, saved_lifted_awaits);
    let result = result?;
    let awaits = match lifted_awaits {
        Some(la) => process_lifted_awaits(la)?,
        None => failwith("lifted awaits should not be None")?,
    };
    Ok((awaits, result))
}

fn process_lifted_awaits(mut awaits: LiftedAwaits) -> Result<LiftedAwaitExprs, Error> {
    for await_ in awaits.awaits.iter() {
        if (await_.1).1.is_none() {
            return failwith("none pos in lifted awaits");
        }
    }
    awaits
        .awaits
        .sort_unstable_by(|a1, a2| Pos::cmp(&(a1.1).1, &(a2.1).1));
    Ok(awaits.awaits)
}

fn clear_statement_scope<'a, F, R>(f: F, env: &mut Env<'a>) -> R
where
    F: FnOnce(&mut Env<'a>) -> R,
{
    use LiftedAwaitKind::*;
    match &env.lifted_awaits {
        Some(LiftedAwaits { lift_kind, .. }) if *lift_kind == LiftedFromStatement => {
            let saved_lifted_awaits = env.lifted_awaits.take();
            let result = f(env);
            env.lifted_awaits = saved_lifted_awaits;
            result
        }
        _ => f(env),
    }
}

fn lift_awaits_in_statement<'a, F>(f: F, node: S<'a>, env: &mut Env<'a>) -> Result<ast::Stmt, Error>
where
    F: FnOnce(&mut Env<'a>) -> Result<ast::Stmt, Error>,
{
    lift_awaits_in_statement_(f, Either::Left(node), env)
}

fn lift_awaits_in_statement_<'a, F>(
    f: F,
    pos: Either<S<'a>, &Pos>,
    env: &mut Env<'a>,
) -> Result<ast::Stmt, Error>
where
    F: FnOnce(&mut Env<'a>) -> Result<ast::Stmt, Error>,
{
    use LiftedAwaitKind::*;
    let (lifted_awaits, result) = match env.lifted_awaits {
        Some(LiftedAwaits { lift_kind, .. }) if lift_kind == LiftedFromConcurrent => {
            (None, f(env)?)
        }
        _ => {
            let saved = env.lifted_awaits.replace(LiftedAwaits {
                awaits: vec![],
                lift_kind: LiftedFromStatement,
            });
            let result = f(env);
            let lifted_awaits = mem::replace(&mut env.lifted_awaits, saved);
            let result = result?;
            (lifted_awaits, result)
        }
    };
    if let Some(lifted_awaits) = lifted_awaits {
        if !lifted_awaits.awaits.is_empty() {
            let awaits = process_lifted_awaits(lifted_awaits)?;
            let pos = match pos {
                Either::Left(n) => p_pos(n, env),
                Either::Right(p) => p.clone(),
            };
            return Ok(ast::Stmt::new(
                pos,
                ast::Stmt_::mk_awaitall(awaits, vec![result]),
            ));
        }
    }
    Ok(result)
}

fn lift_await<'a>(
    parent_pos: Pos,
    expr: ast::Expr,
    env: &mut Env<'a>,
    location: ExprLocation,
) -> Result<ast::Expr_, Error> {
    use ExprLocation::*;
    match (&env.lifted_awaits, location) {
        (_, UsingStatement) | (_, RightOfAssignmentInUsingStatement) | (None, _) => {
            Ok(E_::mk_await(expr))
        }
        (Some(_), _) => {
            if location != AsStatement {
                let name = env.make_tmp_var_name();
                let lid = ast::Lid::new(parent_pos, name.clone());
                let await_lid = ast::Lid::new(expr.1.clone(), name);
                let await_ = (Some(await_lid), expr);
                if let Some(aw) = env.lifted_awaits.as_mut() {
                    aw.awaits.push(await_)
                }
                Ok(E_::mk_lvar(lid))
            } else {
                if let Some(aw) = env.lifted_awaits.as_mut() {
                    aw.awaits.push((None, expr))
                }
                Ok(E_::Null)
            }
        }
    }
}

fn p_stmt<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Stmt, Error> {
    clear_statement_scope(
        |e: &mut Env<'a>| {
            let docblock = extract_docblock(node, e);
            e.push_docblock(docblock);
            let result = p_stmt_(node, e);
            e.pop_docblock();
            result
        },
        env,
    )
}

fn p_stmt_<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Stmt, Error> {
    let pos = p_pos(node, env);
    use ast::{Stmt, Stmt_ as S_};
    let new = Stmt::new;
    match &node.children {
        SwitchStatement(c) => {
            let p_label = |n: S<'a>, e: &mut Env<'a>| -> Result<ast::GenCase, Error> {
                match &n.children {
                    CaseLabel(c) => Ok(aast::GenCase::Case(aast::Case(
                        p_expr(&c.expression, e)?,
                        vec![],
                    ))),
                    DefaultLabel(_) => Ok(aast::GenCase::Default(aast::DefaultCase(
                        p_pos(n, e),
                        vec![],
                    ))),
                    _ => missing_syntax("switch label", n, e),
                }
            };
            let p_section = |n: S<'a>, e: &mut Env<'a>| -> Result<Vec<ast::GenCase>, Error> {
                match &n.children {
                    SwitchSection(c) => {
                        let mut blk = could_map(p_stmt, &c.statements, e)?;
                        if !c.fallthrough.is_missing() {
                            blk.push(new(e.mk_none_pos(), S_::Fallthrough));
                        }
                        let mut labels = could_map(p_label, &c.labels, e)?;
                        match labels.last_mut() {
                            Some(aast::GenCase::Default(aast::DefaultCase(_, b))) => *b = blk,
                            Some(aast::GenCase::Case(aast::Case(_, b))) => *b = blk,
                            _ => raise_parsing_error(n, e, "Malformed block result"),
                        }
                        Ok(labels)
                    }
                    _ => missing_syntax("switch section", n, e),
                }
            };

            let f = |env: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                let cases = itertools::concat(could_map(p_section, &c.sections, env)?);

                let last_is_default = matches!(cases.last(), Some(aast::GenCase::Default(_)));

                let (cases, mut defaults): (Vec<ast::Case>, Vec<ast::DefaultCase>) =
                    cases.into_iter().partition_map(|case| match case {
                        aast::GenCase::Case(x @ aast::Case(..)) => Either::Left(x),
                        aast::GenCase::Default(x @ aast::DefaultCase(..)) => Either::Right(x),
                    });

                if defaults.len() > 1 {
                    let aast::DefaultCase(pos, _) = &defaults[1];
                    raise_parsing_error_pos(pos, env, &syntax_error::multiple_defaults_in_switch);
                }

                let default = match defaults.pop() {
                    Some(default @ aast::DefaultCase(..)) => {
                        if last_is_default {
                            Some(default)
                        } else {
                            let aast::DefaultCase(pos, _) = default;
                            raise_parsing_error_pos(
                                &pos,
                                env,
                                &syntax_error::default_switch_case_not_last,
                            );
                            None
                        }
                    }

                    None => None,
                };

                Ok(new(
                    pos,
                    S_::mk_switch(p_expr(&c.expression, env)?, cases, default),
                ))
            };
            lift_awaits_in_statement(f, node, env)
        }
        IfStatement(c) => {
            let p_else_if = |n: S<'a>, e: &mut Env<'a>| -> Result<(ast::Expr, ast::Block), Error> {
                match &n.children {
                    ElseifClause(c) => {
                        Ok((p_expr(&c.condition, e)?, p_block(true, &c.statement, e)?))
                    }
                    _ => missing_syntax("elseif clause", n, e),
                }
            };
            let f = |env: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                let condition = p_expr(&c.condition, env)?;
                let statement = p_block(true /* remove noop */, &c.statement, env)?;
                let else_ = match &c.else_clause.children {
                    ElseClause(c) => p_block(true, &c.statement, env)?,
                    Missing => vec![mk_noop(env)],
                    _ => missing_syntax("else clause", &c.else_clause, env)?,
                };
                let else_ifs = could_map(p_else_if, &c.elseif_clauses, env)?;
                let else_if = else_ifs
                    .into_iter()
                    .rev()
                    .fold(else_, |child, (cond, stmts)| {
                        vec![new(pos.clone(), S_::mk_if(cond, stmts, child))]
                    });
                Ok(new(pos, S_::mk_if(condition, statement, else_if)))
            };
            lift_awaits_in_statement(f, node, env)
        }
        ExpressionStatement(c) => {
            let expr = &c.expression;
            let f = |e: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                if expr.is_missing() {
                    Ok(new(pos, S_::Noop))
                } else {
                    Ok(new(
                        pos,
                        S_::mk_expr(p_expr_with_loc(ExprLocation::AsStatement, expr, e)?),
                    ))
                }
            };
            if is_simple_assignment_await_expression(expr) || is_simple_await_expression(expr) {
                f(env)
            } else {
                lift_awaits_in_statement(f, node, env)
            }
        }
        CompoundStatement(c) => handle_loop_body(pos, &c.statements, env),
        SyntaxList(_) => handle_loop_body(pos, node, env),
        ThrowStatement(c) => lift_awaits_in_statement(
            |e: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                Ok(new(pos, S_::mk_throw(p_expr(&c.expression, e)?)))
            },
            node,
            env,
        ),
        DoStatement(c) => Ok(new(
            pos,
            S_::mk_do(
                p_block(false /* remove noop */, &c.body, env)?,
                p_expr(&c.condition, env)?,
            ),
        )),
        WhileStatement(c) => Ok(new(
            pos,
            S_::mk_while(p_expr(&c.condition, env)?, p_block(true, &c.body, env)?),
        )),
        UsingStatementBlockScoped(c) => {
            let f = |e: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                Ok(new(
                    pos,
                    S_::mk_using(ast::UsingStmt {
                        is_block_scoped: true,
                        has_await: !&c.await_keyword.is_missing(),
                        exprs: p_exprs_with_loc(&c.expressions, e)?,
                        block: p_block(false, &c.body, e)?,
                    }),
                ))
            };
            lift_awaits_in_statement(f, node, env)
        }
        UsingStatementFunctionScoped(c) => {
            let f = |e: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                Ok(new(
                    pos,
                    S_::mk_using(ast::UsingStmt {
                        is_block_scoped: false,
                        has_await: !&c.await_keyword.is_missing(),
                        exprs: p_exprs_with_loc(&c.expression, e)?,
                        block: vec![mk_noop(e)],
                    }),
                ))
            };
            lift_awaits_in_statement(f, node, env)
        }
        ForStatement(c) => {
            let f = |e: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                let ini = p_expr_l(&c.initializer, e)?;
                let ctr = mp_optional(p_expr, &c.control, e)?;
                let eol = p_expr_l(&c.end_of_loop, e)?;
                let blk = p_block(true, &c.body, e)?;
                Ok(Stmt::new(pos, S_::mk_for(ini, ctr, eol, blk)))
            };
            lift_awaits_in_statement(f, node, env)
        }
        ForeachStatement(c) => {
            let f = |e: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                let col = p_expr(&c.collection, e)?;
                let akw = match token_kind(&c.await_keyword) {
                    Some(TK::Await) => Some(p_pos(&c.await_keyword, e)),
                    _ => None,
                };
                let value = p_expr(&c.value, e)?;
                let akv = match (akw, &c.key.children) {
                    (Some(p), Missing) => ast::AsExpr::AwaitAsV(p, value),
                    (None, Missing) => ast::AsExpr::AsV(value),
                    (Some(p), _) => ast::AsExpr::AwaitAsKv(p, p_expr(&c.key, e)?, value),
                    (None, _) => ast::AsExpr::AsKv(p_expr(&c.key, e)?, value),
                };
                let blk = p_block(true, &c.body, e)?;
                Ok(new(pos, S_::mk_foreach(col, akv, blk)))
            };
            lift_awaits_in_statement(f, node, env)
        }
        TryStatement(c) => Ok(new(
            pos,
            S_::mk_try(
                p_block(false, &c.compound_statement, env)?,
                could_map(
                    |n: S<'a>, e| match &n.children {
                        CatchClause(c) => Ok(ast::Catch(
                            pos_name(&c.type_, e)?,
                            lid_from_name(&c.variable, e)?,
                            p_block(true, &c.body, e)?,
                        )),
                        _ => missing_syntax("catch clause", n, e),
                    },
                    &c.catch_clauses,
                    env,
                )?,
                match &c.finally_clause.children {
                    FinallyClause(c) => p_block(false, &c.body, env)?,
                    _ => vec![],
                },
            ),
        )),
        ReturnStatement(c) => {
            let f = |e: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                let expr = match &c.expression.children {
                    Missing => None,
                    _ => Some(p_expr_with_loc(
                        ExprLocation::RightOfReturn,
                        &c.expression,
                        e,
                    )?),
                };
                Ok(ast::Stmt::new(pos, ast::Stmt_::mk_return(expr)))
            };
            if is_simple_await_expression(&c.expression) {
                f(env)
            } else {
                lift_awaits_in_statement(f, node, env)
            }
        }
        YieldBreakStatement(_) => Ok(ast::Stmt::new(pos, ast::Stmt_::mk_yield_break())),
        EchoStatement(c) => {
            let f = |e: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                let echo = match &c.keyword.children {
                    QualifiedName(_) | SimpleTypeSpecifier(_) | Token(_) => {
                        let name = pos_name(&c.keyword, e)?;
                        ast::Expr::new((), name.0.clone(), E_::mk_id(name))
                    }
                    _ => missing_syntax("id", &c.keyword, e)?,
                };
                let args = could_map(p_expr_for_normal_argument, &c.expressions, e)?;
                Ok(new(
                    pos.clone(),
                    S_::mk_expr(ast::Expr::new(
                        (),
                        pos,
                        E_::mk_call(echo, vec![], args, None),
                    )),
                ))
            };
            lift_awaits_in_statement(f, node, env)
        }
        UnsetStatement(c) => {
            let f = |e: &mut Env<'a>| -> Result<ast::Stmt, Error> {
                let args = could_map(p_expr_for_normal_argument, &c.variables, e)?;
                args.iter()
                    .for_each(|(_, arg)| check_mutate_class_const(arg, node, e));
                let unset = match &c.keyword.children {
                    QualifiedName(_) | SimpleTypeSpecifier(_) | Token(_) => {
                        let name = pos_name(&c.keyword, e)?;
                        ast::Expr::new((), name.0.clone(), E_::mk_id(name))
                    }
                    _ => missing_syntax("id", &c.keyword, e)?,
                };
                Ok(new(
                    pos.clone(),
                    S_::mk_expr(ast::Expr::new(
                        (),
                        pos,
                        E_::mk_call(unset, vec![], args, None),
                    )),
                ))
            };
            lift_awaits_in_statement(f, node, env)
        }
        BreakStatement(_) => Ok(new(pos, S_::Break)),
        ContinueStatement(_) => Ok(new(pos, S_::Continue)),
        ConcurrentStatement(c) => {
            let keyword_pos = p_pos(&c.keyword, env);
            let (lifted_awaits, Stmt(stmt_pos, stmt)) =
                with_new_concurrent_scope(|e: &mut Env<'a>| p_stmt(&c.statement, e), env)?;
            let stmt = match stmt {
                S_::Block(stmts) => {
                    use ast::Bop::Eq;
                    use ast::Expr as E;
                    /* Reuse tmp vars from lifted_awaits, this is safe because there will
                     * always be more awaits with tmp vars than statements with assignments */
                    let mut tmp_vars = lifted_awaits
                        .iter()
                        .filter_map(|lifted_await| lifted_await.0.as_ref().map(|x| &x.1));
                    let mut body_stmts = vec![];
                    let mut assign_stmts = vec![];
                    for n in stmts.into_iter() {
                        if !n.is_assign_expr() {
                            body_stmts.push(n);
                            continue;
                        }

                        if let Some(tv) = tmp_vars.next() {
                            if let Stmt(p1, S_::Expr(expr)) = n {
                                if let E(_, p2, E_::Binop(bop)) = *expr {
                                    if let (Eq(op), e1, e2) = *bop {
                                        let tmp_n = E::mk_lvar(&e2.1, &(tv.1));
                                        if tmp_n.lvar_name() != e2.lvar_name() {
                                            let new_n = new(
                                                p1.clone(),
                                                S_::mk_expr(E::new(
                                                    (),
                                                    p2.clone(),
                                                    E_::mk_binop(
                                                        Eq(None),
                                                        tmp_n.clone(),
                                                        e2.clone(),
                                                    ),
                                                )),
                                            );
                                            body_stmts.push(new_n);
                                        }
                                        let assign_stmt = new(
                                            p1,
                                            S_::mk_expr(E::new(
                                                (),
                                                p2,
                                                E_::mk_binop(Eq(op), e1, tmp_n),
                                            )),
                                        );
                                        assign_stmts.push(assign_stmt);
                                        continue;
                                    }
                                }
                            }

                            failwith("Expect assignment stmt")?;
                        } else {
                            raise_parsing_error_pos(
                                &stmt_pos,
                                env,
                                &syntax_error::statement_without_await_in_concurrent_block,
                            );
                            body_stmts.push(n)
                        }
                    }
                    body_stmts.append(&mut assign_stmts);
                    new(stmt_pos, S_::mk_block(body_stmts))
                }
                _ => failwith("Unexpected concurrent stmt structure")?,
            };
            Ok(new(keyword_pos, S_::mk_awaitall(lifted_awaits, vec![stmt])))
        }
        MarkupSection(_) => p_markup(node, env),
        _ => missing_syntax_(
            Some(new(env.mk_none_pos(), S_::Noop)),
            "statement",
            node,
            env,
        ),
    }
}
fn check_mutate_class_const<'a>(e: &ast::Expr, node: S<'a>, env: &mut Env<'a>) {
    match &e.2 {
        E_::ArrayGet(c) if c.1.is_some() => check_mutate_class_const(&c.0, node, env),
        E_::ClassConst(_) => raise_parsing_error(node, env, &syntax_error::const_mutation),
        _ => {}
    }
}

fn p_markup<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Stmt, Error> {
    match &node.children {
        MarkupSection(c) => {
            let markup_hashbang = &c.hashbang;
            let markup_suffix = &c.suffix;
            let pos = p_pos(node, env);
            let f = pos.filename();
            let expected_suffix_offset = if markup_hashbang.is_missing() {
                0
            } else {
                markup_hashbang.width() + 1 /* for newline */
            };
            if (f.has_extension("hack") || f.has_extension("hackpartial"))
                && !(markup_suffix.is_missing())
            {
                let ext = f.path().extension().unwrap(); // has_extension ensures this is a Some
                raise_parsing_error(node, env, &syntax_error::error1060(ext.to_str().unwrap()));
            } else if f.has_extension("php")
                && !markup_suffix.is_missing()
                && markup_suffix.offset() != Some(expected_suffix_offset)
            {
                raise_parsing_error(markup_suffix, env, &syntax_error::error1001);
            }
            let stmt_ = ast::Stmt_::mk_markup((pos.clone(), text(markup_hashbang, env)));
            Ok(ast::Stmt::new(pos, stmt_))
        }
        _ => failwith("invalid node"),
    }
}

fn p_modifiers<'a, F: Fn(R, modifier::Kind) -> R, R>(
    on_kind: F,
    mut init: R,
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<(modifier::KindSet, R), Error> {
    let mut kind_set = modifier::KindSet::new();
    for n in node.syntax_node_to_list_skip_separator() {
        let token_kind = token_kind(n).and_then(modifier::from_token_kind);
        match token_kind {
            Some(kind) => {
                kind_set.add(kind);
                init = on_kind(init, kind);
            }
            _ => missing_syntax("kind", n, env)?,
        }
    }
    Ok((kind_set, init))
}

fn p_kinds<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<modifier::KindSet, Error> {
    p_modifiers(|_, _| {}, (), node, env).map(|r| r.0)
}

/// Apply `f` to every item in `node`, and build a vec of the values returned.
fn could_map<'a, R, F>(f: F, node: S<'a>, env: &mut Env<'a>) -> Result<Vec<R>, Error>
where
    F: Fn(S<'a>, &mut Env<'a>) -> Result<R, Error>,
{
    let nodes = node.syntax_node_to_list_skip_separator();
    let mut res = Vec::with_capacity(nodes.size_hint().0);
    for n in nodes {
        res.push(f(n, env)?);
    }
    Ok(res)
}

fn p_visibility<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<Option<ast::Visibility>, Error> {
    let first_vis = |r: Option<ast::Visibility>, kind| r.or_else(|| modifier::to_visibility(kind));
    p_modifiers(first_vis, None, node, env).map(|r| r.1)
}

fn p_visibility_last_win<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<Option<ast::Visibility>, Error> {
    let last_vis = |r, kind| modifier::to_visibility(kind).or(r);
    p_modifiers(last_vis, None, node, env).map(|r| r.1)
}

fn p_visibility_last_win_or<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
    default: ast::Visibility,
) -> Result<ast::Visibility, Error> {
    p_visibility_last_win(node, env).map(|v| v.unwrap_or(default))
}

fn has_soft(attrs: &[ast::UserAttribute]) -> bool {
    attrs.iter().any(|attr| attr.name.1 == special_attrs::SOFT)
}

fn soften_hint(attrs: &[ast::UserAttribute], hint: ast::Hint) -> ast::Hint {
    if has_soft(attrs) {
        ast::Hint::new(hint.0.clone(), ast::Hint_::Hsoft(hint))
    } else {
        hint
    }
}

fn strip_ns(name: &str) -> &str {
    match name.chars().next() {
        Some('\\') => &name[1..],
        _ => name,
    }
}

fn has_polymorphic_context_single<'a>(env: &mut Env<'a>, hint: &ast::Hint) -> bool {
    use ast::Hint_::{Haccess, Happly, HfunContext, Hvar};
    match *hint.1 {
        HfunContext(_) => true,
        Haccess(ref root, _) => match &*root.1 {
            Happly(oxidized::ast::Id(_, id), _) => {
                let s = id.as_str();
                /* TODO(coeffects) There is an opportunity to represent this structurally
                 * in the AST if we refactor so generic hints lower as Habstr instead of
                 * Happly, like we do in the direct decl parser. */
                strip_ns(s) == naming_special_names_rust::typehints::THIS
                    || env.fn_generics_mut().contains_key(s)
                    || env.cls_generics_mut().contains_key(s)
            }
            Hvar(_) => true,
            _ => false,
        },
        _ => false,
    }
}

fn has_polymorphic_context<'a>(env: &mut Env<'a>, contexts: Option<&ast::Contexts>) -> bool {
    if let Some(ast::Contexts(_, ref context_hints)) = contexts {
        return context_hints
            .iter()
            .any(|c| has_polymorphic_context_single(env, c));
    } else {
        false
    }
}

fn has_any_policied_context(contexts: Option<&ast::Contexts>) -> bool {
    if let Some(ast::Contexts(_, ref context_hints)) = contexts {
        return context_hints.iter().any(|hint| match &*hint.1 {
            ast::Hint_::Happly(ast::Id(_, id), _) => {
                naming_special_names_rust::coeffects::is_any_zoned(id)
            }
            _ => false,
        });
    } else {
        false
    }
}

// For polymorphic context with form `ctx $f`
// require that `(function (ts)[_]: t) $f` exists
// rewrite as `(function (ts)[ctx $f]: t) $f`
// add a type parameter named "T/[ctx $f]"
fn rewrite_fun_ctx<'a>(
    env: &mut Env<'a>,
    tparams: &mut Vec<ast::Tparam>,
    hint: &mut ast::Hint,
    name: &str,
) {
    use ast::{Hint_, ReifyKind, Variance};

    let mut invalid =
        |p| raise_parsing_error_pos(p, env, &syntax_error::ctx_fun_invalid_type_hint(name));
    match *hint.1 {
        Hint_::Hfun(ref mut hf) => {
            if let Some(ast::Contexts(ref p, ref mut hl)) = &mut hf.ctxs {
                if let [ref mut h] = *hl.as_mut_slice() {
                    if let Hint_::Happly(ast::Id(ref pos, s), _) = &*h.1 {
                        if s == "_" {
                            *h.1 = Hint_::HfunContext(name.to_string());
                            tparams.push(ast::Tparam {
                                variance: Variance::Invariant,
                                name: ast::Id(h.0.clone(), format!("T/[ctx {}]", name)),
                                parameters: vec![],
                                constraints: vec![],
                                reified: ReifyKind::Erased,
                                user_attributes: vec![],
                            });
                        } else {
                            invalid(pos);
                        }
                    } else {
                        invalid(p);
                    }
                } else {
                    invalid(p);
                }
            } else {
                invalid(&hint.0);
            }
        }
        Hint_::Hlike(ref mut h) | Hint_::Hoption(ref mut h) => {
            rewrite_fun_ctx(env, tparams, h, name)
        }
        Hint_::Happly(ast::Id(_, ref type_name), ref mut targs)
            if type_name == special_typehints::SUPPORTDYN =>
        {
            if let Some(ref mut h) = targs.first_mut() {
                rewrite_fun_ctx(env, tparams, h, name)
            } else {
                invalid(&hint.0)
            }
        }
        _ => invalid(&hint.0),
    }
}
fn rewrite_effect_polymorphism<'a>(
    env: &mut Env<'a>,
    params: &mut Vec<ast::FunParam>,
    tparams: &mut Vec<ast::Tparam>,
    contexts: Option<&ast::Contexts>,
    where_constraints: &mut Vec<ast::WhereConstraintHint>,
) {
    use ast::{Hint, Hint_, ReifyKind, Variance};
    use Hint_::{Haccess, Happly, HfunContext, Hvar};

    if !has_polymorphic_context(env, contexts) {
        return;
    }
    let ast::Contexts(ref _p, ref context_hints) = contexts.as_ref().unwrap();
    let tp = |name, v| ast::Tparam {
        variance: Variance::Invariant,
        name,
        parameters: vec![],
        constraints: v,
        reified: ReifyKind::Erased,
        user_attributes: vec![],
    };

    // For polymorphic context with form `$g::C`
    // if $g's type is not a type parameter
    //   add one named "T/$g" constrained by $g's type
    //   replace $g's type hint
    // let Tg denote $g's final type (must be a type parameter).
    // add a type parameter "T/[$g::C]"
    // add a where constraint T/[$g::C] = Tg :: C
    let rewrite_arg_ctx = |
        env: &mut Env<'a>,
        tparams: &mut Vec<ast::Tparam>,
        where_constraints: &mut Vec<ast::WhereConstraintHint>,
        hint: &mut Hint,
        param_pos: &Pos,
        name: &str,
        context_pos: &Pos,
        cst: &ast::Id,
    | match *hint.1 {
        Happly(ast::Id(_, ref type_name), _) => {
            if !tparams.iter().any(|h| h.name.1 == *type_name) {
                // If the parameter is X $g, create tparam `T$g as X` and replace $g's type hint
                let id = ast::Id(param_pos.clone(), "T/".to_string() + name);
                tparams.push(tp(
                    id.clone(),
                    vec![(ast::ConstraintKind::ConstraintAs, hint.clone())],
                ));
                *hint = ast::Hint::new(param_pos.clone(), Happly(id, vec![]));
            };
            let right = ast::Hint::new(
                context_pos.clone(),
                Haccess(hint.clone(), vec![cst.clone()]),
            );
            let left_id = ast::Id(context_pos.clone(), format!("T/[{}::{}]", name, &cst.1));
            tparams.push(tp(left_id.clone(), vec![]));
            let left = ast::Hint::new(context_pos.clone(), Happly(left_id, vec![]));
            where_constraints.push(ast::WhereConstraintHint(
                left,
                ast::ConstraintKind::ConstraintEq,
                right,
            ))
        }
        _ => raise_parsing_error_pos(&hint.0, env, &syntax_error::ctx_var_invalid_type_hint(name)),
    };

    let mut hint_by_param: HashMap<&str, (&mut Option<ast::Hint>, &Pos, aast::IsVariadic)> =
        HashMap::default();
    for param in params.iter_mut() {
        hint_by_param.insert(
            param.name.as_ref(),
            (&mut param.type_hint.1, &param.pos, param.is_variadic),
        );
    }


    for context_hint in context_hints {
        match *context_hint.1 {
            HfunContext(ref name) => match hint_by_param.get_mut::<str>(name) {
                Some((hint_opt, param_pos, _is_variadic)) => match hint_opt {
                    Some(_) if env.codegen() => {}
                    Some(ref mut param_hint) => rewrite_fun_ctx(env, tparams, param_hint, name),
                    None => raise_parsing_error_pos(
                        param_pos,
                        env,
                        &syntax_error::ctx_var_missing_type_hint(name),
                    ),
                },

                None => raise_parsing_error_pos(
                    &context_hint.0,
                    env,
                    &syntax_error::ctx_var_invalid_parameter(name),
                ),
            },
            Haccess(ref root, ref csts) => {
                if let Hvar(ref name) = *root.1 {
                    match hint_by_param.get_mut::<str>(name) {
                        Some((hint_opt, param_pos, is_variadic)) => {
                            if *is_variadic {
                                raise_parsing_error_pos(
                                    param_pos,
                                    env,
                                    &syntax_error::ctx_var_variadic(name),
                                )
                            } else {
                                match hint_opt {
                                    Some(_) if env.codegen() => {}
                                    Some(ref mut param_hint) => {
                                        let mut rewrite = |h| {
                                            rewrite_arg_ctx(
                                                env,
                                                tparams,
                                                where_constraints,
                                                h,
                                                param_pos,
                                                name,
                                                &context_hint.0,
                                                &csts[0],
                                            )
                                        };
                                        match *param_hint.1 {
                                            Hint_::Hlike(ref mut h) => match *h.1 {
                                                Hint_::Hoption(ref mut hinner) => rewrite(hinner),
                                                _ => rewrite(h),
                                            },
                                            Hint_::Hoption(ref mut h) => rewrite(h),
                                            _ => rewrite(param_hint),
                                        }
                                    }
                                    None => raise_parsing_error_pos(
                                        param_pos,
                                        env,
                                        &syntax_error::ctx_var_missing_type_hint(name),
                                    ),
                                }
                            }
                        }
                        None => raise_parsing_error_pos(
                            &root.0,
                            env,
                            &syntax_error::ctx_var_invalid_parameter(name),
                        ),
                    }
                } else if let Happly(ast::Id(_, ref id), _) = *root.1 {
                    // For polymorphic context with form `T::*::C` where `T` is a reified generic
                    // add a type parameter "T/[T::*::C]"
                    // add a where constraint T/[T::*::C] = T :: C
                    let haccess_string = |id, csts: &Vec<aast::Sid>| {
                        format!("{}::{}", id, csts.iter().map(|c| c.1.clone()).join("::"))
                    };

                    match env.get_reification(id) {
                        None => {} // not a generic
                        Some(false) => raise_parsing_error_pos(
                            &root.0,
                            env,
                            &syntax_error::ctx_generic_invalid(id, haccess_string(id, csts)),
                        ),
                        Some(true) if env.codegen() => {}
                        Some(true) => {
                            let left_id = ast::Id(
                                context_hint.0.clone(),
                                format!("T/[{}]", haccess_string(id, csts)),
                            );
                            tparams.push(tp(left_id.clone(), vec![]));
                            let left =
                                ast::Hint::new(context_hint.0.clone(), Happly(left_id, vec![]));
                            where_constraints.push(ast::WhereConstraintHint(
                                left,
                                ast::ConstraintKind::ConstraintEq,
                                context_hint.clone(),
                            ));
                        }
                    }
                }
            }
            _ => {}
        }
    }
}

fn p_fun_param_default_value<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<Option<ast::Expr>, Error> {
    match &node.children {
        SimpleInitializer(c) => mp_optional(p_expr, &c.value, env),
        _ => Ok(None),
    }
}

fn p_param_kind<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::ParamKind, Error> {
    match token_kind(node) {
        Some(TK::Inout) => Ok(ast::ParamKind::Pinout(p_pos(node, env))),
        None => Ok(ast::ParamKind::Pnormal),
        _ => missing_syntax("param kind", node, env),
    }
}

fn p_readonly<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::ReadonlyKind, Error> {
    match token_kind(node) {
        Some(TK::Readonly) => Ok(ast::ReadonlyKind::Readonly),
        _ => missing_syntax("readonly", node, env),
    }
}

fn param_template<'a>(node: S<'a>, env: &Env<'_>) -> ast::FunParam {
    let pos = p_pos(node, env);
    ast::FunParam {
        annotation: (),
        type_hint: ast::TypeHint((), None),
        is_variadic: false,
        pos,
        name: text(node, env),
        expr: None,
        callconv: ast::ParamKind::Pnormal,
        readonly: None,
        user_attributes: vec![],
        visibility: None,
    }
}

fn p_fun_param<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::FunParam, Error> {
    match &node.children {
        ParameterDeclaration(ParameterDeclarationChildren {
            attribute,
            visibility,
            call_convention,
            readonly,
            type_,
            name,
            default_value,
        }) => {
            let (is_variadic, name) = match &name.children {
                DecoratedExpression(DecoratedExpressionChildren {
                    decorator,
                    expression,
                }) => {
                    let decorator = text_str(decorator, env);
                    match &expression.children {
                        DecoratedExpression(c) => {
                            let nested_expression = &c.expression;
                            let nested_decorator = text_str(&c.decorator, env);
                            (
                                decorator == "..." || nested_decorator == "...",
                                nested_expression,
                            )
                        }
                        _ => (decorator == "...", expression),
                    }
                }
                _ => (false, name),
            };
            let user_attributes = p_user_attributes(attribute, env)?;
            let pos = p_pos(name, env);
            let name = text(name, env);
            let hint = mp_optional(p_hint, type_, env)?;
            let hint = hint.map(|h| soften_hint(&user_attributes, h));

            if is_variadic && !user_attributes.is_empty() {
                raise_parsing_error(
                    node,
                    env,
                    &syntax_error::no_attributes_on_variadic_parameter,
                );
            }
            Ok(ast::FunParam {
                annotation: (),
                type_hint: ast::TypeHint((), hint),
                user_attributes,
                is_variadic,
                pos,
                name,
                expr: p_fun_param_default_value(default_value, env)?,
                callconv: p_param_kind(call_convention, env)?,
                readonly: mp_optional(p_readonly, readonly, env)?,
                /* implicit field via constructor parameter.
                 * This is always None except for constructors and the modifier
                 * can be only Public or Protected or Private.
                 */
                visibility: p_visibility(visibility, env)?,
            })
        }
        VariadicParameter(_) => {
            let mut param = param_template(node, env);
            param.is_variadic = true;
            Ok(param)
        }
        Token(_) if text_str(node, env) == "..." => {
            let mut param = param_template(node, env);
            param.is_variadic = true;
            Ok(param)
        }
        _ => missing_syntax("function parameter", node, env),
    }
}

fn p_tconstraint_ty<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Hint, Error> {
    match &node.children {
        TypeConstraint(c) => p_hint(&c.type_, env),
        _ => missing_syntax("type constraint", node, env),
    }
}

fn p_tconstraint<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<(ast::ConstraintKind, ast::Hint), Error> {
    match &node.children {
        TypeConstraint(c) => Ok((
            match token_kind(&c.keyword) {
                Some(TK::As) => ast::ConstraintKind::ConstraintAs,
                Some(TK::Super) => ast::ConstraintKind::ConstraintSuper,
                Some(TK::Equal) => ast::ConstraintKind::ConstraintEq,
                _ => missing_syntax("constraint operator", &c.keyword, env)?,
            },
            p_hint(&c.type_, env)?,
        )),
        _ => missing_syntax("type constraint", node, env),
    }
}

fn p_tparam<'a>(is_class: bool, node: S<'a>, env: &mut Env<'a>) -> Result<ast::Tparam, Error> {
    match &node.children {
        TypeParameter(TypeParameterChildren {
            attribute_spec,
            reified,
            variance,
            name,
            param_params,
            constraints,
        }) => {
            let user_attributes = p_user_attributes(attribute_spec, env)?;
            let is_reified = !reified.is_missing();

            let type_name = text(name, env);
            if is_class {
                env.cls_generics_mut().insert(type_name, is_reified);
            } else {
                // this is incorrect for type aliases, but it doesn't affect any check
                env.fn_generics_mut().insert(type_name, is_reified);
            }

            let variance = match token_kind(variance) {
                Some(TK::Plus) => ast::Variance::Covariant,
                Some(TK::Minus) => ast::Variance::Contravariant,
                _ => ast::Variance::Invariant,
            };
            if is_reified && variance != ast::Variance::Invariant {
                raise_parsing_error(node, env, &syntax_error::non_invariant_reified_generic);
            }
            let reified = match (is_reified, has_soft(&user_attributes)) {
                (true, true) => ast::ReifyKind::SoftReified,
                (true, false) => ast::ReifyKind::Reified,
                _ => ast::ReifyKind::Erased,
            };
            let parameters = p_tparam_l(is_class, param_params, env)?;
            Ok(ast::Tparam {
                variance,
                name: pos_name(name, env)?,
                parameters,
                constraints: could_map(p_tconstraint, constraints, env)?,
                reified,
                user_attributes,
            })
        }
        _ => missing_syntax("type parameter", node, env),
    }
}

fn p_tparam_l<'a>(
    is_class: bool,
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<Vec<ast::Tparam>, Error> {
    match &node.children {
        Missing => Ok(vec![]),
        TypeParameters(c) => could_map(|n, e| p_tparam(is_class, n, e), &c.parameters, env),
        _ => missing_syntax("type parameter", node, env),
    }
}

/// Lowers multiple constraints into a hint pair (lower_bound, upper_bound)
fn p_ctx_constraints<'a>(
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<(Option<ast::Hint>, Option<ast::Hint>), Error> {
    let constraints = could_map(
        |node, env| {
            if let ContextConstraint(c) = &node.children {
                if let Some(hint) = p_context_list_to_intersection(&c.ctx_list, env)? {
                    Ok(match token_kind(&c.keyword) {
                        Some(TK::Super) => Either::Left(hint),
                        Some(TK::As) => Either::Right(hint),
                        _ => missing_syntax("constraint operator", &c.keyword, env)?,
                    })
                } else {
                    missing_syntax("contexts", &c.keyword, env)?
                }
            } else {
                missing_syntax("context constraint", node, env)?
            }
        },
        node,
        env,
    )?;
    let (super_constraint, as_constraint) = constraints.into_iter().partition_map(|x| x);
    let require_one = &mut |kind: &str, cs: Vec<_>| {
        if cs.len() > 1 {
            let msg = format!(
                "Multiple `{}` constraints on a ctx constant are not allowed",
                kind
            );
            raise_parsing_error(node, env, &msg);
        }
        cs.into_iter().next()
    };
    Ok((
        require_one("super", super_constraint),
        require_one("as", as_constraint),
    ))
}

fn p_contexts<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<Option<ast::Contexts>, Error> {
    match &node.children {
        Missing => Ok(None),
        Contexts(c) => {
            let hints = could_map(&p_hint, &c.types, env)?;
            let pos = p_pos(node, env);
            let ctxs = ast::Contexts(pos, hints);
            Ok(Some(ctxs))
        }
        _ => missing_syntax("contexts", node, env),
    }
}

fn p_context_list_to_intersection<'a>(
    ctx_list: S<'a>,
    env: &mut Env<'a>,
) -> Result<Option<ast::Hint>, Error> {
    Ok(p_contexts(ctx_list, env)?.map(|t| ast::Hint::new(t.0, ast::Hint_::Hintersection(t.1))))
}

fn p_fun_hdr<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<FunHdr, Error> {
    match &node.children {
        FunctionDeclarationHeader(FunctionDeclarationHeaderChildren {
            modifiers,
            name,
            where_clause,
            type_parameter_list,
            parameter_list,
            type_,
            contexts,
            readonly_return,
            ..
        }) => {
            if name.value.is_missing() {
                raise_parsing_error(name, env, &syntax_error::empty_method_name);
            }
            let kinds = p_kinds(modifiers, env)?;
            let has_async = kinds.has(modifier::ASYNC);
            let readonly_this = if kinds.has(modifier::READONLY) {
                Some(ast::ReadonlyKind::Readonly)
            } else {
                None
            };
            let readonly_ret = mp_optional(p_readonly, readonly_return, env)?;
            let mut type_parameters = p_tparam_l(false, type_parameter_list, env)?;
            let mut parameters = could_map(p_fun_param, parameter_list, env)?;
            let contexts = p_contexts(contexts, env)?;
            let mut constrs = p_where_constraint(false, node, where_clause, env)?;
            rewrite_effect_polymorphism(
                env,
                &mut parameters,
                &mut type_parameters,
                contexts.as_ref(),
                &mut constrs,
            );
            let return_type = mp_optional(p_hint, type_, env)?;
            let suspension_kind = mk_suspension_kind_(has_async);
            let name = pos_name(name, env)?;
            let unsafe_contexts = contexts.clone();
            Ok(FunHdr {
                suspension_kind,
                readonly_this,
                name,
                constrs,
                type_parameters,
                parameters,
                contexts,
                unsafe_contexts,
                readonly_return: readonly_ret,
                return_type,
            })
        }
        LambdaSignature(LambdaSignatureChildren {
            parameters,
            contexts,
            type_,
            readonly_return,
            ..
        }) => {
            let readonly_ret = mp_optional(p_readonly, readonly_return, env)?;
            let mut header = FunHdr::make_empty(env);
            header.parameters = could_map(p_fun_param, parameters, env)?;
            let contexts = p_contexts(contexts, env)?;
            let unsafe_contexts = contexts.clone();
            header.contexts = contexts;
            header.unsafe_contexts = unsafe_contexts;
            header.return_type = mp_optional(p_hint, type_, env)?;
            header.readonly_return = readonly_ret;
            Ok(header)
        }
        Token(_) => Ok(FunHdr::make_empty(env)),
        _ => missing_syntax("function header", node, env),
    }
}

fn p_fun_pos<'a>(node: S<'a>, env: &Env<'_>) -> Pos {
    let get_pos = |n: S<'a>, p: Pos| -> Pos {
        if let FunctionDeclarationHeader(c1) = &n.children {
            if !c1.keyword.is_missing() {
                return Pos::btw_nocheck(p_pos(&c1.keyword, env), p);
            }
        }
        p
    };
    let p = p_pos(node, env);
    match &node.children {
        FunctionDeclaration(c) if env.codegen() => get_pos(&c.declaration_header, p),
        MethodishDeclaration(c) if env.codegen() => get_pos(&c.function_decl_header, p),
        _ => p,
    }
}

fn p_block<'a>(remove_noop: bool, node: S<'a>, env: &mut Env<'a>) -> Result<ast::Block, Error> {
    let ast::Stmt(p, stmt_) = p_stmt(node, env)?;
    if let ast::Stmt_::Block(blk) = stmt_ {
        if remove_noop && blk.len() == 1 && blk[0].1.is_noop() {
            return Ok(vec![]);
        }
        Ok(blk)
    } else {
        Ok(vec![ast::Stmt(p, stmt_)])
    }
}

fn mk_noop(env: &Env<'_>) -> ast::Stmt {
    ast::Stmt::noop(env.mk_none_pos())
}

fn p_function_body<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Block, Error> {
    let mk_noop_result = |e: &Env<'_>| Ok(vec![mk_noop(e)]);
    let f = |e: &mut Env<'a>| -> Result<ast::Block, Error> {
        match &node.children {
            Missing => Ok(vec![]),
            CompoundStatement(c) => {
                let compound_statements = &c.statements.children;
                let compound_right_brace = &c.right_brace.children;
                match (compound_statements, compound_right_brace) {
                    (Missing, Token(_)) => mk_noop_result(e),
                    (SyntaxList(t), _) if t.len() == 1 && t[0].is_yield() => {
                        e.saw_yield = true;
                        mk_noop_result(e)
                    }
                    _ => {
                        if !e.top_level_statements
                            && ((e.file_mode() == file_info::Mode::Mhhi && !e.codegen())
                                || e.quick_mode)
                        {
                            mk_noop_result(e)
                        } else {
                            p_block(false /*remove noop*/, node, e)
                        }
                    }
                }
            }
            _ => {
                let f = |e: &mut Env<'a>| {
                    let expr = p_expr(node, e)?;
                    Ok(ast::Stmt::new(
                        expr.1.clone(),
                        ast::Stmt_::mk_return(Some(expr)),
                    ))
                };
                if is_simple_await_expression(node) {
                    Ok(vec![f(e)?])
                } else {
                    Ok(vec![lift_awaits_in_statement(f, node, e)?])
                }
            }
        }
    };
    with_new_nonconcurrent_scope(f, env)
}

fn mk_suspension_kind<'a>(async_keyword: S<'a>) -> SuspensionKind {
    mk_suspension_kind_(!async_keyword.is_missing())
}

fn mk_suspension_kind_(has_async: bool) -> SuspensionKind {
    if has_async {
        SuspensionKind::SKAsync
    } else {
        SuspensionKind::SKSync
    }
}

fn mk_fun_kind(suspension_kind: SuspensionKind, yield_: bool) -> ast::FunKind {
    use ast::FunKind::*;
    use SuspensionKind::*;
    match (suspension_kind, yield_) {
        (SKSync, true) => FGenerator,
        (SKAsync, true) => FAsyncGenerator,
        (SKSync, false) => FSync,
        (SKAsync, false) => FAsync,
    }
}

fn process_attribute_constructor_call<'a>(
    node: S<'a>,
    constructor_call_argument_list: S<'a>,
    constructor_call_type: S<'a>,
    env: &mut Env<'a>,
) -> Result<ast::UserAttribute, Error> {
    let name = pos_name(constructor_call_type, env)?;
    if name.1.eq_ignore_ascii_case("__reified") || name.1.eq_ignore_ascii_case("__hasreifiedparent")
    {
        raise_parsing_error(node, env, &syntax_error::reified_attribute);
    } else if name.1.eq_ignore_ascii_case(special_attrs::SOFT)
        && constructor_call_argument_list
            .syntax_node_to_list_skip_separator()
            .count()
            > 0
    {
        raise_parsing_error(node, env, &syntax_error::soft_no_arguments);
    }
    let params = could_map(
        |n: S<'a>, e: &mut Env<'a>| -> Result<ast::Expr, Error> {
            is_valid_attribute_arg(n, e);
            p_expr(n, e)
        },
        constructor_call_argument_list,
        env,
    )?;
    Ok(ast::UserAttribute { name, params })
}

// Arguments to attributes must be literals (int, string, etc), collections
// (eg vec, dict, keyset, etc), Foo::class strings, shapes, string
// concatenations, or tuples.
fn is_valid_attribute_arg<'a>(node: S<'a>, env: &mut Env<'a>) {
    let mut is_valid_list = |nodes: S<'a>| {
        let _ = could_map(
            |n, e| {
                is_valid_attribute_arg(n, e);
                Ok(())
            },
            nodes,
            env,
        );
    };
    match &node.children {
        ParenthesizedExpression(c) => is_valid_attribute_arg(&c.expression, env),
        // Normal literals (string, int, etc)
        LiteralExpression(_) => {}
        // ::class strings
        ScopeResolutionExpression(c) => {
            if let Some(TK::Class) = token_kind(&c.name) {
            } else {
                raise_parsing_error(node, env, &syntax_error::expression_as_attribute_arguments);
            }
        }
        // Negations
        PrefixUnaryExpression(c) => {
            is_valid_attribute_arg(&c.operand, env);
            match token_kind(&c.operator) {
                Some(TK::Minus) => {}
                Some(TK::Plus) => {}
                _ => {
                    raise_parsing_error(node, env, &syntax_error::expression_as_attribute_arguments)
                }
            }
        }
        // String concatenation
        BinaryExpression(c) => {
            if let Some(TK::Dot) = token_kind(&c.operator) {
                is_valid_attribute_arg(&c.left_operand, env);
                is_valid_attribute_arg(&c.right_operand, env);
            } else {
                raise_parsing_error(node, env, &syntax_error::expression_as_attribute_arguments);
            }
        }
        // Top-level Collections
        DarrayIntrinsicExpression(c) => is_valid_list(&c.members),
        DictionaryIntrinsicExpression(c) => is_valid_list(&c.members),
        KeysetIntrinsicExpression(c) => is_valid_list(&c.members),
        VarrayIntrinsicExpression(c) => is_valid_list(&c.members),
        VectorIntrinsicExpression(c) => is_valid_list(&c.members),
        ShapeExpression(c) => is_valid_list(&c.fields),
        TupleExpression(c) => is_valid_list(&c.items),
        // Collection Internals
        FieldInitializer(c) => {
            is_valid_attribute_arg(&c.name, env);
            is_valid_attribute_arg(&c.value, env);
        }
        ElementInitializer(c) => {
            is_valid_attribute_arg(&c.key, env);
            is_valid_attribute_arg(&c.value, env);
        }
        // Everything else is not allowed
        _ => raise_parsing_error(node, env, &syntax_error::expression_as_attribute_arguments),
    }
}

fn p_user_attribute<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<Vec<ast::UserAttribute>, Error> {
    let p_attr = |n: S<'a>, e: &mut Env<'a>| -> Result<ast::UserAttribute, Error> {
        match &n.children {
            ConstructorCall(c) => {
                process_attribute_constructor_call(node, &c.argument_list, &c.type_, e)
            }
            _ => missing_syntax("attribute", node, e),
        }
    };
    match &node.children {
        FileAttributeSpecification(c) => could_map(p_attr, &c.attributes, env),
        OldAttributeSpecification(c) => could_map(p_attr, &c.attributes, env),
        AttributeSpecification(c) => could_map(
            |n: S<'a>, e: &mut Env<'a>| -> Result<ast::UserAttribute, Error> {
                match &n.children {
                    Attribute(c) => p_attr(&c.attribute_name, e),
                    _ => missing_syntax("attribute", node, e),
                }
            },
            &c.attributes,
            env,
        ),
        _ => missing_syntax("attribute specification", node, env),
    }
}

fn p_user_attributes<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<Vec<ast::UserAttribute>, Error> {
    let attributes = could_map(&p_user_attribute, node, env)?;
    Ok(attributes.into_iter().flatten().collect())
}

fn mp_yielding<'a, F, R>(p: F, node: S<'a>, env: &mut Env<'a>) -> Result<(R, bool), Error>
where
    F: FnOnce(S<'a>, &mut Env<'a>) -> Result<R, Error>,
{
    let outer_saw_yield = env.saw_yield;
    env.saw_yield = false;
    let r = p(node, env);
    let saw_yield = env.saw_yield;
    env.saw_yield = outer_saw_yield;
    Ok((r?, saw_yield))
}

fn mk_empty_ns_env(env: &Env<'_>) -> RcOc<NamespaceEnv> {
    RcOc::clone(&env.empty_ns_env)
}

fn extract_docblock<'a>(node: S<'a>, env: &Env<'_>) -> Option<DocComment> {
    #[derive(Copy, Clone, Eq, PartialEq)]
    enum ScanState {
        DocComment,
        EmbeddedCmt,
        EndDoc,
        EndEmbedded,
        Free,
        LineCmt,
        MaybeDoc,
        MaybeDoc2,
        SawSlash,
    }
    use ScanState::*;
    // `parse` mixes loop and recursion to use less stack space.
    fn parse(
        str: &str,
        start: usize,
        state: ScanState,
        idx: usize,
    ) -> Option<(usize, usize, String)> {
        let is_whitespace = |c| c == ' ' || c == '\t' || c == '\n' || c == '\r';
        let mut s = (start, state, idx);
        let chars = str.as_bytes();
        loop {
            if s.2 == str.len() {
                break None;
            }

            let next = s.2 + 1;
            match (s.1, chars[s.2] as char) {
                (LineCmt, '\n') => s = (next, Free, next),
                (EndEmbedded, '/') => s = (next, Free, next),
                (EndDoc, '/') => {
                    let r = parse(str, next, Free, next);
                    match r {
                        d @ Some(_) => break d,
                        None => break Some((s.0, s.2 + 1, String::from(&str[s.0..s.2 + 1]))),
                    }
                }
                /* PHP has line comments delimited by a # */
                (Free, '#') => s = (next, LineCmt, next),
                /* All other comment delimiters start with a / */
                (Free, '/') => s = (s.2, SawSlash, next),
                /* After a / in trivia, we must see either another / or a * */
                (SawSlash, '/') => s = (next, LineCmt, next),
                (SawSlash, '*') => s = (s.0, MaybeDoc, next),
                (MaybeDoc, '*') => s = (s.0, MaybeDoc2, next),
                (MaybeDoc, _) => s = (s.0, EmbeddedCmt, next),
                (MaybeDoc2, '/') => s = (next, Free, next),
                /* Doc comments have a space after the second star */
                (MaybeDoc2, c) if is_whitespace(c) => s = (s.0, DocComment, s.2),
                (MaybeDoc2, _) => s = (s.0, EmbeddedCmt, next),
                (DocComment, '*') => s = (s.0, EndDoc, next),
                (DocComment, _) => s = (s.0, DocComment, next),
                (EndDoc, _) => s = (s.0, DocComment, next),
                /* A * without a / does not end an embedded comment */
                (EmbeddedCmt, '*') => s = (s.0, EndEmbedded, next),
                (EndEmbedded, '*') => s = (s.0, EndEmbedded, next),
                (EndEmbedded, _) => s = (s.0, EmbeddedCmt, next),
                /* Whitespace skips everywhere else */
                (_, c) if is_whitespace(c) => s = (s.0, s.1, next),
                /* When scanning comments, anything else is accepted */
                (LineCmt, _) => s = (s.0, s.1, next),
                (EmbeddedCmt, _) => s = (s.0, s.1, next),
                _ => break None,
            }
        }
    }
    let str = node.leading_text(env.indexed_source_text.source_text());
    parse(str, 0, Free, 0).map(|(start, end, txt)| {
        let anchor = node.leading_start_offset();
        let pos = env
            .indexed_source_text
            .relative_pos(anchor + start, anchor + end);
        let ps = (pos, txt);
        oxidized::doc_comment::DocComment::new(ps)
    })
}

fn p_xhp_child<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::XhpChild, Error> {
    use ast::XhpChild::*;
    use ast::XhpChildOp::*;
    match &node.children {
        Token(_) => pos_name(node, env).map(ChildName),
        PostfixUnaryExpression(c) => {
            let operand = p_xhp_child(&c.operand, env)?;
            let operator = match token_kind(&c.operator) {
                Some(TK::Question) => ChildQuestion,
                Some(TK::Plus) => ChildPlus,
                Some(TK::Star) => ChildStar,
                _ => missing_syntax("xhp children operator", node, env)?,
            };
            Ok(ChildUnary(Box::new(operand), operator))
        }
        BinaryExpression(c) => {
            let left = p_xhp_child(&c.left_operand, env)?;
            let right = p_xhp_child(&c.right_operand, env)?;
            Ok(ChildBinary(Box::new(left), Box::new(right)))
        }
        XHPChildrenParenthesizedList(c) => {
            let children: Result<Vec<_>, _> = c
                .xhp_children
                .syntax_node_to_list_skip_separator()
                .map(|c| p_xhp_child(c, env))
                .collect();
            Ok(ChildList(children?))
        }
        _ => missing_syntax("xhp children", node, env),
    }
}

fn p_class_elt_<'a>(class: &mut ast::Class_, node: S<'a>, env: &mut Env<'a>) -> Result<(), Error> {
    use ast::Visibility;
    let doc_comment_opt = extract_docblock(node, env);
    let has_fun_header =
        |m: &MethodishDeclarationChildren<'_, PositionedToken<'a>, PositionedValue<'a>>| {
            matches!(
                m.function_decl_header.children,
                FunctionDeclarationHeader(_)
            )
        };
    let p_method_vis =
        |node: S<'a>, name_pos: &Pos, env: &mut Env<'a>| -> Result<Visibility, Error> {
            match p_visibility_last_win(node, env)? {
                None => {
                    raise_hh_error(env, Naming::method_needs_visibility(name_pos.clone()));
                    Ok(Visibility::Public)
                }
                Some(v) => Ok(v),
            }
        };
    match &node.children {
        ConstDeclaration(c) => {
            let user_attributes = p_user_attributes(&c.attribute_spec, env)?;
            let kinds = p_kinds(&c.modifiers, env)?;
            let has_abstract = kinds.has(modifier::ABSTRACT);
            // TODO: make wrap `type_` `doc_comment` by `Rc` in ClassConst to avoid clone
            let type_ = mp_optional(p_hint, &c.type_specifier, env)?;
            // ocaml's behavior is that if anything throw, it will
            // discard all lowered elements. So adding to class
            // must be at the last.
            let mut class_consts = could_map(
                |n: S<'a>, e: &mut Env<'a>| -> Result<ast::ClassConst, Error> {
                    match &n.children {
                        ConstantDeclarator(c) => {
                            let id = pos_name(&c.name, e)?;
                            use aast::ClassConstKind::*;
                            let kind = if has_abstract {
                                CCAbstract(mp_optional(p_simple_initializer, &c.initializer, e)?)
                            } else {
                                CCConcrete(p_simple_initializer(&c.initializer, e)?)
                            };
                            Ok(ast::ClassConst {
                                user_attributes: user_attributes.clone(),
                                type_: type_.clone(),
                                id,
                                kind,
                                doc_comment: doc_comment_opt.clone(),
                            })
                        }
                        _ => missing_syntax("constant declarator", n, e),
                    }
                },
                &c.declarators,
                env,
            )?;
            Ok(class.consts.append(&mut class_consts))
        }
        TypeConstDeclaration(c) => {
            use ast::ClassTypeconst::{TCAbstract, TCConcrete};
            if !c.type_parameters.is_missing() {
                raise_parsing_error(node, env, &syntax_error::tparams_in_tconst);
            }
            let user_attributes = p_user_attributes(&c.attribute_spec, env)?;
            let type__ = mp_optional(p_hint, &c.type_specifier, env)?
                .map(|hint| soften_hint(&user_attributes, hint));
            let kinds = p_kinds(&c.modifiers, env)?;
            let name = pos_name(&c.name, env)?;
            let as_constraint = mp_optional(p_tconstraint_ty, &c.type_constraint, env)?;
            let span = p_pos(node, env);
            let has_abstract = kinds.has(modifier::ABSTRACT);
            let kind = if has_abstract {
                TCAbstract(ast::ClassAbstractTypeconst {
                    as_constraint,
                    super_constraint: None,
                    default: type__,
                })
            } else if let Some(type_) = type__ {
                if env.is_typechecker() && as_constraint.is_some() {
                    raise_hh_error(
                        env,
                        NastCheck::partially_abstract_typeconst_definition(name.0.clone()),
                    );
                }
                TCConcrete(ast::ClassConcreteTypeconst { c_tc_type: type_ })
            } else {
                raise_hh_error(
                    env,
                    NastCheck::not_abstract_without_typeconst(name.0.clone()),
                );
                missing_syntax("value for the type constant", node, env)?
            };
            Ok(class.typeconsts.push(ast::ClassTypeconstDef {
                name,
                kind,
                user_attributes,
                span,
                doc_comment: doc_comment_opt,
                is_ctx: false,
            }))
        }
        ContextConstDeclaration(c) => {
            use ast::ClassTypeconst::{TCAbstract, TCConcrete};
            if !c.type_parameters.is_missing() {
                raise_parsing_error(node, env, &syntax_error::tparams_in_tconst);
            }
            let name = pos_name(&c.name, env)?;
            let context = p_context_list_to_intersection(&c.ctx_list, env)?;
            if let Some(ref hint) = context {
                use ast::Hint_::{Happly, Hintersection};
                let ast::Hint(_, ref h) = hint;
                if let Hintersection(hl) = &**h {
                    for h in hl {
                        if has_polymorphic_context_single(env, h) {
                            raise_parsing_error(
                                &c.constraint,
                                env,
                                "Polymorphic contexts on ctx constants are not allowed",
                            );
                        }
                        let ast::Hint(_, ref h) = h;
                        if let Happly(oxidized::ast::Id(_, id), _) = &**h {
                            if id.as_str().ends_with("_local") {
                                raise_parsing_error(
                                    &c.ctx_list,
                                    env,
                                    "Local contexts on ctx constants are not allowed",
                                );
                            }
                        }
                    }
                }
            }
            let span = p_pos(node, env);
            let kinds = p_kinds(&c.modifiers, env)?;
            let has_abstract = kinds.has(modifier::ABSTRACT);
            let (super_constraint, as_constraint) = p_ctx_constraints(&c.constraint, env)?;
            let kind = if has_abstract {
                TCAbstract(ast::ClassAbstractTypeconst {
                    as_constraint,
                    super_constraint,
                    default: context,
                })
            } else if let Some(c_tc_type) = context {
                if env.is_typechecker() && (super_constraint.is_some() || as_constraint.is_some()) {
                    raise_parsing_error(
                        node,
                        env,
                        "Constraints on a context constant requires it to be abstract",
                    )
                };
                TCConcrete(ast::ClassConcreteTypeconst { c_tc_type })
            } else {
                raise_hh_error(
                    env,
                    NastCheck::not_abstract_without_typeconst(name.0.clone()),
                );
                missing_syntax("value for the context constant", node, env)?
            };
            Ok(class.typeconsts.push(ast::ClassTypeconstDef {
                name,
                kind,
                user_attributes: vec![],
                span,
                doc_comment: doc_comment_opt,
                is_ctx: true,
            }))
        }
        PropertyDeclaration(c) => {
            let user_attributes = p_user_attributes(&c.attribute_spec, env)?;
            let type_ =
                mp_optional(p_hint, &c.type_, env)?.map(|t| soften_hint(&user_attributes, t));
            let kinds = p_kinds(&c.modifiers, env)?;
            let vis = p_visibility_last_win_or(&c.modifiers, env, Visibility::Public)?;
            let doc_comment = if env.quick_mode {
                None
            } else {
                doc_comment_opt
            };
            let name_exprs = could_map(
                |n, e| -> Result<(Pos, ast::Sid, Option<ast::Expr>), Error> {
                    match &n.children {
                        PropertyDeclarator(c) => {
                            let name = pos_name_(&c.name, e, Some('$'))?;
                            let pos = p_pos(n, e);
                            let expr = mp_optional(p_simple_initializer, &c.initializer, e)?;
                            Ok((pos, name, expr))
                        }
                        _ => missing_syntax("property declarator", n, e),
                    }
                },
                &c.declarators,
                env,
            )?;

            for (i, name_expr) in name_exprs.into_iter().enumerate() {
                class.vars.push(ast::ClassVar {
                    final_: kinds.has(modifier::FINAL),
                    xhp_attr: None,
                    abstract_: kinds.has(modifier::ABSTRACT),
                    readonly: kinds.has(modifier::READONLY),
                    visibility: vis,
                    type_: ast::TypeHint((), type_.clone()),
                    id: name_expr.1,
                    expr: name_expr.2,
                    user_attributes: user_attributes.clone(),
                    doc_comment: if i == 0 { doc_comment.clone() } else { None },
                    is_promoted_variadic: false,
                    is_static: kinds.has(modifier::STATIC),
                    span: name_expr.0,
                });
            }
            Ok(())
        }
        MethodishDeclaration(c) if has_fun_header(c) => {
            // keep cls_generics
            *env.fn_generics_mut() = HashMap::default();
            let classvar_init = |param: &ast::FunParam| -> (ast::Stmt, ast::ClassVar) {
                let cvname = drop_prefix(&param.name, '$');
                let p = &param.pos;
                let span = match &param.expr {
                    Some(ast::Expr(_, pos_end, _)) => {
                        Pos::btw(p, pos_end).unwrap_or_else(|_| p.clone())
                    }
                    _ => p.clone(),
                };
                let e = |expr_: ast::Expr_| -> ast::Expr { ast::Expr::new((), p.clone(), expr_) };
                let lid = |s: &str| -> ast::Lid { ast::Lid(p.clone(), (0, s.to_string())) };
                (
                    ast::Stmt::new(
                        p.clone(),
                        ast::Stmt_::mk_expr(e(E_::mk_binop(
                            ast::Bop::Eq(None),
                            e(E_::mk_obj_get(
                                e(E_::mk_lvar(lid(special_idents::THIS))),
                                e(E_::mk_id(ast::Id(p.clone(), cvname.to_string()))),
                                ast::OgNullFlavor::OGNullthrows,
                                ast::PropOrMethod::IsProp,
                            )),
                            e(E_::mk_lvar(lid(&param.name))),
                        ))),
                    ),
                    ast::ClassVar {
                        final_: false,
                        xhp_attr: None,
                        abstract_: false,
                        // We use the param readonlyness here to represent the
                        // ClassVar's readonlyness once lowered
                        // TODO(jjwu): Convert this to an enum when we support
                        // multiple types of readonlyness
                        readonly: param.readonly.is_some(),
                        visibility: param.visibility.unwrap(),
                        type_: param.type_hint.clone(),
                        id: ast::Id(p.clone(), cvname.to_string()),
                        expr: None,
                        user_attributes: param.user_attributes.clone(),
                        doc_comment: None,
                        is_promoted_variadic: param.is_variadic,
                        is_static: false,
                        span,
                    },
                )
            };
            let header = &c.function_decl_header;
            let h = match &header.children {
                FunctionDeclarationHeader(h) => h,
                _ => panic!(),
            };
            let hdr = p_fun_hdr(header, env)?;
            let (mut member_init, mut member_def): (Vec<ast::Stmt>, Vec<ast::ClassVar>) = hdr
                .parameters
                .iter()
                .filter_map(|p| p.visibility.map(|_| classvar_init(p)))
                .unzip();

            let kinds = p_kinds(&h.modifiers, env)?;
            let visibility = p_method_vis(&h.modifiers, &hdr.name.0, env)?;
            let is_static = kinds.has(modifier::STATIC);
            let readonly_this = kinds.has(modifier::READONLY);
            *env.in_static_method() = is_static;
            check_effect_polymorphic_reification(hdr.contexts.as_ref(), env, node);
            let (mut body, body_has_yield) = mp_yielding(p_function_body, &c.function_body, env)?;
            if env.codegen() {
                member_init.reverse();
            }
            member_init.append(&mut body);
            let body = member_init;
            *env.in_static_method() = false;
            let is_abstract = kinds.has(modifier::ABSTRACT);
            let is_external = !is_abstract && c.function_body.is_external();
            let user_attributes = p_user_attributes(&c.attribute, env)?;
            check_effect_memoized(hdr.contexts.as_ref(), &user_attributes, "method", env);
            let method = ast::Method_ {
                span: p_fun_pos(node, env),
                annotation: (),
                final_: kinds.has(modifier::FINAL),
                readonly_this,
                abstract_: is_abstract,
                static_: is_static,
                name: hdr.name,
                visibility,
                tparams: hdr.type_parameters,
                where_constraints: hdr.constrs,
                params: hdr.parameters,
                ctxs: hdr.contexts,
                unsafe_ctxs: hdr.unsafe_contexts,
                body: ast::FuncBody { fb_ast: body },
                fun_kind: mk_fun_kind(hdr.suspension_kind, body_has_yield),
                user_attributes,
                readonly_ret: hdr.readonly_return,
                ret: ast::TypeHint((), hdr.return_type),
                external: is_external,
                doc_comment: doc_comment_opt,
            };
            class.vars.append(&mut member_def);
            Ok(class.methods.push(method))
        }
        TraitUseConflictResolution(c) => {
            type Ret = Result<Either<ast::InsteadofAlias, ast::UseAsAlias>, Error>;
            let p_item = |n: S<'a>, e: &mut Env<'a>| -> Ret {
                match &n.children {
                    TraitUsePrecedenceItem(c) => {
                        let (qualifier, name) = match &c.name.children {
                            ScopeResolutionExpression(c) => {
                                (pos_name(&c.qualifier, e)?, p_pstring(&c.name, e)?)
                            }
                            _ => missing_syntax("trait use precedence item", n, e)?,
                        };
                        let removed_names = could_map(pos_name, &c.removed_names, e)?;
                        raise_hh_error(e, Naming::unsupported_instead_of(name.0.clone()));
                        Ok(Either::Left(ast::InsteadofAlias(
                            qualifier,
                            name,
                            removed_names,
                        )))
                    }
                    TraitUseAliasItem(c) => {
                        let (qualifier, name) = match &c.aliasing_name.children {
                            ScopeResolutionExpression(c) => {
                                (Some(pos_name(&c.qualifier, e)?), p_pstring(&c.name, e)?)
                            }
                            _ => (None, p_pstring(&c.aliasing_name, e)?),
                        };
                        let (kinds, mut vis_raw) = p_modifiers(
                            |mut acc, kind| -> Vec<ast::UseAsVisibility> {
                                if let Some(v) = modifier::to_use_as_visibility(kind) {
                                    acc.push(v);
                                }
                                acc
                            },
                            vec![],
                            &c.modifiers,
                            e,
                        )?;
                        let vis = if kinds.is_empty() || kinds.has_any(modifier::VISIBILITIES) {
                            vis_raw
                        } else {
                            let mut v = vec![ast::UseAsVisibility::UseAsPublic];
                            v.append(&mut vis_raw);
                            v
                        };
                        let aliased_name = if !c.aliased_name.is_missing() {
                            Some(pos_name(&c.aliased_name, e)?)
                        } else {
                            None
                        };
                        raise_hh_error(e, Naming::unsupported_trait_use_as(name.0.clone()));
                        Ok(Either::Right(ast::UseAsAlias(
                            qualifier,
                            name,
                            aliased_name,
                            vis,
                        )))
                    }
                    _ => missing_syntax("trait use conflict resolution item", n, e),
                }
            };
            let mut uses = could_map(p_hint, &c.names, env)?;
            let elts = could_map(p_item, &c.clauses, env)?;
            class.uses.append(&mut uses);
            for elt in elts.into_iter() {
                match elt {
                    Either::Left(l) => class.insteadof_alias.push(l),
                    Either::Right(r) => class.use_as_alias.push(r),
                }
            }
            Ok(())
        }
        TraitUse(c) => {
            let mut uses = could_map(p_hint, &c.names, env)?;
            Ok(class.uses.append(&mut uses))
        }
        RequireClause(c) => {
            let hint = p_hint(&c.name, env)?;
            let is_extends = match token_kind(&c.kind) {
                Some(TK::Implements) => false,
                Some(TK::Extends) => true,
                _ => missing_syntax("trait require kind", &c.kind, env)?,
            };
            Ok(class.reqs.push((hint, is_extends)))
        }
        XHPClassAttributeDeclaration(c) => {
            type Ret = Result<Either<ast::XhpAttr, ast::Hint>, Error>;
            let p_attr = |node: S<'a>, env: &mut Env<'a>| -> Ret {
                let mk_attr_use = |n: S<'a>, env: &mut Env<'a>| {
                    Ok(Either::Right(ast::Hint(
                        p_pos(n, env),
                        Box::new(ast::Hint_::Happly(pos_name(n, env)?, vec![])),
                    )))
                };
                match &node.children {
                    XHPClassAttribute(c) => {
                        let ast::Id(p, name) = pos_name(&c.name, env)?;
                        if let TypeConstant(_) = &c.type_.children {
                            if env.is_typechecker() {
                                raise_parsing_error(
                                    &c.type_,
                                    env,
                                    &syntax_error::xhp_class_attribute_type_constant,
                                )
                            }
                        }
                        let req = match &c.required.children {
                            XHPRequired(_) => Some(ast::XhpAttrTag::Required),
                            XHPLateinit(_) => Some(ast::XhpAttrTag::LateInit),
                            _ => None,
                        };
                        let pos = if c.initializer.is_missing() {
                            p.clone()
                        } else {
                            Pos::btw(&p, &p_pos(&c.initializer, env)).map_err(Error::Failwith)?
                        };
                        let (hint, like, enum_values, enum_) = match &c.type_.children {
                            XHPEnumType(c1) => {
                                let p = p_pos(&c.type_, env);
                                let like = match &c1.like.children {
                                    Missing => None,
                                    _ => Some(p_pos(&c1.like, env)),
                                };
                                let vals = could_map(p_expr, &c1.values, env)?;
                                let mut enum_vals = vec![];
                                for val in vals.clone() {
                                    match val {
                                        ast::Expr(_, _, E_::String(xev)) => enum_vals
                                            .push(ast::XhpEnumValue::XEVString(xev.to_string())),
                                        ast::Expr(_, _, E_::Int(xev)) => match xev.parse() {
                                            Ok(n) => enum_vals.push(ast::XhpEnumValue::XEVInt(n)),
                                            Err(_) =>
                                                // Since we have parse checks for
                                                // malformed integer literals already,
                                                // we assume this won't happen and ignore
                                                // the case.
                                                {}
                                        },
                                        _ => {}
                                    }
                                }
                                (None, like, enum_vals, Some((p, vals)))
                            }
                            _ => (Some(p_hint(&c.type_, env)?), None, vec![], None),
                        };
                        let init_expr = mp_optional(p_simple_initializer, &c.initializer, env)?;
                        let xhp_attr = ast::XhpAttr(
                            ast::TypeHint((), hint.clone()),
                            ast::ClassVar {
                                final_: false,
                                xhp_attr: Some(ast::XhpAttrInfo {
                                    like,
                                    tag: req,
                                    enum_values,
                                }),
                                abstract_: false,
                                readonly: false,
                                visibility: ast::Visibility::Public,
                                type_: ast::TypeHint((), hint),
                                id: ast::Id(p, String::from(":") + &name),
                                expr: init_expr,
                                user_attributes: vec![],
                                doc_comment: None,
                                is_promoted_variadic: false,
                                is_static: false,
                                span: pos,
                            },
                            req,
                            enum_,
                        );
                        Ok(Either::Left(xhp_attr))
                    }
                    XHPSimpleClassAttribute(c) => mk_attr_use(&c.type_, env),
                    Token(_) => mk_attr_use(node, env),
                    _ => missing_syntax("XHP attribute", node, env),
                }
            };
            let attrs = could_map(p_attr, &c.attributes, env)?;
            for attr in attrs.into_iter() {
                match attr {
                    Either::Left(attr) => class.xhp_attrs.push(attr),
                    Either::Right(xhp_attr_use) => class.xhp_attr_uses.push(xhp_attr_use),
                }
            }
            Ok(())
        }
        XHPChildrenDeclaration(c) => {
            let p = p_pos(node, env);
            Ok(class
                .xhp_children
                .push((p, p_xhp_child(&c.expression, env)?)))
        }
        XHPCategoryDeclaration(c) => {
            let p = p_pos(node, env);
            let categories = could_map(|n, e| p_pstring_(n, e, Some('%')), &c.categories, env)?;
            if let Some((_, cs)) = &class.xhp_category {
                if let Some(category) = cs.first() {
                    raise_hh_error(env, NastCheck::multiple_xhp_category(category.0.clone()))
                }
            }
            Ok(class.xhp_category = Some((p, categories)))
        }
        _ => missing_syntax("class element", node, env),
    }
}

fn p_class_elt<'a>(class: &mut ast::Class_, node: S<'a>, env: &mut Env<'a>) -> Result<(), Error> {
    let r = p_class_elt_(class, node, env);
    match r {
        // match ocaml behavior, don't throw if missing syntax when fail_open is true
        Err(Error::MissingSyntax { .. }) if env.fail_open() => Ok(()),
        _ => r,
    }
}

fn contains_class_body<'a>(
    c: &ClassishDeclarationChildren<'_, PositionedToken<'a>, PositionedValue<'a>>,
) -> bool {
    matches!(&c.body.children, ClassishBody(_))
}

fn p_where_constraint<'a>(
    is_class: bool,
    parent: S<'a>,
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<Vec<ast::WhereConstraintHint>, Error> {
    match &node.children {
        Missing => Ok(vec![]),
        WhereClause(c) => {
            let f = |n: S<'a>, e: &mut Env<'a>| -> Result<ast::WhereConstraintHint, Error> {
                match &n.children {
                    WhereConstraint(c) => {
                        use ast::ConstraintKind::*;
                        let l = p_hint(&c.left_type, e)?;
                        let o = &c.operator;
                        let o = match token_kind(o) {
                            Some(TK::Equal) => ConstraintEq,
                            Some(TK::As) => ConstraintAs,
                            Some(TK::Super) => ConstraintSuper,
                            _ => missing_syntax("constraint operator", o, e)?,
                        };
                        Ok(ast::WhereConstraintHint(l, o, p_hint(&c.right_type, e)?))
                    }
                    _ => missing_syntax("where constraint", n, e),
                }
            };
            c.constraints
                .syntax_node_to_list_skip_separator()
                .map(|n| f(n, env))
                .collect()
        }
        _ => {
            if is_class {
                missing_syntax("classish declaration constraints", parent, env)
            } else {
                missing_syntax("function header constraints", parent, env)
            }
        }
    }
}

fn p_namespace_use_kind<'a>(kind: S<'a>, env: &mut Env<'a>) -> Result<ast::NsKind, Error> {
    use ast::NsKind::*;
    match &kind.children {
        Missing => Ok(NSClassAndNamespace),
        _ => match token_kind(kind) {
            Some(TK::Namespace) => Ok(NSNamespace),
            Some(TK::Type) => Ok(NSClass),
            Some(TK::Function) => Ok(NSFun),
            Some(TK::Const) => Ok(NSConst),
            _ => missing_syntax("namespace use kind", kind, env),
        },
    }
}

fn p_namespace_use_clause<'a>(
    prefix: Option<S<'a>>,
    kind: Result<ast::NsKind, Error>,
    node: S<'a>,
    env: &mut Env<'a>,
) -> Result<(ast::NsKind, ast::Sid, ast::Sid), Error> {
    lazy_static! {
        static ref NAMESPACE_USE: regex::Regex = regex::Regex::new("[^\\\\]*$").unwrap();
    }

    match &node.children {
        NamespaceUseClause(NamespaceUseClauseChildren {
            clause_kind,
            alias,
            name,
            ..
        }) => {
            let ast::Id(p, n) = match (prefix, pos_name(name, env)?) {
                (None, id) => id,
                (Some(prefix), ast::Id(p, n)) => ast::Id(p, pos_name(prefix, env)?.1 + &n),
            };
            let alias = if alias.is_missing() {
                let x = NAMESPACE_USE.find(&n).unwrap().as_str();
                ast::Id(p.clone(), x.to_string())
            } else {
                pos_name(alias, env)?
            };
            let kind = if clause_kind.is_missing() {
                kind
            } else {
                p_namespace_use_kind(clause_kind, env)
            }?;
            Ok((
                kind,
                ast::Id(
                    p,
                    if !n.is_empty() && n.starts_with('\\') {
                        n
                    } else {
                        String::from("\\") + &n
                    },
                ),
                alias,
            ))
        }
        _ => missing_syntax("namespace use clause", node, env),
    }
}

fn check_effect_memoized<'a>(
    contexts: Option<&ast::Contexts>,
    user_attributes: &[aast::UserAttribute<(), ()>],
    kind: &str,
    env: &mut Env<'a>,
) {
    if has_polymorphic_context(env, contexts) {
        if let Some(u) = user_attributes
            .iter()
            .find(|u| naming_special_names_rust::user_attributes::is_memoized(&u.name.1))
        {
            raise_parsing_error_pos(
                &u.name.0,
                env,
                &syntax_error::effect_polymorphic_memoized(kind),
            )
        }
    }
    let has_policied = has_any_policied_context(contexts);
    if has_policied {
        if let Some(u) = user_attributes
            .iter()
            .find(|u| naming_special_names_rust::user_attributes::is_memoized_regular(&u.name.1))
        {
            raise_parsing_error_pos(
                &u.name.0,
                env,
                &syntax_error::effect_policied_memoized(kind),
            )
        }
    }
    if let Some(u) = user_attributes
        .iter()
        .find(|u| naming_special_names_rust::user_attributes::is_memoized_policy_sharded(&u.name.1))
    {
        if !has_policied {
            raise_parsing_error_pos(
                &u.name.0,
                env,
                &syntax_error::policy_sharded_memoized_without_policied(kind),
            )
        }
    }
}

fn check_context_has_this<'a>(contexts: Option<&ast::Contexts>, env: &mut Env<'a>) {
    use ast::Hint_::{Haccess, Happly};
    if let Some(ast::Contexts(pos, ref context_hints)) = contexts {
        context_hints.iter().for_each(|c| match *c.1 {
            Haccess(ref root, _) => match &*root.1 {
                Happly(oxidized::ast::Id(_, id), _)
                    if strip_ns(id.as_str()) == naming_special_names_rust::typehints::THIS =>
                {
                    raise_parsing_error_pos(
                        pos,
                        env,
                        "this:: context is not allowed on top level functions",
                    )
                }
                _ => {}
            },
            _ => {}
        });
    }
}

fn check_effect_polymorphic_reification<'a>(
    contexts: Option<&ast::Contexts>,
    env: &mut Env<'a>,
    node: S<'a>,
) {
    use ast::Hint_::{Haccess, Happly};
    if let Some(ast::Contexts(_, ref context_hints)) = contexts {
        context_hints.iter().for_each(|c| match *c.1 {
            Haccess(ref root, _) => match &*root.1 {
                Happly(oxidized::ast::Id(_, id), _) => {
                    fail_if_invalid_reified_generic(node, env, strip_ns(id.as_str()))
                }
                _ => {}
            },
            _ => {}
        });
    }
}

fn p_def<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<Vec<ast::Def>, Error> {
    let doc_comment_opt = extract_docblock(node, env);
    match &node.children {
        FunctionDeclaration(FunctionDeclarationChildren {
            attribute_spec,
            declaration_header,
            body,
        }) => {
            let mut env = Env::clone_and_unset_toplevel_if_toplevel(env);
            let env = env.as_mut();
            env.clear_generics();
            let hdr = p_fun_hdr(declaration_header, env)?;
            let is_external = body.is_external();
            let (block, yield_) = if is_external {
                (vec![], false)
            } else {
                mp_yielding(&p_function_body, body, env)?
            };
            let user_attributes = p_user_attributes(attribute_spec, env)?;
            check_effect_memoized(hdr.contexts.as_ref(), &user_attributes, "function", env);
            check_context_has_this(hdr.contexts.as_ref(), env);
            let ret = ast::TypeHint((), hdr.return_type);

            let fun = ast::Fun_ {
                span: p_fun_pos(node, env),
                readonly_this: hdr.readonly_this,
                annotation: (),
                ret,
                readonly_ret: hdr.readonly_return,
                name: hdr.name,
                tparams: hdr.type_parameters,
                where_constraints: hdr.constrs,
                params: hdr.parameters,
                ctxs: hdr.contexts,
                unsafe_ctxs: hdr.unsafe_contexts,
                body: ast::FuncBody { fb_ast: block },
                fun_kind: mk_fun_kind(hdr.suspension_kind, yield_),
                user_attributes,
                external: is_external,
                doc_comment: doc_comment_opt,
            };

            Ok(vec![ast::Def::mk_fun(ast::FunDef {
                namespace: mk_empty_ns_env(env),
                file_attributes: vec![],
                mode: env.file_mode(),
                fun,
            })])
        }
        ClassishDeclaration(c) if contains_class_body(c) => {
            let mut env = Env::clone_and_unset_toplevel_if_toplevel(env);
            let env = env.as_mut();
            let mode = env.file_mode();
            let user_attributes = p_user_attributes(&c.attribute, env)?;
            let kinds = p_kinds(&c.modifiers, env)?;
            let final_ = kinds.has(modifier::FINAL);
            let is_xhp = matches!(
                token_kind(&c.name),
                Some(TK::XHPElementName) | Some(TK::XHPClassName)
            );
            let has_xhp_keyword = matches!(token_kind(&c.xhp), Some(TK::XHP));
            let name = pos_name(&c.name, env)?;
            env.clear_generics();
            let tparams = p_tparam_l(true, &c.type_parameters, env)?;
            let class_kind = match token_kind(&c.keyword) {
                Some(TK::Class) if kinds.has(modifier::ABSTRACT) => {
                    ast::ClassishKind::Cclass(ast::Abstraction::Abstract)
                }
                Some(TK::Class) => ast::ClassishKind::Cclass(ast::Abstraction::Concrete),
                Some(TK::Interface) => ast::ClassishKind::Cinterface,
                Some(TK::Trait) => ast::ClassishKind::Ctrait,
                Some(TK::Enum) => ast::ClassishKind::Cenum,
                _ => missing_syntax("class kind", &c.keyword, env)?,
            };
            let extends = could_map(p_hint, &c.extends_list, env)?;
            *env.parent_maybe_reified() = match extends.first().map(|h| h.1.as_ref()) {
                Some(ast::Hint_::Happly(_, hl)) => !hl.is_empty(),
                _ => false,
            };
            let implements = could_map(p_hint, &c.implements_list, env)?;
            let where_constraints = p_where_constraint(true, node, &c.where_clause, env)?;
            let namespace = mk_empty_ns_env(env);
            let span = p_pos(node, env);
            let mut class_ = ast::Class_ {
                span,
                annotation: (),
                mode,
                final_,
                is_xhp,
                has_xhp_keyword,
                kind: class_kind,
                name,
                tparams,
                extends,
                uses: vec![],
                use_as_alias: vec![],
                insteadof_alias: vec![],
                xhp_attr_uses: vec![],
                xhp_category: None,
                reqs: vec![],
                implements,
                where_constraints,
                consts: vec![],
                typeconsts: vec![],
                vars: vec![],
                methods: vec![],
                // TODO: what is this attbiute? check ast_to_aast
                attributes: vec![],
                xhp_children: vec![],
                xhp_attrs: vec![],
                namespace,
                user_attributes,
                file_attributes: vec![],
                enum_: None,
                doc_comment: doc_comment_opt,
                emit_id: None,
            };
            match &c.body.children {
                ClassishBody(c1) => {
                    for elt in c1.elements.syntax_node_to_list_skip_separator() {
                        p_class_elt(&mut class_, elt, env)?;
                    }
                }
                _ => missing_syntax("classish body", &c.body, env)?,
            }
            Ok(vec![ast::Def::mk_class(class_)])
        }
        ConstDeclaration(c) => {
            let ty = &c.type_specifier;
            let decls = c.declarators.syntax_node_to_list_skip_separator();
            let mut defs = vec![];
            for decl in decls {
                let def = match &decl.children {
                    ConstantDeclarator(c) => {
                        let name = &c.name;
                        let init = &c.initializer;
                        let gconst = ast::Gconst {
                            annotation: (),
                            mode: env.file_mode(),
                            name: pos_name(name, env)?,
                            type_: mp_optional(p_hint, ty, env)?,
                            value: p_simple_initializer(init, env)?,
                            namespace: mk_empty_ns_env(env),
                            span: p_pos(node, env),
                            emit_id: None,
                        };
                        ast::Def::mk_constant(gconst)
                    }
                    _ => missing_syntax("constant declaration", decl, env)?,
                };
                defs.push(def);
            }
            Ok(defs)
        }
        AliasDeclaration(c) => {
            let tparams = p_tparam_l(false, &c.generic_parameter, env)?;
            for tparam in tparams.iter() {
                if tparam.reified != ast::ReifyKind::Erased {
                    raise_parsing_error(node, env, &syntax_error::invalid_reified)
                }
            }
            Ok(vec![ast::Def::mk_typedef(ast::Typedef {
                annotation: (),
                name: pos_name(&c.name, env)?,
                tparams,
                constraint: mp_optional(p_tconstraint, &c.constraint, env)?.map(|x| x.1),
                user_attributes: itertools::concat(
                    c.attribute_spec
                        .syntax_node_to_list_skip_separator()
                        .map(|attr| p_user_attribute(attr, env))
                        .collect::<Result<Vec<Vec<_>>, _>>()?,
                ),
                file_attributes: vec![],
                namespace: mk_empty_ns_env(env),
                mode: env.file_mode(),
                vis: match token_kind(&c.keyword) {
                    Some(TK::Type) => ast::TypedefVisibility::Transparent,
                    Some(TK::Newtype) => ast::TypedefVisibility::Opaque,
                    _ => missing_syntax("kind", &c.keyword, env)?,
                },
                kind: p_hint(&c.type_, env)?,
                span: p_pos(node, env),
                emit_id: None,
                is_ctx: false,
            })])
        }
        ContextAliasDeclaration(c) => {
            let (_super_constraint, as_constraint) = p_ctx_constraints(&c.as_constraint, env)?;

            let pos_name = pos_name(&c.name, env)?;
            if let Some(first_char) = pos_name.1.chars().next() {
                if first_char.is_lowercase() {
                    raise_parsing_error(
                        &c.name,
                        env,
                        &syntax_error::user_ctx_should_be_caps(&pos_name.1),
                    )
                }
            }
            if as_constraint.is_none() {
                raise_parsing_error(
                    &c.name,
                    env,
                    &syntax_error::user_ctx_require_as(&pos_name.1),
                )
            }
            let kind = match p_context_list_to_intersection(&c.context, env)? {
                Some(h) => h,
                None => {
                    let pos = pos_name.0.clone();
                    let hint_ =
                        ast::Hint_::Happly(ast::Id(pos.clone(), String::from("defaults")), vec![]);
                    ast::Hint::new(pos, hint_)
                }
            };
            Ok(vec![ast::Def::mk_typedef(ast::Typedef {
                annotation: (),
                name: pos_name,
                tparams: vec![],
                constraint: as_constraint,
                user_attributes: itertools::concat(
                    c.attribute_spec
                        .syntax_node_to_list_skip_separator()
                        .map(|attr| p_user_attribute(attr, env))
                        .collect::<Result<Vec<Vec<_>>, _>>()?,
                ),
                namespace: mk_empty_ns_env(env),
                mode: env.file_mode(),
                file_attributes: vec![],
                vis: ast::TypedefVisibility::Opaque,
                kind,
                span: p_pos(node, env),
                emit_id: None,
                is_ctx: true,
            })])
        }
        EnumDeclaration(c) => {
            let p_enumerator = |n: S<'a>, e: &mut Env<'a>| -> Result<ast::ClassConst, Error> {
                match &n.children {
                    Enumerator(c) => Ok(ast::ClassConst {
                        user_attributes: vec![],
                        type_: None,
                        id: pos_name(&c.name, e)?,
                        kind: ast::ClassConstKind::CCConcrete(p_expr(&c.value, e)?),
                        doc_comment: None,
                    }),
                    _ => missing_syntax("enumerator", n, e),
                }
            };

            let mut includes = vec![];

            let mut p_enum_use = |n: S<'a>, e: &mut Env<'a>| -> Result<(), Error> {
                match &n.children {
                    EnumUse(c) => {
                        let mut uses = could_map(p_hint, &c.names, e)?;
                        Ok(includes.append(&mut uses))
                    }
                    _ => missing_syntax("enum_use", node, e),
                }
            };

            for elt in c.use_clauses.syntax_node_to_list_skip_separator() {
                p_enum_use(elt, env)?;
            }

            Ok(vec![ast::Def::mk_class(ast::Class_ {
                annotation: (),
                mode: env.file_mode(),
                user_attributes: p_user_attributes(&c.attribute_spec, env)?,
                file_attributes: vec![],
                final_: false,
                kind: ast::ClassishKind::Cenum,
                is_xhp: false,
                has_xhp_keyword: false,
                name: pos_name(&c.name, env)?,
                tparams: vec![],
                extends: vec![],
                implements: vec![],
                where_constraints: vec![],
                consts: could_map(p_enumerator, &c.enumerators, env)?,
                namespace: mk_empty_ns_env(env),
                span: p_pos(node, env),
                enum_: Some(ast::Enum_ {
                    base: p_hint(&c.base, env)?,
                    constraint: mp_optional(p_tconstraint_ty, &c.type_, env)?,
                    includes,
                }),
                doc_comment: doc_comment_opt,
                uses: vec![],
                use_as_alias: vec![],
                insteadof_alias: vec![],
                xhp_attr_uses: vec![],
                xhp_category: None,
                reqs: vec![],
                vars: vec![],
                typeconsts: vec![],
                methods: vec![],
                attributes: vec![],
                xhp_children: vec![],
                xhp_attrs: vec![],
                emit_id: None,
            })])
        }

        EnumClassDeclaration(c) => {
            let name = pos_name(&c.name, env)?;
            // Adding __EnumClass
            let mut user_attributes = p_user_attributes(&c.attribute_spec, env)?;
            let enum_class_attribute = ast::UserAttribute {
                name: ast::Id(name.0.clone(), special_attrs::ENUM_CLASS.to_string()),
                params: vec![],
            };
            user_attributes.push(enum_class_attribute);
            // During lowering we store the base type as is. It will be updated during
            // the naming phase
            let base_type = p_hint(&c.base, env)?;

            let name_s = name.1.clone(); // TODO: can I avoid this clone ?

            let kinds = p_kinds(&c.modifiers, env)?;

            let class_kind = if kinds.has(modifier::ABSTRACT) {
                ast::ClassishKind::CenumClass(ast::Abstraction::Abstract)
            } else {
                ast::ClassishKind::CenumClass(ast::Abstraction::Concrete)
            };

            // Helper to build X -> HH\MemberOf<enum_name, X>
            let build_elt = |p: Pos, ty: ast::Hint| -> ast::Hint {
                let enum_name = ast::Id(p.clone(), name_s.clone());
                let enum_class = ast::Hint_::mk_happly(enum_name, vec![]);
                let enum_class = ast::Hint::new(p.clone(), enum_class);
                let elt_id = ast::Id(p.clone(), special_classes::MEMBER_OF.to_string());
                let full_type = ast::Hint_::mk_happly(elt_id, vec![enum_class, ty]);
                ast::Hint::new(p, full_type)
            };

            let extends = could_map(p_hint, &c.extends_list, env)?;

            let mut enum_class = ast::Class_ {
                annotation: (),
                mode: env.file_mode(),
                user_attributes,
                file_attributes: vec![],
                final_: false, // TODO(T77095784): support final EDTs
                kind: class_kind,
                is_xhp: false,
                has_xhp_keyword: false,
                name,
                tparams: vec![],
                extends: extends.clone(),
                implements: vec![],
                where_constraints: vec![],
                consts: vec![],
                namespace: mk_empty_ns_env(env),
                span: p_pos(node, env),
                enum_: Some(ast::Enum_ {
                    base: base_type,
                    constraint: None,
                    includes: extends,
                }),
                doc_comment: doc_comment_opt,
                uses: vec![],
                use_as_alias: vec![],
                insteadof_alias: vec![],
                xhp_attr_uses: vec![],
                xhp_category: None,
                reqs: vec![],
                vars: vec![],
                typeconsts: vec![],
                methods: vec![],
                attributes: vec![],
                xhp_children: vec![],
                xhp_attrs: vec![],
                emit_id: None,
            };

            for n in c.elements.syntax_node_to_list_skip_separator() {
                match &n.children {
                    // TODO(T77095784): check pos and span usage
                    EnumClassEnumerator(c) => {
                        // we turn:
                        // - type name = expression;
                        // into
                        // - const MemberOf<enum_name, type> name = expression
                        let name = pos_name(&c.name, env)?;
                        let pos = &name.0;
                        let kinds = p_kinds(&c.modifiers, env)?;
                        let has_abstract = kinds.has(modifier::ABSTRACT);
                        let elt_type = p_hint(&c.type_, env)?;
                        let full_type = build_elt(pos.clone(), elt_type);
                        let kind = if has_abstract {
                            ast::ClassConstKind::CCAbstract(None)
                        } else {
                            ast::ClassConstKind::CCConcrete(p_simple_initializer(
                                &c.initializer,
                                env,
                            )?)
                        };
                        let class_const = ast::ClassConst {
                            user_attributes: vec![],
                            type_: Some(full_type),
                            id: name,
                            kind,
                            doc_comment: None,
                        };
                        enum_class.consts.push(class_const)
                    }
                    _ => {
                        let pos = p_pos(n, env);
                        raise_parsing_error_pos(
                            &pos,
                            env,
                            &syntax_error::invalid_enum_class_enumerator,
                        )
                    }
                }
            }
            Ok(vec![ast::Def::mk_class(enum_class)])
        }
        InclusionDirective(c) if env.file_mode() != file_info::Mode::Mhhi || env.codegen() => {
            let expr = p_expr(&c.expression, env)?;
            Ok(vec![ast::Def::mk_stmt(ast::Stmt::new(
                p_pos(node, env),
                ast::Stmt_::mk_expr(expr),
            ))])
        }
        NamespaceDeclaration(c) => {
            let name = if let NamespaceDeclarationHeader(h) = &c.header.children {
                &h.name
            } else {
                return missing_syntax("namespace_declaration_header", node, env);
            };
            let defs = match &c.body.children {
                NamespaceBody(c) => {
                    let mut env1 = Env::clone_and_unset_toplevel_if_toplevel(env);
                    let env1 = env1.as_mut();
                    itertools::concat(
                        c.declarations
                            .syntax_node_to_list_skip_separator()
                            .map(|n| p_def(n, env1))
                            .collect::<Result<Vec<Vec<_>>, _>>()?,
                    )
                }
                _ => vec![],
            };
            Ok(vec![ast::Def::mk_namespace(pos_name(name, env)?, defs)])
        }
        NamespaceGroupUseDeclaration(c) => {
            let uses: Result<Vec<_>, _> = c
                .clauses
                .syntax_node_to_list_skip_separator()
                .map(|n| {
                    p_namespace_use_clause(
                        Some(&c.prefix),
                        p_namespace_use_kind(&c.kind, env),
                        n,
                        env,
                    )
                })
                .collect();
            Ok(vec![ast::Def::mk_namespace_use(uses?)])
        }
        NamespaceUseDeclaration(c) => {
            let uses: Result<Vec<_>, _> = c
                .clauses
                .syntax_node_to_list_skip_separator()
                .map(|n| p_namespace_use_clause(None, p_namespace_use_kind(&c.kind, env), n, env))
                .collect();
            Ok(vec![ast::Def::mk_namespace_use(uses?)])
        }
        FileAttributeSpecification(_) => {
            Ok(vec![ast::Def::mk_file_attributes(ast::FileAttribute {
                user_attributes: p_user_attribute(node, env)?,
                namespace: mk_empty_ns_env(env),
            })])
        }
        ModuleDeclaration(md) => Ok(vec![ast::Def::mk_module(ast::ModuleDef {
            name: pos_name(&md.name, env)?,
            user_attributes: p_user_attributes(&md.attribute_spec, env)?,
            span: p_pos(node, env),
            mode: env.file_mode(),
        })]),
        _ if env.file_mode() == file_info::Mode::Mhhi => Ok(vec![]),
        _ => Ok(vec![ast::Def::mk_stmt(p_stmt(node, env)?)]),
    }
}

fn post_process<'a>(env: &mut Env<'a>, program: Vec<ast::Def>, acc: &mut Vec<ast::Def>) {
    use aast::{Def, Def::*, Stmt_::*};
    let mut saw_ns: Option<(ast::Sid, Vec<ast::Def>)> = None;
    for def in program.into_iter() {
        if let Namespace(_) = &def {
            if let Some((n, ns_acc)) = saw_ns {
                acc.push(Def::mk_namespace(n, ns_acc));
                saw_ns = None;
            }
        }

        if let Namespace(ns) = def {
            let (n, defs) = *ns;
            if defs.is_empty() {
                saw_ns = Some((n, vec![]));
            } else {
                let mut acc_ = vec![];
                post_process(env, defs, &mut acc_);
                acc.push(Def::mk_namespace(n, acc_));
            }

            continue;
        }

        if let Stmt(s) = &def {
            if s.1.is_noop() {
                continue;
            }
            let raise_error = match &s.1 {
                Markup(_) => false,
                Expr(expr)
                    if expr.as_ref().is_import()
                        && !env.parser_options.po_disallow_toplevel_requires =>
                {
                    false
                }
                _ => {
                    use file_info::Mode::*;
                    let mode = env.file_mode();
                    env.keep_errors && env.is_typechecker() && (mode == Mstrict)
                }
            };
            if raise_error {
                raise_parsing_error_pos(&s.0, env, &syntax_error::toplevel_statements);
            }
        }

        if let Some((_, ns_acc)) = &mut saw_ns {
            ns_acc.push(def);
        } else {
            acc.push(def);
        };
    }
    if let Some((n, defs)) = saw_ns {
        acc.push(Def::mk_namespace(n, defs));
    }
}

fn p_program<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Program, Error> {
    let nodes = node.syntax_node_to_list_skip_separator();
    let mut acc = vec![];
    for n in nodes {
        match &n.children {
            EndOfFile(_) => break,
            _ => match p_def(n, env) {
                Err(Error::MissingSyntax { .. }) if env.fail_open => {}
                Err(e) => return Err(e),
                Ok(mut def) => acc.append(&mut def),
            },
        }
    }
    let mut program = vec![];
    post_process(env, acc, &mut program);
    Ok(ast::Program(program))
}

fn p_script<'a>(node: S<'a>, env: &mut Env<'a>) -> Result<ast::Program, Error> {
    match &node.children {
        Script(c) => p_program(&c.declarations, env),
        _ => missing_syntax("script", node, env),
    }
}

pub fn lower<'a>(env: &mut Env<'a>, script: S<'a>) -> Result<ast::Program, String> {
    p_script(script, env).map_err(|e| match e {
        Error::MissingSyntax {
            expecting,
            pos,
            node_name,
            kind,
        } => format!(
            "missing case in {:?}.\n - pos: {:?}\n - unexpected: '{:?}'\n - kind: {:?}\n",
            expecting, pos, node_name, kind,
        ),
        Error::Failwith(msg) => msg,
    })
}
