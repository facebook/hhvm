// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use escaper::*;
use hh_autoimport_rust as hh_autoimport;
use itertools::{Either, Either::Left, Either::Right};
use lint_rust::LintError;
use naming_special_names_rust::{
    classes as special_classes, literal, members as special_members, rx, special_functions,
    special_idents, typehints as special_typehints, user_attributes as special_attrs,
};
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
    s_map,
};
use parser_core_types::{
    indexed_source_text::IndexedSourceText, lexable_token::LexablePositionedToken,
    positioned_syntax::PositionedSyntaxTrait, positioned_syntax::PositionedValue,
    positioned_token::PositionedToken, source_text::SourceText, syntax::*, syntax_error,
    syntax_kind, syntax_trait::SyntaxTrait, token_kind::TokenKind as TK,
};
use regex::bytes::Regex;
use stack_limit::StackLimit;
use std::{
    cell::{Ref, RefCell, RefMut},
    collections::HashSet,
    mem,
    rc::Rc,
    slice::Iter,
};

use crate::lowerer_modifier as modifier;

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
    InBacktickedString,
    AsStatement,
    RightOfAssignment,
    RightOfAssignmentInUsingStatement,
    RightOfReturn,
    UsingStatement,
}

impl ExprLocation {
    fn in_string(self) -> bool {
        self == ExprLocation::InDoubleQuotedString || self == ExprLocation::InBacktickedString
    }
}

#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum SuspensionKind {
    SKSync,
    SKAsync,
    SKCoroutine,
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
    name: ast::Sid,
    constrs: Vec<ast::WhereConstraint>,
    type_parameters: Vec<ast::Tparam>,
    parameters: Vec<ast::FunParam>,
    return_type: Option<ast::Hint>,
}

impl FunHdr {
    fn make_empty(env: &Env) -> Self {
        Self {
            suspension_kind: SuspensionKind::SKSync,
            name: ast::Id(env.mk_none_pos(), String::from("<ANONYMOUS>")),
            constrs: vec![],
            type_parameters: vec![],
            parameters: vec![],
            return_type: None,
        }
    }
}

#[derive(Debug)]
pub struct State {
    pub cls_reified_generics: HashSet<String>,
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

#[derive(Debug, Clone)]
pub struct Env<'a> {
    codegen: bool,
    pub keep_errors: bool,
    quick_mode: bool,
    /// Show errors even in quick mode. Does not override keep_errors. Hotfix
    /// until we can properly set up saved states to surface parse errors during
    /// typechecking properly.
    pub show_all_errors: bool,
    fail_open: bool,
    file_mode: file_info::Mode,
    disable_lowering_parsing_error: bool,
    pub top_level_statements: bool, /* Whether we are (still) considering TLSs*/

    // Cache none pos, lazy_static doesn't allow Rc.
    pos_none: Pos,
    empty_ns_env: RcOc<NamespaceEnv>,

    pub saw_yield: bool, /* Information flowing back up */
    pub lifted_awaits: Option<LiftedAwaits>,
    pub tmp_var_counter: isize,

    pub indexed_source_text: &'a IndexedSourceText<'a>,
    pub parser_options: &'a GlobalOptions,
    pub stack_limit: Option<&'a StackLimit>,

    state: Rc<RefCell<State>>,
}

