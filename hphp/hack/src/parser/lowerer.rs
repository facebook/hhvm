// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{
    namespace_env::Env as NamespaceEnv,
    pos::Pos,
    prim_defs::Comment,
    relative_path::RelativePath,
    s_map::SMap,
    s_set::SSet,
    {aast, aast_defs, ast_defs, file_info},
};

use parser_rust::{
    indexed_source_text::IndexedSourceText, lexable_token::LexablePositionedToken,
    positioned_syntax::PositionedSyntaxTrait, source_text::SourceText, syntax::*, syntax_error,
    syntax_trait::SyntaxTrait, token_kind::TokenKind as TK,
};

use utils_rust::*;

use ocamlvalue_macro::Ocamlvalue;

use std::collections::HashSet;
use std::result::Result::{Err, Ok};

macro_rules! not_impl {
    () => {
        panic!("NOT IMPLEMENTED")
    };
}

macro_rules! aast {
    ($ty:ident) =>  {oxidized::aast::$ty};
    // NOTE: In <,> pattern, comma prevents rustfmt eating <>
    ($ty:ident<,>) =>  {oxidized::aast::$ty<Pos, (), (), ()>}
}

macro_rules! ret {
    ($ty:ty) => { std::result::Result<$ty, Error<Syntax<T, V>>> }
}

macro_rules! ret_aast {
    ($ty:ident) => { std::result::Result<aast!($ty), Error<Syntax<T, V>>> };
    ($ty:ident<,>) => { std::result::Result<aast!($ty<,>), Error<Syntax<T, V>>> }
}

#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum LiftedAwaitKind {
    LiftedFromStatement,
    LiftedFromConcurrent,
}

#[derive(Debug, Clone)]
pub struct LiftedAwaits {
    pub awaits: Vec<(Option<aast::Id>, aast!(Expr<,>))>,
    lift_kind: LiftedAwaitKind,
}

impl LiftedAwaits {
    fn lift_kind(&self) -> LiftedAwaitKind {
        self.lift_kind.clone()
    }
}

#[derive(Debug, Clone, Copy, Eq, PartialEq)]
enum ExprLocation {
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

#[derive(Debug, Clone)]
pub struct Env<'a> {
    codegen: bool,
    elaborate_namespaces: bool,
    include_line_comments: bool,
    keep_errors: bool,
    quick_mode: bool,
    /* Show errors even in quick mode. Does not override keep_errors. Hotfix
     * until we can properly set up saved states to surface parse errors during
     * typechecking properly. */
    show_all_errors: bool,
    lower_coroutines: bool,
    fail_open: bool,
    file_mode: file_info::Mode,
    top_level_statements: bool, /* Whether we are (still) considering TLSs*/

    pub saw_yield: bool, /* Information flowing back up */
    pub lifted_awaits: Option<LiftedAwaits>,
    pub tmp_var_counter: isize,
    /* Whether we've seen COMPILER_HALT_OFFSET. The value of COMPILER_HALT_OFFSET
      defaults to 0 if HALT_COMPILER isn't called.
      None -> COMPILER_HALT_OFFSET isn't in the source file
      Some 0 -> COMPILER_HALT_OFFSET is in the source file, but HALT_COMPILER isn't
      Some x -> COMPILER_HALT_OFFSET is in the source file,
                HALT_COMPILER is at x bytes offset in the file.
    */
    pub saw_compiler_halt_offset: Option<isize>,
    pub cls_reified_generics: HashSet<String>,
    pub in_static_method: bool,
    pub parent_maybe_reified: bool,
    /* This provides a generic mechanism to delay raising parsing errors;
     * since we're moving FFP errors away from CST to a stage after lowering
     * _and_ want to prioritize errors before lowering, the lowering errors
     * must be merely stored when the lowerer runs (until check for FFP runs (on AST)
     * and raised _after_ FFP error checking (unless we run the lowerer twice,
     * which would be expensive). */
    pub lowpri_errors: Vec<(Pos, String)>,

