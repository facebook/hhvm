// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{
    namespace_env::Env as NamespaceEnv,
    pos::Pos,
    prim_defs::Comment,
    s_map, {aast, aast_defs, ast_defs, file_info},
};

use parser_rust::{
    indexed_source_text::IndexedSourceText, lexable_token::LexablePositionedToken,
    positioned_syntax::PositionedSyntaxTrait, source_text::SourceText, syntax::*, syntax_error,
    syntax_kind, syntax_trait::SyntaxTrait, token_kind::TokenKind as TK,
};

use escaper::*;

use ocamlvalue_macro::Ocamlvalue;

use std::collections::HashSet;
use std::result::Result::{Err, Ok};

use crate::lowerer_modifier as modifier;

use itertools::Either;

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
    ($ty:ty) => { std::result::Result<$ty, Error> }
}

macro_rules! ret_aast {
    ($ty:ident) => { std::result::Result<aast!($ty), Error> };
    ($ty:ident<,>) => { std::result::Result<aast!($ty<,>), Error> }
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

#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum SuspensionKind {
    SKSync,
    SKAsync,
    SKCoroutine,
}

#[derive(Debug)]
pub struct FunHdr {
    fh_suspension_kind: SuspensionKind,
    fh_name: aast!(Sid),
    fh_constrs: Vec<aast_defs::WhereConstraint>,
    fh_type_parameters: Vec<aast!(Tparam<,>)>,
    fh_parameters: Vec<aast!(FunParam<,>)>,
    fh_return_type: Option<aast!(Hint)>,
}