impl<'a> Env<'a> {
    pub fn make(
        codegen: bool,
        quick_mode: bool,
        keep_errors: bool,
        show_all_errors: bool,
        fail_open: bool,
        disable_lowering_parsing_error: bool,
        mode: file_info::Mode,
        indexed_source_text: &'a IndexedSourceText<'a>,
        parser_options: &'a GlobalOptions,
        use_default_namespace: bool,
        stack_limit: Option<&'a StackLimit>,
    ) -> Self {
        // (hrust) Ported from namespace_env.ml
        let empty_ns_env = if use_default_namespace {
            let mut nsenv = NamespaceEnv::default();
            nsenv.is_codegen = codegen;
            nsenv.disable_xhp_element_mangling = parser_options.po_disable_xhp_element_mangling;
            nsenv
        } else {
            let mut ns_uses = hh_autoimport::NAMESPACES_MAP.clone();
            parser_options
                .po_auto_namespace_map
                .iter()
                .for_each(|(k, v)| {
                    &ns_uses.insert(k.into(), v.into());
                });
            NamespaceEnv {
                ns_uses,
                class_uses: hh_autoimport::TYPES_MAP.clone(),
                fun_uses: hh_autoimport::FUNCS_MAP.clone(),
                const_uses: hh_autoimport::CONSTS_MAP.clone(),
                record_def_uses: s_map::SMap::new(),
                name: None,
                auto_ns_map: parser_options.po_auto_namespace_map.clone(),
                is_codegen: codegen,
                disable_xhp_element_mangling: parser_options.po_disable_xhp_element_mangling,
            }
        };

        Env {
            codegen,
            keep_errors,
            quick_mode,
            show_all_errors,
            fail_open,
            disable_lowering_parsing_error,
            file_mode: mode,
            top_level_statements: true,
            saw_yield: false,
            lifted_awaits: None,
            tmp_var_counter: 1,
            indexed_source_text,
            parser_options,
            pos_none: Pos::make_none(),
            empty_ns_env: RcOc::new(empty_ns_env),
            stack_limit,

            state: Rc::new(RefCell::new(State {
                cls_reified_generics: HashSet::new(),
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

    fn cls_reified_generics(&mut self) -> RefMut<HashSet<String>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.cls_reified_generics)
    }

    fn in_static_method(&mut self) -> RefMut<bool> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.in_static_method)
    }

    fn parent_maybe_reified(&mut self) -> RefMut<bool> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.parent_maybe_reified)
    }

    pub fn lowpri_errors(&mut self) -> RefMut<Vec<(Pos, String)>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.lowpri_errors)
    }

    pub fn hh_errors(&mut self) -> RefMut<Vec<HHError>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.hh_errors)
    }

    pub fn lint_errors(&mut self) -> RefMut<Vec<LintError>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.lint_errors)
    }

    fn top_docblock(&self) -> Ref<Option<DocComment>> {
        Ref::map(self.state.borrow(), |s| {
            s.doc_comments.last().unwrap_or(&None)
        })
    }

    fn exp_recursion_depth(&self) -> RefMut<usize> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.exp_recursion_depth)
    }

    fn next_local_id(&self) -> isize {
        let mut id = RefMut::map(self.state.borrow_mut(), |s| &mut s.local_id_counter);
        *id = *id + 1;
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
        self.stack_limit
            .as_ref()
            .map(|limit| limit.panic_if_exceeded());
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

type Result<T> = std::result::Result<T, Error>;

use parser_core_types::syntax::SyntaxVariant::*;

trait Lowerer<'a, T, V>
where
    T: LexablePositionedToken<'a>,
    Syntax<T, V>: PositionedSyntaxTrait,
    V: SyntaxValueWithKind + SyntaxValueType<T> + std::fmt::Debug,
{
    fn mode_annotation(mode: file_info::Mode) -> file_info::Mode {
        match mode {
            file_info::Mode::Mphp => file_info::Mode::Mdecl,
            m => m,
        }
    }

    // Turns a syntax node into a list of nodes; if it is a separated syntax
    // list then the separators are filtered from the resulting list.
    fn syntax_to_list(include_separators: bool, node: &Syntax<T, V>) -> Vec<&Syntax<T, V>> {
        fn on_list_item<T1, V1>(sep: bool, x: &ListItemChildren<T1, V1>) -> Vec<&Syntax<T1, V1>> {
            if sep {
                vec![&x.list_item, &x.list_separator]
            } else {
                vec![&x.list_item]
            }
        }
        match &node.syntax {
            Missing => vec![],
            SyntaxList(s) => s
                .iter()
                .map(|x| match &x.syntax {
                    ListItem(x) => on_list_item(include_separators, x),
                    _ => vec![node],
                })
                .flatten()
                .collect(),
            ListItem(x) => on_list_item(include_separators, x),
            _ => vec![node],
        }
    }

    fn p_pos(node: &Syntax<T, V>, env: &Env) -> Pos {
        node.position_exclusive(env.indexed_source_text)
            .unwrap_or_else(|| env.mk_none_pos())
    }

    fn raise_parsing_error(node: &Syntax<T, V>, env: &mut Env, msg: &str) {
        Self::raise_parsing_error_(Either::Left(node), env, msg)
    }

    fn raise_parsing_error_pos(pos: &Pos, env: &mut Env, msg: &str) {
        Self::raise_parsing_error_(Either::Right(pos), env, msg)
    }

    fn raise_parsing_error_(node_or_pos: Either<&Syntax<T, V>, &Pos>, env: &mut Env, msg: &str) {
        if env.should_surface_error() {
            let pos = node_or_pos.either(|node| Self::p_pos(node, env), |pos| pos.clone());
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

    fn raise_hh_error(env: &mut Env, err: HHError) {
        env.hh_errors().push(err);
    }

    fn raise_lint_error(env: &mut Env, err: LintError) {
        env.lint_errors().push(err);
    }

    #[inline]
    fn failwith<N>(msg: impl Into<String>) -> Result<N> {
        Err(Error::Failwith(msg.into()))
    }

    #[inline]
    fn text(node: &Syntax<T, V>, env: &Env) -> String {
        String::from(node.text(env.source_text()))
    }

    #[inline]
    fn text_str<'b>(node: &'b Syntax<T, V>, env: &'b Env) -> &'b str {
        node.text(env.source_text())
    }

    fn lowering_error(env: &mut Env, pos: &Pos, text: &str, syntax_kind: &str) {
        if env.is_typechecker()
            && !env.disable_lowering_parsing_error
            && env.lowpri_errors().is_empty()
        {
            Self::raise_parsing_error_pos(
                pos,
                env,
                &syntax_error::lowering_parsing_error(text, syntax_kind),
            )
        }
    }

    fn missing_syntax_<N>(
        fallback: Option<N>,
        expecting: &str,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<N> {
        let pos = Self::p_pos(node, env);
        let text = Self::text(node, env);
        Self::lowering_error(env, &pos, &text, expecting);
        if let Some(x) = fallback {
            if env.fail_open() {
                return Ok(x);
            }
        }
        Err(Error::MissingSyntax {
            expecting: String::from(expecting),
            pos: Self::p_pos(node, env),
            node_name: text,
            kind: node.kind(),
        })
    }

    fn missing_syntax<N>(expecting: &str, node: &Syntax<T, V>, env: &mut Env) -> Result<N> {
        Self::missing_syntax_(None, expecting, node, env)
    }

    fn is_num_octal_lit(s: &str) -> bool {
        !s.chars().any(|c| c == '8' || c == '9')
    }

    fn mp_optional<F, R>(p: F, node: &Syntax<T, V>, env: &mut Env) -> Result<Option<R>>
    where
        F: FnOnce(&Syntax<T, V>, &mut Env) -> Result<R>,
    {
        match &node.syntax {
            Missing => Ok(None),
            _ => p(node, env).map(Some),
        }
    }

    fn pos_qualified_name(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Sid> {
        if let QualifiedName(c) = &node.syntax {
            if let SyntaxList(l) = &c.qualified_name_parts.syntax {
                let p = Self::p_pos(node, env);
                let mut s = String::with_capacity(node.width());
                for i in l.iter() {
                    match &i.syntax {
                        ListItem(li) => {
                            s += li.list_item.text(env.source_text());
                            s += li.list_separator.text(env.source_text());
                        }
                        _ => s += i.text(env.source_text()),
                    }
                }
                return Ok(ast::Id(p, s));
            }
        }
        Self::missing_syntax("qualified name", node, env)
    }

    #[inline]
    fn pos_name(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Sid> {
        Self::pos_name_(node, env, None)
    }

    fn lid_from_pos_name(pos: Pos, name: &Syntax<T, V>, env: &mut Env) -> Result<ast::Lid> {
        let name = Self::pos_name(name, env)?;
        Ok(ast::Lid::new(pos, name.1))
    }

    fn lid_from_name(name: &Syntax<T, V>, env: &mut Env) -> Result<ast::Lid> {
        let name = Self::pos_name(name, env)?;
        Ok(ast::Lid::new(name.0, name.1))
    }

    // TODO: after porting unify Sid and Pstring
    #[inline]
    fn p_pstring(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Pstring> {
        Self::p_pstring_(node, env, None)
    }

    #[inline]
    fn p_pstring_(
        node: &Syntax<T, V>,
        env: &mut Env,
        drop_prefix: Option<char>,
    ) -> Result<ast::Pstring> {
        let ast::Id(p, id) = Self::pos_name_(node, env, drop_prefix)?;
        Ok((p, id))
    }

    #[inline]
    fn drop_prefix(s: &str, prefix: char) -> &str {
        if s.len() > 0 && s.chars().nth(0) == Some(prefix) {
            &s[1..]
        } else {
            s
        }
    }

    fn pos_name_(
        node: &Syntax<T, V>,
        env: &mut Env,
        drop_prefix: Option<char>,
    ) -> Result<ast::Sid> {
        match &node.syntax {
            QualifiedName(_) => Self::pos_qualified_name(node, env),
            SimpleTypeSpecifier(c) => Self::pos_name_(&c.simple_type_specifier, env, drop_prefix),
            _ => {
                let mut name = node.text(env.indexed_source_text.source_text());
                if let Some(prefix) = drop_prefix {
                    name = Self::drop_prefix(name, prefix);
                }
                let p = Self::p_pos(node, env);
                Ok(ast::Id(p, String::from(name)))
            }
        }
    }

    fn mk_str<F>(node: &Syntax<T, V>, env: &mut Env, unescaper: F, mut content: &str) -> String
    where
        F: Fn(&str) -> std::result::Result<String, InvalidString>,
    {
        if let Some('b') = content.chars().nth(0) {
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
                        Self::raise_parsing_error(
                            node,
                            env,
                            &format!("Malformed string literal <<{}>>", &no_quotes),
                        );
                        String::from("")
                    }
                }
            }
            Err(_) => {
                Self::raise_parsing_error(
                    node,
                    env,
                    &format!("Malformed string literal <<{}>>", &content),
                );
                String::from("")
            }
        }
    }

    fn unesc_dbl(s: &str) -> std::result::Result<String, InvalidString> {
        let unesc_s = unescape_double(s)?;
        match unesc_s.as_str() {
            "''" | "\"\"" => Ok(String::from("")),
            _ => Ok(unesc_s),
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
        // TODO: change unesc_dbl to &[u8] -> Vec<u8>
        let r = Self::get_quoted_content(s);
        let r = unsafe { std::str::from_utf8_unchecked(r) };
        Self::unesc_dbl(r).unwrap().into_bytes()
    }

    fn get_quoted_content(s: &[u8]) -> &[u8] {
        lazy_static! {
            static ref QUOTED: Regex = Regex::new(r#"^[\x20\t\n\r\x0c]*"((?:.|\n)*)""#).unwrap();
        }
        QUOTED
            .captures(s)
            .map_or(None, |c| c.get(1))
            .map_or(s, |m| m.as_bytes())
    }

    fn as_list(node: &Syntax<T, V>) -> Vec<&Syntax<T, V>> {
        fn strip_list_item<T1, V1>(node: &Syntax<T1, V1>) -> &Syntax<T1, V1> {
            match &node.syntax {
                ListItem(i) => &i.list_item,
                _ => node,
            }
        }
        match &node.syntax {
            SyntaxList(l) => l.iter().map(strip_list_item).collect(),
            Missing => vec![],
            _ => vec![node],
        }
    }

    fn token_kind(node: &Syntax<T, V>) -> Option<TK> {
        match &node.syntax {
            Token(t) => Some(t.kind()),
            _ => None,
        }
    }

    fn check_valid_reified_hint(env: &mut Env, node: &Syntax<T, V>, hint: &ast::Hint) {
        struct Checker<F: FnMut(&String)>(F);
        impl<F: FnMut(&String)> Visitor for Checker<F> {
            type P = AstParams<(), ()>;

            fn object(&mut self) -> &mut dyn Visitor<P = Self::P> {
                self
            }

            fn visit_hint(&mut self, c: &mut (), h: &ast::Hint) -> std::result::Result<(), ()> {
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
                Self::fail_if_invalid_reified_generic(node, env, id);
            };
            let mut visitor = Checker(f);
            visitor.visit_hint(&mut (), hint).unwrap();
        }
    }

    fn p_closure_parameter(
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<(ast::Hint, Option<ast::ParamKind>)> {
        match &node.syntax {
            ClosureParameterTypeSpecifier(c) => {
                let kind = Self::mp_optional(
                    Self::p_param_kind,
                    &c.closure_parameter_call_convention,
                    env,
                )?;
                let hint = Self::p_hint(&c.closure_parameter_type, env)?;
                Ok((hint, kind))
            }
            _ => Self::missing_syntax("closure parameter", node, env),
        }
    }

    fn mp_shape_expression_field<F, R>(
        f: F,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<(ast::ShapeFieldName, R)>
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> Result<R>,
    {
        match &node.syntax {
            FieldInitializer(c) => {
                let name = Self::p_shape_field_name(&c.field_initializer_name, env)?;
                let value = f(&c.field_initializer_value, env)?;
                Ok((name, value))
            }
            _ => Self::missing_syntax("shape field", node, env),
        }
    }

    fn p_shape_field_name(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::ShapeFieldName> {
        use ast::ShapeFieldName::*;
        let is_valid_shape_literal = |t: &T| {
            let is_str = t.kind() == TK::SingleQuotedStringLiteral
                || t.kind() == TK::DoubleQuotedStringLiteral;
            let text = t.text(env.source_text());
            let is_empty = text == "\'\'" || text == "\"\"";
            is_str && !is_empty
        };
        if let LiteralExpression(c) = &node.syntax {
            if let Token(t) = &c.literal_expression.syntax {
                if is_valid_shape_literal(t) {
                    let ast::Id(p, n) = Self::pos_name(node, env)?;
                    let str_ = Self::mk_str(node, env, Self::unesc_dbl, &n);
                    if let Some(_) = ocaml_helper::int_of_string_opt(&str_.as_bytes()) {
                        Self::raise_parsing_error(
                            node,
                            env,
                            &syntax_error::shape_field_int_like_string,
                        )
                    }
                    return Ok(SFlitStr((p, str_)));
                }
            }
        }
        match &node.syntax {
            ScopeResolutionExpression(c) => Ok(SFclassConst(
                Self::pos_name(&c.scope_resolution_qualifier, env)?,
                Self::p_pstring(&c.scope_resolution_name, env)?,
            )),
            _ => {
                Self::raise_parsing_error(node, env, &syntax_error::invalid_shape_field_name);
                let ast::Id(p, n) = Self::pos_name(node, env)?;
                Ok(SFlitStr((p, Self::mk_str(node, env, Self::unesc_dbl, &n))))
            }
        }
    }

    fn p_shape_field(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::ShapeFieldInfo> {
        match &node.syntax {
            FieldSpecifier(c) => {
                let optional = !c.field_question.is_missing();
                let name = Self::p_shape_field_name(&c.field_name, env)?;
                let hint = Self::p_hint(&c.field_type, env)?;
                Ok(ast::ShapeFieldInfo {
                    optional,
                    hint,
                    name,
                })
            }
            _ => {
                let (name, hint) = Self::mp_shape_expression_field(Self::p_hint, node, env)?;
                Ok(ast::ShapeFieldInfo {
                    optional: false,
                    name,
                    hint,
                })
            }
        }
    }

    fn p_targ(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Targ> {
        Ok(ast::Targ((), Self::p_hint(node, env)?))
    }

    fn p_hint_(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Hint_> {
        use ast::Hint_::*;
        let unary = |kw, ty, env: &mut Env| {
            Ok(Happly(
                Self::pos_name(kw, env)?,
                Self::could_map(Self::p_hint, ty, env)?,
            ))
        };
        let binary = |kw, key, ty, env: &mut Env| {
            let kw = Self::pos_name(kw, env)?;
            let key = Self::p_hint(key, env)?;
            Ok(Happly(
                kw,
                Self::map_flatten_(&Self::p_hint, ty, env, vec![key])?,
            ))
        };
        match &node.syntax {
            /* Dirty hack; CastExpression can have type represented by token */
            Token(_) | SimpleTypeSpecifier(_) | QualifiedName(_) => {
                let ast::Id(pos, name) = Self::pos_name(node, env)?;
                let mut suggest = |name: &str, canonical: &str| {
                    Self::raise_parsing_error(
                        node,
                        env,
                        &syntax_error::invalid_typehint_alias(name, canonical),
                    );
                };
                if "integer".eq_ignore_ascii_case(&name) {
                    suggest(&name, special_typehints::INT);
                } else if "boolean".eq_ignore_ascii_case(&name) {
                    suggest(&name, special_typehints::BOOL);
                } else if "double".eq_ignore_ascii_case(&name) {
                    suggest(&name, special_typehints::FLOAT);
                } else if "real".eq_ignore_ascii_case(&name) {
                    suggest(&name, special_typehints::FLOAT);
                }
                Ok(Happly(ast::Id(pos, name), vec![]))
            }
            ShapeTypeSpecifier(c) => {
                let allows_unknown_fields = !c.shape_type_ellipsis.is_missing();
                /* if last element lacks a separator and ellipsis is present, error */
                if let Some(l) = Self::syntax_to_list(true, &c.shape_type_fields).last() {
                    if l.is_missing() && allows_unknown_fields {
                        Self::raise_parsing_error(
                            node,
                            env,
                            &syntax_error::shape_type_ellipsis_without_trailing_comma,
                        )
                    }
                }

                let field_map = Self::could_map(Self::p_shape_field, &c.shape_type_fields, env)?;
                // TODO:(shiqicao) improve perf
                // 1. replace HashSet by fnv hash map or something faster,
                // 2. move `set` to Env to avoid allocation,
                let mut set: HashSet<&str> = HashSet::with_capacity(field_map.len());
                for f in field_map.iter() {
                    if !set.insert(f.name.get_name()) {
                        Self::raise_hh_error(
                            env,
                            Naming::fd_name_already_bound(f.name.get_pos().clone()),
                        );
                    }
                }

                Ok(Hshape(ast::NastShapeInfo {
                    allows_unknown_fields,
                    field_map,
                }))
            }
            TupleTypeSpecifier(c) => {
                Ok(Htuple(Self::could_map(Self::p_hint, &c.tuple_types, env)?))
            }
            UnionTypeSpecifier(c) => {
                Ok(Hunion(Self::could_map(&Self::p_hint, &c.union_types, env)?))
            }
            IntersectionTypeSpecifier(c) => Ok(Hintersection(Self::could_map(
                &Self::p_hint,
                &c.intersection_types,
                env,
            )?)),
            KeysetTypeSpecifier(c) => Ok(Happly(
                Self::pos_name(&c.keyset_type_keyword, env)?,
                Self::could_map(Self::p_hint, &c.keyset_type_type, env)?,
            )),
            VectorTypeSpecifier(c) => unary(&c.vector_type_keyword, &c.vector_type_type, env),
            ClassnameTypeSpecifier(c) => unary(&c.classname_keyword, &c.classname_type, env),
            TupleTypeExplicitSpecifier(c) => unary(&c.tuple_type_keyword, &c.tuple_type_types, env),
            VarrayTypeSpecifier(c) => unary(&c.varray_keyword, &c.varray_type, env),
            VectorArrayTypeSpecifier(c) => {
                unary(&c.vector_array_keyword, &c.vector_array_type, env)
            }
            DarrayTypeSpecifier(c) => {
                binary(&c.darray_keyword, &c.darray_key, &c.darray_value, env)
            }
            MapArrayTypeSpecifier(c) => binary(
                &c.map_array_keyword,
                &c.map_array_key,
                &c.map_array_value,
                env,
            ),
            DictionaryTypeSpecifier(c) => {
                unary(&c.dictionary_type_keyword, &c.dictionary_type_members, env)
            }
            GenericTypeSpecifier(c) => {
                let name = Self::pos_name(&c.generic_class_type, env)?;
                let args = &c.generic_argument_list;
                let type_args = match &args.syntax {
                    TypeArguments(c) => {
                        Self::could_map(Self::p_hint, &c.type_arguments_types, env)?
                    }
                    _ => Self::missing_syntax("generic type arguments", args, env)?,
                };
                if env.codegen() {
                    let process = |name: ast::Sid, mut args: Vec<ast::Hint>| -> ast::Hint_ {
                        if args.len() == 1 {
                            let eq = |s| name.1.eq_ignore_ascii_case(s);
                            if (args[0].1.is_hfun()
                                && (eq(rx::RX) || eq(rx::RX_LOCAL) || eq(rx::RX_SHALLOW)))
                                || (args[0].1.is_happly()
                                    && (eq(rx::MUTABLE)
                                        || eq(rx::MAYBE_MUTABLE)
                                        || eq(rx::OWNED_MUTABLE)))
                            {
                                return *args.pop().unwrap().1;
                            }
                        }
                        Happly(name, args)
                    };
                    Ok(process(name, type_args))
                } else {
                    Ok(Happly(name, type_args))
                }
            }
            NullableTypeSpecifier(c) => Ok(Hoption(Self::p_hint(&c.nullable_type, env)?)),
            LikeTypeSpecifier(c) => Ok(Hlike(Self::p_hint(&c.like_type, env)?)),
            SoftTypeSpecifier(c) => Ok(Hsoft(Self::p_hint(&c.soft_type, env)?)),
            ClosureTypeSpecifier(c) => {
                let (param_list, variadic_hints): (Vec<&Syntax<T, V>>, Vec<&Syntax<T, V>>) =
                    Self::as_list(&c.closure_parameter_list)
                        .iter()
                        .partition(|n| match &n.syntax {
                            VariadicParameter(_) => false,
                            _ => true,
                        });
                let (type_hints, kinds) = param_list
                    .iter()
                    .map(|p| Self::p_closure_parameter(p, env))
                    .collect::<std::result::Result<Vec<_>, _>>()?
                    .into_iter()
                    .unzip();
                let variadic_hints = variadic_hints
                    .iter()
                    .map(|v| match &v.syntax {
                        VariadicParameter(c) => {
                            let vtype = &c.variadic_parameter_type;
                            if vtype.is_missing() {
                                Self::raise_parsing_error(
                                    v,
                                    env,
                                    "Cannot use ... without a typehint",
                                );
                            }
                            Ok(Some(Self::p_hint(vtype, env)?))
                        }
                        _ => panic!("expect variadic parameter"),
                    })
                    .collect::<std::result::Result<Vec<_>, _>>()?;
                if variadic_hints.len() > 1 {
                    return Self::failwith(format!(
                        "{} variadic parameters found. There should be no more than one.",
                        variadic_hints.len().to_string()
                    ));
                }
                Ok(Hfun(ast::HintFun {
                    reactive_kind: ast::FuncReactive::FNonreactive,
                    is_coroutine: !c.closure_coroutine.is_missing(),
                    param_tys: type_hints,
                    param_kinds: kinds,
                    param_mutability: vec![],
                    variadic_ty: variadic_hints.into_iter().next().unwrap_or(None),
                    return_ty: Self::p_hint(&c.closure_return_type, env)?,
                    is_mutable_return: true,
                }))
            }
            AttributizedSpecifier(c) => {
                let attrs = Self::p_user_attribute(&c.attributized_specifier_attribute_spec, env)?;
                let hint = Self::p_hint(&c.attributized_specifier_type, env)?;
                if attrs.iter().any(|attr| attr.name.1 != special_attrs::SOFT) {
                    Self::raise_parsing_error(node, env, &syntax_error::only_soft_allowed);
                }
                Ok(*Self::soften_hint(&attrs, hint).1)
            }
            TypeConstant(c) => {
                let child = Self::pos_name(&c.type_constant_right_type, env)?;
                match Self::p_hint_(&c.type_constant_left_type, env)? {
                    Haccess(root, mut cs) => {
                        cs.push(child);
                        Ok(Haccess(root, cs))
                    }
                    Happly(ty, param) => {
                        if param.is_empty() {
                            let root = ast::Hint::new(ty.0.clone(), Happly(ty, param));
                            Ok(Haccess(root, vec![child]))
                        } else {
                            Self::missing_syntax("type constant base", node, env)
                        }
                    }
                    _ => Self::missing_syntax("type constant base", node, env),
                }
            }
            PUAccess(c) => {
                let pos = Self::p_pos(&c.pu_access_left_type, env);
                let child = Self::pos_name(&c.pu_access_right_type, env)?;
                match Self::p_hint_(&c.pu_access_left_type, env)? {
                    h @ HpuAccess(_, _, _) => Ok(HpuAccess(
                        ast::Hint::new(pos, h),
                        child,
                        ast::PuLoc::Unknown,
                    )),
                    Happly(id, hints) => {
                        if hints.is_empty() {
                            Ok(HpuAccess(
                                ast::Hint::new(pos, Happly(id, hints)),
                                child,
                                ast::PuLoc::Unknown,
                            ))
                        } else {
                            Self::missing_syntax("pocket universe access base", node, env)
                        }
                    }
                    _ => Self::missing_syntax("pocket universe access base", node, env),
                }
            }
            ReifiedTypeArgument(_) => {
                Self::raise_parsing_error(node, env, &syntax_error::invalid_reified);
                Self::missing_syntax("refied type", node, env)
            }
            _ => Self::missing_syntax("type hint", node, env),
        }
    }

    fn p_hint(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Hint> {
        let hint_ = Self::p_hint_(node, env)?;
        let pos = Self::p_pos(node, env);
        let hint = ast::Hint::new(pos, hint_);
        Self::check_valid_reified_hint(env, node, &hint);
        Ok(hint)
    }

    fn p_simple_initializer(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Expr> {
        match &node.syntax {
            SimpleInitializer(c) => Self::p_expr(&c.simple_initializer_value, env),
            _ => Self::missing_syntax("simple initializer", node, env),
        }
    }

    fn p_member(node: &Syntax<T, V>, env: &mut Env) -> Result<(ast::Expr, ast::Expr)> {
        match &node.syntax {
            ElementInitializer(c) => Ok((
                Self::p_expr(&c.element_key, env)?,
                Self::p_expr(&c.element_value, env)?,
            )),
            _ => Self::missing_syntax("darray intrinsic expression element", node, env),
        }
    }

    fn expand_type_args(ty: &Syntax<T, V>, env: &mut Env) -> Result<Vec<ast::Hint>> {
        match &ty.syntax {
            TypeArguments(c) => Self::could_map(Self::p_hint, &c.type_arguments_types, env),
            _ => Ok(vec![]),
        }
    }

    fn p_afield(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Afield> {
        match &node.syntax {
            ElementInitializer(c) => Ok(ast::Afield::AFkvalue(
                Self::p_expr(&c.element_key, env)?,
                Self::p_expr(&c.element_value, env)?,
            )),
            _ => Ok(ast::Afield::AFvalue(Self::p_expr(node, env)?)),
        }
    }

    fn check_intrinsic_type_arg_varity(
        node: &Syntax<T, V>,
        env: &mut Env,
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
                Self::raise_parsing_error(
                    node,
                    env,
                    &syntax_error::collection_intrinsic_many_typeargs,
                );
                None
            }
        }
    }

    fn verify_function_pointer_recv(node: &Syntax<T, V>, env: &mut Env, recv: &ast::Expr) {
        use aast::Expr_::*;
        match &recv.1 {
            Id(_) => {
                return;
            }
            ClassConst(c) => {
                if let aast::ClassId_::CIexpr(aast::Expr(_, Id(_))) = (c.0).1 {
                    return;
                }
            }
            ObjGet(c) => {
                if let Id(_) = (c.1).1 {
                    return;
                }
            }
            _ => {}
        }
        Self::raise_parsing_error(node, env, &syntax_error::function_pointer_bad_recv);
        return;
    }

    fn p_import_flavor(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::ImportFlavor> {
        use ast::ImportFlavor::*;
        match Self::token_kind(node) {
            Some(TK::Include) => Ok(Include),
            Some(TK::Require) => Ok(Require),
            Some(TK::Include_once) => Ok(IncludeOnce),
            Some(TK::Require_once) => Ok(RequireOnce),
            _ => Self::missing_syntax("import flavor", node, env),
        }
    }

    fn p_null_flavor(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::OgNullFlavor> {
        use ast::OgNullFlavor::*;
        match Self::token_kind(node) {
            Some(TK::QuestionMinusGreaterThan) => Ok(OGNullsafe),
            Some(TK::MinusGreaterThan) => Ok(OGNullthrows),
            _ => Self::missing_syntax("null flavor", node, env),
        }
    }

    #[inline]
    fn wrap_unescaper<F>(unescaper: F, s: &str) -> Result<String>
    where
        F: FnOnce(&str) -> std::result::Result<String, InvalidString>,
    {
        unescaper(s).map_err(|e| Error::Failwith(e.msg))
    }

    fn fail_if_invalid_class_creation(node: &Syntax<T, V>, env: &mut Env, id: impl AsRef<str>) {
        let id = id.as_ref();
        let is_in_static_method = *env.in_static_method();
        if is_in_static_method
            && ((id == special_classes::SELF && !env.cls_reified_generics().is_empty())
                || (id == special_classes::PARENT && *env.parent_maybe_reified()))
        {
            Self::raise_parsing_error(node, env, &syntax_error::static_method_reified_obj_creation);
        }
    }

    fn fail_if_invalid_reified_generic(node: &Syntax<T, V>, env: &mut Env, id: impl AsRef<str>) {
        let is_in_static_method = *env.in_static_method();
        if is_in_static_method && env.cls_reified_generics().contains(id.as_ref()) {
            Self::raise_parsing_error(
                node,
                env,
                &syntax_error::cls_reified_generic_in_static_method,
            );
        }
    }

    // (hrust) `i` is initial index
    fn rfind(s: &[u8], mut i: usize, c: u8) -> Result<usize> {
        if i >= s.len() {
            return Self::failwith("index out of range");
        }
        i += 1;
        while i > 0 {
            i -= 1;
            if s[i] == c {
                return Ok(i);
            }
        }
        Self::failwith("char not found")
    }

    fn prep_string2(nodes: &[Syntax<T, V>], env: &mut Env) -> Result<(TokenOp, TokenOp)> {
        use TokenOp::*;
        let is_qoute = |c| c == b'\"' || c == b'`';
        let start_is_qoute = |s: &[u8]| {
            (s.len() > 0 && is_qoute(s[0])) || (s.len() > 1 && (s[0] == b'b' && s[1] == b'\"'))
        };
        let last_is_qoute = |s: &[u8]| s.len() > 0 && is_qoute(s[s.len() - 1]);
        let is_heredoc = |s: &[u8]| (s.len() > 3 && &s[0..3] == b"<<<");
        let mut nodes = nodes.iter();
        let first = nodes.next();
        match first.map(|n| &n.syntax) {
            Some(Token(t)) => {
                let raise = |env| {
                    Self::raise_parsing_error(first.unwrap(), env, "Malformed String2 SyntaxList");
                };
                let text = t.text_raw(env.source_text());
                if start_is_qoute(text) {
                    let first_token_op = match text[0] {
                        b'b' if text.len() > 2 => LeftTrim(2),
                        _ if is_qoute(text[0]) && text.len() > 1 => LeftTrim(1),
                        _ => Skip,
                    };
                    if let Some(Token(t)) = nodes.last().map(|n| &n.syntax) {
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
                    if let Some(Token(t)) = nodes.last().map(|n| &n.syntax) {
                        let text = t.text_raw(env.source_text());
                        let len = text.len();
                        if len != 0 {
                            let n = Self::rfind(text, len - 2, b'\n')?;
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

    fn process_token_op(op: TokenOp, node: &Syntax<T, V>) -> Result<Option<Syntax<T, V>>> {
        use TokenOp::*;
        match op {
            LeftTrim(n) => match &node.syntax {
                Token(t) => {
                    let mut token = t.clone_value();
                    token.trim_left(n).map_err(Error::Failwith)?;
                    let node = <Syntax<T, V>>::make_token(token);
                    Ok(Some(node))
                }
                _ => Self::failwith("Token expected"),
            },
            RightTrim(n) => match &node.syntax {
                Token(t) => {
                    let mut token = t.clone_value();
                    token.trim_right(n).map_err(Error::Failwith)?;
                    let node = <Syntax<T, V>>::make_token(token);
                    Ok(Some(node))
                }
                _ => Self::failwith("Token expected"),
            },
            _ => Ok(None),
        }
    }

    fn p_string2(nodes: &[Syntax<T, V>], env: &mut Env) -> Result<Vec<ast::Expr>> {
        use TokenOp::*;
        let (head_op, tail_op) = Self::prep_string2(nodes, env)?;
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

            let node = Self::process_token_op(op, &nodes[i])?;
            let node = node.as_ref().unwrap_or(&nodes[i]);

            if Self::token_kind(node) == Some(TK::Dollar) && i < last {
                if let EmbeddedBracedExpression(_) = &nodes[i + 1].syntax {
                    Self::raise_parsing_error(
                        &nodes[i + 1],
                        env,
                        &syntax_error::outside_dollar_str_interp,
                    );

                    result.push(Self::p_expr_with_loc(
                        ExprLocation::InDoubleQuotedString,
                        &nodes[i + 1],
                        env,
                    )?);
                    i += 2;
                    continue;
                }
            }

            result.push(Self::p_expr_with_loc(
                ExprLocation::InDoubleQuotedString,
                node,
                env,
            )?);
            i += 1;
        }
        Ok(result)
    }

    fn p_expr_l(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Expr> {
        Self::p_expr_l_with_loc(ExprLocation::TopLevel, node, env)
    }

    fn p_expr_l_with_loc(
        loc: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<ast::Expr> {
        let p_expr = |n: &Syntax<T, V>, e: &mut Env| -> Result<ast::Expr> {
            Self::p_expr_with_loc(loc, n, e)
        };
        Ok(ast::Expr::new(
            Self::p_pos(node, env),
            E_::ExprList(Self::could_map(p_expr, node, env)?),
        ))
    }

    #[inline]
    fn p_expr(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Expr> {
        Self::p_expr_with_loc(ExprLocation::TopLevel, node, env)
    }

    #[inline]
    fn p_expr_with_loc(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<ast::Expr> {
        Self::p_expr_impl(location, node, env, None)
    }

    fn p_expr_impl(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
        parent_pos: Option<Pos>,
    ) -> Result<ast::Expr> {
        match &node.syntax {
            BracedExpression(c) => {
                let expr = &c.braced_expression_expression;
                let inner = Self::p_expr_impl(location, expr, env, parent_pos)?;
                let inner_pos = &inner.0;
                let inner_expr_ = &inner.1;
                use aast::Expr_::*;
                match inner_expr_ {
                    Lvar(_) | String(_) | Int(_) | Float(_) => Ok(inner),
                    _ => Ok(ast::Expr::new(inner_pos.clone(), E_::mk_braced_expr(inner))),
                }
            }
            ParenthesizedExpression(c) => Self::p_expr_impl(
                location,
                &c.parenthesized_expression_expression,
                env,
                parent_pos,
            ),
            _ => {
                let pos = Self::p_pos(node, env);
                let expr_ = Self::p_expr_impl_(location, node, env, parent_pos)?;
                Ok(ast::Expr::new(pos, expr_))
            }
        }
    }

    fn p_expr_lit(
        location: ExprLocation,
        parent: &Syntax<T, V>,
        expr: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<ast::Expr_> {
        match &expr.syntax {
            Token(_) => {
                let s = expr.text(env.indexed_source_text.source_text());
                let check_lint_err = |e: &mut Env, s: &str, expected: &str| {
                    if !e.codegen() && s != expected {
                        Self::raise_lint_error(
                            e,
                            LintError::lowercase_constant(Self::p_pos(expr, e), s),
                        );
                    }
                };
                match (location, Self::token_kind(expr)) {
                    (ExprLocation::InDoubleQuotedString, _) if env.codegen() => {
                        Ok(E_::String(Self::mk_str(expr, env, Self::unesc_dbl, s)))
                    }
                    (ExprLocation::InBacktickedString, _) if env.codegen() => {
                        Ok(E_::String(Self::mk_str(expr, env, unescape_backtick, s)))
                    }
                    (_, Some(TK::OctalLiteral))
                        if env.is_typechecker() && !Self::is_num_octal_lit(s) =>
                    {
                        Self::raise_parsing_error(
                            parent,
                            env,
                            &syntax_error::invalid_octal_integer,
                        );
                        Self::missing_syntax("octal", expr, env)
                    }
                    (_, Some(TK::DecimalLiteral))
                    | (_, Some(TK::OctalLiteral))
                    | (_, Some(TK::HexadecimalLiteral))
                    | (_, Some(TK::BinaryLiteral)) => Ok(E_::Int(s.replace("_", ""))),
                    (_, Some(TK::FloatingLiteral)) => Ok(E_::Float(String::from(s))),
                    (_, Some(TK::SingleQuotedStringLiteral)) => {
                        Ok(E_::String(Self::mk_str(expr, env, unescape_single, s)))
                    }
                    (_, Some(TK::DoubleQuotedStringLiteral)) => {
                        Ok(E_::String(Self::mk_str(expr, env, unescape_double, s)))
                    }
                    (_, Some(TK::HeredocStringLiteral)) => {
                        Ok(E_::String(Self::mk_str(expr, env, unescape_heredoc, s)))
                    }
                    (_, Some(TK::NowdocStringLiteral)) => {
                        Ok(E_::String(Self::mk_str(expr, env, unescape_nowdoc, s)))
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
                            Self::missing_syntax(&format!("boolean (not: {})", s), expr, env)
                        }
                    }
                    _ => Self::missing_syntax("literal", expr, env),
                }
            }
            SyntaxList(ts) => Ok(E_::String2(Self::p_string2(ts.as_slice(), env)?)),
            _ => Self::missing_syntax("literal expressoin", expr, env),
        }
    }

    fn p_expr_with_loc_(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<ast::Expr_> {
        Self::p_expr_impl_(location, node, env, None)
    }

    fn p_expr_impl_(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
        parent_pos: Option<Pos>,
    ) -> Result<ast::Expr_> {
        if *env.exp_recursion_depth() >= EXP_RECUSION_LIMIT {
            Err(Error::Failwith("Expression recursion limit reached".into()))
        } else {
            *env.exp_recursion_depth() += 1;
            let r = Self::p_expr_impl__(location, node, env, parent_pos);
            *env.exp_recursion_depth() -= 1;
            r
        }
    }

    fn p_expr_impl__(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
        parent_pos: Option<Pos>,
    ) -> Result<ast::Expr_> {
        env.check_stack_limit();
        use ast::Expr as E;
        let split_args_vararg = |arg_list_node: &Syntax<T, V>,
                                 e: &mut Env|
         -> Result<(Vec<ast::Expr>, Option<ast::Expr>)> {
            let mut arg_list = Self::as_list(arg_list_node);
            if let Some(last_arg) = arg_list.last() {
                if let DecoratedExpression(c) = &last_arg.syntax {
                    if Self::token_kind(&c.decorated_expression_decorator) == Some(TK::DotDotDot) {
                        let _ = arg_list.pop();
                        let args: std::result::Result<Vec<_>, _> =
                            arg_list.iter().map(|a| Self::p_expr(a, e)).collect();
                        let args = args?;
                        let vararg = Self::p_expr(&c.decorated_expression_expression, e)?;
                        return Ok((args, Some(vararg)));
                    }
                }
            }
            Ok((Self::could_map(Self::p_expr, arg_list_node, e)?, None))
        };
        let mk_lid = |p: Pos, s: String| ast::Lid(p, (0, s));
        let mk_name_lid = |name: &Syntax<T, V>, env: &mut Env| {
            let name = Self::pos_name(name, env)?;
            Ok(mk_lid(name.0, name.1))
        };
        let mk_lvar = |name: &Syntax<T, V>, env: &mut Env| Ok(E_::mk_lvar(mk_name_lid(name, env)?));
        let mk_id_expr = |name: ast::Sid| E::new(name.0.clone(), E_::mk_id(name));
        let p_intri_expr = |kw, ty, v, e: &mut Env| {
            let hints = Self::expand_type_args(ty, e)?;
            let hints = Self::check_intrinsic_type_arg_varity(node, e, hints);
            Ok(E_::mk_collection(
                Self::pos_name(kw, e)?,
                hints,
                Self::could_map(Self::p_afield, v, e)?,
            ))
        };
        let p_special_call =
            |recv: &Syntax<T, V>, args: &Syntax<T, V>, e: &mut Env| -> Result<ast::Expr_> {
                let pos_if_has_parens = match &recv.syntax {
                    ParenthesizedExpression(_) => Some(Self::p_pos(recv, e)),
                    _ => None,
                };
                let recv = Self::p_expr(recv, e)?;
                let recv = match (&recv.1, pos_if_has_parens) {
                    (E_::ObjGet(_), Some(p)) => E::new(p, E_::mk_parenthesized_expr(recv)),
                    (E_::ClassGet(_), Some(p)) => E::new(p, E_::mk_parenthesized_expr(recv)),
                    _ => recv,
                };
                let (args, varargs) = split_args_vararg(args, e)?;
                Ok(E_::mk_call(
                    ast::CallType::Cnormal,
                    recv,
                    vec![],
                    args,
                    varargs,
                ))
            };
        let p_obj_get = |recv: &Syntax<T, V>,
                         op: &Syntax<T, V>,
                         name: &Syntax<T, V>,
                         e: &mut Env|
         -> Result<ast::Expr_> {
            if recv.is_object_creation_expression() && !e.codegen() {
                Self::raise_parsing_error(recv, e, &syntax_error::invalid_constructor_method_call);
            }
            let recv = Self::p_expr(recv, e)?;
            let name = Self::p_expr_with_loc(ExprLocation::MemberSelect, name, e)?;
            let op = Self::p_null_flavor(op, e)?;
            Ok(E_::mk_obj_get(recv, name, op))
        };
        let pos = match parent_pos {
            None => Self::p_pos(node, env),
            Some(p) => p,
        };
        match &node.syntax {
            LambdaExpression(c) => {
                let suspension_kind =
                    Self::mk_suspension_kind(node, env, &c.lambda_async, &c.lambda_coroutine);
                let (params, ret) = match &c.lambda_signature.syntax {
                    LambdaSignature(c) => (
                        Self::could_map(Self::p_fun_param, &c.lambda_parameters, env)?,
                        Self::mp_optional(Self::p_hint, &c.lambda_type, env)?,
                    ),
                    Token(_) => {
                        let ast::Id(p, n) = Self::pos_name(&c.lambda_signature, env)?;
                        (
                            vec![ast::FunParam {
                                annotation: p.clone(),
                                type_hint: ast::TypeHint((), None),
                                is_variadic: false,
                                pos: p,
                                name: n,
                                expr: None,
                                callconv: None,
                                user_attributes: vec![],
                                visibility: None,
                            }],
                            None,
                        )
                    }
                    _ => Self::missing_syntax("lambda signature", &c.lambda_signature, env)?,
                };
                let (body, yield_) = if !c.lambda_body.is_compound_statement() {
                    Self::mp_yielding(Self::p_function_body, &c.lambda_body, env)?
                } else {
                    let mut env1 = Env::clone_and_unset_toplevel_if_toplevel(env);
                    Self::mp_yielding(&Self::p_function_body, &c.lambda_body, env1.as_mut())?
                };
                let external = c.lambda_body.is_external();
                let fun = ast::Fun_ {
                    span: pos.clone(),
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    ret: ast::TypeHint((), ret),
                    name: ast::Id(pos, String::from(";anonymous")),
                    tparams: vec![],
                    where_constraints: vec![],
                    body: ast::FuncBody {
                        ast: body,
                        annotation: (),
                    },
                    fun_kind: Self::mk_fun_kind(suspension_kind, yield_),
                    variadic: Self::determine_variadicity(&params),
                    params,
                    user_attributes: Self::p_user_attributes(&c.lambda_attribute_spec, env)?,
                    file_attributes: vec![],
                    external,
                    namespace: Self::mk_empty_ns_env(env),
                    doc_comment: None,
                    static_: false,
                };
                Ok(E_::mk_lfun(fun, vec![]))
            }
            BracedExpression(c) => {
                Self::p_expr_with_loc_(location, &c.braced_expression_expression, env)
            }
            EmbeddedBracedExpression(c) => Self::p_expr_impl_(
                location,
                &c.embedded_braced_expression_expression,
                env,
                Some(pos),
            ),
            ParenthesizedExpression(c) => {
                Self::p_expr_with_loc_(location, &c.parenthesized_expression_expression, env)
            }
            DictionaryIntrinsicExpression(c) => p_intri_expr(
                &c.dictionary_intrinsic_keyword,
                &c.dictionary_intrinsic_explicit_type,
                &c.dictionary_intrinsic_members,
                env,
            ),
            KeysetIntrinsicExpression(c) => p_intri_expr(
                &c.keyset_intrinsic_keyword,
                &c.keyset_intrinsic_explicit_type,
                &c.keyset_intrinsic_members,
                env,
            ),
            VectorIntrinsicExpression(c) => p_intri_expr(
                &c.vector_intrinsic_keyword,
                &c.vector_intrinsic_explicit_type,
                &c.vector_intrinsic_members,
                env,
            ),
            CollectionLiteralExpression(c) => {
                let collection_name = &c.collection_literal_name;
                let (collection_name, hints) = match &collection_name.syntax {
                    SimpleTypeSpecifier(c) => {
                        (Self::pos_name(&c.simple_type_specifier, env)?, None)
                    }
                    GenericTypeSpecifier(c) => {
                        let hints = Self::expand_type_args(&c.generic_argument_list, env)?;
                        let hints = Self::check_intrinsic_type_arg_varity(node, env, hints);
                        (Self::pos_name(&c.generic_class_type, env)?, hints)
                    }
                    _ => (Self::pos_name(collection_name, env)?, None),
                };
                Ok(E_::mk_collection(
                    collection_name,
                    hints,
                    Self::could_map(Self::p_afield, &c.collection_literal_initializers, env)?,
                ))
            }
            VarrayIntrinsicExpression(c) => {
                let hints = Self::expand_type_args(&c.varray_intrinsic_explicit_type, env)?;
                let hints = Self::check_intrinsic_type_arg_varity(node, env, hints);
                let targ = match hints {
                    Some(ast::CollectionTarg::CollectionTV(ty)) => Some(ty),
                    None => None,
                    _ => Self::missing_syntax("VarrayIntrinsicExpression type args", node, env)?,
                };
                Ok(E_::mk_varray(
                    targ,
                    Self::could_map(Self::p_expr, &c.varray_intrinsic_members, env)?,
                ))
            }
            DarrayIntrinsicExpression(c) => {
                let hints = Self::expand_type_args(&c.darray_intrinsic_explicit_type, env)?;
                let hints = Self::check_intrinsic_type_arg_varity(node, env, hints);
                match hints {
                    Some(ast::CollectionTarg::CollectionTKV(tk, tv)) => Ok(E_::mk_darray(
                        Some((tk, tv)),
                        Self::could_map(Self::p_member, &c.darray_intrinsic_members, env)?,
                    )),
                    None => Ok(E_::mk_darray(
                        None,
                        Self::could_map(Self::p_member, &c.darray_intrinsic_members, env)?,
                    )),
                    _ => Self::missing_syntax("DarrayIntrinsicExpression type args", node, env),
                }
            }
            ArrayIntrinsicExpression(c) => {
                if env.parser_options.po_disable_array {
                    Self::raise_parsing_error_pos(
                        &pos,
                        env,
                        "Array literals are no longer legal; use varray or darray instead",
                    );
                }
                /* TODO: Or tie in with other intrinsics and post-process to Array */
                Ok(E_::Array(Self::could_map(
                    Self::p_afield,
                    &c.array_intrinsic_members,
                    env,
                )?))
            }
            ListExpression(c) => {
                /* TODO: Or tie in with other intrinsics and post-process to List */
                let p_binder_or_ignore = |n: &Syntax<T, V>, e: &mut Env| -> Result<ast::Expr> {
                    match &n.syntax {
                        Missing => Ok(E::new(e.mk_none_pos(), E_::Omitted)),
                        _ => Self::p_expr(n, e),
                    }
                };
                Ok(E_::List(Self::could_map(
                    &p_binder_or_ignore,
                    &c.list_members,
                    env,
                )?))
            }
            EvalExpression(c) => p_special_call(&c.eval_keyword, &c.eval_argument, env),
            IssetExpression(c) => p_special_call(&c.isset_keyword, &c.isset_argument_list, env),
            TupleExpression(c) => {
                p_special_call(&c.tuple_expression_keyword, &c.tuple_expression_items, env)
            }
            FunctionCallExpression(c) => {
                let recv = &c.function_call_receiver;
                let args = &c.function_call_argument_list;
                let get_hhas_adata = || {
                    if Self::text_str(recv, env) == "__hhas_adata" {
                        if let SyntaxList(l) = &args.syntax {
                            if let Some(li) = l.first() {
                                if let ListItem(i) = &li.syntax {
                                    if let LiteralExpression(le) = &i.list_item.syntax {
                                        let expr = &le.literal_expression;
                                        if Self::token_kind(expr) == Some(TK::NowdocStringLiteral) {
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
                        let literal_expression_pos = Self::p_pos(expr, env);
                        let s = extract_unquoted_string(Self::text_str(expr, env), 0, expr.width())
                            .map_err(|e| Error::Failwith(e.msg))?;
                        Ok(E_::mk_call(
                            ast::CallType::Cnormal,
                            Self::p_expr(recv, env)?,
                            vec![],
                            vec![E::new(literal_expression_pos, E_::String(s))],
                            None,
                        ))
                    }
                    None => {
                        let targs = match (&recv.syntax, &c.function_call_type_args.syntax) {
                            (_, TypeArguments(c)) => {
                                Self::could_map(Self::p_targ, &c.type_arguments_types, env)?
                            }
                            /* TODO might not be needed */
                            (GenericTypeSpecifier(c), _) => match &c.generic_argument_list.syntax {
                                TypeArguments(c) => {
                                    Self::could_map(Self::p_targ, &c.type_arguments_types, env)?
                                }
                                _ => vec![],
                            },
                            _ => vec![],
                        };

                        /* preserve parens on receiver of call expression
                        to allow distinguishing between
                        ($a->b)() // invoke on callable property
                        $a->b()   // method call */
                        let pos_if_has_parens = match &recv.syntax {
                            ParenthesizedExpression(_) => Some(Self::p_pos(recv, env)),
                            _ => None,
                        };
                        let recv = Self::p_expr(recv, env)?;
                        let recv = match (&recv.1, pos_if_has_parens) {
                            (E_::ObjGet(_), Some(p)) => E::new(p, E_::mk_parenthesized_expr(recv)),
                            (E_::ClassGet(_), Some(p)) => {
                                E::new(p, E_::mk_parenthesized_expr(recv))
                            }
                            _ => recv,
                        };
                        let (args, varargs) = split_args_vararg(args, env)?;
                        Ok(E_::mk_call(
                            ast::CallType::Cnormal,
                            recv,
                            targs,
                            args,
                            varargs,
                        ))
                    }
                }
            }
            FunctionPointerExpression(c) => {
                let targs = match &c.function_pointer_type_args.syntax {
                    TypeArguments(c) => {
                        Self::could_map(Self::p_targ, &c.type_arguments_types, env)?
                    }
                    _ => vec![],
                };
                let recv = Self::p_expr(&c.function_pointer_receiver, env)?;
                Self::verify_function_pointer_recv(&c.function_pointer_receiver, env, &recv);
                Ok(E_::mk_function_pointer(recv, targs))
            }
            QualifiedName(_) => {
                if location.in_string() {
                    let ast::Id(_, n) = Self::pos_qualified_name(node, env)?;
                    Ok(E_::String(n))
                } else {
                    Ok(E_::mk_id(Self::pos_qualified_name(node, env)?))
                }
            }
            VariableExpression(c) => Ok(E_::mk_lvar(Self::lid_from_pos_name(
                pos,
                &c.variable_expression,
                env,
            )?)),
            PipeVariableExpression(_) => Ok(E_::mk_lvar(mk_lid(
                pos,
                special_idents::DOLLAR_DOLLAR.into(),
            ))),
            InclusionExpression(c) => Ok(E_::mk_import(
                Self::p_import_flavor(&c.inclusion_require, env)?,
                Self::p_expr(&c.inclusion_filename, env)?,
            )),
            MemberSelectionExpression(c) => {
                p_obj_get(&c.member_object, &c.member_operator, &c.member_name, env)
            }
            SafeMemberSelectionExpression(c) => p_obj_get(
                &c.safe_member_object,
                &c.safe_member_operator,
                &c.safe_member_name,
                env,
            ),
            EmbeddedMemberSelectionExpression(c) => p_obj_get(
                &c.embedded_member_object,
                &c.embedded_member_operator,
                &c.embedded_member_name,
                env,
            ),
            PrefixUnaryExpression(_) | PostfixUnaryExpression(_) | DecoratedExpression(_) => {
                let (operand, op, postfix) = match &node.syntax {
                    PrefixUnaryExpression(c) => {
                        (&c.prefix_unary_operand, &c.prefix_unary_operator, false)
                    }
                    PostfixUnaryExpression(c) => {
                        (&c.postfix_unary_operand, &c.postfix_unary_operator, true)
                    }
                    DecoratedExpression(c) => (
                        &c.decorated_expression_expression,
                        &c.decorated_expression_decorator,
                        false,
                    ),
                    _ => Self::missing_syntax("unary exppr", node, env)?,
                };

                /**
                 * FFP does not destinguish between ++$i and $i++ on the level of token
                 * kind annotation. Prevent duplication by switching on `postfix` for
                 * the two operatores for which AST /does/ differentiate between
                 * fixities.
                 */
                use ast::Uop::*;
                let mk_unop = |op, e| Ok(E_::mk_unop(op, e));
                let op_kind = Self::token_kind(op);
                if let Some(TK::At) = op_kind {
                    if env.parser_options.po_disallow_silence {
                        Self::raise_parsing_error(op, env, &syntax_error::no_silence);
                    }
                    if env.codegen() {
                        let expr = Self::p_expr(operand, env)?;
                        mk_unop(Usilence, expr)
                    } else {
                        let expr =
                            Self::p_expr_impl(ExprLocation::TopLevel, operand, env, Some(pos))?;
                        Ok(expr.1)
                    }
                } else {
                    let expr = Self::p_expr(operand, env)?;
                    match op_kind {
                        Some(TK::PlusPlus) if postfix => mk_unop(Upincr, expr),
                        Some(TK::MinusMinus) if postfix => mk_unop(Updecr, expr),
                        Some(TK::PlusPlus) => mk_unop(Uincr, expr),
                        Some(TK::MinusMinus) => mk_unop(Udecr, expr),
                        Some(TK::Exclamation) => mk_unop(Unot, expr),
                        Some(TK::Tilde) => mk_unop(Utild, expr),
                        Some(TK::Plus) => mk_unop(Uplus, expr),
                        Some(TK::Minus) => mk_unop(Uminus, expr),
                        Some(TK::Inout) => Ok(E_::mk_callconv(ast::ParamKind::Pinout, expr)),
                        Some(TK::Await) => Self::lift_await(pos, expr, env, location),
                        Some(TK::Suspend) => Ok(E_::mk_suspend(expr)),
                        Some(TK::Clone) => Ok(E_::mk_clone(expr)),
                        Some(TK::Print) => Ok(E_::mk_call(
                            ast::CallType::Cnormal,
                            E::new(
                                pos.clone(),
                                E_::mk_id(ast::Id(pos, special_functions::ECHO.into())),
                            ),
                            vec![],
                            vec![expr],
                            None,
                        )),
                        Some(TK::Dollar) => {
                            let E(_, expr_) = expr;
                            match expr_ {
                                E_::String(s) | E_::Int(s) | E_::Float(s) => {
                                    if !env.codegen() {
                                        Self::raise_parsing_error(
                                            op,
                                            env,
                                            &syntax_error::invalid_variable_name,
                                        )
                                    }
                                    let id = String::with_capacity(1 + s.len()) + "$" + &s;
                                    let lid = ast::Lid(pos, (0, id));
                                    Ok(E_::mk_lvar(lid))
                                }
                                _ => {
                                    Self::raise_parsing_error(
                                        op,
                                        env,
                                        &syntax_error::invalid_variable_variable,
                                    );
                                    Ok(E_::Omitted)
                                }
                            }
                        }
                        _ => Self::missing_syntax("unary operator", node, env),
                    }
                }
            }
            BinaryExpression(c) => {
                use ExprLocation::*;
                let rlocation = if Self::token_kind(&c.binary_operator) == Some(TK::Equal) {
                    match location {
                        AsStatement => RightOfAssignment,
                        UsingStatement => RightOfAssignmentInUsingStatement,
                        _ => TopLevel,
                    }
                } else {
                    TopLevel
                };
                let bop_ast_node = Self::p_bop(
                    pos,
                    &c.binary_operator,
                    Self::p_expr(&c.binary_left_operand, env)?,
                    Self::p_expr_with_loc(rlocation, &c.binary_right_operand, env)?,
                    env,
                )?;
                if let Some((ast::Bop::Eq(_), lhs, _)) = bop_ast_node.as_binop() {
                    Self::check_lvalue(lhs, env);
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
                        Self::wrap_unescaper(unescape_heredoc, Self::text_str(node, env))?,
                    )),
                    (InDoubleQuotedString, _) => Ok(E_::String(Self::wrap_unescaper(
                        Self::unesc_dbl,
                        Self::text_str(node, env),
                    )?)),
                    (InBacktickedString, _) => Ok(E_::String(Self::wrap_unescaper(
                        unescape_backtick,
                        Self::text_str(node, env),
                    )?)),
                    (MemberSelect, _)
                    | (TopLevel, _)
                    | (AsStatement, _)
                    | (UsingStatement, _)
                    | (RightOfAssignment, _)
                    | (RightOfAssignmentInUsingStatement, _)
                    | (RightOfReturn, _) => Ok(E_::mk_id(Self::pos_name(node, env)?)),
                }
            }
            YieldExpression(c) => {
                use ExprLocation::*;
                env.saw_yield = true;
                if location != AsStatement
                    && location != RightOfAssignment
                    && location != RightOfAssignmentInUsingStatement
                {
                    Self::raise_parsing_error(node, env, &syntax_error::invalid_yield);
                }
                if c.yield_operand.text(env.source_text()) == "break" {
                    Ok(E_::YieldBreak)
                } else if c.yield_operand.is_missing() {
                    Ok(E_::mk_yield(ast::Afield::AFvalue(E::new(pos, E_::Null))))
                } else {
                    Ok(E_::mk_yield(Self::p_afield(&c.yield_operand, env)?))
                }
            }
            YieldFromExpression(c) => {
                use ExprLocation::*;
                env.saw_yield = true;
                if location != AsStatement
                    && location != RightOfAssignment
                    && location != RightOfAssignmentInUsingStatement
                    && location != RightOfReturn
                {
                    Self::raise_parsing_error(node, env, &syntax_error::invalid_yield_from);
                }
                Ok(E_::mk_yield_from(Self::p_expr(&c.yield_from_operand, env)?))
            }
            DefineExpression(c) => {
                let name = Self::pos_name(&c.define_keyword, env)?;
                Ok(E_::mk_call(
                    ast::CallType::Cnormal,
                    mk_id_expr(name),
                    vec![],
                    Self::as_list(&c.define_argument_list)
                        .iter()
                        .map(|x| Self::p_expr(x, env))
                        .collect::<std::result::Result<Vec<_>, _>>()?,
                    None,
                ))
            }
            ScopeResolutionExpression(c) => {
                let qual = Self::p_expr(&c.scope_resolution_qualifier, env)?;
                if let E_::Id(id) = &qual.1 {
                    Self::fail_if_invalid_reified_generic(node, env, &id.1);
                }
                match &c.scope_resolution_name.syntax {
                    Token(token) if token.kind() == TK::Variable => {
                        let ast::Id(p, name) = Self::pos_name(&c.scope_resolution_name, env)?;
                        Ok(E_::mk_class_get(
                            ast::ClassId(pos, ast::ClassId_::CIexpr(qual)),
                            ast::ClassGetExpr::CGstring((p, name)),
                        ))
                    }
                    _ => {
                        let E(p, expr_) = Self::p_expr(&c.scope_resolution_name, env)?;
                        match expr_ {
                            E_::String(id) => Ok(E_::mk_class_const(
                                ast::ClassId(pos, ast::ClassId_::CIexpr(qual)),
                                (p, id),
                            )),
                            E_::Id(id) => {
                                let ast::Id(p, n) = *id;
                                Ok(E_::mk_class_const(
                                    ast::ClassId(pos, ast::ClassId_::CIexpr(qual)),
                                    (p, n),
                                ))
                            }
                            E_::Lvar(id) => {
                                let ast::Lid(p, (_, n)) = *id;
                                Ok(E_::mk_class_get(
                                    ast::ClassId(pos, ast::ClassId_::CIexpr(qual)),
                                    ast::ClassGetExpr::CGstring((p, n)),
                                ))
                            }
                            _ => Ok(E_::mk_class_get(
                                ast::ClassId(pos, ast::ClassId_::CIexpr(qual)),
                                ast::ClassGetExpr::CGexpr(E(p, expr_)),
                            )),
                        }
                    }
                }
            }
            CastExpression(c) => Ok(E_::mk_cast(
                Self::p_hint(&c.cast_type, env)?,
                Self::p_expr(&c.cast_operand, env)?,
            )),
            ConditionalExpression(c) => {
                let alter = Self::p_expr(&c.conditional_alternative, env)?;
                let consequence = Self::mp_optional(Self::p_expr, &c.conditional_consequence, env)?;
                let condition = Self::p_expr(&c.conditional_test, env)?;
                Ok(E_::mk_eif(condition, consequence, alter))
            }
            SubscriptExpression(c) => Ok(E_::mk_array_get(
                Self::p_expr(&c.subscript_receiver, env)?,
                Self::mp_optional(Self::p_expr, &c.subscript_index, env)?,
            )),
            EmbeddedSubscriptExpression(c) => Ok(E_::mk_array_get(
                Self::p_expr(&c.embedded_subscript_receiver, env)?,
                Self::mp_optional(
                    |n, e| Self::p_expr_with_loc(location, n, e),
                    &c.embedded_subscript_index,
                    env,
                )?,
            )),
            ShapeExpression(c) => Ok(E_::Shape(Self::could_map(
                |n: &Syntax<T, V>, e: &mut Env| {
                    Self::mp_shape_expression_field(&Self::p_expr, n, e)
                },
                &c.shape_expression_fields,
                env,
            )?)),
            ObjectCreationExpression(c) => {
                Self::p_expr_impl_(location, &c.object_creation_object, env, Some(pos))
            }
            ConstructorCall(c) => {
                let (args, varargs) = split_args_vararg(&c.constructor_call_argument_list, env)?;
                let (e, hl) = match &c.constructor_call_type.syntax {
                    GenericTypeSpecifier(c) => {
                        let name = Self::pos_name(&c.generic_class_type, env)?;
                        let hints = match &c.generic_argument_list.syntax {
                            TypeArguments(c) => {
                                Self::could_map(Self::p_targ, &c.type_arguments_types, env)?
                            }
                            _ => Self::missing_syntax(
                                "generic type arguments",
                                &c.generic_argument_list,
                                env,
                            )?,
                        };
                        (mk_id_expr(name), hints)
                    }
                    SimpleTypeSpecifier(_) => {
                        let name = Self::pos_name(&c.constructor_call_type, env)?;
                        (mk_id_expr(name), vec![])
                    }
                    _ => (Self::p_expr(&c.constructor_call_type, env)?, vec![]),
                };
                if let E_::Id(name) = &e.1 {
                    Self::fail_if_invalid_reified_generic(node, env, &name.1);
                    Self::fail_if_invalid_class_creation(node, env, &name.1);
                }
                Ok(E_::mk_new(
                    ast::ClassId(pos.clone(), ast::ClassId_::CIexpr(e)),
                    hl,
                    args,
                    varargs,
                    pos,
                ))
            }
            GenericTypeSpecifier(c) => {
                if !c.generic_argument_list.is_missing() {
                    Self::raise_parsing_error(
                        &c.generic_argument_list,
                        env,
                        &syntax_error::targs_not_allowed,
                    )
                }
                let name = Self::pos_name(&c.generic_class_type, env)?;
                Ok(E_::mk_id(name))
            }
            RecordCreationExpression(c) => {
                let id = Self::pos_name(&c.record_creation_type, env)?;
                let is_record_array =
                    Self::token_kind(&c.record_creation_array_token) == Some(TK::At);
                Ok(E_::mk_record(
                    id,
                    is_record_array,
                    Self::could_map(Self::p_member, &c.record_creation_members, env)?,
                ))
            }
            LiteralExpression(c) => Self::p_expr_lit(location, node, &c.literal_expression, env),
            PrefixedStringExpression(c) => {
                /* Temporarily allow only`re`- prefixed strings */
                let name_text = Self::text(&c.prefixed_string_name, env);
                if name_text != "re" {
                    Self::raise_parsing_error(node, env, &syntax_error::non_re_prefix);
                }
                Ok(E_::mk_prefixed_string(
                    name_text,
                    Self::p_expr(&c.prefixed_string_str, env)?,
                ))
            }
            IsExpression(c) => Ok(E_::mk_is(
                Self::p_expr(&c.is_left_operand, env)?,
                Self::p_hint(&c.is_right_operand, env)?,
            )),
            AsExpression(c) => Ok(E_::mk_as(
                Self::p_expr(&c.as_left_operand, env)?,
                Self::p_hint(&c.as_right_operand, env)?,
                false,
            )),
            NullableAsExpression(c) => Ok(E_::mk_as(
                Self::p_expr(&c.nullable_as_left_operand, env)?,
                Self::p_hint(&c.nullable_as_right_operand, env)?,
                true,
            )),
            AnonymousFunction(c) => {
                if env.parser_options.po_disable_static_closures
                    && Self::token_kind(&c.anonymous_static_keyword) == Some(TK::Static)
                {
                    Self::raise_parsing_error(
                        node,
                        env,
                        &syntax_error::static_closures_are_disabled,
                    );
                }
                let p_arg = |n: &Syntax<T, V>, e: &mut Env| match &n.syntax {
                    Token(_) => mk_name_lid(n, e),
                    _ => Self::missing_syntax("use variable", n, e),
                };
                let p_use = |n: &Syntax<T, V>, e: &mut Env| match &n.syntax {
                    AnonymousFunctionUseClause(c) => {
                        Self::could_map(p_arg, &c.anonymous_use_variables, e)
                    }
                    _ => Ok(vec![]),
                };
                let suspension_kind = Self::mk_suspension_kind(
                    node,
                    env,
                    &c.anonymous_async_keyword,
                    &c.anonymous_coroutine_keyword,
                );
                let (body, yield_) = {
                    let mut env1 = Env::clone_and_unset_toplevel_if_toplevel(env);
                    Self::mp_yielding(&Self::p_function_body, &c.anonymous_body, env1.as_mut())?
                };
                let doc_comment =
                    Self::extract_docblock(node, env).or_else(|| env.top_docblock().clone());
                let user_attributes = Self::p_user_attributes(&c.anonymous_attribute_spec, env)?;
                let external = c.anonymous_body.is_external();
                let params = Self::could_map(Self::p_fun_param, &c.anonymous_parameters, env)?;
                let name_pos = Self::p_fun_pos(node, env);
                let fun = ast::Fun_ {
                    span: Self::p_pos(node, env),
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    ret: ast::TypeHint(
                        (),
                        Self::mp_optional(Self::p_hint, &c.anonymous_type, env)?,
                    ),
                    name: ast::Id(name_pos, String::from(";anonymous")),
                    tparams: vec![],
                    where_constraints: vec![],
                    body: ast::FuncBody {
                        ast: body,
                        annotation: (),
                    },
                    fun_kind: Self::mk_fun_kind(suspension_kind, yield_),
                    variadic: Self::determine_variadicity(&params),
                    params,
                    user_attributes,
                    file_attributes: vec![],
                    external,
                    namespace: Self::mk_empty_ns_env(env),
                    doc_comment,
                    static_: !c.anonymous_static_keyword.is_missing(),
                };
                let uses = p_use(&c.anonymous_use, env).unwrap_or_else(|_| vec![]);
                Ok(E_::mk_efun(fun, uses))
            }
            AwaitableCreationExpression(c) => {
                let suspension_kind =
                    Self::mk_suspension_kind(node, env, &c.awaitable_async, &c.awaitable_coroutine);
                let (blk, yld) = Self::mp_yielding(
                    &Self::p_function_body,
                    &c.awaitable_compound_statement,
                    env,
                )?;
                let user_attributes = Self::p_user_attributes(&c.awaitable_attribute_spec, env)?;
                let external = c.awaitable_compound_statement.is_external();
                let name_pos = Self::p_fun_pos(node, env);
                let body = ast::Fun_ {
                    span: pos.clone(),
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    ret: ast::TypeHint((), None),
                    name: ast::Id(name_pos, String::from(";anonymous")),
                    tparams: vec![],
                    where_constraints: vec![],
                    body: ast::FuncBody {
                        ast: if blk.len() == 0 {
                            let pos = Self::p_pos(&c.awaitable_compound_statement, env);
                            vec![ast::Stmt::noop(pos)]
                        } else {
                            blk
                        },
                        annotation: (),
                    },
                    fun_kind: Self::mk_fun_kind(suspension_kind, yld),
                    variadic: Self::determine_variadicity(&[]),
                    params: vec![],
                    user_attributes,
                    file_attributes: vec![],
                    external,
                    namespace: Self::mk_empty_ns_env(env),
                    doc_comment: None,
                    static_: false,
                };
                Ok(E_::mk_call(
                    ast::CallType::Cnormal,
                    E::new(pos, E_::mk_lfun(body, vec![])),
                    vec![],
                    vec![],
                    None,
                ))
            }
            XHPExpression(c) if c.xhp_open.is_xhp_open() => {
                if let XHPOpen(c1) = &c.xhp_open.syntax {
                    let name = Self::pos_name(&c1.xhp_open_name, env)?;
                    let attrs = Self::could_map(Self::p_xhp_attr, &c1.xhp_open_attributes, env)?;
                    let exprs = Self::aggregate_xhp_tokens(&c.xhp_body)?
                        .iter()
                        .map(|n| match n {
                            Either::Left(n) => Self::p_xhp_embedded(Self::unesc_xhp, &n, env),
                            Either::Right(n) => Self::p_xhp_embedded(Self::unesc_xhp, n, env),
                        })
                        .collect::<std::result::Result<Vec<_>, _>>()?;

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
                    Self::failwith("expect xhp open")
                }
            }
            PocketAtomExpression(c) => Ok(E_::PUAtom(
                Self::pos_name(&c.pocket_atom_expression, env)?.1,
            )),
            PocketIdentifierExpression(c) => {
                let mk_class_id = |e: ast::Expr| ast::ClassId(pos, ast::ClassId_::CIexpr(e));
                let qual = Self::p_expr(&c.pocket_identifier_qualifier, env)?;
                let qual = if env.codegen() {
                    mk_class_id(qual)
                } else if let E_::Lvar(a) = qual.1 {
                    let p = qual.0;
                    let expr = E::new(p.clone(), E_::mk_id(ast::Id(p, (a.1).1)));
                    mk_class_id(expr)
                } else {
                    mk_class_id(qual)
                };
                let E(p, expr_) = Self::p_expr(&c.pocket_identifier_field, env)?;
                let field = match expr_ {
                    E_::String(id) => (p, id),
                    E_::Id(id) => {
                        let ast::Id(p, n) = *id;
                        (p, n)
                    }
                    _ => Self::missing_syntax("PocketIdentifierExpression field", node, env)?,
                };
                let E(p, expr_) = Self::p_expr(&c.pocket_identifier_name, env)?;
                let name = match expr_ {
                    E_::String(id) => (p, id),
                    E_::Id(id) => {
                        let ast::Id(p, n) = *id;
                        (p, n)
                    }
                    _ => Self::missing_syntax("PocketIdentifierExpression name", node, env)?,
                };
                Ok(E_::mk_puidentifier(qual, field, name))
            }
            _ => Self::missing_syntax_(Some(E_::Null), "expression", node, env),
        }
    }

    fn check_lvalue(ast::Expr(p, expr_): &ast::Expr, env: &mut Env) {
        use aast::Expr_::*;
        let mut raise = |s| Self::raise_parsing_error_pos(p, env, s);
        match expr_ {
            ObjGet(og) => match og.as_ref() {
                (_, ast::Expr(_, Id(_)), ast::OgNullFlavor::OGNullsafe) => {
                    raise("?-> syntax is not supported for lvalues")
                }
                (_, ast::Expr(_, Id(sid)), _) if sid.1.as_bytes()[0] == b':' => {
                    raise("->: syntax is not supported for lvalues")
                }
                _ => {}
            },
            ArrayGet(ag) => {
                if let ClassConst(_) = (ag.0).1 {
                    raise("Array-like class consts are not valid lvalues");
                }
            }
            Call(c) => match &(c.1).1 {
                Id(sid) if sid.1 == "tuple" => {
                    raise("Tuple cannot be used as an lvalue. Maybe you meant list?")
                }
                _ => raise("Invalid lvalue"),
            },
            List(l) => {
                for i in l.iter() {
                    Self::check_lvalue(i, env);
                }
            }
            Array(_) | Darray(_) | Varray(_) | Shape(_) | Collection(_) | Record(_) | Null
            | True | False | Id(_) | Clone(_) | ClassConst(_) | Int(_) | Float(_)
            | PrefixedString(_) | String(_) | String2(_) | Yield(_) | YieldBreak | YieldFrom(_)
            | Await(_) | Suspend(_) | ExprList(_) | Cast(_) | Unop(_) | Binop(_) | Eif(_)
            | New(_) | Efun(_) | Lfun(_) | Xml(_) | Import(_) | Pipe(_) | Callconv(_) | Is(_)
            | As(_) | ParenthesizedExpr(_) | PUIdentifier(_) => raise("Invalid lvalue"),
            _ => {}
        }
    }

    fn p_xhp_embedded<F>(escaper: F, node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Expr>
    where
        F: FnOnce(&[u8]) -> Vec<u8>,
    {
        if let Some(kind) = Self::token_kind(node) {
            if env.codegen() && TK::XHPStringLiteral == kind {
                let p = Self::p_pos(node, env);
                /* for XHP string literals (attribute values) just extract
                value from quotes and decode HTML entities  */
                let text = html_entities::decode(&Self::get_quoted_content(
                    node.full_text(env.source_text()),
                ));
                Ok(ast::Expr::new(p, E_::make_string(text)))
            } else if env.codegen() && TK::XHPBody == kind {
                let p = Self::p_pos(node, env);
                /* for XHP body - only decode HTML entities */
                let text =
                    html_entities::decode(&Self::unesc_xhp(node.full_text(env.source_text())));
                Ok(ast::Expr::new(p, E_::make_string(text)))
            } else {
                let p = Self::p_pos(node, env);
                let s = escaper(node.full_text(env.source_text()));
                Ok(ast::Expr::new(p, E_::make_string(s)))
            }
        } else {
            let expr = Self::p_expr(node, env)?;
            match expr.1 {
                E_::BracedExpr(e) => Ok(*e),
                _ => Ok(expr),
            }
        }
    }

    fn p_xhp_attr(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::XhpAttribute> {
        match &node.syntax {
            XHPSimpleAttribute(c) => {
                let attr_expr = &c.xhp_simple_attribute_expression;
                let name = Self::p_pstring(&c.xhp_simple_attribute_name, env)?;
                let expr = if attr_expr.is_braced_expression()
                    && env.file_mode() == file_info::Mode::Mdecl
                    && !env.codegen()
                {
                    ast::Expr::new(env.mk_none_pos(), E_::Null)
                } else {
                    Self::p_xhp_embedded(Self::unesc_xhp_attr, attr_expr, env)?
                };
                Ok(ast::XhpAttribute::XhpSimple(name, expr))
            }
            XHPSpreadAttribute(c) => {
                let expr =
                    Self::p_xhp_embedded(Self::unesc_xhp, &c.xhp_spread_attribute_expression, env)?;
                Ok(ast::XhpAttribute::XhpSpread(expr))
            }
            _ => Self::missing_syntax("XHP attribute", node, env),
        }
    }

    fn aggregate_xhp_tokens<'b>(
        nodes: &'b Syntax<T, V>,
    ) -> Result<Vec<Either<Syntax<T, V>, &'b Syntax<T, V>>>> {
        let nodes = Self::as_list(nodes);
        let mut state = (None, None, vec![]); // (start, end, result)
        let combine = |state: &mut (
            Option<&'b Syntax<T, V>>,
            Option<&'b Syntax<T, V>>,
            Vec<Either<Syntax<T, V>, &'b Syntax<T, V>>>,
        )| {
            match (state.0, state.1) {
                (Some(s), None) => state.2.push(Right(s)),
                (Some(s), Some(e)) => {
                    let token = T::concatenate(s.get_token().unwrap(), e.get_token().unwrap())
                        .map_err(Error::Failwith)?;
                    let node = <Syntax<T, V>>::make_token(token);
                    state.2.push(Left(node))
                }
                _ => {}
            }
            state.0 = None;
            state.1 = None;
            Ok(())
        };
        for n in nodes {
            match &n.syntax {
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
                    state.2.push(Right(n));
                }
            }
        }
        combine(&mut state)?;
        Ok(state.2)
    }

    fn p_bop(
        pos: Pos,
        node: &Syntax<T, V>,
        lhs: ast::Expr,
        rhs: ast::Expr,
        env: &mut Env,
    ) -> Result<ast::Expr_> {
        use ast::Bop::*;
        let mk = |op, l, r| Ok(E_::mk_binop(op, l, r));
        let mk_eq = |op, l, r| Ok(E_::mk_binop(Eq(Some(Box::new(op))), l, r));
        match Self::token_kind(node) {
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
            _ => Self::missing_syntax("binary operator", node, env),
        }
    }

    fn is_noop(stmt: &ast::Stmt) -> bool {
        if let ast::Stmt_::Noop = stmt.1 {
            true
        } else {
            false
        }
    }

    fn p_stmt_list_(
        pos: &Pos,
        mut nodes: Iter<&Syntax<T, V>>,
        env: &mut Env,
    ) -> Result<Vec<ast::Stmt>> {
        let mut r = vec![];
        loop {
            match nodes.next() {
                Some(n) => match &n.syntax {
                    UsingStatementFunctionScoped(c) => {
                        let body = Self::p_stmt_list_(pos, nodes, env)?;
                        let f = |e: &mut Env| {
                            Ok(ast::Stmt::new(
                                pos.clone(),
                                ast::Stmt_::mk_using(ast::UsingStmt {
                                    is_block_scoped: false,
                                    has_await: !c.using_function_await_keyword.is_missing(),
                                    expr: Self::p_expr_l_with_loc(
                                        ExprLocation::UsingStatement,
                                        &c.using_function_expression,
                                        e,
                                    )?,
                                    block: body,
                                }),
                            ))
                        };
                        let using = Self::lift_awaits_in_statement_(f, Either::Right(pos), env)?;
                        r.push(using);
                        break Ok(r);
                    }
                    _ => {
                        r.push(Self::p_stmt(n, env)?);
                    }
                },
                _ => break Ok(r),
            }
        }
    }

    // TODO: rename to p_stmt_list
    fn handle_loop_body(pos: Pos, node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Stmt> {
        let list = Self::as_list(node);
        let blk: Vec<_> = Self::p_stmt_list_(&pos, list.iter(), env)?
            .into_iter()
            .filter(|stmt| !Self::is_noop(stmt))
            .collect();
        let body = if blk.len() == 0 {
            vec![Self::mk_noop(env)]
        } else {
            blk
        };
        Ok(ast::Stmt::new(pos, ast::Stmt_::mk_block(body)))
    }

    fn is_simple_assignment_await_expression(node: &Syntax<T, V>) -> bool {
        match &node.syntax {
            BinaryExpression(c) => {
                Self::token_kind(&c.binary_operator) == Some(TK::Equal)
                    && Self::is_simple_await_expression(&c.binary_right_operand)
            }
            _ => false,
        }
    }

    fn is_simple_await_expression(node: &Syntax<T, V>) -> bool {
        match &node.syntax {
            PrefixUnaryExpression(c) => {
                Self::token_kind(&c.prefix_unary_operator) == Some(TK::Await)
            }
            _ => false,
        }
    }

    fn with_new_nonconcurrent_scrope<F, R>(f: F, env: &mut Env) -> R
    where
        F: FnOnce(&mut Env) -> R,
    {
        let saved_lifted_awaits = env.lifted_awaits.take();
        let result = f(env);
        env.lifted_awaits = saved_lifted_awaits;
        result
    }

    fn with_new_concurrent_scrope<F, R>(f: F, env: &mut Env) -> Result<(LiftedAwaitExprs, R)>
    where
        F: FnOnce(&mut Env) -> Result<R>,
    {
        let saved_lifted_awaits = env.lifted_awaits.replace(LiftedAwaits {
            awaits: vec![],
            lift_kind: LiftedAwaitKind::LiftedFromConcurrent,
        });
        let result = f(env);
        let lifted_awaits = mem::replace(&mut env.lifted_awaits, saved_lifted_awaits);
        let result = result?;
        let awaits = match lifted_awaits {
            Some(la) => Self::process_lifted_awaits(la)?,
            None => Self::failwith("lifted awaits should not be None")?,
        };
        Ok((awaits, result))
    }

    fn process_lifted_awaits(mut awaits: LiftedAwaits) -> Result<LiftedAwaitExprs> {
        for await_ in awaits.awaits.iter() {
            if (await_.1).0.is_none() {
                return Self::failwith("none pos in lifted awaits");
            }
        }
        awaits
            .awaits
            .sort_unstable_by(|a1, a2| Pos::cmp(&(a1.1).0, &(a2.1).0));
        Ok(awaits.awaits)
    }

    fn clear_statement_scope<F, R>(f: F, env: &mut Env) -> R
    where
        F: FnOnce(&mut Env) -> R,
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

    fn lift_awaits_in_statement<F>(f: F, node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Stmt>
    where
        F: FnOnce(&mut Env) -> Result<ast::Stmt>,
    {
        Self::lift_awaits_in_statement_(f, Either::Left(node), env)
    }

    fn lift_awaits_in_statement_<F>(
        f: F,
        pos: Either<&Syntax<T, V>, &Pos>,
        env: &mut Env,
    ) -> Result<ast::Stmt>
    where
        F: FnOnce(&mut Env) -> Result<ast::Stmt>,
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
                let awaits = Self::process_lifted_awaits(lifted_awaits)?;
                let pos = match pos {
                    Either::Left(n) => Self::p_pos(n, env),
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

    fn lift_await(
        parent_pos: Pos,
        expr: ast::Expr,
        env: &mut Env,
        location: ExprLocation,
    ) -> Result<ast::Expr_> {
        use ExprLocation::*;
        match (&env.lifted_awaits, location) {
            (_, UsingStatement) | (_, RightOfAssignmentInUsingStatement) | (None, _) => {
                Ok(E_::mk_await(expr))
            }
            (Some(_), _) => {
                if location != AsStatement {
                    let name = env.make_tmp_var_name();
                    let lid = ast::Lid::new(parent_pos, name.clone());
                    let await_lid = ast::Lid::new(expr.0.clone(), name);
                    let await_ = (Some(await_lid), expr);
                    env.lifted_awaits.as_mut().map(|aw| aw.awaits.push(await_));
                    Ok(E_::mk_lvar(lid))
                } else {
                    env.lifted_awaits
                        .as_mut()
                        .map(|aw| aw.awaits.push((None, expr)));
                    Ok(E_::Null)
                }
            }
        }
    }

    fn p_stmt(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Stmt> {
        Self::clear_statement_scope(
            |e: &mut Env| {
                let docblock = Self::extract_docblock(node, e);
                e.push_docblock(docblock);
                let result = Self::p_stmt_(node, e);
                e.pop_docblock();
                result
            },
            env,
        )
    }

    fn p_stmt_(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Stmt> {
        let pos = Self::p_pos(node, env);
        use ast::{Stmt as S, Stmt_ as S_};
        match &node.syntax {
            SwitchStatement(c) => {
                let p_label = |n: &Syntax<T, V>, e: &mut Env| -> Result<ast::Case> {
                    match &n.syntax {
                        CaseLabel(c) => Ok(ast::Case::Case(
                            Self::p_expr(&c.case_expression, e)?,
                            vec![],
                        )),
                        DefaultLabel(_) => Ok(ast::Case::Default(Self::p_pos(n, e), vec![])),
                        _ => Self::missing_syntax("switch label", n, e),
                    }
                };
                let p_section = |n: &Syntax<T, V>, e: &mut Env| -> Result<Vec<ast::Case>> {
                    match &n.syntax {
                        SwitchSection(c) => {
                            let mut blk =
                                Self::could_map(Self::p_stmt, &c.switch_section_statements, e)?;
                            if !c.switch_section_fallthrough.is_missing() {
                                blk.push(S::new(e.mk_none_pos(), S_::Fallthrough));
                            }
                            let mut labels = Self::could_map(p_label, &c.switch_section_labels, e)?;
                            match labels.last_mut() {
                                Some(ast::Case::Default(_, b)) => *b = blk,
                                Some(ast::Case::Case(_, b)) => *b = blk,
                                _ => Self::raise_parsing_error(n, e, "Malformed block result"),
                            }
                            Ok(labels)
                        }
                        _ => Self::missing_syntax("switch section", n, e),
                    }
                };
                let f = |env: &mut Env| -> Result<ast::Stmt> {
                    Ok(S::new(
                        pos,
                        S_::mk_switch(
                            Self::p_expr(&c.switch_expression, env)?,
                            itertools::concat(Self::could_map(p_section, &c.switch_sections, env)?),
                        ),
                    ))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            IfStatement(c) => {
                let p_else_if =
                    |n: &Syntax<T, V>, e: &mut Env| -> Result<(ast::Expr, ast::Block)> {
                        match &n.syntax {
                            ElseifClause(c) => Ok((
                                Self::p_expr(&c.elseif_condition, e)?,
                                Self::p_block(true, &c.elseif_statement, e)?,
                            )),
                            _ => Self::missing_syntax("elseif clause", n, e),
                        }
                    };
                let f = |env: &mut Env| -> Result<ast::Stmt> {
                    let condition = Self::p_expr(&c.if_condition, env)?;
                    let statement =
                        Self::p_block(true /* remove noop */, &c.if_statement, env)?;
                    let else_ = match &c.if_else_clause.syntax {
                        ElseClause(c) => Self::p_block(true, &c.else_statement, env)?,
                        Missing => vec![Self::mk_noop(env)],
                        _ => Self::missing_syntax("else clause", &c.if_else_clause, env)?,
                    };
                    let else_ifs = Self::could_map(p_else_if, &c.if_elseif_clauses, env)?;
                    let else_if = else_ifs
                        .into_iter()
                        .rev()
                        .fold(else_, |child, (cond, stmts)| {
                            vec![S::new(pos.clone(), S_::mk_if(cond, stmts, child))]
                        });
                    Ok(S::new(pos, S_::mk_if(condition, statement, else_if)))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            ExpressionStatement(c) => {
                let expr = &c.expression_statement_expression;
                let f = |e: &mut Env| -> Result<ast::Stmt> {
                    if expr.is_missing() {
                        Ok(S::new(pos, S_::Noop))
                    } else {
                        Ok(S::new(
                            pos,
                            S_::mk_expr(Self::p_expr_with_loc(ExprLocation::AsStatement, expr, e)?),
                        ))
                    }
                };
                if Self::is_simple_assignment_await_expression(expr)
                    || Self::is_simple_await_expression(expr)
                {
                    f(env)
                } else {
                    Self::lift_awaits_in_statement(f, node, env)
                }
            }
            CompoundStatement(c) => Self::handle_loop_body(pos, &c.compound_statements, env),
            SyntaxList(_) => Self::handle_loop_body(pos, node, env),
            ThrowStatement(c) => Self::lift_awaits_in_statement(
                |e: &mut Env| -> Result<ast::Stmt> {
                    Ok(S::new(
                        pos,
                        S_::mk_throw(Self::p_expr(&c.throw_expression, e)?),
                    ))
                },
                node,
                env,
            ),
            DoStatement(c) => Ok(S::new(
                pos,
                S_::mk_do(
                    Self::p_block(false /* remove noop */, &c.do_body, env)?,
                    Self::p_expr(&c.do_condition, env)?,
                ),
            )),
            WhileStatement(c) => Ok(S::new(
                pos,
                S_::mk_while(
                    Self::p_expr(&c.while_condition, env)?,
                    Self::p_block(true, &c.while_body, env)?,
                ),
            )),
            UsingStatementBlockScoped(c) => {
                let f = |e: &mut Env| -> Result<ast::Stmt> {
                    Ok(S::new(
                        pos,
                        S_::mk_using(ast::UsingStmt {
                            is_block_scoped: true,
                            has_await: !&c.using_block_await_keyword.is_missing(),
                            expr: Self::p_expr_l_with_loc(
                                ExprLocation::UsingStatement,
                                &c.using_block_expressions,
                                e,
                            )?,
                            block: Self::p_block(false, &c.using_block_body, e)?,
                        }),
                    ))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            UsingStatementFunctionScoped(c) => {
                let f = |e: &mut Env| -> Result<ast::Stmt> {
                    Ok(S::new(
                        pos,
                        S_::mk_using(ast::UsingStmt {
                            is_block_scoped: false,
                            has_await: !&c.using_function_await_keyword.is_missing(),
                            expr: Self::p_expr_with_loc(
                                ExprLocation::UsingStatement,
                                &c.using_function_expression,
                                e,
                            )?,
                            block: vec![Self::mk_noop(e)],
                        }),
                    ))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            ForStatement(c) => {
                let f = |e: &mut Env| -> Result<ast::Stmt> {
                    let ini = Self::p_expr_l(&c.for_initializer, e)?;
                    let ctr = Self::p_expr_l(&c.for_control, e)?;
                    let eol = Self::p_expr_l(&c.for_end_of_loop, e)?;
                    let blk = Self::p_block(true, &c.for_body, e)?;
                    Ok(S::new(pos, S_::mk_for(ini, ctr, eol, blk)))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            ForeachStatement(c) => {
                let f = |e: &mut Env| -> Result<ast::Stmt> {
                    let col = Self::p_expr(&c.foreach_collection, e)?;
                    let akw = match Self::token_kind(&c.foreach_await_keyword) {
                        Some(TK::Await) => Some(Self::p_pos(&c.foreach_await_keyword, e)),
                        _ => None,
                    };
                    let value = Self::p_expr(&c.foreach_value, e)?;
                    let akv = match (akw, &c.foreach_key.syntax) {
                        (Some(p), Missing) => ast::AsExpr::AwaitAsV(p, value),
                        (None, Missing) => ast::AsExpr::AsV(value),
                        (Some(p), _) => {
                            ast::AsExpr::AwaitAsKv(p, Self::p_expr(&c.foreach_key, e)?, value)
                        }
                        (None, _) => ast::AsExpr::AsKv(Self::p_expr(&c.foreach_key, e)?, value),
                    };
                    let blk = Self::p_block(true, &c.foreach_body, e)?;
                    Ok(S::new(pos, S_::mk_foreach(col, akv, blk)))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            TryStatement(c) => Ok(S::new(
                pos,
                S_::mk_try(
                    Self::p_block(false, &c.try_compound_statement, env)?,
                    Self::could_map(
                        |n: &Syntax<T, V>, e| match &n.syntax {
                            CatchClause(c) => Ok(ast::Catch(
                                Self::pos_name(&c.catch_type, e)?,
                                Self::lid_from_name(&c.catch_variable, e)?,
                                Self::p_block(true, &c.catch_body, e)?,
                            )),
                            _ => Self::missing_syntax("catch clause", n, e),
                        },
                        &c.try_catch_clauses,
                        env,
                    )?,
                    match &c.try_finally_clause.syntax {
                        FinallyClause(c) => Self::p_block(false, &c.finally_body, env)?,
                        _ => vec![],
                    },
                ),
            )),
            ReturnStatement(c) => {
                let f = |e: &mut Env| -> Result<ast::Stmt> {
                    let expr = match &c.return_expression.syntax {
                        Missing => None,
                        _ => Some(Self::p_expr_with_loc(
                            ExprLocation::RightOfReturn,
                            &c.return_expression,
                            e,
                        )?),
                    };
                    Ok(ast::Stmt::new(pos, ast::Stmt_::mk_return(expr)))
                };
                if Self::is_simple_await_expression(&c.return_expression) {
                    f(env)
                } else {
                    Self::lift_awaits_in_statement(f, node, env)
                }
            }
            GotoLabel(c) => {
                if env.is_typechecker() && !env.parser_options.po_allow_goto {
                    Self::raise_parsing_error(node, env, &syntax_error::goto_label);
                }
                Ok(S::new(
                    pos,
                    S_::mk_goto_label((
                        Self::p_pos(&c.goto_label_name, env),
                        Self::text(&c.goto_label_name, env),
                    )),
                ))
            }
            GotoStatement(c) => {
                if env.is_typechecker() && !env.parser_options.po_allow_goto {
                    Self::raise_parsing_error(node, env, &syntax_error::goto);
                }
                Ok(S::new(
                    pos,
                    S_::mk_goto(Self::p_pstring(&c.goto_statement_label_name, env)?),
                ))
            }
            EchoStatement(c) => {
                let f = |e: &mut Env| -> Result<ast::Stmt> {
                    let echo = match &c.echo_keyword.syntax {
                        QualifiedName(_) | SimpleTypeSpecifier(_) | Token(_) => {
                            let name = Self::pos_name(&c.echo_keyword, e)?;
                            ast::Expr::new(name.0.clone(), E_::mk_id(name))
                        }
                        _ => Self::missing_syntax("id", &c.echo_keyword, e)?,
                    };
                    let args = Self::could_map(Self::p_expr, &c.echo_expressions, e)?;
                    Ok(S::new(
                        pos.clone(),
                        S_::mk_expr(ast::Expr::new(
                            pos,
                            E_::mk_call(ast::CallType::Cnormal, echo, vec![], args, None),
                        )),
                    ))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            UnsetStatement(c) => {
                let f = |e: &mut Env| -> Result<ast::Stmt> {
                    let args = Self::could_map(Self::p_expr, &c.unset_variables, e)?;
                    if e.parser_options.po_disable_unset_class_const {
                        args.iter()
                            .for_each(|arg| Self::check_mutate_class_const(arg, node, e))
                    }
                    let unset = match &c.unset_keyword.syntax {
                        QualifiedName(_) | SimpleTypeSpecifier(_) | Token(_) => {
                            let name = Self::pos_name(&c.unset_keyword, e)?;
                            ast::Expr::new(name.0.clone(), E_::mk_id(name))
                        }
                        _ => Self::missing_syntax("id", &c.unset_keyword, e)?,
                    };
                    Ok(S::new(
                        pos.clone(),
                        S_::mk_expr(ast::Expr::new(
                            pos,
                            E_::mk_call(ast::CallType::Cnormal, unset, vec![], args, None),
                        )),
                    ))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            BreakStatement(_) => Ok(S::new(pos, S_::Break)),
            ContinueStatement(_) => Ok(S::new(pos, S_::Continue)),
            ConcurrentStatement(c) => {
                let (lifted_awaits, S(stmt_pos, stmt)) = Self::with_new_concurrent_scrope(
                    |e: &mut Env| Self::p_stmt(&c.concurrent_statement, e),
                    env,
                )?;
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
                                if let S(p1, S_::Expr(expr)) = n {
                                    if let E(p2, E_::Binop(bop)) = *expr {
                                        if let (Eq(op), e1, e2) = *bop {
                                            let tmp_n = E::mk_lvar(&e2.0, &(tv.1));
                                            if tmp_n.lvar_name() != e2.lvar_name() {
                                                let new_n = S::new(
                                                    p1.clone(),
                                                    S_::mk_expr(E::new(
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
                                            let assign_stmt = S::new(
                                                p1,
                                                S_::mk_expr(E::new(
                                                    p2,
                                                    E_::mk_binop(Eq(op), e1, tmp_n),
                                                )),
                                            );
                                            assign_stmts.push(assign_stmt);
                                            continue;
                                        }
                                    }
                                }

                                Self::failwith("Expect assignment stmt")?;
                            } else {
                                Self::raise_parsing_error_pos(
                                    &stmt_pos,
                                    env,
                                    &syntax_error::statement_without_await_in_concurrent_block,
                                );
                                body_stmts.push(n)
                            }
                        }
                        body_stmts.append(&mut assign_stmts);
                        S::new(stmt_pos, S_::mk_block(body_stmts))
                    }
                    _ => Self::failwith("Unexpected concurrent stmt structure")?,
                };
                Ok(S::new(pos, S_::mk_awaitall(lifted_awaits, vec![stmt])))
            }
            MarkupSection(_) => Self::p_markup(node, env),
            _ => Self::missing_syntax_(
                Some(S::new(env.mk_none_pos(), S_::Noop)),
                "statement",
                node,
                env,
            ),
        }
    }

    fn check_mutate_class_const(e: &ast::Expr, node: &Syntax<T, V>, env: &mut Env) {
        match &e.1 {
            E_::ArrayGet(c) if c.1.is_some() => Self::check_mutate_class_const(&c.0, node, env),
            E_::ClassConst(_) => {
                Self::raise_parsing_error(node, env, &syntax_error::const_mutation)
            }
            _ => {}
        }
    }

    fn is_hashbang(node: &Syntax<T, V>, env: &Env) -> bool {
        let text = Self::text_str(node, env);
        lazy_static! {
            static ref RE: regex::Regex = regex::Regex::new("^#!.*\n").unwrap();
        }
        text.lines().nth(1).is_none() && // only one line
        RE.is_match(text)
    }

    fn p_markup(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Stmt> {
        match &node.syntax {
            MarkupSection(c) => {
                let markup_text = &c.markup_text;
                let pos = Self::p_pos(node, env);
                let has_dot_hack_extension = pos.filename().has_extension("hack");
                if has_dot_hack_extension {
                    Self::raise_parsing_error(node, env, &syntax_error::error1060);
                } else if markup_text.width() > 0 && !Self::is_hashbang(&markup_text, env) {
                    Self::raise_parsing_error(node, env, &syntax_error::error1001);
                }
                let stmt_ = ast::Stmt_::mk_markup((pos.clone(), Self::text(&markup_text, env)));
                Ok(ast::Stmt::new(pos, stmt_))
            }
            _ => Self::failwith("invalid node"),
        }
    }

    fn p_modifiers<F: Fn(R, modifier::Kind) -> R, R>(
        on_kind: F,
        mut init: R,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<(modifier::KindSet, R)> {
        let nodes = Self::as_list(node);
        let mut kind_set = modifier::KindSet::new();
        for n in nodes.iter() {
            let token_kind = Self::token_kind(n).map_or(None, modifier::from_token_kind);
            match token_kind {
                Some(kind) => {
                    kind_set.add(kind);
                    init = on_kind(init, kind);
                }
                _ => Self::missing_syntax("kind", n, env)?,
            }
        }
        Ok((kind_set, init))
    }

    fn p_kinds(node: &Syntax<T, V>, env: &mut Env) -> Result<modifier::KindSet> {
        Self::p_modifiers(|_, _| {}, (), node, env).map(|r| r.0)
    }

    // TODO: change name to map_flatten after porting
    #[inline]
    fn could_map<R, F>(f: F, node: &Syntax<T, V>, env: &mut Env) -> Result<Vec<R>>
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> Result<R>,
    {
        Self::map_flatten_(f, node, env, vec![])
    }

    #[inline]
    fn map_flatten_<R, F>(f: F, node: &Syntax<T, V>, env: &mut Env, acc: Vec<R>) -> Result<Vec<R>>
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> Result<R>,
    {
        let op = |mut v: Vec<R>, a| {
            v.push(a);
            v
        };
        Self::map_fold(&f, &op, node, env, acc)
    }

    fn map_fold<A, R, F, O>(f: &F, op: &O, node: &Syntax<T, V>, env: &mut Env, acc: A) -> Result<A>
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> Result<R>,
        O: Fn(A, R) -> A,
    {
        match &node.syntax {
            Missing => Ok(acc),
            SyntaxList(xs) => {
                let mut a = acc;
                for x in xs.iter() {
                    a = Self::map_fold(f, op, &x, env, a)?;
                }
                Ok(a)
            }
            ListItem(x) => Ok(op(acc, f(&x.list_item, env)?)),
            _ => Ok(op(acc, f(node, env)?)),
        }
    }

    fn p_visibility(node: &Syntax<T, V>, env: &mut Env) -> Result<Option<ast::Visibility>> {
        let first_vis =
            |r: Option<ast::Visibility>, kind| r.or_else(|| modifier::to_visibility(kind));
        Self::p_modifiers(first_vis, None, node, env).map(|r| r.1)
    }

    fn p_visibility_or(
        node: &Syntax<T, V>,
        env: &mut Env,
        default: ast::Visibility,
    ) -> Result<ast::Visibility> {
        Self::p_visibility(node, env).map(|v| v.unwrap_or(default))
    }

    fn p_visibility_last_win(
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<Option<ast::Visibility>> {
        let last_vis = |r, kind| modifier::to_visibility(kind).or(r);
        Self::p_modifiers(last_vis, None, node, env).map(|r| r.1)
    }

    fn p_visibility_last_win_or(
        node: &Syntax<T, V>,
        env: &mut Env,
        default: ast::Visibility,
    ) -> Result<ast::Visibility> {
        Self::p_visibility_last_win(node, env).map(|v| v.unwrap_or(default))
    }

    fn has_soft(attrs: &[ast::UserAttribute]) -> bool {
        attrs.iter().any(|attr| attr.name.1 == special_attrs::SOFT)
    }

    fn soften_hint(attrs: &[ast::UserAttribute], hint: ast::Hint) -> ast::Hint {
        if Self::has_soft(attrs) {
            ast::Hint::new(hint.0.clone(), ast::Hint_::Hsoft(hint))
        } else {
            hint
        }
    }

    fn p_fun_param_default_value(node: &Syntax<T, V>, env: &mut Env) -> Result<Option<ast::Expr>> {
        match &node.syntax {
            SimpleInitializer(c) => {
                Self::mp_optional(Self::p_expr, &c.simple_initializer_value, env)
            }
            _ => Ok(None),
        }
    }

    fn p_param_kind(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::ParamKind> {
        match Self::token_kind(node) {
            Some(TK::Inout) => Ok(ast::ParamKind::Pinout),
            _ => Self::missing_syntax("param kind", node, env),
        }
    }

    fn param_template(node: &Syntax<T, V>, env: &Env) -> ast::FunParam {
        let pos = Self::p_pos(node, env);
        ast::FunParam {
            annotation: pos.clone(),
            type_hint: ast::TypeHint((), None),
            is_variadic: false,
            pos,
            name: Self::text(node, env),
            expr: None,
            callconv: None,
            user_attributes: vec![],
            visibility: None,
        }
    }

    fn p_fun_param(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::FunParam> {
        match &node.syntax {
            ParameterDeclaration(c) => {
                let parameter_attribute = &c.parameter_attribute;
                let parameter_visibility = &c.parameter_visibility;
                let parameter_call_convention = &c.parameter_call_convention;
                let parameter_type = &c.parameter_type;
                let parameter_name = &c.parameter_name;
                let parameter_default_value = &c.parameter_default_value;
                let (is_variadic, name) = match &parameter_name.syntax {
                    DecoratedExpression(c) => {
                        let decorated_expression_decorator = &c.decorated_expression_decorator;
                        let decorated_expression_expression = &c.decorated_expression_expression;
                        let decorator = Self::text_str(decorated_expression_decorator, env);
                        match &decorated_expression_expression.syntax {
                            DecoratedExpression(c) => {
                                let nested_expression = &c.decorated_expression_expression;
                                let nested_decorator =
                                    Self::text_str(&c.decorated_expression_decorator, env);
                                (
                                    decorator == "..." || nested_decorator == "...",
                                    nested_expression,
                                )
                            }
                            _ => (decorator == "...", decorated_expression_expression),
                        }
                    }
                    _ => (false, parameter_name),
                };
                let user_attributes = Self::p_user_attributes(&parameter_attribute, env)?;
                let hint = Self::mp_optional(Self::p_hint, parameter_type, env)?
                    .map(|h| Self::soften_hint(&user_attributes, h));
                if is_variadic && !user_attributes.is_empty() {
                    Self::raise_parsing_error(
                        node,
                        env,
                        &syntax_error::no_attributes_on_variadic_parameter,
                    );
                }
                let pos = Self::p_pos(name, env);
                Ok(ast::FunParam {
                    annotation: pos.clone(),
                    type_hint: ast::TypeHint((), hint),
                    user_attributes,
                    is_variadic,
                    pos,
                    name: Self::text(name, env),
                    expr: Self::p_fun_param_default_value(parameter_default_value, env)?,
                    callconv: Self::mp_optional(
                        Self::p_param_kind,
                        parameter_call_convention,
                        env,
                    )?,
                    /* implicit field via constructor parameter.
                     * This is always None except for constructors and the modifier
                     * can be only Public or Protected or Private.
                     */
                    visibility: Self::p_visibility(parameter_visibility, env)?,
                })
            }
            VariadicParameter(_) => {
                let mut param = Self::param_template(node, env);
                param.is_variadic = true;
                Ok(param)
            }
            Token(_) if Self::text_str(node, env) == "..." => {
                let mut param = Self::param_template(node, env);
                param.is_variadic = true;
                Ok(param)
            }
            _ => Self::missing_syntax("function parameter", node, env),
        }
    }

    fn p_tconstraint_ty(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Hint> {
        match &node.syntax {
            TypeConstraint(c) => Self::p_hint(&c.constraint_type, env),
            _ => Self::missing_syntax("type constraint", node, env),
        }
    }

    fn p_tconstraint(
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<(ast::ConstraintKind, ast::Hint)> {
        match &node.syntax {
            TypeConstraint(c) => Ok((
                match Self::token_kind(&c.constraint_keyword) {
                    Some(TK::As) => ast::ConstraintKind::ConstraintAs,
                    Some(TK::Super) => ast::ConstraintKind::ConstraintSuper,
                    Some(TK::Equal) => ast::ConstraintKind::ConstraintEq,
                    _ => Self::missing_syntax("constraint operator", &c.constraint_keyword, env)?,
                },
                Self::p_hint(&c.constraint_type, env)?,
            )),
            _ => Self::missing_syntax("type constraint", node, env),
        }
    }

    fn p_tparam(is_class: bool, node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Tparam> {
        match &node.syntax {
            TypeParameter(c) => {
                let user_attributes = Self::p_user_attributes(&c.type_attribute_spec, env)?;
                let is_reified = !&c.type_reified.is_missing();
                if is_class && is_reified {
                    let type_name = Self::text(&c.type_name, env);
                    env.cls_reified_generics().insert(type_name);
                }
                let variance = match Self::token_kind(&c.type_variance) {
                    Some(TK::Plus) => ast::Variance::Covariant,
                    Some(TK::Minus) => ast::Variance::Contravariant,
                    _ => ast::Variance::Invariant,
                };
                if is_reified && variance != ast::Variance::Invariant {
                    Self::raise_parsing_error(
                        node,
                        env,
                        &syntax_error::non_invariant_reified_generic,
                    );
                }
                let reified = match (is_reified, Self::has_soft(&user_attributes)) {
                    (true, true) => ast::ReifyKind::SoftReified,
                    (true, false) => ast::ReifyKind::Reified,
                    _ => ast::ReifyKind::Erased,
                };
                Ok(ast::Tparam {
                    variance,
                    name: Self::pos_name(&c.type_name, env)?,
                    constraints: Self::could_map(Self::p_tconstraint, &c.type_constraints, env)?,
                    reified,
                    user_attributes,
                })
            }
            _ => Self::missing_syntax("type parameter", node, env),
        }
    }

    fn p_tparam_l(is_class: bool, node: &Syntax<T, V>, env: &mut Env) -> Result<Vec<ast::Tparam>> {
        match &node.syntax {
            Missing => Ok(vec![]),
            TypeParameters(c) => Self::could_map(
                |n, e| Self::p_tparam(is_class, n, e),
                &c.type_parameters_parameters,
                env,
            ),
            _ => Self::missing_syntax("type parameter", node, env),
        }
    }

    fn p_fun_hdr(node: &Syntax<T, V>, env: &mut Env) -> Result<FunHdr> {
        match &node.syntax {
            FunctionDeclarationHeader(c) => {
                let function_modifiers = &c.function_modifiers;
                let function_name = &c.function_name;
                let function_where_clause = &c.function_where_clause;
                let function_type_parameter_list = &c.function_type_parameter_list;
                let function_parameter_list = &c.function_parameter_list;
                let function_type = &c.function_type;
                let is_autoload = Self::text_str(function_name, env)
                    .eq_ignore_ascii_case(special_functions::AUTOLOAD);
                if function_name.value.is_missing() {
                    Self::raise_parsing_error(function_name, env, &syntax_error::empty_method_name);
                }
                let num_params = Self::syntax_to_list(false, function_parameter_list).len();
                if is_autoload && num_params > 1 {
                    Self::raise_parsing_error(
                        node,
                        env,
                        &syntax_error::autoload_takes_one_argument,
                    );
                }
                let kinds = Self::p_kinds(function_modifiers, env)?;
                let has_async = kinds.has(modifier::ASYNC);
                let has_coroutine = kinds.has(modifier::COROUTINE);
                let parameters = Self::could_map(Self::p_fun_param, function_parameter_list, env)?;
                let return_type = Self::mp_optional(Self::p_hint, function_type, env)?;
                let suspension_kind =
                    Self::mk_suspension_kind_(node, env, has_async, has_coroutine);
                let name = Self::pos_name(function_name, env)?;
                let constrs = Self::p_where_constraint(false, node, function_where_clause, env)?;
                let type_parameters = Self::p_tparam_l(false, function_type_parameter_list, env)?;
                Ok(FunHdr {
                    suspension_kind,
                    name,
                    constrs,
                    type_parameters,
                    parameters,
                    return_type,
                })
            }
            LambdaSignature(c) => {
                let mut header = FunHdr::make_empty(env);
                header.parameters = Self::could_map(Self::p_fun_param, &c.lambda_parameters, env)?;
                header.return_type = Self::mp_optional(Self::p_hint, &c.lambda_type, env)?;
                Ok(header)
            }
            Token(_) => Ok(FunHdr::make_empty(env)),
            _ => Self::missing_syntax("function header", node, env),
        }
    }

    fn determine_variadicity(params: &[ast::FunParam]) -> ast::FunVariadicity {
        use aast::FunVariadicity::*;
        if let Some(x) = params.last() {
            match (x.is_variadic, &x.name) {
                (false, _) => FVnonVariadic,
                (true, name) if name == "..." => FVellipsis(x.pos.clone()),
                (true, _) => FVvariadicArg(x.clone()),
            }
        } else {
            FVnonVariadic
        }
    }

    fn p_fun_pos(node: &Syntax<T, V>, env: &Env) -> Pos {
        let get_pos = |n: &Syntax<T, V>, p: Pos| -> Pos {
            if let FunctionDeclarationHeader(c1) = &n.syntax {
                if !c1.function_keyword.is_missing() {
                    return Pos::btw_nocheck(Self::p_pos(&c1.function_keyword, env), p);
                }
            }
            p
        };
        let p = Self::p_pos(node, env);
        match &node.syntax {
            FunctionDeclaration(c) if env.codegen() => get_pos(&c.function_declaration_header, p),
            MethodishDeclaration(c) if env.codegen() => {
                get_pos(&c.methodish_function_decl_header, p)
            }
            _ => p,
        }
    }

    fn p_block(remove_noop: bool, node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Block> {
        let ast::Stmt(p, stmt_) = Self::p_stmt(node, env)?;
        if let ast::Stmt_::Block(blk) = stmt_ {
            if remove_noop && blk.len() == 1 && blk[0].1.is_noop() {
                return Ok(vec![]);
            }
            return Ok(blk);
        } else {
            Ok(vec![ast::Stmt(p, stmt_)])
        }
    }

    fn mk_noop(env: &Env) -> ast::Stmt {
        ast::Stmt::noop(env.mk_none_pos())
    }

    fn p_function_body(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Block> {
        let mk_noop_result = |e: &Env| Ok(vec![Self::mk_noop(e)]);
        let f = |e: &mut Env| -> Result<ast::Block> {
            match &node.syntax {
                Missing => Ok(vec![]),
                CompoundStatement(c) => {
                    let compound_statements = &c.compound_statements.syntax;
                    let compound_right_brace = &c.compound_right_brace.syntax;
                    match (compound_statements, compound_right_brace) {
                        (Missing, Token(_)) => mk_noop_result(e),
                        (SyntaxList(t), _) if t.len() == 1 && t[0].is_yield() => {
                            e.saw_yield = true;
                            mk_noop_result(e)
                        }
                        _ => {
                            if !e.top_level_statements
                                && ((e.file_mode() == file_info::Mode::Mdecl && !e.codegen())
                                    || e.quick_mode)
                            {
                                mk_noop_result(e)
                            } else {
                                Self::p_block(false /*remove noop*/, node, e)
                            }
                        }
                    }
                }
                _ => {
                    let f = |e: &mut Env| {
                        let expr = Self::p_expr(node, e)?;
                        Ok(ast::Stmt::new(
                            expr.0.clone(),
                            ast::Stmt_::mk_return(Some(expr)),
                        ))
                    };
                    if Self::is_simple_await_expression(node) {
                        Ok(vec![f(e)?])
                    } else {
                        Ok(vec![Self::lift_awaits_in_statement(f, node, e)?])
                    }
                }
            }
        };
        Self::with_new_nonconcurrent_scrope(f, env)
    }

    fn mk_suspension_kind(
        node: &Syntax<T, V>,
        env: &mut Env,
        async_keyword: &Syntax<T, V>,
        coroutine_keyword: &Syntax<T, V>,
    ) -> SuspensionKind {
        Self::mk_suspension_kind_(
            node,
            env,
            !async_keyword.is_missing(),
            !coroutine_keyword.is_missing(),
        )
    }

    fn mk_suspension_kind_(
        node: &Syntax<T, V>,
        env: &mut Env,
        has_async: bool,
        has_coroutine: bool,
    ) -> SuspensionKind {
        use SuspensionKind::*;
        match (has_async, has_coroutine) {
            (false, false) => SKSync,
            (true, false) => SKAsync,
            (false, true) => SKCoroutine,
            (true, true) => {
                Self::raise_parsing_error(node, env, "Coroutine functions may not be async");
                SKCoroutine
            }
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
            (SKCoroutine, _) => FCoroutine,
        }
    }

    fn process_attribute_constructor_call(
        node: &Syntax<T, V>,
        constructor_call_argument_list: &Syntax<T, V>,
        constructor_call_type: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<ast::UserAttribute> {
        let name = Self::pos_name(constructor_call_type, env)?;
        if name.1.eq_ignore_ascii_case("__reified")
            || name.1.eq_ignore_ascii_case("__hasreifiedparent")
        {
            Self::raise_parsing_error(node, env, &syntax_error::reified_attribute);
        } else if name.1.eq_ignore_ascii_case(special_attrs::SOFT)
            && Self::as_list(constructor_call_argument_list).len() > 0
        {
            Self::raise_parsing_error(node, env, &syntax_error::soft_no_arguments);
        }
        let params = Self::could_map(
            |n: &Syntax<T, V>, e: &mut Env| -> Result<ast::Expr> {
                if let ScopeResolutionExpression(c) = &n.syntax {
                    if let Some(TK::Name) = Self::token_kind(&c.scope_resolution_name) {
                        Self::raise_parsing_error(
                            n,
                            e,
                            &syntax_error::constants_as_attribute_arguments,
                        );
                    }
                } else if let Some(TK::Name) = Self::token_kind(n) {
                    Self::raise_parsing_error(
                        n,
                        e,
                        &syntax_error::constants_as_attribute_arguments,
                    );
                }
                Self::p_expr(n, e)
            },
            constructor_call_argument_list,
            env,
        )?;
        Ok(ast::UserAttribute { name, params })
    }

    fn p_user_attribute(node: &Syntax<T, V>, env: &mut Env) -> Result<Vec<ast::UserAttribute>> {
        let p_attr = |n: &Syntax<T, V>, e: &mut Env| -> Result<ast::UserAttribute> {
            match &n.syntax {
                ConstructorCall(c) => Self::process_attribute_constructor_call(
                    node,
                    &c.constructor_call_argument_list,
                    &c.constructor_call_type,
                    e,
                ),
                _ => Self::missing_syntax("attribute", node, e),
            }
        };
        match &node.syntax {
            FileAttributeSpecification(c) => {
                Self::could_map(p_attr, &c.file_attribute_specification_attributes, env)
            }
            OldAttributeSpecification(c) => {
                Self::could_map(p_attr, &c.old_attribute_specification_attributes, env)
            }
            AttributeSpecification(c) => Self::could_map(
                |n: &Syntax<T, V>, e: &mut Env| -> Result<ast::UserAttribute> {
                    match &n.syntax {
                        Attribute(c) => p_attr(&c.attribute_attribute_name, e),
                        _ => Self::missing_syntax("attribute", node, e),
                    }
                },
                &c.attribute_specification_attributes,
                env,
            ),
            _ => Self::missing_syntax("attribute specification", node, env),
        }
    }

    fn p_user_attributes(node: &Syntax<T, V>, env: &mut Env) -> Result<Vec<ast::UserAttribute>> {
        Self::map_fold(
            &Self::p_user_attribute,
            &|mut acc: Vec<ast::UserAttribute>, mut x: Vec<ast::UserAttribute>| {
                acc.append(&mut x);
                acc
            },
            node,
            env,
            vec![],
        )
    }

    fn mp_yielding<F, R>(p: F, node: &Syntax<T, V>, env: &mut Env) -> Result<(R, bool)>
    where
        F: FnOnce(&Syntax<T, V>, &mut Env) -> Result<R>,
    {
        let outer_saw_yield = env.saw_yield;
        env.saw_yield = false;
        let r = p(node, env);
        let saw_yield = env.saw_yield;
        env.saw_yield = outer_saw_yield;
        Ok((r?, saw_yield))
    }

    fn mk_empty_ns_env(env: &Env) -> RcOc<NamespaceEnv> {
        RcOc::clone(&env.empty_ns_env)
    }

    fn extract_docblock(node: &Syntax<T, V>, env: &Env) -> Option<DocComment> {
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
        fn parse(str: &str, start: usize, state: ScanState, idx: usize) -> Option<String> {
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
                            None => break Some(String::from(&str[s.0..s.2 + 1])),
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
        parse(str, 0, Free, 0).map(oxidized::doc_comment::DocComment::new)
    }

    fn p_xhp_child(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::XhpChild> {
        use ast::XhpChild::*;
        use ast::XhpChildOp::*;
        match &node.syntax {
            Token(_) => Self::pos_name(node, env).map(ChildName),
            PostfixUnaryExpression(c) => {
                let operand = Self::p_xhp_child(&c.postfix_unary_operand, env)?;
                let operator = match Self::token_kind(&c.postfix_unary_operator) {
                    Some(TK::Question) => ChildQuestion,
                    Some(TK::Plus) => ChildPlus,
                    Some(TK::Star) => ChildStar,
                    _ => Self::missing_syntax("xhp children operator", node, env)?,
                };
                Ok(ChildUnary(Box::new(operand), operator))
            }
            BinaryExpression(c) => {
                let left = Self::p_xhp_child(&c.binary_left_operand, env)?;
                let right = Self::p_xhp_child(&c.binary_right_operand, env)?;
                Ok(ChildBinary(Box::new(left), Box::new(right)))
            }
            XHPChildrenParenthesizedList(c) => {
                let children = Self::as_list(&c.xhp_children_list_xhp_children);
                let children: std::result::Result<Vec<_>, _> =
                    children.iter().map(|c| Self::p_xhp_child(c, env)).collect();
                Ok(ChildList(children?))
            }
            _ => Self::missing_syntax("xhp children", node, env),
        }
    }

    fn p_class_elt_(class: &mut ast::Class_, node: &Syntax<T, V>, env: &mut Env) -> Result<()> {
        let doc_comment_opt = Self::extract_docblock(node, env);
        let has_fun_header = |m: &MethodishDeclarationChildren<T, V>| {
            if let FunctionDeclarationHeader(_) = m.methodish_function_decl_header.syntax {
                return true;
            }
            false
        };
        let has_fun_header_mtr = |m: &MethodishTraitResolutionChildren<T, V>| {
            if let FunctionDeclarationHeader(_) = m.methodish_trait_function_decl_header.syntax {
                return true;
            }
            false
        };
        let p_method_vis = |node: &Syntax<T, V>,
                            name_pos: &Pos,
                            env: &mut Env|
         -> Result<ast::Visibility> {
            match Self::p_visibility_last_win(node, env)? {
                None => {
                    Self::raise_hh_error(env, Naming::method_needs_visibility(name_pos.clone()));
                    Ok(ast::Visibility::Public)
                }
                Some(v) => Ok(v),
            }
        };
        match &node.syntax {
            ConstDeclaration(c) => {
                // TODO: make wrap `type_` `doc_comment` by `Rc` in ClassConst to avoid clone
                let type_ = Self::mp_optional(Self::p_hint, &c.const_type_specifier, env)?;
                // using map_fold can save one Vec allocation, but ocaml's behavior is that
                // if anything throw, it will discard all lowered elements. So adding to class
                // must be at the last.
                let mut class_consts = Self::could_map(
                    |n: &Syntax<T, V>, e: &mut Env| -> Result<ast::ClassConst> {
                        match &n.syntax {
                            ConstantDeclarator(c) => {
                                let id = Self::pos_name(&c.constant_declarator_name, e)?;
                                let expr = if n.is_abstract() {
                                    None
                                } else {
                                    Self::mp_optional(
                                        Self::p_simple_initializer,
                                        &c.constant_declarator_initializer,
                                        e,
                                    )?
                                };
                                Ok(ast::ClassConst {
                                    type_: type_.clone(),
                                    id,
                                    expr,
                                    doc_comment: doc_comment_opt.clone(),
                                })
                            }
                            _ => Self::missing_syntax("constant declarator", n, e),
                        }
                    },
                    &c.const_declarators,
                    env,
                )?;
                Ok(class.consts.append(&mut class_consts))
            }
            TypeConstDeclaration(c) => {
                if !c.type_const_type_parameters.is_missing() {
                    Self::raise_parsing_error(node, env, &syntax_error::tparams_in_tconst);
                }
                let user_attributes = Self::p_user_attributes(&c.type_const_attribute_spec, env)?;
                let type__ = Self::mp_optional(Self::p_hint, &c.type_const_type_specifier, env)?
                    .map(|hint| Self::soften_hint(&user_attributes, hint));
                let kinds = Self::p_kinds(&c.type_const_modifiers, env)?;
                let name = Self::pos_name(&c.type_const_name, env)?;
                let constraint =
                    Self::mp_optional(Self::p_tconstraint_ty, &c.type_const_type_constraint, env)?;
                let span = Self::p_pos(node, env);
                let has_abstract = kinds.has(modifier::ABSTRACT);
                let (type_, abstract_kind) = match (has_abstract, &constraint, &type__) {
                    (false, _, None) => {
                        Self::raise_hh_error(
                            env,
                            NastCheck::not_abstract_without_typeconst(name.0.clone()),
                        );
                        (constraint.clone(), ast::TypeconstAbstractKind::TCConcrete)
                    }
                    (false, None, Some(_)) => (type__, ast::TypeconstAbstractKind::TCConcrete),
                    (false, Some(_), Some(_)) => {
                        (type__, ast::TypeconstAbstractKind::TCPartiallyAbstract)
                    }
                    (true, _, None) => (
                        type__.clone(),
                        ast::TypeconstAbstractKind::TCAbstract(type__),
                    ),
                    (true, _, Some(_)) => (None, ast::TypeconstAbstractKind::TCAbstract(type__)),
                };
                Ok(class.typeconsts.push(ast::ClassTypeconst {
                    abstract_: abstract_kind,
                    name,
                    constraint,
                    type_,
                    user_attributes,
                    span,
                    doc_comment: doc_comment_opt,
                }))
            }
            PropertyDeclaration(c) => {
                let user_attributes = Self::p_user_attributes(&c.property_attribute_spec, env)?;
                let type_ = Self::mp_optional(Self::p_hint, &c.property_type, env)?
                    .map(|t| Self::soften_hint(&user_attributes, t));
                let kinds = Self::p_kinds(&c.property_modifiers, env)?;
                let vis = Self::p_visibility_last_win_or(
                    &c.property_modifiers,
                    env,
                    ast::Visibility::Public,
                )?;
                let doc_comment = if env.quick_mode {
                    None
                } else {
                    doc_comment_opt
                };
                let name_exprs = Self::could_map(
                    |n, e| -> Result<(Pos, ast::Sid, Option<ast::Expr>)> {
                        match &n.syntax {
                            PropertyDeclarator(c) => {
                                let name = Self::pos_name_(&c.property_name, e, Some('$'))?;
                                let pos = Self::p_pos(n, e);
                                let expr = Self::mp_optional(
                                    Self::p_simple_initializer,
                                    &c.property_initializer,
                                    e,
                                )?;
                                Ok((pos, name, expr))
                            }
                            _ => Self::missing_syntax("property declarator", n, e),
                        }
                    },
                    &c.property_declarators,
                    env,
                )?;

                let mut i = 0;
                for name_expr in name_exprs.into_iter() {
                    class.vars.push(ast::ClassVar {
                        final_: kinds.has(modifier::FINAL),
                        xhp_attr: None,
                        abstract_: kinds.has(modifier::ABSTRACT),
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
                    i += 1;
                }
                Ok(())
            }
            MethodishDeclaration(c) if has_fun_header(c) => {
                let classvar_init = |param: &ast::FunParam| -> (ast::Stmt, ast::ClassVar) {
                    let cvname = Self::drop_prefix(&param.name, '$');
                    let p = &param.pos;
                    let span = match &param.expr {
                        Some(ast::Expr(pos_end, _)) => {
                            Pos::btw(p, pos_end).unwrap_or_else(|_| p.clone())
                        }
                        _ => p.clone(),
                    };
                    let e = |expr_: ast::Expr_| -> ast::Expr { ast::Expr::new(p.clone(), expr_) };
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
                                )),
                                e(E_::mk_lvar(lid(&param.name))),
                            ))),
                        ),
                        ast::ClassVar {
                            final_: false,
                            xhp_attr: None,
                            abstract_: false,
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
                let header = &c.methodish_function_decl_header;
                let h = match &header.syntax {
                    FunctionDeclarationHeader(h) => h,
                    _ => panic!(),
                };
                let hdr = Self::p_fun_hdr(header, env)?;
                if hdr.name.1 == special_members::__CONSTRUCT && !hdr.type_parameters.is_empty() {
                    Self::raise_parsing_error(
                        header,
                        env,
                        &syntax_error::no_generics_on_constructors,
                    );
                }
                let (mut member_init, mut member_def): (Vec<ast::Stmt>, Vec<ast::ClassVar>) = hdr
                    .parameters
                    .iter()
                    .filter_map(|p| p.visibility.map(|_| classvar_init(p)))
                    .unzip();

                let kinds = Self::p_kinds(&h.function_modifiers, env)?;
                let visibility = p_method_vis(&h.function_modifiers, &hdr.name.0, env)?;
                let is_static = kinds.has(modifier::STATIC);
                *env.in_static_method() = is_static;
                let (mut body, body_has_yield) =
                    Self::mp_yielding(Self::p_function_body, &c.methodish_function_body, env)?;
                if env.codegen() {
                    member_init.reverse();
                }
                member_init.append(&mut body);
                let body = member_init;
                *env.in_static_method() = false;
                let is_abstract = kinds.has(modifier::ABSTRACT);
                let is_external = !is_abstract && c.methodish_function_body.is_external();
                let user_attributes = Self::p_user_attributes(&c.methodish_attribute, env)?;
                let method = ast::Method_ {
                    span: Self::p_fun_pos(node, env),
                    annotation: (),
                    final_: kinds.has(modifier::FINAL),
                    abstract_: is_abstract,
                    static_: is_static,
                    name: hdr.name,
                    visibility,
                    tparams: hdr.type_parameters,
                    where_constraints: hdr.constrs,
                    variadic: Self::determine_variadicity(&hdr.parameters),
                    params: hdr.parameters,
                    body: ast::FuncBody {
                        annotation: (),
                        ast: body,
                    },
                    fun_kind: Self::mk_fun_kind(hdr.suspension_kind, body_has_yield),
                    user_attributes,
                    ret: ast::TypeHint((), hdr.return_type),
                    external: is_external,
                    doc_comment: doc_comment_opt,
                };
                class.vars.append(&mut member_def);
                Ok(class.methods.push(method))
            }
            MethodishTraitResolution(c) if has_fun_header_mtr(c) => {
                let header = &c.methodish_trait_function_decl_header;
                let h = match &header.syntax {
                    FunctionDeclarationHeader(h) => h,
                    _ => panic!(),
                };
                let hdr = Self::p_fun_hdr(header, env)?;
                let kind = Self::p_kinds(&h.function_modifiers, env)?;
                let (qualifier, name) = match &c.methodish_trait_name.syntax {
                    ScopeResolutionExpression(c) => (
                        Self::p_hint(&c.scope_resolution_qualifier, env)?,
                        Self::p_pstring(&c.scope_resolution_name, env)?,
                    ),
                    _ => Self::missing_syntax("trait method redeclaration", node, env)?,
                };
                let user_attributes = Self::p_user_attributes(&c.methodish_trait_attribute, env)?;
                let visibility = p_method_vis(&h.function_modifiers, &hdr.name.0, env)?;
                let mtr = ast::MethodRedeclaration {
                    final_: kind.has(modifier::FINAL),
                    abstract_: kind.has(modifier::ABSTRACT),
                    static_: kind.has(modifier::STATIC),
                    visibility,
                    name: hdr.name,
                    tparams: hdr.type_parameters,
                    where_constraints: hdr.constrs,
                    variadic: Self::determine_variadicity(&hdr.parameters),
                    params: hdr.parameters,
                    fun_kind: Self::mk_fun_kind(hdr.suspension_kind, false),
                    ret: ast::TypeHint((), hdr.return_type),
                    trait_: qualifier,
                    method: name,
                    user_attributes,
                };
                class.method_redeclarations.push(mtr);
                Ok(())
            }
            TraitUseConflictResolution(c) => {
                type Ret = Result<Either<ast::InsteadofAlias, ast::UseAsAlias>>;
                let p_item = |n: &Syntax<T, V>, e: &mut Env| -> Ret {
                    match &n.syntax {
                        TraitUsePrecedenceItem(c) => {
                            let removed_names = &c.trait_use_precedence_item_removed_names;
                            let (qualifier, name) = match &c.trait_use_precedence_item_name.syntax {
                                ScopeResolutionExpression(c) => (
                                    Self::pos_name(&c.scope_resolution_qualifier, e)?,
                                    Self::p_pstring(&c.scope_resolution_name, e)?,
                                ),
                                _ => Self::missing_syntax("trait use precedence item", n, e)?,
                            };
                            let removed_names = Self::could_map(Self::pos_name, removed_names, e)?;
                            Self::raise_hh_error(e, Naming::unsupported_instead_of(name.0.clone()));
                            Ok(Either::Left(ast::InsteadofAlias(
                                qualifier,
                                name,
                                removed_names,
                            )))
                        }
                        TraitUseAliasItem(c) => {
                            let aliasing_name = &c.trait_use_alias_item_aliasing_name;
                            let modifiers = &c.trait_use_alias_item_modifiers;
                            let aliased_name = &c.trait_use_alias_item_aliased_name;
                            let (qualifier, name) = match &aliasing_name.syntax {
                                ScopeResolutionExpression(c) => (
                                    Some(Self::pos_name(&c.scope_resolution_qualifier, e)?),
                                    Self::p_pstring(&c.scope_resolution_name, e)?,
                                ),
                                _ => (None, Self::p_pstring(aliasing_name, e)?),
                            };
                            let (kinds, mut vis_raw) = Self::p_modifiers(
                                |mut acc, kind| -> Vec<ast::UseAsVisibility> {
                                    if let Some(v) = modifier::to_use_as_visibility(kind) {
                                        acc.push(v);
                                    }
                                    acc
                                },
                                vec![],
                                modifiers,
                                e,
                            )?;
                            let vis = if kinds.is_empty() || kinds.has_any(modifier::VISIBILITIES) {
                                vis_raw
                            } else {
                                let mut v = vec![ast::UseAsVisibility::UseAsPublic];
                                v.append(&mut vis_raw);
                                v
                            };
                            let aliased_name = if !aliased_name.is_missing() {
                                Some(Self::pos_name(aliased_name, e)?)
                            } else {
                                None
                            };
                            Self::raise_hh_error(
                                e,
                                Naming::unsupported_trait_use_as(name.0.clone()),
                            );
                            Ok(Either::Right(ast::UseAsAlias(
                                qualifier,
                                name,
                                aliased_name,
                                vis,
                            )))
                        }
                        _ => Self::missing_syntax("trait use conflict resolution item", n, e),
                    }
                };
                let mut uses =
                    Self::could_map(Self::p_hint, &c.trait_use_conflict_resolution_names, env)?;
                let elts = Self::could_map(p_item, &c.trait_use_conflict_resolution_clauses, env)?;
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
                let mut uses = Self::could_map(Self::p_hint, &c.trait_use_names, env)?;
                Ok(class.uses.append(&mut uses))
            }
            RequireClause(c) => {
                let hint = Self::p_hint(&c.require_name, env)?;
                let is_extends = match Self::token_kind(&c.require_kind) {
                    Some(TK::Implements) => false,
                    Some(TK::Extends) => true,
                    _ => Self::missing_syntax("trait require kind", &c.require_kind, env)?,
                };
                Ok(class.reqs.push((hint, is_extends)))
            }
            XHPClassAttributeDeclaration(c) => {
                type Ret = Result<Either<ast::XhpAttr, ast::Hint>>;
                let p_attr = |node: &Syntax<T, V>, env: &mut Env| -> Ret {
                    let mut mk_attr_use = |n: &Syntax<T, V>| {
                        Ok(Either::Right(ast::Hint(
                            Self::p_pos(n, env),
                            Box::new(ast::Hint_::Happly(Self::pos_name(n, env)?, vec![])),
                        )))
                    };
                    match &node.syntax {
                        XHPClassAttribute(c) => {
                            let ty = &c.xhp_attribute_decl_type;
                            let init = &c.xhp_attribute_decl_initializer;
                            let ast::Id(p, name) = Self::pos_name(&c.xhp_attribute_decl_name, env)?;
                            match &ty.syntax {
                                TypeConstant(_) if env.is_typechecker() => {
                                    Self::raise_parsing_error(
                                        ty,
                                        env,
                                        &syntax_error::xhp_class_attribute_type_constant,
                                    )
                                }
                                _ => {}
                            }
                            let req = match &c.xhp_attribute_decl_required.syntax {
                                XHPRequired(_) => Some(ast::XhpAttrTag::Required),
                                XHPLateinit(_) => Some(ast::XhpAttrTag::LateInit),
                                _ => None,
                            };
                            let pos = if init.is_missing() {
                                p.clone()
                            } else {
                                Pos::btw(&p, &Self::p_pos(init, env)).map_err(Error::Failwith)?
                            };
                            let (hint, enum_) = match &ty.syntax {
                                XHPEnumType(c) => {
                                    let p = Self::p_pos(ty, env);
                                    let opt = !(&c.xhp_enum_optional.is_missing());
                                    let vals =
                                        Self::could_map(Self::p_expr, &c.xhp_enum_values, env)?;
                                    (None, Some((p, opt, vals)))
                                }
                                _ => (Some(Self::p_hint(ty, env)?), None),
                            };
                            let init_expr =
                                Self::mp_optional(Self::p_simple_initializer, init, env)?;
                            let xhp_attr = ast::XhpAttr(
                                ast::TypeHint((), hint.clone()),
                                ast::ClassVar {
                                    final_: false,
                                    xhp_attr: Some(ast::XhpAttrInfo { xai_tag: req }),
                                    abstract_: false,
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
                        XHPSimpleClassAttribute(c) => {
                            mk_attr_use(&c.xhp_simple_class_attribute_type)
                        }
                        Token(_) => mk_attr_use(node),
                        _ => Self::missing_syntax("XHP attribute", node, env),
                    }
                };
                let attrs = Self::could_map(p_attr, &c.xhp_attribute_attributes, env)?;
                for attr in attrs.into_iter() {
                    match attr {
                        Either::Left(attr) => class.xhp_attrs.push(attr),
                        Either::Right(xhp_attr_use) => class.xhp_attr_uses.push(xhp_attr_use),
                    }
                }
                Ok(())
            }
            XHPChildrenDeclaration(c) => {
                let p = Self::p_pos(node, env);
                Ok(class
                    .xhp_children
                    .push((p, Self::p_xhp_child(&c.xhp_children_expression, env)?)))
            }
            XHPCategoryDeclaration(c) => {
                let p = Self::p_pos(node, env);
                let categories = Self::could_map(
                    |n, e| Self::p_pstring_(n, e, Some('%')),
                    &c.xhp_category_categories,
                    env,
                )?;
                if let Some((_, cs)) = &class.xhp_category {
                    if let Some(category) = cs.first() {
                        Self::raise_hh_error(
                            env,
                            NastCheck::multiple_xhp_category(category.0.clone()),
                        )
                    }
                }
                Ok(class.xhp_category = Some((p, categories)))
            }
            PocketEnumDeclaration(c) => {
                let is_final = Self::p_kinds(&c.pocket_enum_modifiers, env)?.has(modifier::FINAL);
                let id = Self::pos_name(&c.pocket_enum_name, env)?;
                let flds = Self::as_list(&c.pocket_enum_fields);
                let mut case_types = vec![];
                let mut case_values = vec![];
                let mut members = vec![];
                for fld in flds.iter() {
                    match &fld.syntax {
                        PocketAtomMappingDeclaration(c) => {
                            let id = Self::pos_name(&c.pocket_atom_mapping_name, env)?;
                            let maps = Self::as_list(&c.pocket_atom_mapping_mappings);
                            let mut types = vec![];
                            let mut exprs = vec![];
                            for map in maps.iter() {
                                match &map.syntax {
                                    PocketMappingIdDeclaration(c) => {
                                        let id = Self::pos_name(&c.pocket_mapping_id_name, env)?;
                                        let expr = Self::p_simple_initializer(
                                            &c.pocket_mapping_id_initializer,
                                            env,
                                        )?;
                                        exprs.push((id, expr));
                                    }
                                    PocketMappingTypeDeclaration(c) => {
                                        let id = Self::pos_name(&c.pocket_mapping_type_name, env)?;
                                        let hint = Self::p_hint(&c.pocket_mapping_type_type, env)?;
                                        types.push((id, hint));
                                    }
                                    _ => {
                                        Self::missing_syntax("pumapping", map, env)?;
                                    }
                                }
                            }
                            members.push(ast::PuMember {
                                atom: id,
                                types,
                                exprs,
                            })
                        }
                        PocketFieldTypeExprDeclaration(c) => {
                            let typ = Self::p_hint(&c.pocket_field_type_expr_type, env)?;
                            let id = Self::pos_name(&c.pocket_field_type_expr_name, env)?;
                            case_values.push((id, typ));
                        }
                        PocketFieldTypeDeclaration(c) => {
                            let id = Self::pos_name(&c.pocket_field_type_name, env)?;
                            let is_reified = if !&c.pocket_field_type_reified.is_missing() {
                                ast::ReifyKind::Reified
                            } else {
                                ast::ReifyKind::Erased
                            };
                            case_types.push((id, is_reified));
                        }
                        _ => {
                            Self::missing_syntax("pufield", fld, env)?;
                        }
                    }
                }
                Ok(class.pu_enums.push(ast::PuEnum {
                    annotation: (),
                    name: id,
                    is_final,
                    case_types,
                    case_values,
                    members,
                }))
            }
            _ => Self::missing_syntax("class element", node, env),
        }
    }

    fn p_class_elt(class: &mut ast::Class_, node: &Syntax<T, V>, env: &mut Env) -> Result<()> {
        let r = Self::p_class_elt_(class, node, env);
        match r {
            // match ocaml behavior, don't throw if missing syntax when fail_open is true
            Err(Error::MissingSyntax { .. }) if env.fail_open() => Ok(()),
            _ => r,
        }
    }

    fn contains_class_body(c: &ClassishDeclarationChildren<T, V>) -> bool {
        match &c.classish_body.syntax {
            ClassishBody(_) => true,
            _ => false,
        }
    }

    fn p_where_constraint(
        is_class: bool,
        parent: &Syntax<T, V>,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<Vec<ast::WhereConstraint>> {
        match &node.syntax {
            Missing => Ok(vec![]),
            WhereClause(c) => {
                if is_class && !env.parser_options.po_enable_class_level_where_clauses {
                    Self::raise_parsing_error(
                        parent,
                        env,
                        "Class-level where clauses are disabled",
                    );
                }
                let f = |n: &Syntax<T, V>, e: &mut Env| -> Result<ast::WhereConstraint> {
                    match &n.syntax {
                        WhereConstraint(c) => {
                            use ast::ConstraintKind::*;
                            let l = Self::p_hint(&c.where_constraint_left_type, e)?;
                            let o = &c.where_constraint_operator;
                            let o = match Self::token_kind(o) {
                                Some(TK::Equal) => ConstraintEq,
                                Some(TK::As) => ConstraintAs,
                                Some(TK::Super) => ConstraintSuper,
                                _ => Self::missing_syntax("constraint operator", o, e)?,
                            };
                            let r = Self::p_hint(&c.where_constraint_right_type, e)?;
                            Ok(ast::WhereConstraint(l, o, r))
                        }
                        _ => Self::missing_syntax("where constraint", n, e),
                    }
                };
                Self::as_list(&c.where_clause_constraints)
                    .iter()
                    .map(|n| f(n, env))
                    .collect()
            }
            _ => {
                if is_class {
                    Self::missing_syntax("classish declaration constraints", parent, env)
                } else {
                    Self::missing_syntax("function header constraints", parent, env)
                }
            }
        }
    }

    fn p_namespace_use_kind(kind: &Syntax<T, V>, env: &mut Env) -> Result<ast::NsKind> {
        use ast::NsKind::*;
        match &kind.syntax {
            Missing => Ok(NSClassAndNamespace),
            _ => match Self::token_kind(kind) {
                Some(TK::Namespace) => Ok(NSNamespace),
                Some(TK::Type) => Ok(NSClass),
                Some(TK::Function) => Ok(NSFun),
                Some(TK::Const) => Ok(NSConst),
                _ => Self::missing_syntax("namespace use kind", kind, env),
            },
        }
    }

    fn p_namespace_use_clause(
        prefix: Option<&Syntax<T, V>>,
        kind: Result<ast::NsKind>,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> Result<(ast::NsKind, ast::Sid, ast::Sid)> {
        lazy_static! {
            static ref NAMESPACE_USE: regex::Regex = regex::Regex::new("[^\\\\]*$").unwrap();
        }

        match &node.syntax {
            NamespaceUseClause(c) => {
                let clause_kind = &c.namespace_use_clause_kind;
                let alias = &c.namespace_use_alias;
                let name = &c.namespace_use_name;
                let ast::Id(p, n) = match (prefix, Self::pos_name(name, env)?) {
                    (None, id) => id,
                    (Some(prefix), ast::Id(p, n)) => {
                        ast::Id(p, Self::pos_name(prefix, env)?.1 + &n)
                    }
                };
                let alias = if alias.is_missing() {
                    let x = NAMESPACE_USE.find(&n).unwrap().as_str();
                    ast::Id(p.clone(), x.to_string())
                } else {
                    Self::pos_name(alias, env)?
                };
                let kind = if clause_kind.is_missing() {
                    kind
                } else {
                    Self::p_namespace_use_kind(clause_kind, env)
                }?;
                Ok((
                    kind,
                    ast::Id(
                        p,
                        if n.len() > 0 && n.chars().nth(0) == Some('\\') {
                            n
                        } else {
                            String::from("\\") + &n
                        },
                    ),
                    alias,
                ))
            }
            _ => Self::missing_syntax("namespace use clause", node, env),
        }
    }

    fn p_def(node: &Syntax<T, V>, env: &mut Env) -> Result<Vec<ast::Def>> {
        let doc_comment_opt = Self::extract_docblock(node, env);
        match &node.syntax {
            FunctionDeclaration(c) => {
                let function_attribute_spec = &c.function_attribute_spec;
                let function_declaration_header = &c.function_declaration_header;
                let function_body = &c.function_body;
                let mut env = Env::clone_and_unset_toplevel_if_toplevel(env);
                let env = env.as_mut();
                let hdr = Self::p_fun_hdr(function_declaration_header, env)?;
                let is_external = function_body.is_external();
                let (block, yield_) = if is_external {
                    (vec![], false)
                } else {
                    Self::mp_yielding(&Self::p_function_body, function_body, env)?
                };
                let user_attributes = Self::p_user_attributes(function_attribute_spec, env)?;
                let variadic = Self::determine_variadicity(&hdr.parameters);
                let ret = ast::TypeHint((), hdr.return_type);
                Ok(vec![ast::Def::mk_fun(ast::Fun_ {
                    span: Self::p_fun_pos(node, env),
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    ret,
                    name: hdr.name,
                    tparams: hdr.type_parameters,
                    where_constraints: hdr.constrs,
                    params: hdr.parameters,
                    body: ast::FuncBody {
                        ast: block,
                        annotation: (),
                    },
                    fun_kind: Self::mk_fun_kind(hdr.suspension_kind, yield_),
                    variadic,
                    user_attributes,
                    file_attributes: vec![],
                    external: is_external,
                    namespace: Self::mk_empty_ns_env(env),
                    doc_comment: doc_comment_opt,
                    static_: false,
                })])
            }
            ClassishDeclaration(c) if Self::contains_class_body(c) => {
                let mut env = Env::clone_and_unset_toplevel_if_toplevel(env);
                let env = env.as_mut();
                let mode = Self::mode_annotation(env.file_mode());
                let user_attributes = Self::p_user_attributes(&c.classish_attribute, env)?;
                let kinds = Self::p_kinds(&c.classish_modifiers, env)?;
                let final_ = kinds.has(modifier::FINAL);
                let is_xhp = match Self::token_kind(&c.classish_name) {
                    Some(TK::XHPElementName) => true,
                    Some(TK::XHPClassName) => true,
                    _ => false,
                };
                let has_xhp_keyword = match Self::token_kind(&c.classish_xhp) {
                    Some(TK::XHP) => true,
                    _ => false,
                };
                let name = Self::pos_name(&c.classish_name, env)?;
                *env.cls_reified_generics() = HashSet::new();
                let tparams = ast::ClassTparams {
                    list: Self::p_tparam_l(true, &c.classish_type_parameters, env)?,
                    constraints: s_map::SMap::new(),
                };
                let extends = Self::could_map(Self::p_hint, &c.classish_extends_list, env)?;
                *env.parent_maybe_reified() = match extends.first().map(|h| h.1.as_ref()) {
                    Some(ast::Hint_::Happly(_, hl)) => !hl.is_empty(),
                    _ => false,
                };
                let implements = Self::could_map(Self::p_hint, &c.classish_implements_list, env)?;
                let where_constraints =
                    Self::p_where_constraint(true, node, &c.classish_where_clause, env)?;
                let namespace = Self::mk_empty_ns_env(env);
                let span = Self::p_pos(node, env);
                let class_kind = match Self::token_kind(&c.classish_keyword) {
                    Some(TK::Class) if kinds.has(modifier::ABSTRACT) => ast::ClassKind::Cabstract,
                    Some(TK::Class) => ast::ClassKind::Cnormal,
                    Some(TK::Interface) => ast::ClassKind::Cinterface,
                    Some(TK::Trait) => ast::ClassKind::Ctrait,
                    Some(TK::Enum) => ast::ClassKind::Cenum,
                    _ => Self::missing_syntax("class kind", &c.classish_keyword, env)?,
                };
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
                    method_redeclarations: vec![],
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
                    pu_enums: vec![],
                    doc_comment: doc_comment_opt,
                    emit_id: None,
                };
                match &c.classish_body.syntax {
                    ClassishBody(c1) => {
                        for elt in Self::as_list(&c1.classish_body_elements).iter() {
                            Self::p_class_elt(&mut class_, elt, env)?;
                        }
                    }
                    _ => Self::missing_syntax("classish body", &c.classish_body, env)?,
                }
                Ok(vec![ast::Def::mk_class(class_)])
            }
            ConstDeclaration(c) => {
                let ty = &c.const_type_specifier;
                let decls = Self::as_list(&c.const_declarators);
                let mut defs = vec![];
                for decl in decls.iter() {
                    let def = match &decl.syntax {
                        ConstantDeclarator(c) => {
                            let name = &c.constant_declarator_name;
                            let init = &c.constant_declarator_initializer;
                            let gconst = ast::Gconst {
                                annotation: (),
                                mode: Self::mode_annotation(env.file_mode()),
                                name: Self::pos_name(name, env)?,
                                type_: Self::mp_optional(Self::p_hint, ty, env)?,
                                value: Self::p_simple_initializer(init, env)?,
                                namespace: Self::mk_empty_ns_env(env),
                                span: Self::p_pos(node, env),
                                emit_id: None,
                            };
                            ast::Def::mk_constant(gconst)
                        }
                        _ => Self::missing_syntax("constant declaration", decl, env)?,
                    };
                    defs.push(def);
                }
                Ok(defs)
            }
            AliasDeclaration(c) => {
                let tparams = Self::p_tparam_l(false, &c.alias_generic_parameter, env)?;
                for tparam in tparams.iter() {
                    if tparam.reified != ast::ReifyKind::Erased {
                        Self::raise_parsing_error(node, env, &syntax_error::invalid_reified)
                    }
                }
                Ok(vec![ast::Def::mk_typedef(ast::Typedef {
                    annotation: (),
                    name: Self::pos_name(&c.alias_name, env)?,
                    tparams,
                    constraint: Self::mp_optional(Self::p_tconstraint, &c.alias_constraint, env)?
                        .map(|x| x.1),
                    user_attributes: itertools::concat(
                        Self::as_list(&c.alias_attribute_spec)
                            .iter()
                            .map(|attr| Self::p_user_attribute(attr, env))
                            .collect::<std::result::Result<Vec<Vec<_>>, _>>()?,
                    ),
                    namespace: Self::mk_empty_ns_env(env),
                    mode: Self::mode_annotation(env.file_mode()),
                    vis: match Self::token_kind(&c.alias_keyword) {
                        Some(TK::Type) => ast::TypedefVisibility::Transparent,
                        Some(TK::Newtype) => ast::TypedefVisibility::Opaque,
                        _ => Self::missing_syntax("kind", &c.alias_keyword, env)?,
                    },
                    kind: Self::p_hint(&c.alias_type, env)?,
                    emit_id: None,
                })])
            }
            EnumDeclaration(c) => {
                let p_enumerator = |n: &Syntax<T, V>, e: &mut Env| -> Result<ast::ClassConst> {
                    match &n.syntax {
                        Enumerator(c) => Ok(ast::ClassConst {
                            type_: None,
                            id: Self::pos_name(&c.enumerator_name, e)?,
                            expr: Some(Self::p_expr(&c.enumerator_value, e)?),
                            doc_comment: None,
                        }),
                        _ => Self::missing_syntax("enumerator", n, e),
                    }
                };
                Ok(vec![ast::Def::mk_class(ast::Class_ {
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    user_attributes: Self::p_user_attributes(&c.enum_attribute_spec, env)?,
                    file_attributes: vec![],
                    final_: false,
                    kind: ast::ClassKind::Cenum,
                    is_xhp: false,
                    has_xhp_keyword: false,
                    name: Self::pos_name(&c.enum_name, env)?,
                    tparams: ast::ClassTparams {
                        list: vec![],
                        constraints: s_map::SMap::new(),
                    },
                    extends: vec![],
                    implements: vec![],
                    where_constraints: vec![],
                    consts: Self::could_map(p_enumerator, &c.enum_enumerators, env)?,
                    namespace: Self::mk_empty_ns_env(env),
                    span: Self::p_pos(node, env),
                    enum_: Some(ast::Enum_ {
                        base: Self::p_hint(&c.enum_base, env)?,
                        constraint: Self::mp_optional(Self::p_tconstraint_ty, &c.enum_type, env)?,
                    }),

                    doc_comment: doc_comment_opt,
                    uses: vec![],
                    use_as_alias: vec![],
                    insteadof_alias: vec![],
                    method_redeclarations: vec![],
                    xhp_attr_uses: vec![],
                    xhp_category: None,
                    reqs: vec![],
                    vars: vec![],
                    typeconsts: vec![],
                    methods: vec![],
                    attributes: vec![],
                    xhp_children: vec![],
                    xhp_attrs: vec![],
                    pu_enums: vec![],
                    emit_id: None,
                })])
            }
            RecordDeclaration(c) => {
                let p_field = |n: &Syntax<T, V>, e: &mut Env| match &n.syntax {
                    RecordField(c) => Ok((
                        Self::pos_name(&c.record_field_name, e)?,
                        Self::p_hint(&c.record_field_type, e)?,
                        Self::mp_optional(Self::p_simple_initializer, &c.record_field_init, e)?,
                    )),
                    _ => Self::missing_syntax("record_field", n, e),
                };
                Ok(vec![ast::Def::mk_record_def(ast::RecordDef {
                    annotation: (),
                    name: Self::pos_name(&c.record_name, env)?,
                    extends: Self::could_map(Self::p_hint, &c.record_extends_opt, env)?
                        .into_iter()
                        .next(),
                    abstract_: Self::token_kind(&c.record_modifier) == Some(TK::Abstract),
                    user_attributes: Self::p_user_attributes(&c.record_attribute_spec, env)?,
                    fields: Self::could_map(p_field, &c.record_fields, env)?,
                    namespace: Self::mk_empty_ns_env(env),
                    span: Self::p_pos(node, env),
                    doc_comment: doc_comment_opt,
                    emit_id: None,
                })])
            }
            InclusionDirective(c)
                if env.file_mode() != file_info::Mode::Mdecl
                    && env.file_mode() != file_info::Mode::Mphp
                    || env.codegen() =>
            {
                let expr = Self::p_expr(&c.inclusion_expression, env)?;
                Ok(vec![ast::Def::mk_stmt(ast::Stmt::new(
                    Self::p_pos(node, env),
                    ast::Stmt_::mk_expr(expr),
                ))])
            }
            NamespaceDeclaration(c) => {
                let name = &c.namespace_name;
                let defs = match &c.namespace_body.syntax {
                    NamespaceBody(c) => {
                        let mut env1 = Env::clone_and_unset_toplevel_if_toplevel(env);
                        let env1 = env1.as_mut();
                        itertools::concat(
                            Self::as_list(&c.namespace_declarations)
                                .iter()
                                .map(|n| Self::p_def(n, env1))
                                .collect::<std::result::Result<Vec<Vec<_>>, _>>()?,
                        )
                    }
                    _ => vec![],
                };
                Ok(vec![ast::Def::mk_namespace(
                    Self::pos_name(name, env)?,
                    defs,
                )])
            }
            NamespaceGroupUseDeclaration(c) => {
                let uses: std::result::Result<Vec<_>, _> =
                    Self::as_list(&c.namespace_group_use_clauses)
                        .iter()
                        .map(|n| {
                            Self::p_namespace_use_clause(
                                Some(&c.namespace_group_use_prefix),
                                Self::p_namespace_use_kind(&c.namespace_group_use_kind, env),
                                n,
                                env,
                            )
                        })
                        .collect();
                Ok(vec![ast::Def::mk_namespace_use(uses?)])
            }
            NamespaceUseDeclaration(c) => {
                let uses: std::result::Result<Vec<_>, _> = Self::as_list(&c.namespace_use_clauses)
                    .iter()
                    .map(|n| {
                        Self::p_namespace_use_clause(
                            None,
                            Self::p_namespace_use_kind(&c.namespace_use_kind, env),
                            n,
                            env,
                        )
                    })
                    .collect();
                Ok(vec![ast::Def::mk_namespace_use(uses?)])
            }
            FileAttributeSpecification(_) => {
                Ok(vec![ast::Def::mk_file_attributes(ast::FileAttribute {
                    user_attributes: Self::p_user_attribute(node, env)?,
                    namespace: Self::mk_empty_ns_env(env),
                })])
            }
            _ => Ok(vec![ast::Def::mk_stmt(Self::p_stmt(node, env)?)]),
        }
    }

    fn post_process(env: &mut Env, program: ast::Program, acc: &mut ast::Program) {
        use aast::{Def, Def::*, Stmt_::*};
        let mut saw_ns: Option<(ast::Sid, ast::Program)> = None;
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
                    Self::post_process(env, defs, &mut acc_);
                    acc.push(Def::mk_namespace(n, acc_));
                }

                continue;
            }

            if let Stmt(s) = &def {
                if Self::is_noop(&s) {
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
                        env.keep_errors
                            && env.is_typechecker()
                            && (mode == Mstrict
                                || (mode == Mpartial
                                    && env
                                        .parser_options
                                        .error_codes_treated_strictly
                                        .contains(&1002)))
                    }
                };
                if raise_error {
                    Self::raise_parsing_error_pos(&s.0, env, &syntax_error::toplevel_statements);
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

    fn p_program(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Program> {
        let nodes = Self::as_list(node);
        let mut acc = vec![];
        for i in 0..nodes.len() {
            match &nodes[i].syntax {
                EndOfFile(_) => break,
                _ => match Self::p_def(nodes[i], env) {
                    Err(Error::MissingSyntax { .. }) if env.fail_open => {}
                    e @ Err(_) => return e,
                    Ok(mut def) => acc.append(&mut def),
                },
            }
        }
        let mut program = vec![];
        Self::post_process(env, acc, &mut program);
        Ok(program)
    }

    fn p_script(node: &Syntax<T, V>, env: &mut Env) -> Result<ast::Program> {
        match &node.syntax {
            Script(children) => Self::p_program(&children.script_declarations, env),
            _ => Self::missing_syntax("script", node, env),
        }
    }

    fn lower(
        env: &mut Env<'a>,
        script: &Syntax<T, V>,
    ) -> std::result::Result<ast::Program, String> {
        Self::p_script(script, env).map_err(|e| match e {
            Error::MissingSyntax {
                expecting,
                pos,
                node_name,
                kind,
            } => format!(
                "missing case in {:?}.\n - pos: {:?}\n - unexpected: '{:?}'\n - kind: {:?}\n",
                expecting.to_string(),
                pos,
                node_name.to_string(),
                kind,
            ),
            Error::Failwith(msg) => msg,
        })
    }
}

struct PositionedSyntaxLowerer;
impl<'a> Lowerer<'a, PositionedToken, PositionedValue> for PositionedSyntaxLowerer {}

pub fn lower<'a>(
    env: &mut Env<'a>,
    script: &Syntax<PositionedToken, PositionedValue>,
) -> std::result::Result<ast::Program, String> {
    PositionedSyntaxLowerer::lower(env, script)
}