    pub indexed_source_text: &'a IndexedSourceText<'a>,
    pub auto_ns_map: &'a [(String, String)],
}

impl<'a> Env<'a> {
    pub fn make(mode: file_info::Mode, indexed_source_text: &'a IndexedSourceText<'a>) -> Self {
        Env {
            codegen: false,
            elaborate_namespaces: true,
            include_line_comments: false,
            keep_errors: false,
            quick_mode: false,
            show_all_errors: false,
            lower_coroutines: true,
            fail_open: true,
            file_mode: mode,
            top_level_statements: false,
            saw_yield: false,
            lifted_awaits: None,
            tmp_var_counter: 0,
            saw_compiler_halt_offset: None,
            cls_reified_generics: HashSet::new(),
            in_static_method: false,
            parent_maybe_reified: false,
            lowpri_errors: vec![],
            indexed_source_text,
            auto_ns_map: &[],
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
        self.indexed_source_text.source_text
    }

    fn lower_coroutines(&self) -> bool {
        self.lower_coroutines
    }

    fn fail_open(&self) -> bool {
        self.fail_open
    }
}

pub enum Error<Node> {
    APIMissingSyntax(String, Node),
    LowererInvariantFailure(String, String),
    Failwith(String),
}

#[derive(Ocamlvalue)]
pub struct Result {
    ast: aast!(Program<,>),
    comments: Vec<(Pos, Comment)>,
}

use parser_core_types::syntax::SyntaxVariant::*;

pub trait Lowerer<'a, T, V>
where
    T: LexablePositionedToken<'a>,
    Syntax<T, V>: PositionedSyntaxTrait,
    V: SyntaxValueWithKind,
{
    fn make_empty_ns_env(env: &Env) -> NamespaceEnv {
        NamespaceEnv::empty(Vec::from(env.auto_ns_map), env.codegen())
    }

    fn mode_annotation(mode: file_info::Mode) -> file_info::Mode {
        match mode {
            file_info::Mode::Mphp => file_info::Mode::Mdecl,
            m => m,
        }
    }

    fn p_pos(node: &Syntax<T, V>, env: &Env) -> Pos {
        node.position_exclusive(env.indexed_source_text)
            .unwrap_or(Pos::make_none())
    }

    fn raise_parsing_error(node: &Syntax<T, V>, env: &mut Env, msg: &str) {
        not_impl!()
    }

    fn raise_parsing_error_pos(pos: Pos, env: &mut Env, msg: &str) {
        // TODO: enable should_surface_errors
        if env.codegen() && !env.lower_coroutines() {
            env.lowpri_errors.push((pos, String::from(msg)))
        }
    }

    #[inline]
    fn failwith<N>(msg: &str) -> ret!(N) {
        Err(Error::Failwith(String::from(msg)))
    }

    #[inline]
    fn text(node: &Syntax<T, V>, env: &Env) -> String {
        String::from(node.text(env.source_text()))
    }

    fn missing_syntax<N>(
        fallback: Option<N>,
        expecting: &str,
        node: &Syntax<T, V>,
        env: &Env,
    ) -> ret!(N) {
        //TODO:
        let pos = Self::p_pos(node, env);
        Err(Error::LowererInvariantFailure(
            String::from(""),
            String::from(""),
        ))
    }

    fn is_num_octal_lit(s: &str) -> bool {
        !s.chars().any(|c| c == '8' || c == '9')
    }

    fn mp_optional<S>(
        p: &dyn Fn(&Syntax<T, V>, &mut Env) -> ret!(S),
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!(Option<S>) {
        match &node.syntax {
            Missing => Ok(None),
            _ => p(node, env).map(Some),
        }
    }

    fn pos_qualified_name(node: &Syntax<T, V>, env: &Env) -> ret_aast!(Sid) {
        not_impl!()
    }

    fn pos_name(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Sid) {
        match &node.syntax {
            QualifiedName(_) => Self::pos_qualified_name(node, env),
            SimpleTypeSpecifier(child) => Self::pos_name(&child.simple_type_specifier, env),
            _ => {
                let name = node.text(env.indexed_source_text.source_text);
                if name == "__COMPILER_HALT_OFFSET__" {
                    env.saw_compiler_halt_offset = Some(0);
                }
                let p = if name == "__LINE__" {
                    Pos::make_none()
                } else {
                    Self::p_pos(node, env)
                };
                Ok(ast_defs::Id(p, String::from(name)))
            }
        }
    }

    fn mk_str(
        node: &Syntax<T, V>,
        env: &mut Env,
        unescaper: &Fn(&str) -> std::result::Result<String, InvalidString>,
        mut content: &str,
    ) -> String {
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

    fn as_list(node: &Syntax<T, V>) -> Vec<&Syntax<T, V>> {
        fn strip_list_item<T1, V1>(node: &Syntax<T1, V1>) -> &Syntax<T1, V1> {
            match node {
                Syntax {
                    syntax: ListItem(i),
                    ..
                } => &i.list_item,
                x => x,
            }
        }

        match node {
            Syntax {
                syntax: SyntaxList(synl),
                ..
            } => synl.iter().map(strip_list_item).collect(),
            Syntax {
                syntax: Missing, ..
            } => vec![],
            syn => vec![syn],
        }
    }

    fn token_kind(node: &Syntax<T, V>) -> Option<TK> {
        match &node.syntax {
            Token(t) => Some(t.kind()),
            _ => None,
        }
    }

    fn check_valid_reified_hint(env: &Env, node: &Syntax<T, V>, hint: &aast_defs::Hint) {
        // TODO:
    }

    fn p_hint_(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Hint_) {
        match &node.syntax {
            /* Dirty hack; CastExpression can have type represented by token */
            Token(_) | SimpleTypeSpecifier(_) | QualifiedName(_) => {
                Ok(aast_defs::Hint_::Happly(Self::pos_name(node, env)?, vec![]))
            }
            _ => not_impl!(),
        }
    }

    fn p_hint(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Hint) {
        let hint_ = Self::p_hint_(node, env)?;
        let pos = Self::p_pos(node, env);
        let hint = aast_defs::Hint(pos, Box::new(hint_));
        Self::check_valid_reified_hint(env, node, &hint);
        Ok(hint)
    }

    fn p_simple_initializer(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Expr<,>) {
        match &node.syntax {
            SimpleInitializer(child) => Self::p_expr(&child.simple_initializer_value, env),
            _ => Self::missing_syntax(None, "simple initializer", node, env),
        }
    }

    #[inline]
    fn p_expr(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Expr<,>) {
        Self::p_expr_with_loc(ExprLocation::TopLevel, node, env)
    }

    fn p_expr_with_loc(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret_aast!(Expr<,>) {
        match &node.syntax {
            BracedExpression(child) => {
                let expr = &child.braced_expression_expression;
                let inner = Self::p_expr_with_loc(location, expr, env)?;
                let inner_pos = &inner.0;
                let inner_expr_ = inner.1.as_ref();
                use aast::Expr_::*;
                match inner_expr_ {
                    Lvar(_) | String(_) | Int(_) | Float(_) => Ok(inner),
                    _ => Ok(aast::Expr(
                        inner_pos.clone(),
                        Box::new(aast::Expr_::BracedExpr(inner)),
                    )),
                }
            }
            ParenthesizedExpression(child) => {
                Self::p_expr_with_loc(location, &child.parenthesized_expression_expression, env)
            }
            _ => {
                let expr_ = Self::p_expr_with_loc_(location, node, env)?;
                let p = Self::p_pos(node, env);
                Ok(aast::Expr(p, Box::new(expr_)))
            }
        }
    }

    fn p_expr_lit(
        location: ExprLocation,
        parent: &Syntax<T, V>,
        expr: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret_aast!(Expr_<,>) {
        match &expr.syntax {
            Token(_) => {
                let s = expr.text(env.indexed_source_text.source_text);
                match (location, Self::token_kind(expr)) {
                    (ExprLocation::InDoubleQuotedString, _) if env.codegen() => Ok(
                        aast::Expr_::String(Self::mk_str(expr, env, &Self::unesc_dbl, s)),
                    ),
                    (ExprLocation::InBacktickedString, _) if env.codegen() => Ok(
                        aast::Expr_::String(Self::mk_str(expr, env, &unescape_backtick, s)),
                    ),
                    (_, Some(TK::OctalLiteral))
                        if env.is_typechecker() && !Self::is_num_octal_lit(s) =>
                    {
                        Self::raise_parsing_error(
                            parent,
                            env,
                            &syntax_error::invalid_octal_integer,
                        );
                        Self::missing_syntax(None, "octal", expr, env)
                    }
                    (_, Some(TK::DecimalLiteral))
                    | (_, Some(TK::OctalLiteral))
                    | (_, Some(TK::HexadecimalLiteral))
                    | (_, Some(TK::BinaryLiteral)) => Ok(aast::Expr_::Int(s.replace("_", ""))),
                    (_, Some(TK::FloatingLiteral)) => Ok(aast::Expr_::Float(String::from(s))),
                    (_, Some(TK::SingleQuotedStringLiteral)) => Ok(aast::Expr_::String(
                        Self::mk_str(expr, env, &unescape_single, s),
                    )),
                    (_, Some(TK::DoubleQuotedStringLiteral)) => Ok(aast::Expr_::String(
                        Self::mk_str(expr, env, &unescape_double, s),
                    )),
                    (_, Some(TK::HeredocStringLiteral)) => Ok(aast::Expr_::String(Self::mk_str(
                        expr,
                        env,
                        &unescape_heredoc,
                        s,
                    ))),
                    (_, Some(TK::NowdocStringLiteral)) => Ok(aast::Expr_::String(Self::mk_str(
                        expr,
                        env,
                        &unescape_nowdoc,
                        s,
                    ))),
                    (_, Some(TK::NullLiteral)) => {
                        // TODO: Handle Lint
                        Ok(aast::Expr_::Null)
                    }
                    (_, Some(TK::BooleanLiteral)) => {
                        // TODO: Handle Lint
                        if s.eq_ignore_ascii_case("false") {
                            Ok(aast::Expr_::False)
                        } else if s.eq_ignore_ascii_case("true") {
                            Ok(aast::Expr_::True)
                        } else {
                            Self::missing_syntax(None, &format!("boolean (not: {})", s), expr, env)
                        }
                    }
                    _ => Self::missing_syntax(None, "literal", expr, env),
                }
            }
            SyntaxList(ts) => not_impl!(),
            _ => Self::missing_syntax(None, "literal expressoin", expr, env),
        }
    }

    fn p_expr_with_loc_(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret_aast!(Expr_<,>) {
        let pos = Self::p_pos(node, env);
        match &node.syntax {
            LiteralExpression(child) => {
                Self::p_expr_lit(location, node, &child.literal_expression, env)
            }
            _ => not_impl!(),
        }
    }

    fn p_stmt(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Stmt<,>) {
        // TODO: clear_statement_scope & extract_and_push_docblock
        let pos = Self::p_pos(node, env);
        match &node.syntax {
            MarkupSection(_) => Self::p_markup(node, env),
            _ => not_impl!(),
        }
    }

    fn is_hashbang(text: &Syntax<T, V>) -> bool {
        not_impl!()
    }

    fn p_markup(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Stmt<,>) {
        match &node.syntax {
            MarkupSection(child) => {
                let markup_prefix = &child.markup_prefix;
                let markup_text = &child.markup_text;
                let markup_expression = &child.markup_expression;
                let pos = Self::p_pos(node, env);
                let has_dot_hack_extension = pos.filename().ends_with(".hack");
                if has_dot_hack_extension {
                    Self::raise_parsing_error(node, env, &syntax_error::error1060);
                } else if markup_prefix.value.is_missing()
                    && markup_text.width() > 0
                    && !Self::is_hashbang(&markup_text)
                {
                    Self::raise_parsing_error(node, env, &syntax_error::error1001);
                }
                let expr = match &markup_expression.syntax {
                    Missing => None,
                    ExpressionStatement(e) => {
                        Some(Self::p_expr(&e.expression_statement_expression, env)?)
                    }
                    _ => Self::failwith("expression expected")?,
                };
                let stmt_ = aast::Stmt_::Markup((pos.clone(), Self::text(&markup_text, env)), expr);
                Ok(aast::Stmt(pos, Box::new(stmt_)))
            }
            _ => Self::failwith("invalid node"),
        }
    }

    fn p_def(node: &Syntax<T, V>, env: &mut Env) -> ret!(Vec<aast!(Def<,>)>) {
        match &node.syntax {
            ConstDeclaration(child) => {
                let ty = &child.const_type_specifier;
                let decls = Self::as_list(&child.const_declarators);
                let mut defs = vec![];
                for decl in decls.iter() {
                    let def = match &decl.syntax {
                        ConstantDeclarator(child) => {
                            let name = &child.constant_declarator_name;
                            let init = &child.constant_declarator_initializer;
                            let gconst = aast::Gconst {
                                annotation: (),
                                mode: Self::mode_annotation(env.file_mode()),
                                name: Self::pos_name(name, env)?,
                                type_: Self::mp_optional(&Self::p_hint, ty, env)?,
                                value: Self::p_simple_initializer(init, env)?,
                                namespace: Self::make_empty_ns_env(env),
                                span: Self::p_pos(node, env),
                            };
                            aast::Def::Constant(gconst)
                        }
                        _ => Self::missing_syntax(None, "constant declaration", decl, env)?,
                    };
                    defs.push(def);
                }
                Ok(defs)
            }
            InclusionDirective(child)
                if env.file_mode() != file_info::Mode::Mdecl
                    && env.file_mode() != file_info::Mode::Mphp
                    || env.codegen() =>
            {
                let expr = Self::p_expr(&child.inclusion_expression, env)?;
                Ok(vec![aast::Def::Stmt(aast::Stmt(
                    Self::p_pos(node, env),
                    Box::new(aast::Stmt_::Expr(expr)),
                ))])
            }
            _ => Ok(vec![aast::Def::Stmt(Self::p_stmt(node, env)?)]),
        }
    }

    fn p_program(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Program<,>) {
        let nodes = Self::as_list(node);
        let mut acc = vec![];
        for i in 0..nodes.len() {
            match nodes[i] {
                // TODO: handle Halt
                Syntax {
                    syntax: EndOfFile(_),
                    ..
                } => break,
                node => acc.append(&mut Self::p_def(node, env)?),
            }
        }
        // TODO: post process
        Ok(acc)
    }

    fn p_script(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Program<,>) {
        match &node.syntax {
            Script(children) => Self::p_program(&children.script_declarations, env),
            _ => Self::missing_syntax(None, "script", node, env),
        }
    }

    fn lower(env: &mut Env<'a>, script: &Syntax<T, V>) -> std::result::Result<Result, String> {
        let comments = vec![];
        let ast_result = Self::p_script(script, env);
        // TODO: handle error
        let ast = match ast_result {
            Ok(ast) => ast,
            // TODO: add msg
            Err(_) => return Err(String::from("ERROR")),
        };
        Ok(Result { ast, comments })
    }
}