impl FunHdr {
    fn make_empty() -> Self {
        Self {
            fh_suspension_kind: SuspensionKind::SKSync,
            fh_name: ast_defs::Id(Pos::make_none(), String::from("<ANONYMOUS>")),
            fh_constrs: vec![],
            fh_type_parameters: vec![],
            fh_parameters: vec![],
            fh_return_type: None,
        }
    }
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
    pub saw_compiler_halt_offset: Option<usize>,
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
    pub fn make(
        elaborate_namespaces: bool,
        mode: file_info::Mode,
        indexed_source_text: &'a IndexedSourceText<'a>,
    ) -> Self {
        Env {
            codegen: false,
            elaborate_namespaces,
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

#[derive(Debug)]
pub enum Error {
    APIMissingSyntax {
        expecting: String,
        pos: Pos,
        node_name: String,
        kind: syntax_kind::SyntaxKind,
    },
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
    V: SyntaxValueWithKind + SyntaxValueType<T> + std::fmt::Debug,
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

    fn raise_nast_error(msg: &str) {
        // A placehold for error raised in ast_to_aast.ml
    }

    #[inline]
    fn failwith<N>(msg: &str) -> ret!(N) {
        Err(Error::Failwith(String::from(msg)))
    }

    #[inline]
    fn text(node: &Syntax<T, V>, env: &Env) -> String {
        String::from(node.text(env.source_text()))
    }

    #[inline]
    fn text_str<'b>(node: &'b Syntax<T, V>, env: &'b Env) -> &'b str {
        node.text(env.source_text())
    }

    fn lowering_error(env: &mut Env, pos: Pos, text: &str, syntax_kind: &str) {
        if env.is_typechecker() && env.lowpri_errors.is_empty() {
            // TODO: Ocaml also checks Errors.currently_has_errors
            Self::raise_parsing_error_pos(
                pos,
                env,
                &syntax_error::lowering_parsing_error(text, syntax_kind),
            )
        }
    }

    fn missing_syntax<N>(
        fallback: Option<N>,
        expecting: &str,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!(N) {
        let pos = Self::p_pos(node, env);
        let text = Self::text(node, env);
        Self::lowering_error(env, pos, &text, expecting);
        Err(Error::APIMissingSyntax {
            expecting: String::from(expecting),
            pos: Self::p_pos(node, env),
            node_name: text,
            kind: node.kind(),
        })
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

    #[inline]
    fn pos_name(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Sid) {
        Self::pos_name_(node, env, None)
    }

    // TODO: after porting unify Sid and Pstring
    #[inline]
    fn p_pstring(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Pstring) {
        Self::p_pstring_(node, env, None)
    }

    #[inline]
    fn p_pstring_(
        node: &Syntax<T, V>,
        env: &mut Env,
        drop_prefix: Option<char>,
    ) -> ret_aast!(Pstring) {
        let ast_defs::Id(p, id) = Self::pos_name_(node, env, drop_prefix)?;
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

    fn pos_name_(node: &Syntax<T, V>, env: &mut Env, drop_prefix: Option<char>) -> ret_aast!(Sid) {
        match &node.syntax {
            QualifiedName(_) => Self::pos_qualified_name(node, env),
            SimpleTypeSpecifier(child) => {
                Self::pos_name_(&child.simple_type_specifier, env, drop_prefix)
            }
            _ => {
                let mut name = node.text(env.indexed_source_text.source_text);
                if name == "__COMPILER_HALT_OFFSET__" {
                    env.saw_compiler_halt_offset = Some(0);
                }
                if let Some(prefix) = drop_prefix {
                    name = Self::drop_prefix(name, prefix);
                }
                let p = Self::p_pos(node, env);
                Ok(ast_defs::Id(p, String::from(name)))
            }
        }
    }

    fn mk_str(
        node: &Syntax<T, V>,
        env: &mut Env,
        unescaper: &dyn Fn(&str) -> std::result::Result<String, InvalidString>,
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
        use aast_defs::Hint_::*;
        let unary = |kw, ty, env: &mut Env| {
            Ok(Happly(
                Self::pos_name(kw, env)?,
                Self::could_map(&Self::p_hint, ty, env)?,
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
                Ok(Happly(Self::pos_name(node, env)?, vec![]))
            }
            ShapeTypeSpecifier(_) => not_impl!(),
            TupleTypeSpecifier(c) => {
                Ok(Htuple(Self::could_map(&Self::p_hint, &c.tuple_types, env)?))
            }
            KeysetTypeSpecifier(c) => Ok(Happly(
                Self::pos_name(&c.keyset_type_keyword, env)?,
                Self::could_map(&Self::p_hint, &c.keyset_type_type, env)?,
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
                let type_args = match &c.generic_argument_list.syntax {
                    TypeArguments(c) => {
                        Self::could_map(&Self::p_hint, &c.type_arguments_types, env)?
                    }
                    _ => Self::missing_syntax(
                        None,
                        "generic type arguments",
                        &c.generic_argument_list,
                        env,
                    )?,
                };
                if env.codegen() {
                    not_impl!()
                } else {
                    Ok(Happly(name, type_args))
                }
            }
            NullableTypeSpecifier(c) => Ok(Hoption(Self::p_hint(&c.nullable_type, env)?)),
            LikeTypeSpecifier(c) => Ok(Hlike(Self::p_hint(&c.like_type, env)?)),
            SoftTypeSpecifier(c) => Ok(Hsoft(Self::p_hint(&c.soft_type, env)?)),
            ClosureTypeSpecifier(_) => not_impl!(),
            AttributizedSpecifier(_) => not_impl!(),
            TypeConstant(_) => not_impl!(),
            ReifiedTypeArgument(_) => not_impl!(),
            _ => Self::missing_syntax(None, "type hint", node, env),
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

    #[inline]
    fn wrap_unescaper(
        unescaper: &dyn Fn(&str) -> std::result::Result<String, InvalidString>,
        s: &str,
    ) -> ret!(String) {
        unescaper(s).map_err(|e| Error::Failwith(e.msg))
    }

    fn p_expr_with_loc_(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret_aast!(Expr_<,>) {
        let mk_lvar = |name, env| {
            let name = Self::pos_name(name, env)?;
            let lid = aast::Lid(name.0, (0, name.1));
            Ok(aast::Expr_::Lvar(lid))
        };
        let pos = Self::p_pos(node, env);
        match &node.syntax {
            VariableExpression(c) => mk_lvar(&c.variable_expression, env),
            Token(t) => {
                use ExprLocation::*;
                match (location, t.kind()) {
                    (MemberSelect, TK::Variable) => mk_lvar(node, env),
                    (InDoubleQuotedString, _) => Ok(aast::Expr_::String(Self::wrap_unescaper(
                        &Self::unesc_dbl,
                        Self::text_str(node, env),
                    )?)),
                    (InBacktickedString, _) => Ok(aast::Expr_::String(Self::wrap_unescaper(
                        &unescape_backtick,
                        Self::text_str(node, env),
                    )?)),
                    (MemberSelect, _)
                    | (TopLevel, _)
                    | (AsStatement, _)
                    | (UsingStatement, _)
                    | (RightOfAssignment, _)
                    | (RightOfAssignmentInUsingStatement, _)
                    | (RightOfReturn, _) => Ok(aast::Expr_::Id(Self::pos_name(node, env)?)),
                }
            }
            LiteralExpression(child) => {
                Self::p_expr_lit(location, node, &child.literal_expression, env)
            }
            _ => not_impl!(),
        }
    }

    fn is_noop(stmt: &aast!(Stmt<,>)) -> bool {
        if let aast::Stmt_::Noop = *stmt.1 {
            true
        } else {
            false
        }
    }

    // TODO: rename to p_stmt_list
    fn handle_loop_body(pos: Pos, node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Stmt<,>) {
        let list = Self::as_list(node);
        let mut result = vec![];
        for n in list.iter() {
            match &n.syntax {
                UsingStatementFunctionScoped(_) => not_impl!(),
                _ => {
                    let mut h = Self::p_stmt_unsafe(n, env)?;
                    result.append(&mut h);
                }
            }
        }
        let blk: Vec<_> = result
            .into_iter()
            .filter(|stmt| !Self::is_noop(stmt))
            .collect();
        let body = if blk.len() == 0 {
            vec![Self::mk_noop()]
        } else {
            blk
        };
        Ok(aast::Stmt(pos, Box::new(aast::Stmt_::Block(body))))
    }

    fn p_stmt(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Stmt<,>) {
        // TODO: clear_statement_scope & extract_and_push_docblock
        // TODO: add lift_awaits_in_statement
        let pos = Self::p_pos(node, env);
        match &node.syntax {
            CompoundStatement(c) => Self::handle_loop_body(pos, &c.compound_statements, env),
            ReturnStatement(c) => {
                let expr = match &c.return_expression.syntax {
                    Missing => None,
                    _ => Some(Self::p_expr_with_loc(
                        ExprLocation::RightOfReturn,
                        &c.return_expression,
                        env,
                    )?),
                };

                Ok(aast::Stmt(pos, Box::new(aast::Stmt_::Return(expr))))
            }
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

    fn p_modifiers<R>(
        on_kind: &dyn Fn(R, modifier::Kind, &Syntax<T, V>, &mut Env) -> R,
        mut init: R,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!((modifier::KindSet, R)) {
        let nodes = Self::as_list(node);
        let mut kind_set = modifier::KindSet::new();
        for n in nodes.iter() {
            let token_kind = Self::token_kind(n).map_or(None, modifier::from_token_kind);
            match token_kind {
                Some(kind) => {
                    kind_set.add(kind);
                    init = on_kind(init, kind, n, env);
                }
                _ => Self::missing_syntax(None, "kind", n, env)?,
            }
        }
        Ok((kind_set, init))
    }

    fn p_kinds(node: &Syntax<T, V>, env: &mut Env) -> ret!(modifier::KindSet) {
        Self::p_modifiers(&|_, _, _, _| {}, (), node, env).map(|r| r.0)
    }

    // TODO: change name to map_flatten after porting
    #[inline]
    fn could_map<R, F>(f: &F, node: &Syntax<T, V>, env: &mut Env) -> ret!(Vec<R>)
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> ret!(R),
    {
        Self::map_flatten_(f, node, env, vec![])
    }

    #[inline]
    fn map_flatten_<R, F>(f: &F, node: &Syntax<T, V>, env: &mut Env, acc: Vec<R>) -> ret!(Vec<R>)
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> ret!(R),
    {
        Self::map_fold(
            f,
            &|mut v: Vec<R>, a| {
                v.push(a);
                v
            },
            node,
            env,
            acc,
        )
    }

    fn map_fold<A, R, F, O>(f: &F, op: &O, node: &Syntax<T, V>, env: &mut Env, acc: A) -> ret!(A)
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> ret!(R),
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

    fn p_visibility(node: &Syntax<T, V>, env: &mut Env) -> ret!(Option<aast!(Visibility)>) {
        let first_vis = |r: Option<aast!(Visibility)>, kind, _: &Syntax<T, V>, _: &mut Env| {
            r.or_else(|| modifier::to_visibility(kind))
        };
        Self::p_modifiers(&first_vis, None, node, env).map(|r| r.1)
    }

    fn p_visibility_or(
        node: &Syntax<T, V>,
        env: &mut Env,
        default: aast!(Visibility),
    ) -> ret!(aast!(Visibility)) {
        Self::p_visibility(node, env).map(|v| v.unwrap_or(default))
    }

    fn p_visibility_last_win(
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!(Option<aast!(Visibility)>) {
        let last_vis = |r, kind, _: &Syntax<T, V>, _: &mut Env| modifier::to_visibility(kind).or(r);
        Self::p_modifiers(&last_vis, None, node, env).map(|r| r.1)
    }

    fn p_visibility_last_win_or(
        node: &Syntax<T, V>,
        env: &mut Env,
        default: aast!(Visibility),
    ) -> ret!(aast!(Visibility)) {
        Self::p_visibility_last_win(node, env).map(|v| v.unwrap_or(default))
    }

    fn has_soft(attrs: &[aast!(UserAttribute<,>)]) -> bool {
        attrs.iter().any(|attr| attr.name.1 == "__Soft")
    }

    fn soften_hint(attrs: &[aast!(UserAttribute<,>)], hint: aast!(Hint)) -> aast!(Hint) {
        if Self::has_soft(attrs) {
            aast::Hint(hint.0.clone(), Box::new(aast::Hint_::Hsoft(hint)))
        } else {
            hint
        }
    }

    fn p_fun_param_default_value(
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!(Option<aast!(Expr<,>)>) {
        match &node.syntax {
            SimpleInitializer(c) => {
                Self::mp_optional(&Self::p_expr, &c.simple_initializer_value, env)
            }
            _ => Ok(None),
        }
    }

    fn p_param_kind(node: &Syntax<T, V>, env: &mut Env) -> ret!(ast_defs::ParamKind) {
        match Self::token_kind(node) {
            Some(TK::Inout) => Ok(ast_defs::ParamKind::Pinout),
            _ => Self::missing_syntax(None, "param kind", node, env),
        }
    }

    fn param_template(node: &Syntax<T, V>, env: &Env) -> aast!(FunParam<,>) {
        let pos = Self::p_pos(node, env);
        aast::FunParam {
            annotation: pos.clone(),
            type_hint: aast::TypeHint((), None),
            is_reference: false,
            is_variadic: false,
            pos,
            name: Self::text(node, env),
            expr: None,
            callconv: None,
            user_attributes: vec![],
            visibility: None,
        }
    }

    fn p_fun_param(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(FunParam<,>) {
        match &node.syntax {
            ParameterDeclaration(child) => {
                let parameter_attribute = &child.parameter_attribute;
                let parameter_visibility = &child.parameter_visibility;
                let parameter_call_convention = &child.parameter_call_convention;
                let parameter_type = &child.parameter_type;
                let parameter_name = &child.parameter_name;
                let parameter_default_value = &child.parameter_default_value;
                let (is_reference, is_variadic, name) = match &parameter_name.syntax {
                    DecoratedExpression(child) => {
                        let decorated_expression_decorator = &child.decorated_expression_decorator;
                        let decorated_expression_expression =
                            &child.decorated_expression_expression;
                        let decorator = Self::text_str(decorated_expression_decorator, env);
                        match &decorated_expression_expression.syntax {
                            DecoratedExpression(child) => {
                                let nested_expression = &child.decorated_expression_expression;
                                let nested_decorator =
                                    Self::text_str(&child.decorated_expression_decorator, env);
                                (
                                    decorator == "&" || nested_decorator == "&",
                                    decorator == "..." || nested_decorator == "...",
                                    nested_expression,
                                )
                            }
                            _ => (
                                decorator == "&",
                                decorator == "...",
                                decorated_expression_expression,
                            ),
                        }
                    }
                    _ => (false, false, parameter_name),
                };
                let user_attributes = Self::p_user_attributes(&parameter_attribute, env)?;
                let hint = Self::mp_optional(&Self::p_hint, parameter_type, env)?
                    .map(|h| Self::soften_hint(&user_attributes, h));
                let pos = Self::p_pos(name, env);
                Ok(aast::FunParam {
                    annotation: pos.clone(),
                    type_hint: aast::TypeHint((), hint),
                    user_attributes,
                    is_reference,
                    is_variadic,
                    pos,
                    name: Self::text(name, env),
                    expr: Self::p_fun_param_default_value(parameter_default_value, env)?,
                    callconv: Self::mp_optional(
                        &Self::p_param_kind,
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
            _ => Self::missing_syntax(None, "function parameter", node, env),
        }
    }

    fn p_tconstraint_ty(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Hint) {
        match &node.syntax {
            TypeConstraint(c) => Self::p_hint(&c.constraint_type, env),
            _ => Self::missing_syntax(None, "type constraint", node, env),
        }
    }

    fn p_tconstraint(
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!((ast_defs::ConstraintKind, aast!(Hint))) {
        match &node.syntax {
            TypeConstraint(c) => Ok((
                match Self::token_kind(&c.constraint_keyword) {
                    Some(TK::As) => ast_defs::ConstraintKind::ConstraintAs,
                    Some(TK::Super) => ast_defs::ConstraintKind::ConstraintSuper,
                    Some(TK::Equal) => ast_defs::ConstraintKind::ConstraintEq,
                    _ => Self::missing_syntax(
                        None,
                        "constriant operator",
                        &c.constraint_keyword,
                        env,
                    )?,
                },
                Self::p_hint(&c.constraint_type, env)?,
            )),
            _ => Self::missing_syntax(None, "type constriant", node, env),
        }
    }

    fn p_tparam(is_class: bool, node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Tparam<,>) {
        match &node.syntax {
            TypeParameter(c) => {
                let user_attributes = Self::p_user_attributes(&c.type_attribute_spec, env)?;
                let is_reified = !&c.type_reified.is_missing();
                if is_class && is_reified {
                    env.cls_reified_generics
                        .insert(Self::text(&c.type_name, env));
                }
                let variance = match Self::token_kind(&c.type_variance) {
                    Some(TK::Plus) => ast_defs::Variance::Covariant,
                    Some(TK::Minus) => ast_defs::Variance::Contravariant,
                    _ => ast_defs::Variance::Invariant,
                };
                if is_reified && variance != ast_defs::Variance::Invariant {
                    Self::raise_parsing_error(
                        node,
                        env,
                        &syntax_error::non_invariant_reified_generic,
                    );
                }
                let reified = match (is_reified, Self::has_soft(&user_attributes)) {
                    (true, true) => aast::ReifyKind::SoftReified,
                    (true, false) => aast::ReifyKind::Reified,
                    _ => aast::ReifyKind::Erased,
                };
                Ok(aast::Tparam {
                    variance,
                    name: Self::pos_name(&c.type_name, env)?,
                    constraints: Self::could_map(&Self::p_tconstraint, &c.type_constraints, env)?,
                    reified,
                    user_attributes,
                })
            }
            _ => Self::missing_syntax(None, "type parameter", node, env),
        }
    }

    fn p_tparam_l(
        is_class: bool,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!(Vec<aast!(Tparam<,>)>) {
        match &node.syntax {
            Missing => Ok(vec![]),
            TypeParameters(c) => Self::could_map(
                &|n, e| Self::p_tparam(is_class, n, e),
                &c.type_parameters_parameters,
                env,
            ),
            _ => Self::missing_syntax(None, "type parameter", node, env),
        }
    }

    fn p_fun_hdr(
        modifier_checker: &dyn Fn((), modifier::Kind, &Syntax<T, V>, &mut Env),
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!(FunHdr) {
        match &node.syntax {
            FunctionDeclarationHeader(child) => {
                let function_modifiers = &child.function_modifiers;
                let function_name = &child.function_name;
                let function_where_clause = &child.function_where_clause;
                let function_type_parameter_list = &child.function_type_parameter_list;
                let function_parameter_list = &child.function_parameter_list;
                let function_type = &child.function_type;
                // TODO: use Naming_special_names
                let is_autoload =
                    Self::text_str(function_name, env).eq_ignore_ascii_case("__autoload");
                if function_name.value.is_missing() {
                    Self::raise_parsing_error(node, env, &syntax_error::empty_method_name);
                }
                let num_params = Self::syntax_to_list(false, function_parameter_list).len();
                if is_autoload && num_params > 1 {
                    Self::raise_parsing_error(
                        node,
                        env,
                        &syntax_error::autoload_takes_one_argument,
                    );
                }
                let (kinds, _) = Self::p_modifiers(modifier_checker, (), function_modifiers, env)?;
                let has_async = kinds.has(modifier::ASYNC);
                let has_coroutine = kinds.has(modifier::COROUTINE);
                let fh_parameters =
                    Self::could_map(&Self::p_fun_param, function_parameter_list, env)?;
                let fh_return_type = Self::mp_optional(&Self::p_hint, function_type, env)?;
                let fh_suspension_kind =
                    Self::mk_suspension_kind_(node, env, has_async, has_coroutine);
                let fh_name = Self::pos_name(function_name, env)?;
                let fh_constrs = match &function_where_clause.syntax {
                    Missing => vec![],
                    WhereClause(_) => not_impl!(),
                    _ => Self::missing_syntax(None, "function header constraints", node, env)?,
                };

                let fh_type_parameters =
                    Self::p_tparam_l(false, function_type_parameter_list, env)?;
                Ok(FunHdr {
                    fh_suspension_kind,
                    fh_name,
                    fh_constrs,
                    fh_type_parameters,
                    fh_parameters,
                    fh_return_type,
                })
            }
            LambdaSignature(_) => not_impl!(),
            Token(_) => Ok(FunHdr::make_empty()),
            _ => Self::missing_syntax(None, "function header", node, env),
        }
    }

    fn determine_variadicity(params: &[aast!(FunParam<,>)]) -> aast!(FunVariadicity<,>) {
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

    // TODO: change name to p_fun_pos after porting.
    fn p_function(node: &Syntax<T, V>, env: &Env) -> Pos {
        let get_pos = |n: &Syntax<T, V>, p: Pos| -> Pos {
            if let FunctionDeclarationHeader(c1) = &n.syntax {
                if !c1.function_keyword.value.is_missing() {
                    return Pos::btw_nocheck(Self::p_pos(n, env), p);
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

    fn p_stmt_unsafe(node: &Syntax<T, V>, env: &mut Env) -> ret!(Vec<aast!(Stmt<,>)>) {
        Ok(vec![Self::p_stmt(node, env)?])
    }

    fn p_block(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Block<,>) {
        let mut stmts = Self::p_stmt_unsafe(node, env)?;
        let last = stmts.pop();
        if let Some(stmt) = last {
            if let aast::Stmt_::Block(mut b) = *stmt.1 {
                stmts.append(&mut b)
            }
        }
        Ok(stmts)
    }

    fn mk_noop() -> aast!(Stmt<,>) {
        aast::Stmt(Pos::make_none(), Box::new(aast::Stmt_::Noop))
    }

    fn p_function_body(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Block<,>) {
        let mk_noop_result = || Ok(vec![Self::mk_noop()]);
        // TODO: with_new_nonconcurrent_scrope
        match &node.syntax {
            Missing => Ok(vec![]),
            CompoundStatement(c) => {
                let compound_statements = &c.compound_statements.syntax;
                let compound_right_brace = &c.compound_right_brace.syntax;
                match (compound_statements, compound_right_brace) {
                    (Missing, Token(_)) => mk_noop_result(),
                    (SyntaxList(t), _) if t.len() == 1 && t[0].is_yield() => {
                        env.saw_yield = true;
                        mk_noop_result()
                    }
                    _ => {
                        if !env.top_level_statements
                            && (env.file_mode() == file_info::Mode::Mdecl && env.codegen()
                                || env.quick_mode)
                        {
                            mk_noop_result()
                        } else {
                            Self::p_block(node, env)
                        }
                    }
                }
            }
            _ => not_impl!(),
        }
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

    fn mk_fun_kind(suspension_kind: SuspensionKind, yield_: bool) -> ast_defs::FunKind {
        use ast_defs::FunKind::*;
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
    ) -> ret_aast!(UserAttribute<,>) {
        let name = Self::pos_name(constructor_call_type, env)?;
        if name.1.eq_ignore_ascii_case("__reified")
            || name.1.eq_ignore_ascii_case("__hasreifiedparent")
        {
            Self::raise_parsing_error(node, env, &syntax_error::reified_attribute);
        } else if name.1.eq_ignore_ascii_case("__soft")
            && Self::as_list(constructor_call_argument_list).len() > 0
        {
            Self::raise_parsing_error(node, env, &syntax_error::soft_no_arguments);
        }
        let params = Self::could_map(
            &|n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(Expr<,>) {
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
        Ok(aast::UserAttribute { name, params })
    }

    fn p_user_attribute(node: &Syntax<T, V>, env: &mut Env) -> ret!(Vec<aast!(UserAttribute<,>)>) {
        let p_attr = |n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(UserAttribute<,>) {
            match &n.syntax {
                ConstructorCall(c) => Self::process_attribute_constructor_call(
                    node,
                    &c.constructor_call_argument_list,
                    &c.constructor_call_type,
                    e,
                ),
                _ => Self::missing_syntax(None, "attribute", node, e),
            }
        };
        match &node.syntax {
            FileAttributeSpecification(c) => {
                Self::could_map(&p_attr, &c.file_attribute_specification_attributes, env)
            }
            OldAttributeSpecification(c) => {
                Self::could_map(&p_attr, &c.old_attribute_specification_attributes, env)
            }
            AttributeSpecification(c) => Self::could_map(
                &|n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(UserAttribute<,>) {
                    match &n.syntax {
                        Attribute(c) => p_attr(&c.attribute_attribute_name, e),
                        _ => Self::missing_syntax(None, "attribute", node, e),
                    }
                },
                &c.attribute_specification_attributes,
                env,
            ),
            _ => Self::missing_syntax(None, "attribute specification", node, env),
        }
    }

    fn p_user_attributes(node: &Syntax<T, V>, env: &mut Env) -> ret!(Vec<aast!(UserAttribute<,>)>) {
        Self::map_fold(
            &Self::p_user_attribute,
            &|mut acc: Vec<aast!(UserAttribute<,>)>, mut x: Vec<aast!(UserAttribute<,>)>| {
                acc.append(&mut x);
                acc
            },
            node,
            env,
            vec![],
        )
    }

    fn mp_yielding<R>(
        p: &dyn Fn(&Syntax<T, V>, &mut Env) -> ret!(R),
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!((R, bool)) {
        let outer_saw_yield = env.saw_yield;
        env.saw_yield = false;
        let r = p(node, env);
        env.saw_yield = outer_saw_yield;
        let result = (r?, env.saw_yield);
        Ok(result)
    }

    fn mk_empty_ns_env(env: &Env) -> NamespaceEnv {
        NamespaceEnv::empty(env.auto_ns_map.to_vec(), env.codegen())
    }

    fn extract_docblock(node: &Syntax<T, V>, env: &Env) -> Option<String> {
        #[derive(Copy, Clone, Eq, PartialEq)]
        enum State {
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
        use State::*;
        // `parse` mixes loop and recursion to use less stack space.
        fn parse(str: &str, start: usize, state: State, idx: usize) -> Option<String> {
            let is_whitespace = |c| c == ' ' || c == '\t' || c == '\n' || c == '\r';
            let mut s = (start, state, idx); // (start, state, index)
            loop {
                if s.2 == str.len() {
                    break None;
                }

                let next = s.2 + 1;
                match (s.1, str.chars().nth(s.2).unwrap()) {
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
        let str = node.leading_text(env.indexed_source_text.source_text);
        parse(str, 0, Free, 0)
    }

    fn p_xhp_child(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(XhpChild) {
        use aast::XhpChild::*;
        use aast::XhpChildOp::*;
        match &node.syntax {
            Token(_) => Self::pos_name(node, env).map(ChildName),
            PostfixUnaryExpression(c) => {
                let operand = Self::p_xhp_child(&c.postfix_unary_operand, env)?;
                let operator = match Self::token_kind(&c.postfix_unary_operator) {
                    Some(TK::Question) => ChildQuestion,
                    Some(TK::Plus) => ChildPlus,
                    Some(TK::Star) => ChildStar,
                    _ => Self::missing_syntax(None, "xhp children operator", node, env)?,
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
            _ => Self::missing_syntax(None, "xhp children", node, env),
        }
    }

    fn p_class_elt_(class: &mut aast!(Class_<,>), node: &Syntax<T, V>, env: &mut Env) -> ret!(()) {
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
        let p_method_vis = |node: &Syntax<T, V>, env: &mut Env| -> ret_aast!(Visibility) {
            match Self::p_visibility_last_win(node, env)? {
                None => {
                    Self::raise_nast_error("method_needs_visiblity");
                    Ok(aast::Visibility::Public)
                }
                Some(v) => Ok(v),
            }
        };
        match &node.syntax {
            ConstDeclaration(c) => {
                // TODO: make wrap `type_` `doc_comment` by `Rc` in ClassConst to avoid clone
                let vis = Self::p_visibility_or(&c.const_modifiers, env, aast::Visibility::Public)?;
                let type_ = Self::mp_optional(&Self::p_hint, &c.const_type_specifier, env)?;
                // using map_fold can save one Vec allocation, but ocaml's behavior is that
                // if anything throw, it will discard all lowered elements. So adding to class
                // must be at the last.
                let mut class_consts = Self::could_map(
                    &|n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(ClassConst<,>) {
                        match &n.syntax {
                            ConstantDeclarator(c) => {
                                let id = Self::pos_name(&c.constant_declarator_name, e)?;
                                let expr = if n.is_abstract() {
                                    None
                                } else {
                                    Self::mp_optional(
                                        &Self::p_simple_initializer,
                                        &c.constant_declarator_initializer,
                                        e,
                                    )?
                                };
                                Ok(aast::ClassConst {
                                    visibility: vis,
                                    type_: type_.clone(),
                                    id,
                                    expr,
                                    doc_comment: doc_comment_opt.clone(),
                                })
                            }
                            _ => Self::missing_syntax(None, "constant declarator", n, e),
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
                let type__ = Self::mp_optional(&Self::p_hint, &c.type_const_type_specifier, env)?
                    .map(|hint| Self::soften_hint(&user_attributes, hint));
                let kinds = Self::p_kinds(&c.type_const_modifiers, env)?;
                let name = Self::pos_name(&c.type_const_name, env)?;
                let constraint =
                    Self::mp_optional(&Self::p_tconstraint_ty, &c.type_const_type_constraint, env)?;
                let span = Self::p_pos(node, env);
                let visibility = Self::p_visibility(&c.type_const_modifiers, env)?
                    .unwrap_or(aast::Visibility::Public);
                let has_abstract = kinds.has(modifier::ABSTRACT);
                let (type_, abstract_kind) = match (has_abstract, &constraint, &type__) {
                    (false, _, None) => {
                        Self::raise_nast_error("not_abstract_without_typeconst");
                        (constraint.clone(), aast::TypeconstAbstractKind::TCConcrete)
                    }
                    (false, None, Some(_)) => (type__, aast::TypeconstAbstractKind::TCConcrete),
                    (false, Some(_), Some(_)) => {
                        (type__, aast::TypeconstAbstractKind::TCPartiallyAbstract)
                    }
                    (true, _, None) => (
                        type__.clone(),
                        aast::TypeconstAbstractKind::TCAbstract(type__),
                    ),
                    (true, _, Some(_)) => (None, aast::TypeconstAbstractKind::TCAbstract(type__)),
                };
                Ok(class.typeconsts.push(aast::ClassTypeconst {
                    abstract_: abstract_kind,
                    visibility,
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
                let type_ = Self::mp_optional(&Self::p_hint, &c.property_type, env)?
                    .map(|t| Self::soften_hint(&user_attributes, t));
                let modifier_checker = |_, _, n: &Syntax<T, V>, e: &mut Env| -> () {
                    if n.is_final() {
                        Self::raise_parsing_error(n, e, &syntax_error::declared_final("Properties"))
                    }
                };
                let kinds = Self::p_modifiers(&modifier_checker, (), &c.property_modifiers, env)?.0;
                let vis = Self::p_visibility_last_win_or(
                    &c.property_modifiers,
                    env,
                    aast::Visibility::Public,
                )?;
                let doc_comment = if env.quick_mode {
                    None
                } else {
                    doc_comment_opt
                };
                let name_exprs = Self::could_map(
                    &|n, e| -> ret!((Pos, aast!(Sid), Option<aast!(Expr<,>)>)) {
                        match &n.syntax {
                            PropertyDeclarator(c) => {
                                let name = Self::pos_name_(&c.property_name, e, Some('$'))?;
                                let pos = Self::p_pos(n, e);
                                let expr = Self::mp_optional(
                                    &Self::p_simple_initializer,
                                    &c.property_initializer,
                                    e,
                                )?;
                                Ok((pos, name, expr))
                            }
                            _ => Self::missing_syntax(None, "property declarator", n, e),
                        }
                    },
                    &c.property_declarators,
                    env,
                )?;

                let mut i = 0;
                for name_expr in name_exprs.into_iter() {
                    class.vars.push(aast::ClassVar {
                        final_: kinds.has(modifier::FINAL),
                        xhp_attr: None,
                        abstract_: kinds.has(modifier::ABSTRACT),
                        visibility: vis,
                        type_: type_.clone(),
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
                let classvar_init =
                    |param: &aast!(FunParam<,>)| -> (aast!(Stmt<,>), aast!(ClassVar<,>)) {
                        let cvname = Self::drop_prefix(&param.name, '$');
                        let p = &param.pos;
                        let span = match &param.expr {
                            Some(aast::Expr(pos_end, _)) => {
                                Pos::btw(p, pos_end).unwrap_or_else(|_| p.clone())
                            }
                            _ => p.clone(),
                        };
                        let e = |expr_: aast!(Expr_<,>)| -> aast!(Expr<,>) {
                            aast::Expr(p.clone(), Box::new(expr_))
                        };
                        let lid =
                            |s: &str| -> aast!(Lid) { aast::Lid(p.clone(), (0, s.to_string())) };
                        use aast::Expr_::*;
                        (
                            aast::Stmt(
                                p.clone(),
                                Box::new(aast::Stmt_::Expr(e(Binop(
                                    ast_defs::Bop::Eq(None),
                                    e(ObjGet(
                                        e(Lvar(lid("$this"))),
                                        e(Id(ast_defs::Id(p.clone(), cvname.to_string()))),
                                        aast::OgNullFlavor::OGNullthrows,
                                    )),
                                    e(Lvar(lid(&param.name))),
                                )))),
                            ),
                            aast::ClassVar {
                                final_: false,
                                xhp_attr: None,
                                abstract_: false,
                                visibility: param.visibility.unwrap(),
                                type_: param.type_hint.1.clone(),
                                id: ast_defs::Id(p.clone(), cvname.to_string()),
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
                let hdr = Self::p_fun_hdr(&|_, _, _, _| {}, header, env)?;
                if hdr.fh_name.1 == "__construct" && !hdr.fh_type_parameters.is_empty() {
                    Self::raise_parsing_error(
                        header,
                        env,
                        &syntax_error::no_generics_on_constructors,
                    );
                }
                let (mut member_init, mut member_def): (
                    Vec<aast!(Stmt<,>)>,
                    Vec<aast!(ClassVar<,>)>,
                ) = hdr
                    .fh_parameters
                    .iter()
                    .filter_map(|p| p.visibility.map(|_| classvar_init(p)))
                    .unzip();

                let kinds = Self::p_kinds(&h.function_modifiers, env)?;
                let visibility = p_method_vis(&h.function_modifiers, env)?;
                let is_static = kinds.has(modifier::STATIC);
                env.in_static_method = is_static;
                let (mut body, body_has_yield) =
                    Self::mp_yielding(&Self::p_function_body, &c.methodish_function_body, env)?;
                if env.codegen() {
                    member_init.reverse();
                }
                member_init.append(&mut body);
                let body = member_init;
                env.in_static_method = false;
                let is_abstract = kinds.has(modifier::ABSTRACT);
                let is_external = !is_abstract && c.methodish_function_body.is_external();
                let user_attributes = Self::p_user_attributes(&c.methodish_attribute, env)?;
                let method = aast::Method_ {
                    span: Self::p_function(node, env),
                    annotation: (),
                    final_: kinds.has(modifier::FINAL),
                    abstract_: is_abstract,
                    static_: is_static,
                    name: hdr.fh_name,
                    visibility,
                    tparams: hdr.fh_type_parameters,
                    where_constraints: hdr.fh_constrs,
                    variadic: Self::determine_variadicity(&hdr.fh_parameters),
                    params: hdr.fh_parameters,
                    body: aast::FuncBody {
                        annotation: (),
                        ast: body,
                    },
                    fun_kind: Self::mk_fun_kind(hdr.fh_suspension_kind, body_has_yield),
                    user_attributes,
                    ret: aast::TypeHint((), hdr.fh_return_type),
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
                let hdr = Self::p_fun_hdr(&|_, _, _, _| {}, header, env)?;
                let kind = Self::p_kinds(&h.function_modifiers, env)?;
                let (qualifier, name) = match &c.methodish_trait_name.syntax {
                    ScopeResolutionExpression(c) => (
                        Self::p_hint(&c.scope_resolution_qualifier, env)?,
                        Self::p_pstring(&c.scope_resolution_name, env)?,
                    ),
                    _ => Self::missing_syntax(None, "trait method redeclaration", node, env)?,
                };
                let user_attributes = Self::p_user_attributes(&c.methodish_trait_attribute, env)?;
                let visibility = p_method_vis(&h.function_modifiers, env)?;
                let mtr = aast::MethodRedeclaration {
                    final_: kind.has(modifier::FINAL),
                    abstract_: kind.has(modifier::ABSTRACT),
                    static_: kind.has(modifier::STATIC),
                    visibility,
                    name: hdr.fh_name,
                    tparams: hdr.fh_type_parameters,
                    where_constraints: hdr.fh_constrs,
                    variadic: Self::determine_variadicity(&hdr.fh_parameters),
                    params: hdr.fh_parameters,
                    fun_kind: Self::mk_fun_kind(hdr.fh_suspension_kind, false),
                    ret: aast::TypeHint((), hdr.fh_return_type),
                    trait_: qualifier,
                    method: name,
                    user_attributes,
                };
                class.method_redeclarations.push(mtr);
                Ok(())
            }
            TraitUseConflictResolution(c) => {
                type Ret = ret!(Either<aast!(InsteadofAlias), aast!(UseAsAlias)>);
                let p_item = |n: &Syntax<T, V>, e: &mut Env| -> Ret {
                    match &n.syntax {
                        TraitUsePrecedenceItem(c) => {
                            let removed_names = &c.trait_use_precedence_item_removed_names;
                            let (qualifier, name) = match &c.trait_use_precedence_item_name.syntax {
                                ScopeResolutionExpression(c) => (
                                    Self::pos_name(&c.scope_resolution_qualifier, e)?,
                                    Self::p_pstring(&c.scope_resolution_name, e)?,
                                ),
                                _ => Self::missing_syntax(None, "trait use precedence item", n, e)?,
                            };
                            let removed_names = Self::could_map(&Self::pos_name, removed_names, e)?;
                            Self::raise_nast_error("unsupported_instead_of");
                            Ok(Either::Left(aast::InsteadofAlias(
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
                                &|mut acc, kind, n, e| -> Vec<aast_defs::UseAsVisibility> {
                                    if let Some(v) = modifier::to_use_as_visibility(kind) {
                                        acc.push(v);
                                    }
                                    acc
                                },
                                vec![],
                                modifiers,
                                e,
                            )?;
                            if !kinds.has_any(modifier::USE_AS_VISIBILITY) {
                                Self::raise_parsing_error(n, e, &syntax_error::trait_alias_rule_allows_only_final_and_visibility_modifiers);
                            }
                            let vis = if !kinds.has_any(modifier::VISIBILITIES) {
                                let mut v = vec![aast::UseAsVisibility::UseAsPublic];
                                v.append(&mut vis_raw);
                                v
                            } else {
                                vis_raw
                            };
                            let aliased_name = if !aliased_name.is_missing() {
                                Some(Self::pos_name(aliased_name, e)?)
                            } else {
                                None
                            };
                            Self::raise_nast_error("unsupported_trait_use_as");
                            Ok(Either::Right(aast::UseAsAlias(
                                qualifier,
                                name,
                                aliased_name,
                                vis,
                            )))
                        }
                        _ => Self::missing_syntax(None, "trait use conflict resolution item", n, e),
                    }
                };
                let mut uses =
                    Self::could_map(&Self::p_hint, &c.trait_use_conflict_resolution_names, env)?;
                let elts = Self::could_map(&p_item, &c.trait_use_conflict_resolution_clauses, env)?;
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
                let mut uses = Self::could_map(&Self::p_hint, &c.trait_use_names, env)?;
                Ok(class.uses.append(&mut uses))
            }
            RequireClause(c) => {
                let is_extends = match Self::token_kind(&c.require_kind) {
                    Some(TK::Implements) => false,
                    Some(TK::Extends) => true,
                    _ => Self::missing_syntax(None, "trait require kind", &c.require_kind, env)?,
                };
                let hint = Self::p_hint(&c.require_name, env)?;
                Ok(class.reqs.push((hint, is_extends)))
            }
            XHPClassAttributeDeclaration(c) => {
                type Ret = ret!(Either<aast!(XhpAttr<,>), aast!(Hint)>);
                let mut p_attr = |node: &Syntax<T, V>, env: &mut Env| -> Ret {
                    let mut mk_attr_use = |n: &Syntax<T, V>| {
                        Ok(Either::Right(aast::Hint(
                            Self::p_pos(n, env),
                            Box::new(aast::Hint_::Happly(Self::pos_name(n, env)?, vec![])),
                        )))
                    };
                    match &node.syntax {
                        XHPClassAttribute(c) => {
                            let ty = &c.xhp_attribute_decl_type;
                            let init = &c.xhp_attribute_decl_initializer;
                            let ast_defs::Id(p, name) =
                                Self::pos_name(&c.xhp_attribute_decl_name, env)?;
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
                                XHPRequired(_) => Some(aast::XhpAttrTag::Required),
                                XHPLateinit(_) => Some(aast::XhpAttrTag::LateInit),
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
                                        Self::could_map(&Self::p_expr, &c.xhp_enum_values, env)?;
                                    (None, Some((p, opt, vals)))
                                }
                                _ => (Some(Self::p_hint(ty, env)?), None),
                            };
                            let init_expr =
                                Self::mp_optional(&Self::p_simple_initializer, init, env)?;
                            let xhp_attr = aast::XhpAttr(
                                hint.clone(),
                                aast::ClassVar {
                                    final_: false,
                                    xhp_attr: Some(aast::XhpAttrInfo { xai_tag: req }),
                                    abstract_: false,
                                    visibility: aast::Visibility::Public,
                                    type_: hint,
                                    id: ast_defs::Id(p, String::from(":") + &name),
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
                        _ => Self::missing_syntax(None, "XHP attribute", node, env),
                    }
                };
                let attrs = Self::could_map(&mut p_attr, &c.xhp_attribute_attributes, env)?;
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
                    &|n, e| Self::p_pstring_(n, e, Some('%')),
                    &c.xhp_category_categories,
                    env,
                )?;
                if let Some((_, cs)) = &class.xhp_category {
                    if cs.len() > 0 {
                        Self::raise_nast_error("multiple_xhp_category")
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
                                        Self::missing_syntax(None, "pumapping", map, env)?;
                                    }
                                }
                            }
                            members.push(aast::PuMember {
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
                            case_types.push(id);
                        }
                        _ => {
                            Self::missing_syntax(None, "pufield", fld, env)?;
                        }
                    }
                }
                Ok(class.pu_enums.push(aast::PuEnum {
                    name: id,
                    is_final,
                    case_types,
                    case_values,
                    members,
                }))
            }
            _ => Self::missing_syntax(None, "class element", node, env),
        }
    }

    fn p_class_elt(class: &mut aast!(Class_<,>), node: &Syntax<T, V>, env: &mut Env) -> ret!(()) {
        let r = Self::p_class_elt_(class, node, env);
        match r {
            // match ocaml behavior, don't throw if missing syntax when fail_open is true
            Err(Error::APIMissingSyntax { .. }) if env.fail_open => Ok(()),
            _ => r,
        }
    }

    fn contains_class_body(c: &ClassishDeclarationChildren<T, V>) -> bool {
        match &c.classish_body.syntax {
            ClassishBody(_) => true,
            _ => false,
        }
    }

    fn p_def(node: &Syntax<T, V>, env: &mut Env) -> ret!(Vec<aast!(Def<,>)>) {
        let doc_comment_opt = Self::extract_docblock(node, env);
        match &node.syntax {
            FunctionDeclaration(child) => {
                let function_attribute_spec = &child.function_attribute_spec;
                let function_declaration_header = &child.function_declaration_header;
                let function_body = &child.function_body;
                // TOOD: env = non_tls env;
                let allowed_kinds =
                    modifier::KindSet::from_kinds(&[modifier::ASYNC, modifier::COROUTINE]);
                let modifier_checker =
                    |_: (), kind: modifier::Kind, node: &Syntax<T, V>, env: &mut Env| {
                        if !allowed_kinds.has(kind) {
                            Self::raise_parsing_error(
                                node,
                                env,
                                &syntax_error::function_modifier(Self::text_str(node, env)),
                            );
                        }
                    };
                let hdr = Self::p_fun_hdr(&modifier_checker, function_declaration_header, env)?;
                let is_external = function_body.is_external();
                let (block, yield_) = if is_external {
                    (vec![], false)
                } else {
                    Self::mp_yielding(&Self::p_function_body, function_body, env)?
                };
                let user_attributes = Self::p_user_attributes(function_attribute_spec, env)?;
                let variadic = Self::determine_variadicity(&hdr.fh_parameters);
                let ret = aast::TypeHint((), hdr.fh_return_type);
                Ok(vec![aast::Def::Fun(aast::Fun_ {
                    span: Self::p_function(node, env),
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    ret,
                    name: hdr.fh_name,
                    tparams: hdr.fh_type_parameters,
                    where_constraints: hdr.fh_constrs,
                    params: hdr.fh_parameters,
                    body: aast::FuncBody {
                        ast: block,
                        annotation: (),
                    },
                    fun_kind: Self::mk_fun_kind(hdr.fh_suspension_kind, yield_),
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
                // TOOD: env = non_tls env
                let mode = Self::mode_annotation(env.file_mode());
                let user_attributes = Self::p_user_attributes(&c.classish_attribute, env)?;
                let kinds = Self::p_kinds(&c.classish_modifiers, env)?;
                let final_ = kinds.has(modifier::FINAL);
                let is_xhp = match Self::token_kind(&c.classish_name) {
                    Some(TK::XHPElementName) => true,
                    Some(TK::XHPClassName) => true,
                    _ => false,
                };
                let name = Self::pos_name(&c.classish_name, env)?;
                env.cls_reified_generics = HashSet::new();
                let tparams = aast::ClassTparams {
                    list: Self::p_tparam_l(true, &c.classish_type_parameters, env)?,
                    constraints: s_map::SMap::new(),
                };
                let extends = Self::could_map(&Self::p_hint, &c.classish_extends_list, env)?;
                // TODO: env.parent_may_reified =
                let implements = Self::could_map(&Self::p_hint, &c.classish_implements_list, env)?;
                let where_constraints = match &c.classish_where_clause.syntax {
                    Missing => vec![],
                    WhereClause(_) => not_impl!(),
                    _ => Self::missing_syntax(None, "class constraints", node, env)?,
                };
                let namespace = Self::mk_empty_ns_env(env);
                let span = Self::p_pos(node, env);
                let class_kind = match Self::token_kind(&c.classish_keyword) {
                    Some(TK::Class) if kinds.has(modifier::ABSTRACT) => {
                        ast_defs::ClassKind::Cabstract
                    }
                    Some(TK::Class) => ast_defs::ClassKind::Cnormal,
                    Some(TK::Interface) => ast_defs::ClassKind::Cinterface,
                    Some(TK::Trait) => ast_defs::ClassKind::Ctrait,
                    Some(TK::Enum) => ast_defs::ClassKind::Cenum,
                    _ => Self::missing_syntax(None, "class kind", &c.classish_keyword, env)?,
                };
                let mut class_ = aast::Class_ {
                    span,
                    annotation: (),
                    mode,
                    final_,
                    is_xhp,
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
                };
                match &c.classish_body.syntax {
                    ClassishBody(c1) => {
                        for elt in Self::as_list(&c1.classish_body_elements).iter() {
                            Self::p_class_elt(&mut class_, elt, env)?;
                        }
                    }
                    _ => Self::missing_syntax(None, "classish body", &c.classish_body, env)?,
                }
                Ok(vec![aast::Def::Class(class_)])
            }
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
            AliasDeclaration(_) => not_impl!(),
            EnumDeclaration(_) => not_impl!(),
            RecordDeclaration(_) => not_impl!(),
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
            NamespaceDeclaration(_) => not_impl!(),
            NamespaceGroupUseDeclaration(_) => not_impl!(),
            NamespaceUseDeclaration(_) => not_impl!(),
            FileAttributeSpecification(_) => not_impl!(),
            _ if env.file_mode() == file_info::Mode::Mdecl
                || env.file_mode() == file_info::Mode::Mphp && !env.codegen =>
            {
                Ok(vec![])
            }
            _ => Ok(vec![aast::Def::Stmt(Self::p_stmt(node, env)?)]),
        }
    }

    fn p_program(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Program<,>) {
        let is_halt = |syntax: &SyntaxVariant<T, V>| -> bool {
            match syntax {
                ExpressionStatement(c) => match &c.expression_statement_expression.syntax {
                    HaltCompilerExpression(_) => true,
                    _ => false,
                },
                _ => false,
            }
        };
        let nodes = Self::as_list(node);
        let mut acc = vec![];
        for i in 0..nodes.len() {
            match &nodes[i].syntax {
                EndOfFile(_) => break,
                n if is_halt(n) => {
                    let pos = Self::p_pos(nodes[i], env);
                    env.saw_compiler_halt_offset = Some(pos.end_cnum());
                }
                _ => match Self::p_def(nodes[i], env) {
                    Err(Error::APIMissingSyntax { .. }) if env.fail_open => {}
                    e @ Err(_) => return e,
                    Ok(mut def) => acc.append(&mut def),
                },
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

    fn elabarate_halt_compiler(ast: aast!(Program<,>), env: &Env) -> aast!(Program<,>) {
        match env.saw_compiler_halt_offset {
            Some(_) => not_impl!(),
            _ => ast,
        }
    }

    fn lower(env: &mut Env<'a>, script: &Syntax<T, V>) -> std::result::Result<Result, String> {
        let comments = vec![];
        let ast_result = Self::p_script(script, env);
        let ast = match ast_result {
            Ok(ast) => ast,
            Err(err) => match err {
                Error::APIMissingSyntax {
                    expecting,
                    pos,
                    node_name,
                    kind,
                } => Err(format!(
                    "missing case in {:?}.\n - pos: {:?}\n - unexpected: '{:?}'\n - kind: {:?}\n",
                    expecting.to_string(),
                    pos,
                    node_name.to_string(),
                    kind,
                ))?,
                _ => Err(format!("Lowerer Error: {:?}", err))?,
            },
        };

        if env.elaborate_namespaces {
            not_impl!()
        }
        let ast = Self::elabarate_halt_compiler(ast, env);
        Ok(Result { ast, comments })
    }
}
