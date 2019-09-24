// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{
    aast, aast_defs, ast_defs, doc_comment::DocComment, file_info, global_options::GlobalOptions,
    namespace_env::Env as NamespaceEnv, pos::Pos, prim_defs::Comment, s_map,
};

use naming_special_names_rust::{special_functions, special_idents};

use parser_rust::{
    indexed_source_text::IndexedSourceText, lexable_token::LexablePositionedToken,
    positioned_syntax::PositionedSyntaxTrait, source_text::SourceText, syntax::*, syntax_error,
    syntax_kind, syntax_trait::SyntaxTrait, token_kind::TokenKind as TK,
};

use escaper::*;

use ocamlvalue_macro::Ocamlvalue;

use std::{
    cell::{Ref, RefCell, RefMut},
    collections::HashSet,
    mem,
    rc::Rc,
    result::Result::{Err, Ok},
};

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

type LiftedAwaitExprs = Vec<(Option<aast!(Lid)>, aast!(Expr<,>))>;

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
        self == Self::InDoubleQuotedString || self == Self::InBacktickedString
    }
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

#[derive(Debug)]
pub struct State {
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
    pub doc_comments: Vec<Option<DocComment>>,
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

    // Cache none pos, lazy_static doesn't allow Rc.
    pos_none: Pos,

    pub saw_yield: bool, /* Information flowing back up */
    pub lifted_awaits: Option<LiftedAwaits>,
    pub tmp_var_counter: isize,

    pub indexed_source_text: &'a IndexedSourceText<'a>,
    pub auto_ns_map: &'a [(String, String)],
    pub parser_options: &'a GlobalOptions,

    state: Rc<RefCell<State>>,
}

impl<'a> Env<'a> {
    pub fn make(
        codegen: bool,
        elaborate_namespaces: bool,
        quick_mode: bool,
        mode: file_info::Mode,
        indexed_source_text: &'a IndexedSourceText<'a>,
        parser_options: &'a GlobalOptions,
    ) -> Self {
        use file_info::Mode::*;
        Env {
            codegen,
            elaborate_namespaces,
            include_line_comments: false,
            keep_errors: false,
            quick_mode: !codegen
                && (match mode {
                    Mdecl | Mphp => true,
                    _ => quick_mode,
                }),
            show_all_errors: false,
            lower_coroutines: true,
            fail_open: true,
            file_mode: mode,
            top_level_statements: false,
            saw_yield: false,
            lifted_awaits: None,
            tmp_var_counter: 1,
            indexed_source_text,
            auto_ns_map: &[],
            parser_options,
            pos_none: Pos::make_none(),

            state: Rc::new(RefCell::new(State {
                saw_compiler_halt_offset: None,
                cls_reified_generics: HashSet::new(),
                in_static_method: false,
                parent_maybe_reified: false,
                lowpri_errors: vec![],
                doc_comments: vec![],
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
        self.indexed_source_text.source_text
    }

    fn lower_coroutines(&self) -> bool {
        self.lower_coroutines
    }

    fn fail_open(&self) -> bool {
        self.fail_open
    }

    fn saw_compiler_halt_offset(&mut self) -> RefMut<Option<usize>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.saw_compiler_halt_offset)
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

    fn lowpri_errors(&mut self) -> RefMut<Vec<(Pos, String)>> {
        RefMut::map(self.state.borrow_mut(), |s| &mut s.lowpri_errors)
    }

    fn top_docblock(&self) -> Ref<Option<DocComment>> {
        Ref::map(self.state.borrow(), |s| {
            s.doc_comments.last().unwrap_or(&None)
        })
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

    fn pos_none(&self) -> &Pos {
        &self.pos_none
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
}

impl<'a> AsMut<Env<'a>> for Env<'a> {
    fn as_mut(&mut self) -> &mut Env<'a> {
        self
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
    LowererInvariantFailure(Pos, String),
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
        //TODO:
    }

    fn raise_parsing_error_pos(pos: &Pos, env: &mut Env, msg: &str) {
        // TODO: enable should_surface_errors
        if env.codegen() && !env.lower_coroutines() {
            env.lowpri_errors().push((pos.clone(), String::from(msg)))
        }
    }

    fn raise_nast_error(msg: &str) {
        // A placehold for error raised in ast_to_aast.ml
    }

    fn invariant_failure_error<N>(node: &Syntax<T, V>, env: &Env, msg: &str) -> ret!(N) {
        let pos = Self::p_pos(node, env);
        Err(Error::LowererInvariantFailure(pos, String::from(msg)))
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

    fn lowering_error(env: &mut Env, pos: &Pos, text: &str, syntax_kind: &str) {
        if env.is_typechecker() && env.lowpri_errors().is_empty() {
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
        Self::lowering_error(env, &pos, &text, expecting);
        if let Some(x) = fallback {
            if env.fail_open {
                return Ok(x);
            }
        }
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

    fn mp_optional<F, R>(p: F, node: &Syntax<T, V>, env: &mut Env) -> ret!(Option<R>)
    where
        F: FnOnce(&Syntax<T, V>, &mut Env) -> ret!(R),
    {
        match &node.syntax {
            Missing => Ok(None),
            _ => p(node, env).map(Some),
        }
    }

    fn pos_qualified_name(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Sid) {
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
                return Ok(ast_defs::Id(p, s));
            }
        }
        Self::missing_syntax(None, "qualified name", node, env)
    }

    #[inline]
    fn pos_name(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Sid) {
        Self::pos_name_(node, env, None)
    }

    fn lid_from_pos_name(pos: Pos, name: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Lid) {
        let name = Self::pos_name(name, env)?;
        Ok(aast::Lid::new(pos, name.1))
    }

    fn lid_from_name(name: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Lid) {
        let name = Self::pos_name(name, env)?;
        Ok(aast::Lid::new(name.0, name.1))
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
                    *env.saw_compiler_halt_offset() = Some(0);
                }
                if let Some(prefix) = drop_prefix {
                    name = Self::drop_prefix(name, prefix);
                }
                let p = Self::p_pos(node, env);
                Ok(ast_defs::Id(p, String::from(name)))
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

    fn check_valid_reified_hint(env: &Env, node: &Syntax<T, V>, hint: &aast_defs::Hint) {
        // TODO:
    }

    fn p_closure_parameter(
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!((aast!(Hint), Option<ast_defs::ParamKind>)) {
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
            _ => Self::missing_syntax(None, "closure parameter", node, env),
        }
    }

    fn mp_shape_expression_field<F, R>(
        f: F,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!((ast_defs::ShapeFieldName, R))
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> ret!(R),
    {
        match &node.syntax {
            FieldInitializer(c) => {
                let name = Self::p_shape_field_name(&c.field_initializer_name, env)?;
                let value = f(&c.field_initializer_value, env)?;
                Ok((name, value))
            }
            _ => Self::missing_syntax(None, "shape field", node, env),
        }
    }

    fn p_shape_field_name(node: &Syntax<T, V>, env: &mut Env) -> ret!(ast_defs::ShapeFieldName) {
        use ast_defs::ShapeFieldName::*;
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
                    let ast_defs::Id(p, n) = Self::pos_name(node, env)?;
                    let str_ = Self::mk_str(node, env, Self::unesc_dbl, &n);
                    match isize::from_str_radix(&str_, 10) {
                        Ok(_) => Self::raise_parsing_error(
                            node,
                            env,
                            &syntax_error::shape_field_int_like_string,
                        ),
                        _ => {}
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
                let ast_defs::Id(p, n) = Self::pos_name(node, env)?;
                Ok(SFlitStr((p, Self::mk_str(node, env, Self::unesc_dbl, &n))))
            }
        }
    }

    fn p_shape_field(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(ShapeFieldInfo) {
        match &node.syntax {
            FieldSpecifier(c) => {
                let optional = !c.field_question.is_missing();
                let name = Self::p_shape_field_name(&c.field_name, env)?;
                let hint = Self::p_hint(&c.field_type, env)?;
                Ok(aast::ShapeFieldInfo {
                    optional,
                    hint,
                    name,
                })
            }
            _ => {
                let (name, hint) = Self::mp_shape_expression_field(&Self::p_hint, node, env)?;
                Ok(aast::ShapeFieldInfo {
                    optional: false,
                    name,
                    hint,
                })
            }
        }
    }

    fn p_hint_(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Hint_) {
        use aast_defs::Hint_::*;
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
                Ok(Happly(Self::pos_name(node, env)?, vec![]))
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

                Ok(Hshape(aast::NastShapeInfo {
                    allows_unknown_fields,
                    field_map,
                }))
            }
            TupleTypeSpecifier(c) => {
                Ok(Htuple(Self::could_map(Self::p_hint, &c.tuple_types, env)?))
            }
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
                    _ => Self::missing_syntax(None, "generic type arguments", args, env)?,
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
                    let msg = format!(
                        "{} variadic parameters found. There should be no more than one.",
                        variadic_hints.len().to_string()
                    );
                    Self::invariant_failure_error(node, env, &msg)?;
                }
                Ok(Hfun {
                    reactive_kind: aast::FuncReactive::FNonreactive,
                    is_coroutine: !c.closure_coroutine.is_missing(),
                    param_tys: type_hints,
                    param_kinds: kinds,
                    param_mutability: vec![],
                    variadic_ty: variadic_hints.into_iter().next().unwrap_or(None),
                    return_ty: Self::p_hint(&c.closure_return_type, env)?,
                    is_mutable_return: true,
                })
            }
            AttributizedSpecifier(c) => {
                let attrs = Self::p_user_attribute(&c.attributized_specifier_attribute_spec, env)?;
                let hint = Self::p_hint(&c.attributized_specifier_type, env)?;
                if attrs.iter().any(|attr| attr.name.1 != "__Soft") {
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
                            let root = aast::Hint::new(ty.0.clone(), Happly(ty, param));
                            Ok(Haccess(root, vec![child]))
                        } else {
                            Self::missing_syntax(None, "type constant base", node, env)
                        }
                    }
                    _ => Self::missing_syntax(None, "type constant base", node, env),
                }
            }
            PUAccess(c) => {
                let pos = Self::p_pos(&c.pu_access_left_type, env);
                let child = Self::pos_name(&c.pu_access_right_type, env)?;
                match Self::p_hint_(&c.pu_access_left_type, env)? {
                    h @ HpuAccess(_, _) => Ok(HpuAccess(aast::Hint::new(pos, h), child)),
                    Happly(id, hints) => {
                        if hints.is_empty() {
                            Ok(HpuAccess(aast::Hint::new(pos, Happly(id, hints)), child))
                        } else {
                            Self::missing_syntax(None, "pocket universe access base", node, env)
                        }
                    }
                    _ => Self::missing_syntax(None, "pocket universe access base", node, env),
                }
            }
            ReifiedTypeArgument(_) => {
                Self::raise_parsing_error(node, env, &syntax_error::invalid_reified);
                Self::missing_syntax(None, "refied type", node, env)
            }
            _ => Self::missing_syntax(None, "type hint", node, env),
        }
    }

    fn p_hint(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Hint) {
        let hint_ = Self::p_hint_(node, env)?;
        let pos = Self::p_pos(node, env);
        let hint = aast::Hint::new(pos, hint_);
        Self::check_valid_reified_hint(env, node, &hint);
        Ok(hint)
    }

    fn p_simple_initializer(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Expr<,>) {
        match &node.syntax {
            SimpleInitializer(child) => Self::p_expr(&child.simple_initializer_value, env),
            _ => Self::missing_syntax(None, "simple initializer", node, env),
        }
    }

    fn p_member(node: &Syntax<T, V>, env: &mut Env) -> ret!((aast!(Expr<,>), aast!(Expr<,>))) {
        match &node.syntax {
            ElementInitializer(c) => Ok((
                Self::p_expr(&c.element_key, env)?,
                Self::p_expr(&c.element_value, env)?,
            )),
            _ => Self::missing_syntax(None, "darray intrinsic expression element", node, env),
        }
    }

    fn expand_type_args(ty: &Syntax<T, V>, env: &mut Env) -> ret!(Vec<aast!(Hint)>) {
        match &ty.syntax {
            TypeArguments(c) => Self::could_map(Self::p_hint, &c.type_arguments_types, env),
            _ => Ok(vec![]),
        }
    }

    fn p_afield(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Afield<,>) {
        match &node.syntax {
            ElementInitializer(c) => Ok(aast::Afield::AFkvalue(
                Self::p_expr(&c.element_key, env)?,
                Self::p_expr(&c.element_value, env)?,
            )),
            _ => Ok(aast::Afield::AFvalue(Self::p_expr(node, env)?)),
        }
    }

    fn check_intrinsic_type_arg_varity(
        node: &Syntax<T, V>,
        env: &mut Env,
        tys: Vec<aast!(Hint)>,
    ) -> Option<aast!(CollectionTarg)> {
        let count = tys.len();
        let mut tys = tys.into_iter();
        match count {
            2 => Some(aast::CollectionTarg::CollectionTKV(
                tys.next().unwrap(),
                tys.next().unwrap(),
            )),
            1 => Some(aast::CollectionTarg::CollectionTV(tys.next().unwrap())),
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

    fn p_import_flavor(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(ImportFlavor) {
        use aast::ImportFlavor::*;
        match Self::token_kind(node) {
            Some(TK::Include) => Ok(Include),
            Some(TK::Require) => Ok(Require),
            Some(TK::Include_once) => Ok(IncludeOnce),
            Some(TK::Require_once) => Ok(RequireOnce),
            _ => Self::missing_syntax(None, "import flavor", node, env),
        }
    }

    fn p_null_flavor(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(OgNullFlavor) {
        use aast::OgNullFlavor::*;
        match Self::token_kind(node) {
            Some(TK::QuestionMinusGreaterThan) => Ok(OGNullsafe),
            Some(TK::MinusGreaterThan) => Ok(OGNullthrows),
            _ => Self::missing_syntax(None, "null flavor", node, env),
        }
    }

    #[inline]
    fn wrap_unescaper<F>(unescaper: F, s: &str) -> ret!(String)
    where
        F: FnOnce(&str) -> std::result::Result<String, InvalidString>,
    {
        unescaper(s).map_err(|e| Error::Failwith(e.msg))
    }

    fn fail_if_invalid_reified_generic(node: &Syntax<T, V>, env: &mut Env, id: &str) {
        let in_static_method = *env.in_static_method();
        if in_static_method && env.cls_reified_generics().contains(id) {
            Self::raise_parsing_error(
                node,
                env,
                &syntax_error::cls_reified_generic_in_static_method,
            );
        }
    }

    fn p_expr_l(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Expr<,>) {
        Self::p_expr_l_with_loc(ExprLocation::TopLevel, node, env)
    }

    fn p_expr_l_with_loc(
        loc: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret_aast!(Expr<,>) {
        let p_expr = |n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(Expr<,>) {
            Self::p_expr_with_loc(loc, n, e)
        };
        Ok(aast::Expr::new(
            Self::p_pos(node, env),
            aast::Expr_::ExprList(Self::could_map(p_expr, node, env)?),
        ))
    }

    #[inline]
    fn p_expr(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Expr<,>) {
        Self::p_expr_with_loc(ExprLocation::TopLevel, node, env)
    }

    #[inline]
    fn p_expr_with_loc(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret_aast!(Expr<,>) {
        Self::p_expr_impl(location, node, env, None)
    }

    fn p_expr_impl(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
        parent_pos: Option<Pos>,
    ) -> ret_aast!(Expr<,>) {
        match &node.syntax {
            BracedExpression(child) => {
                let expr = &child.braced_expression_expression;
                let inner = Self::p_expr_impl(location, expr, env, parent_pos)?;
                let inner_pos = &inner.0;
                let inner_expr_ = inner.1.as_ref();
                use aast::Expr_::*;
                match inner_expr_ {
                    Lvar(_) | String(_) | Int(_) | Float(_) => Ok(inner),
                    _ => Ok(aast::Expr::new(
                        inner_pos.clone(),
                        aast::Expr_::BracedExpr(inner),
                    )),
                }
            }
            ParenthesizedExpression(child) => Self::p_expr_impl(
                location,
                &child.parenthesized_expression_expression,
                env,
                parent_pos,
            ),
            _ => {
                let pos = Self::p_pos(node, env);
                let expr_ = Self::p_expr_impl_(location, node, env, parent_pos)?;
                Ok(aast::Expr::new(pos, expr_))
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
                        aast::Expr_::String(Self::mk_str(expr, env, Self::unesc_dbl, s)),
                    ),
                    (ExprLocation::InBacktickedString, _) if env.codegen() => Ok(
                        aast::Expr_::String(Self::mk_str(expr, env, unescape_backtick, s)),
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
                        Self::mk_str(expr, env, unescape_single, s),
                    )),
                    (_, Some(TK::DoubleQuotedStringLiteral)) => Ok(aast::Expr_::String(
                        Self::mk_str(expr, env, unescape_double, s),
                    )),
                    (_, Some(TK::HeredocStringLiteral)) => Ok(aast::Expr_::String(Self::mk_str(
                        expr,
                        env,
                        unescape_heredoc,
                        s,
                    ))),
                    (_, Some(TK::NowdocStringLiteral)) => Ok(aast::Expr_::String(Self::mk_str(
                        expr,
                        env,
                        unescape_nowdoc,
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
        Self::p_expr_impl_(location, node, env, None)
    }

    fn p_expr_impl_(
        location: ExprLocation,
        node: &Syntax<T, V>,
        env: &mut Env,
        parent_pos: Option<Pos>,
    ) -> ret_aast!(Expr_<,>) {
        use aast::{Expr as E, Expr_ as E_};
        let split_args_varargs = |arg_list_node: &Syntax<T, V>,
                                  e: &mut Env|
         -> ret!((Vec<aast!(Expr<,>)>, Vec<aast!(Expr<,>)>)) {
            let mut arg_list = Self::as_list(arg_list_node);
            if let Some(last_arg) = arg_list.last() {
                if let DecoratedExpression(c) = &last_arg.syntax {
                    if Self::token_kind(&c.decorated_expression_decorator) == Some(TK::DotDotDot) {
                        let _ = arg_list.pop();
                        let args: std::result::Result<Vec<_>, _> =
                            arg_list.iter().map(|a| Self::p_expr(a, e)).collect();
                        let args = args?;
                        let vararg = Self::p_expr(&c.decorated_expression_expression, e)?;
                        return Ok((args, vec![vararg]));
                    }
                }
            }
            Ok((Self::could_map(Self::p_expr, arg_list_node, e)?, vec![]))
        };
        let mk_lid = |p: Pos, s: String| aast::Lid(p, (0, s));
        let mk_name_lid = |name: &Syntax<T, V>, env: &mut Env| {
            let name = Self::pos_name(name, env)?;
            Ok(mk_lid(name.0, name.1))
        };
        let mk_lvar = |name: &Syntax<T, V>, env: &mut Env| Ok(E_::Lvar(mk_name_lid(name, env)?));
        let mk_id_expr = |name: aast!(Sid)| E::new(name.0.clone(), E_::Id(name));
        let p_intri_expr = |kw, ty, v, e: &mut Env| {
            let hints = Self::expand_type_args(ty, e)?;
            let hints = Self::check_intrinsic_type_arg_varity(node, e, hints);
            Ok(E_::Collection(
                Self::pos_name(kw, e)?,
                hints,
                Self::could_map(Self::p_afield, v, e)?,
            ))
        };
        let p_special_call =
            |recv: &Syntax<T, V>, args: &Syntax<T, V>, e: &mut Env| -> ret_aast!(Expr_<,>) {
                let pos_if_has_parens = match &recv.syntax {
                    ParenthesizedExpression(_) => Some(Self::p_pos(recv, e)),
                    _ => None,
                };
                let recv = Self::p_expr(recv, e)?;
                let recv = match (recv.1.as_ref(), pos_if_has_parens) {
                    (E_::ObjGet(_, _, _), Some(p)) => E::new(p, E_::ParenthesizedExpr(recv)),
                    (E_::ClassGet(_, _), Some(p)) => E::new(p, E_::ParenthesizedExpr(recv)),
                    _ => recv,
                };
                let (args, varargs) = split_args_varargs(args, e)?;
                Ok(E_::Call(
                    aast::CallType::Cnormal,
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
         -> ret_aast!(Expr_<,>) {
            if recv.is_object_creation_expression() && !e.codegen() {
                Self::raise_parsing_error(recv, e, &syntax_error::invalid_constructor_method_call);
            }
            let recv = Self::p_expr(recv, e)?;
            let name = Self::p_expr_with_loc(ExprLocation::MemberSelect, name, e)?;
            let op = Self::p_null_flavor(op, e)?;
            Ok(E_::ObjGet(recv, name, op))
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
                        let ast_defs::Id(p, n) = Self::pos_name(&c.lambda_signature, env)?;
                        (
                            vec![aast::FunParam {
                                annotation: p.clone(),
                                type_hint: aast::TypeHint((), None),
                                is_reference: false,
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
                    _ => Self::missing_syntax(None, "lambda signature", &c.lambda_signature, env)?,
                };
                let (body, yield_) = if !c.lambda_body.is_compound_statement() {
                    Self::mp_yielding(Self::p_function_body, &c.lambda_body, env)?
                } else {
                    let mut env1 = Env::clone_and_unset_toplevel_if_toplevel(env);
                    Self::mp_yielding(&Self::p_function_body, &c.lambda_body, env1.as_mut())?
                };
                let external = c.lambda_body.is_external();
                let fun = aast::Fun_ {
                    span: pos.clone(),
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    ret: aast::TypeHint((), ret),
                    name: ast_defs::Id(pos, String::from(";anonymous")),
                    tparams: vec![],
                    where_constraints: vec![],
                    body: aast::FuncBody {
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
                Ok(E_::Lfun(fun, vec![]))
            }
            BracedExpression(c) => {
                Self::p_expr_with_loc_(location, &c.braced_expression_expression, env)
            }
            EmbeddedBracedExpression(c) => {
                Self::p_expr_with_loc_(location, &c.embedded_braced_expression_expression, env)
            }
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
                Ok(E_::Collection(
                    collection_name,
                    hints,
                    Self::could_map(Self::p_afield, &c.collection_literal_initializers, env)?,
                ))
            }
            VarrayIntrinsicExpression(c) => {
                let hints = Self::expand_type_args(&c.varray_intrinsic_explicit_type, env)?;
                let hints = Self::check_intrinsic_type_arg_varity(node, env, hints);
                let targ = match hints {
                    Some(aast_defs::CollectionTarg::CollectionTV(ty)) => Some(ty),
                    None => None,
                    _ => Self::missing_syntax(
                        None,
                        "VarrayIntrinsicExpression type args",
                        node,
                        env,
                    )?,
                };
                Ok(E_::Varray(
                    targ,
                    Self::could_map(Self::p_expr, &c.varray_intrinsic_members, env)?,
                ))
            }
            DarrayIntrinsicExpression(c) => {
                let hints = Self::expand_type_args(&c.darray_intrinsic_explicit_type, env)?;
                let hints = Self::check_intrinsic_type_arg_varity(node, env, hints);
                match hints {
                    Some(aast_defs::CollectionTarg::CollectionTKV(tk, tv)) => Ok(E_::Darray(
                        Some((tk, tv)),
                        Self::could_map(Self::p_member, &c.darray_intrinsic_members, env)?,
                    )),
                    None => Ok(E_::Darray(
                        None,
                        Self::could_map(Self::p_member, &c.darray_intrinsic_members, env)?,
                    )),
                    _ => {
                        Self::missing_syntax(None, "DarrayIntrinsicExpression type args", node, env)
                    }
                }
            }
            ArrayIntrinsicExpression(c) => {
                /* TODO: Or tie in with other intrinsics and post-process to Array */
                Ok(E_::Array(Self::could_map(
                    Self::p_afield,
                    &c.array_intrinsic_members,
                    env,
                )?))
            }
            ArrayCreationExpression(c) => {
                /* TODO: Or tie in with other intrinsics and post-process to Array */
                Ok(E_::Array(Self::could_map(
                    Self::p_afield,
                    &c.array_creation_members,
                    env,
                )?))
            }
            ListExpression(c) => {
                /* TODO: Or tie in with other intrinsics and post-process to List */
                let p_binder_or_ignore = |n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(Expr<,>) {
                    match &n.syntax {
                        Missing => Ok(E::new(Pos::make_none(), E_::Omitted)),
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
                        Ok(E_::Call(
                            aast::CallType::Cnormal,
                            Self::p_expr(recv, env)?,
                            vec![],
                            vec![E::new(literal_expression_pos, E_::String(s))],
                            vec![],
                        ))
                    }
                    None => {
                        let hints = match (&recv.syntax, &c.function_call_type_args.syntax) {
                            (_, TypeArguments(c)) => {
                                Self::could_map(Self::p_hint, &c.type_arguments_types, env)?
                            }
                            /* TODO might not be needed */
                            (GenericTypeSpecifier(c), _) => match &c.generic_argument_list.syntax {
                                TypeArguments(c) => {
                                    Self::could_map(Self::p_hint, &c.type_arguments_types, env)?
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
                        let recv = match (recv.1.as_ref(), pos_if_has_parens) {
                            (E_::ObjGet(_, _, _), Some(p)) => {
                                E::new(p, E_::ParenthesizedExpr(recv))
                            }
                            (E_::ClassGet(_, _), Some(p)) => E::new(p, E_::ParenthesizedExpr(recv)),
                            _ => recv,
                        };
                        let (args, varargs) = split_args_varargs(args, env)?;
                        Ok(E_::Call(
                            aast::CallType::Cnormal,
                            recv,
                            hints,
                            args,
                            varargs,
                        ))
                    }
                }
            }
            QualifiedName(_) => {
                if location.in_string() {
                    let ast_defs::Id(_, n) = Self::pos_qualified_name(node, env)?;
                    Ok(E_::String(n))
                } else {
                    Ok(E_::Id(Self::pos_qualified_name(node, env)?))
                }
            }
            VariableExpression(c) => Ok(E_::Lvar(Self::lid_from_pos_name(
                pos,
                &c.variable_expression,
                env,
            )?)),
            PipeVariableExpression(_) => Ok(E_::Lvar(mk_lid(pos, String::from("$$")))),
            InclusionExpression(c) => Ok(E_::Import(
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
                    _ => Self::missing_syntax(None, "unary exppr", node, env)?,
                };

                let expr = Self::p_expr(operand, env)?;
                /**
                 * FFP does not destinguish between ++$i and $i++ on the level of token
                 * kind annotation. Prevent duplication by switching on `postfix` for
                 * the two operatores for which AST /does/ differentiate between
                 * fixities.
                 */
                use ast_defs::Uop::*;
                match Self::token_kind(op) {
                    Some(TK::PlusPlus) if postfix => Ok(E_::Unop(Upincr, expr)),
                    Some(TK::MinusMinus) if postfix => Ok(E_::Unop(Updecr, expr)),
                    Some(TK::PlusPlus) => Ok(E_::Unop(Uincr, expr)),
                    Some(TK::MinusMinus) => Ok(E_::Unop(Udecr, expr)),
                    Some(TK::Exclamation) => Ok(E_::Unop(Unot, expr)),
                    Some(TK::Tilde) => Ok(E_::Unop(Utild, expr)),
                    Some(TK::Plus) => Ok(E_::Unop(Uplus, expr)),
                    Some(TK::Minus) => Ok(E_::Unop(Uminus, expr)),
                    Some(TK::Ampersand) => Ok(E_::Unop(Uref, expr)),
                    Some(TK::At) => {
                        if env.parser_options.po_disallow_silence {
                            Self::raise_parsing_error(op, env, &syntax_error::no_silence);
                        }
                        if env.codegen() {
                            Ok(E_::Unop(Usilence, expr))
                        } else {
                            let expr =
                                Self::p_expr_impl(ExprLocation::TopLevel, operand, env, Some(pos))?;
                            Ok(*expr.1)
                        }
                    }
                    Some(TK::Inout) => Ok(E_::Callconv(ast_defs::ParamKind::Pinout, expr)),
                    Some(TK::Await) => Self::lift_await(pos, expr, env, location),
                    Some(TK::Suspend) => Ok(E_::Suspend(expr)),
                    Some(TK::Clone) => Ok(E_::Clone(expr)),
                    Some(TK::Print) => Ok(E_::Call(
                        aast::CallType::Cnormal,
                        E::new(pos.clone(), E_::Id(ast_defs::Id(pos, String::from("echo")))),
                        vec![],
                        vec![expr],
                        vec![],
                    )),
                    Some(TK::Dollar) => {
                        let E(p, expr_) = expr;
                        match *expr_ {
                            E_::String(s) | E_::Int(s) | E_::Float(s) => {
                                if !env.codegen() {
                                    Self::raise_parsing_error(
                                        op,
                                        env,
                                        &syntax_error::invalid_variable_name,
                                    )
                                }
                                let id = String::with_capacity(1 + s.len()) + "$" + &s;
                                let lid = aast::Lid(pos, (0, id));
                                Ok(E_::Lvar(lid))
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
                    _ => Self::missing_syntax(None, "unary operator", node, env),
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
                // Ocaml evaluates right first then left, ordering will affect
                // tmp var count in lifted await statement.
                let right = Self::p_expr_with_loc(rlocation, &c.binary_right_operand, env)?;
                let left = Self::p_expr(&c.binary_left_operand, env)?;
                let bop_ast_node = Self::p_bop(&c.binary_operator, left, right, env)?;
                match &bop_ast_node {
                    // TODO: Ast_check.check_lvalue (fun pos error -> raise_parsing_error env (`Pos pos) error) lhs
                    E_::Binop(ast_defs::Bop::Eq(_), lhs, _) => {}
                    _ => {}
                }
                Ok(bop_ast_node)
            }
            Token(t) => {
                use ExprLocation::*;
                match (location, t.kind()) {
                    (MemberSelect, TK::Variable) => mk_lvar(node, env),
                    (InDoubleQuotedString, _) => Ok(aast::Expr_::String(Self::wrap_unescaper(
                        Self::unesc_dbl,
                        Self::text_str(node, env),
                    )?)),
                    (InBacktickedString, _) => Ok(aast::Expr_::String(Self::wrap_unescaper(
                        unescape_backtick,
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
                    Ok(E_::Yield(aast::Afield::AFvalue(E::new(pos, E_::Null))))
                } else {
                    Ok(E_::Yield(Self::p_afield(&c.yield_operand, env)?))
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
                Ok(E_::YieldFrom(Self::p_expr(&c.yield_from_operand, env)?))
            }
            DefineExpression(c) => {
                let name = Self::pos_name(&c.define_keyword, env)?;
                Ok(E_::Call(
                    aast_defs::CallType::Cnormal,
                    mk_id_expr(name),
                    vec![],
                    Self::as_list(&c.define_argument_list)
                        .iter()
                        .map(|x| Self::p_expr(x, env))
                        .collect::<std::result::Result<Vec<_>, _>>()?,
                    vec![],
                ))
            }
            ScopeResolutionExpression(c) => {
                let qual = Self::p_expr(&c.scope_resolution_qualifier, env)?;
                let qual = if env.codegen() {
                    qual
                } else if let E_::Lvar(a) = *qual.1 {
                    let pos = qual.0;
                    E::new(pos.clone(), E_::Id(ast_defs::Id(pos, (a.1).1)))
                } else {
                    qual
                };
                if let E_::Id(ast_defs::Id(_, x)) = qual.1.as_ref() {
                    Self::fail_if_invalid_reified_generic(node, env, &x);
                }
                match &c.scope_resolution_name.syntax {
                    Token(token) if token.kind() == TK::Variable => {
                        let ast_defs::Id(p, name) = Self::pos_name(&c.scope_resolution_name, env)?;
                        Ok(E_::ClassGet(
                            aast::ClassId(pos, aast::ClassId_::CIexpr(qual)),
                            aast::ClassGetExpr::CGstring((p, name)),
                        ))
                    }
                    _ => {
                        let E(p, expr_) = Self::p_expr(&c.scope_resolution_name, env)?;
                        match *expr_ {
                            E_::String(id) => Ok(E_::ClassConst(
                                aast::ClassId(pos, aast::ClassId_::CIexpr(qual)),
                                (p, id),
                            )),
                            E_::Id(ast_defs::Id(p, n)) => Ok(E_::ClassConst(
                                aast::ClassId(pos, aast::ClassId_::CIexpr(qual)),
                                (p, n),
                            )),
                            E_::Lvar(aast::Lid(p, (_, n))) => Ok(E_::ClassGet(
                                aast::ClassId(pos, aast::ClassId_::CIexpr(qual)),
                                aast::ClassGetExpr::CGstring((p, n)),
                            )),
                            _ => Ok(E_::ClassGet(
                                aast::ClassId(pos, aast::ClassId_::CIexpr(qual)),
                                aast::ClassGetExpr::CGexpr(E(p, expr_)),
                            )),
                        }
                    }
                }
            }
            CastExpression(c) => Ok(E_::Cast(
                Self::p_hint(&c.cast_type, env)?,
                Self::p_expr(&c.cast_operand, env)?,
            )),
            ConditionalExpression(c) => Ok(E_::Eif(
                Self::p_expr(&c.conditional_test, env)?,
                Self::mp_optional(Self::p_expr, &c.conditional_consequence, env)?,
                Self::p_expr(&c.conditional_alternative, env)?,
            )),
            SubscriptExpression(c) => Ok(E_::ArrayGet(
                Self::p_expr(&c.subscript_receiver, env)?,
                Self::mp_optional(Self::p_expr, &c.subscript_index, env)?,
            )),
            EmbeddedSubscriptExpression(c) => Ok(E_::ArrayGet(
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
                let (args, varargs) = split_args_varargs(&c.constructor_call_argument_list, env)?;
                let (e, hl) = match &c.constructor_call_type.syntax {
                    GenericTypeSpecifier(c) => {
                        let name = Self::pos_name(&c.generic_class_type, env)?;
                        let hints = match &c.generic_argument_list.syntax {
                            TypeArguments(c) => {
                                Self::could_map(Self::p_hint, &c.type_arguments_types, env)?
                            }
                            _ => Self::missing_syntax(
                                None,
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
                match e.1.as_ref() {
                    E_::Id(_) => {
                        // TODO: report
                    }
                    _ => {}
                }
                Ok(E_::New(
                    aast::ClassId(pos.clone(), aast::ClassId_::CIexpr(e)),
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
                Ok(E_::Id(name))
            }
            RecordCreationExpression(c) => {
                let rec_type = &c.record_creation_type;
                let e = match &rec_type.syntax {
                    SimpleTypeSpecifier(_) => {
                        let name = Self::pos_name(rec_type, env)?;
                        E::new(name.0.clone(), E_::Id(name))
                    }
                    _ => Self::p_expr(rec_type, env)?,
                };
                let is_record_array =
                    Self::token_kind(&c.record_creation_array_token) == Some(TK::At);
                Ok(E_::Record(
                    aast::ClassId(pos, aast::ClassId_::CIexpr(e)),
                    is_record_array,
                    Self::could_map(Self::p_member, &c.record_creation_members, env)?,
                ))
            }
            LiteralExpression(child) => {
                Self::p_expr_lit(location, node, &child.literal_expression, env)
            }
            PrefixedStringExpression(c) => {
                /* Temporarily allow only`re`- prefixed strings */
                let name_text = Self::text(&c.prefixed_string_name, env);
                if name_text != "re" {
                    Self::raise_parsing_error(node, env, &syntax_error::non_re_prefix);
                }
                Ok(E_::PrefixedString(
                    name_text,
                    Self::p_expr(&c.prefixed_string_str, env)?,
                ))
            }
            IsExpression(c) => Ok(E_::Is(
                Self::p_expr(&c.is_left_operand, env)?,
                Self::p_hint(&c.is_right_operand, env)?,
            )),
            AsExpression(c) => Ok(E_::As(
                Self::p_expr(&c.as_left_operand, env)?,
                Self::p_hint(&c.as_right_operand, env)?,
                false,
            )),
            NullableAsExpression(c) => Ok(E_::As(
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
                    _ => Self::missing_syntax(None, "use variable", n, e),
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
                let name_pos = Self::p_function(node, env);
                let fun = aast::Fun_ {
                    span: Self::p_pos(node, env),
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    ret: aast::TypeHint(
                        (),
                        Self::mp_optional(Self::p_hint, &c.anonymous_type, env)?,
                    ),
                    name: ast_defs::Id(name_pos, String::from(";anonymous")),
                    tparams: vec![],
                    where_constraints: vec![],
                    body: aast::FuncBody {
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
                Ok(E_::Efun(fun, uses))
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
                let name_pos = Self::p_function(node, env);
                let body = aast::Fun_ {
                    span: pos.clone(),
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    ret: aast::TypeHint((), None),
                    name: ast_defs::Id(name_pos, String::from(";anonymous")),
                    tparams: vec![],
                    where_constraints: vec![],
                    body: aast::FuncBody {
                        ast: if blk.len() == 0 {
                            let pos = Self::p_pos(&c.awaitable_compound_statement, env);
                            vec![aast::Stmt::noop(pos)]
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
                Ok(E_::Call(
                    aast::CallType::Cnormal,
                    E::new(pos, E_::Lfun(body, vec![])),
                    vec![],
                    vec![],
                    vec![],
                ))
            }
            XHPExpression(c) => not_impl!(),
            PocketAtomExpression(c) => Ok(E_::PUAtom(
                Self::pos_name(&c.pocket_atom_expression, env)?.1,
            )),
            PocketIdentifierExpression(c) => {
                let mk_class_id = |e: aast!(Expr<,>)| aast::ClassId(pos, aast::ClassId_::CIexpr(e));
                let qual = Self::p_expr(&c.pocket_identifier_qualifier, env)?;
                let qual = if env.codegen() {
                    mk_class_id(qual)
                } else if let E_::Lvar(a) = *qual.1 {
                    let p = qual.0;
                    let expr = E::new(p.clone(), E_::Id(ast_defs::Id(p, (a.1).1)));
                    mk_class_id(expr)
                } else {
                    mk_class_id(qual)
                };
                let E(p, expr_) = Self::p_expr(&c.pocket_identifier_field, env)?;
                let field = match *expr_ {
                    E_::String(id) => (p, id),
                    E_::Id(ast_defs::Id(p, n)) => (p, n),
                    _ => Self::missing_syntax(None, "PocketIdentifierExpression field", node, env)?,
                };
                let E(p, expr_) = Self::p_expr(&c.pocket_identifier_name, env)?;
                let name = match *expr_ {
                    E_::String(id) => (p, id),
                    E_::Id(ast_defs::Id(p, n)) => (p, n),
                    _ => Self::missing_syntax(None, "PocketIdentifierExpression name", node, env)?,
                };
                Ok(E_::PUIdentifier(qual, field, name))
            }
            _ => Self::missing_syntax(Some(E_::Null), "expression", node, env),
        }
    }

    fn p_bop(
        node: &Syntax<T, V>,
        lhs: aast!(Expr<,>),
        rhs: aast!(Expr<,>),
        env: &mut Env,
    ) -> ret_aast!(Expr_<,>) {
        use aast::Expr_ as E_;
        use ast_defs::Bop::*;
        let mk = |op, l, r| Ok(E_::Binop(op, l, r));
        let mk_eq = |op, l, r| Ok(E_::Binop(Eq(Some(Box::new(op))), l, r));
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
            //Some(TK::BarGreaterThan) => E_::Pipe(lhs, rhs),
            Some(TK::BarGreaterThan) => not_impl!(),
            Some(TK::QuestionColon) => Ok(E_::Eif(lhs, None, rhs)),
            _ => Self::missing_syntax(None, "binary operator", node, env),
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
                    result.push(Self::p_stmt(n, env)?);
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
        Ok(aast::Stmt::new(pos, aast::Stmt_::Block(body)))
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

    fn with_new_concurrent_scrope<F, R>(f: F, env: &mut Env) -> ret!((LiftedAwaitExprs, R))
    where
        F: FnOnce(&mut Env) -> ret!(R),
    {
        let saved_lifted_awaits = env.lifted_awaits.replace(LiftedAwaits {
            awaits: vec![],
            lift_kind: LiftedAwaitKind::LiftedFromConcurrent,
        });
        let result = f(env);
        let lifted_awaits = mem::replace(&mut env.lifted_awaits, saved_lifted_awaits);
        let result = result?;
        let awaits = match lifted_awaits {
            Some(la) => Self::process_lifted_awaits(la, env)?,
            None => Self::failwith("lifted awaits should not be None")?,
        };
        Ok((awaits, result))
    }

    fn process_lifted_awaits(mut awaits: LiftedAwaits, env: &Env) -> ret!(LiftedAwaitExprs) {
        for await_ in awaits.awaits.iter() {
            if &(await_.1).0 == env.pos_none() {
                return Self::failwith("none pos in lifted awaits");
            }
        }
        awaits
            .awaits
            .sort_unstable_by(|a1, a2| Pos::compare(&(a1.1).0, &(a2.1).0));
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

    fn lift_awaits_in_statement<F>(f: F, node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Stmt<,>)
    where
        F: FnOnce(&mut Env) -> ret_aast!(Stmt<,>),
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
                let awaits = Self::process_lifted_awaits(lifted_awaits, env)?;
                return Ok(aast::Stmt::new(
                    Self::p_pos(node, env),
                    aast::Stmt_::Awaitall((awaits, vec![result])),
                ));
            }
        }
        Ok(result)
    }

    fn lift_await(
        parent_pos: Pos,
        expr: aast!(Expr<,>),
        env: &mut Env,
        location: ExprLocation,
    ) -> ret_aast!(Expr_<,>) {
        use ExprLocation::*;
        match (&env.lifted_awaits, location) {
            (_, UsingStatement) | (_, RightOfAssignmentInUsingStatement) | (None, _) => {
                Ok(aast::Expr_::Await(expr))
            }
            (Some(_), _) => {
                if location != AsStatement {
                    let name = env.make_tmp_var_name();
                    let lid = aast::Lid::new(parent_pos, name.clone());
                    let await_lid = aast::Lid::new(expr.0.clone(), name);
                    let await_ = (Some(await_lid), expr);
                    env.lifted_awaits.as_mut().map(|aw| aw.awaits.push(await_));
                    Ok(aast::Expr_::Lvar(lid))
                } else {
                    env.lifted_awaits
                        .as_mut()
                        .map(|aw| aw.awaits.push((None, expr)));
                    Ok(aast::Expr_::Null)
                }
            }
        }
    }

    fn p_stmt(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Stmt<,>) {
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

    fn p_stmt_(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Stmt<,>) {
        let pos = Self::p_pos(node, env);
        use aast::{Stmt as S, Stmt_ as S_};
        match &node.syntax {
            SwitchStatement(c) => {
                let p_label = |n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(Case<,>) {
                    match &n.syntax {
                        CaseLabel(c) => Ok(aast::Case::Case(
                            Self::p_expr(&c.case_expression, e)?,
                            vec![],
                        )),
                        DefaultLabel(_) => Ok(aast::Case::Default(Self::p_pos(n, e), vec![])),
                        _ => Self::missing_syntax(None, "switch label", n, e),
                    }
                };
                let p_section = |n: &Syntax<T, V>, e: &mut Env| -> ret!(Vec<aast!(Case<,>)>) {
                    match &n.syntax {
                        SwitchSection(c) => {
                            let mut blk =
                                Self::could_map(Self::p_stmt, &c.switch_section_statements, e)?;
                            if !c.switch_section_fallthrough.is_missing() {
                                blk.push(S::new(Pos::make_none(), S_::Fallthrough));
                            }
                            let mut labels = Self::could_map(p_label, &c.switch_section_labels, e)?;
                            match labels.last_mut() {
                                Some(aast::Case::Default(_, b)) => *b = blk,
                                Some(aast::Case::Case(_, b)) => *b = blk,
                                _ => Self::raise_parsing_error(n, e, "Malformed block result"),
                            }
                            Ok(labels)
                        }
                        _ => Self::missing_syntax(None, "switch section", n, e),
                    }
                };
                let f = |env: &mut Env| -> ret_aast!(Stmt<,>) {
                    Ok(S::new(
                        pos,
                        S_::Switch(
                            Self::p_expr(&c.switch_expression, env)?,
                            itertools::concat(Self::could_map(p_section, &c.switch_sections, env)?),
                        ),
                    ))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            IfStatement(c) => {
                let p_else_if =
                    |n: &Syntax<T, V>, e: &mut Env| -> ret!((aast!(Expr<,>), aast!(Block<,>))) {
                        match &n.syntax {
                            ElseifClause(c) => Ok((
                                Self::p_expr(&c.elseif_condition, e)?,
                                Self::p_block(true, &c.elseif_statement, e)?,
                            )),
                            _ => Self::missing_syntax(None, "elseif clause", n, e),
                        }
                    };
                let f = |env: &mut Env| -> ret_aast!(Stmt<,>) {
                    let condition = Self::p_expr(&c.if_condition, env)?;
                    let statement =
                        Self::p_block(true /* remove noop */, &c.if_statement, env)?;
                    let else_ = match &c.if_else_clause.syntax {
                        ElseClause(c) => Self::p_block(true, &c.else_statement, env)?,
                        Missing => vec![Self::mk_noop()],
                        _ => Self::missing_syntax(None, "else clause", &c.if_else_clause, env)?,
                    };
                    let else_ifs = Self::could_map(p_else_if, &c.if_elseif_clauses, env)?;
                    let else_if = else_ifs
                        .into_iter()
                        .rev()
                        .fold(else_, |child, (cond, stmts)| {
                            vec![S::new(pos.clone(), S_::If(cond, stmts, child))]
                        });
                    Ok(S::new(pos, S_::If(condition, statement, else_if)))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            ExpressionStatement(c) => {
                let expr = &c.expression_statement_expression;
                let f = |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    if expr.is_missing() {
                        Ok(S::new(pos, S_::Noop))
                    } else {
                        Ok(S::new(
                            pos,
                            S_::Expr(Self::p_expr_with_loc(ExprLocation::AsStatement, expr, e)?),
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
                |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    Ok(S::new(
                        pos,
                        S_::Throw(Self::p_expr(&c.throw_expression, e)?),
                    ))
                },
                node,
                env,
            ),
            DoStatement(c) => Ok(S::new(
                pos,
                S_::Do(
                    Self::p_block(false /* remove noop */, &c.do_body, env)?,
                    Self::p_expr(&c.do_condition, env)?,
                ),
            )),
            WhileStatement(c) => Ok(S::new(
                pos,
                S_::While(
                    Self::p_expr(&c.while_condition, env)?,
                    Self::p_block(true, &c.while_body, env)?,
                ),
            )),
            UsingStatementBlockScoped(c) => {
                let f = |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    Ok(S::new(
                        pos,
                        S_::Using(aast::UsingStmt {
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
                let f = |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    Ok(S::new(
                        pos,
                        S_::Using(aast::UsingStmt {
                            is_block_scoped: false,
                            has_await: !&c.using_function_await_keyword.is_missing(),
                            expr: Self::p_expr_l_with_loc(
                                ExprLocation::UsingStatement,
                                &c.using_function_expression,
                                e,
                            )?,
                            block: vec![Self::mk_noop()],
                        }),
                    ))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            LetStatement(c) => {
                let f = |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    let id = Self::lid_from_pos_name(pos.clone(), &c.let_statement_name, e)?;
                    let ty = Self::mp_optional(Self::p_hint, &c.let_statement_type, e)?;
                    let expr = Self::p_simple_initializer(&c.let_statement_initializer, e)?;
                    Ok(S::new(pos, S_::Let(id, ty, expr)))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            ForStatement(c) => {
                let f = |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    let ini = Self::p_expr_l(&c.for_initializer, e)?;
                    let ctr = Self::p_expr_l(&c.for_control, e)?;
                    let eol = Self::p_expr_l(&c.for_end_of_loop, e)?;
                    let blk = Self::p_block(true, &c.for_body, e)?;
                    Ok(S::new(pos, S_::For(ini, ctr, eol, blk)))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            ForeachStatement(c) => {
                let f = |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    let col = Self::p_expr(&c.foreach_collection, e)?;
                    let akw = match Self::token_kind(&c.foreach_await_keyword) {
                        Some(TK::Await) => Some(Self::p_pos(&c.foreach_await_keyword, e)),
                        _ => None,
                    };
                    let value = Self::p_expr(&c.foreach_value, e)?;
                    let akv = match (akw, &c.foreach_key.syntax) {
                        (Some(p), Missing) => aast::AsExpr::AwaitAsV(p, value),
                        (None, Missing) => aast::AsExpr::AsV(value),
                        (Some(p), _) => {
                            aast::AsExpr::AwaitAsKv(p, Self::p_expr(&c.foreach_key, e)?, value)
                        }
                        (None, _) => aast::AsExpr::AsKv(Self::p_expr(&c.foreach_key, e)?, value),
                    };
                    let blk = Self::p_block(true, &c.foreach_body, e)?;
                    Ok(S::new(pos, S_::Foreach(col, akv, blk)))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            TryStatement(c) => Ok(S::new(
                pos,
                S_::Try(
                    Self::p_block(false, &c.try_compound_statement, env)?,
                    Self::could_map(
                        |n: &Syntax<T, V>, e| match &n.syntax {
                            CatchClause(c) => Ok(aast::Catch(
                                Self::pos_name(&c.catch_type, e)?,
                                Self::lid_from_name(&c.catch_variable, e)?,
                                Self::p_block(true, &c.catch_body, e)?,
                            )),
                            _ => Self::missing_syntax(None, "catch clause", n, e),
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
                let f = |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    let expr = match &c.return_expression.syntax {
                        Missing => None,
                        _ => Some(Self::p_expr_with_loc(
                            ExprLocation::RightOfReturn,
                            &c.return_expression,
                            e,
                        )?),
                    };
                    Ok(aast::Stmt::new(pos, aast::Stmt_::Return(expr)))
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
                    S_::GotoLabel((
                        Self::p_pos(&c.goto_label_name, env),
                        Self::text(&c.goto_label_name, env),
                    )),
                ))
            }
            GotoStatement(c) => {
                if env.is_typechecker() && !env.parser_options.po_allow_goto {
                    Self::raise_parsing_error(node, env, &syntax_error::goto_label);
                }
                Ok(S::new(
                    pos,
                    S_::Goto(Self::p_pstring(&c.goto_statement_label_name, env)?),
                ))
            }
            EchoStatement(c) => {
                let f = |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    let echo = match &c.echo_keyword.syntax {
                        QualifiedName(_) | SimpleTypeSpecifier(_) | Token(_) => {
                            let name = Self::pos_name(&c.echo_keyword, e)?;
                            aast::Expr::new(name.0.clone(), aast::Expr_::Id(name))
                        }
                        _ => Self::missing_syntax(None, "id", &c.echo_keyword, e)?,
                    };
                    let args = Self::could_map(Self::p_expr, &c.echo_expressions, e)?;
                    Ok(S::new(
                        pos.clone(),
                        S_::Expr(aast::Expr::new(
                            pos,
                            aast::Expr_::Call(aast::CallType::Cnormal, echo, vec![], args, vec![]),
                        )),
                    ))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            UnsetStatement(c) => {
                let f = |e: &mut Env| -> ret_aast!(Stmt<,>) {
                    let args = Self::could_map(Self::p_expr, &c.unset_variables, e)?;
                    if e.parser_options.po_disable_unset_class_const {
                        args.iter()
                            .for_each(|arg| Self::check_mutate_class_const(arg, node, e))
                    }
                    let unset = match &c.unset_keyword.syntax {
                        QualifiedName(_) | SimpleTypeSpecifier(_) | Token(_) => {
                            let name = Self::pos_name(&c.unset_keyword, e)?;
                            aast::Expr::new(name.0.clone(), aast::Expr_::Id(name))
                        }
                        _ => Self::missing_syntax(None, "id", &c.unset_keyword, e)?,
                    };
                    Ok(S::new(
                        pos.clone(),
                        S_::Expr(aast::Expr::new(
                            pos,
                            aast::Expr_::Call(aast::CallType::Cnormal, unset, vec![], args, vec![]),
                        )),
                    ))
                };
                Self::lift_awaits_in_statement(f, node, env)
            }
            BreakStatement(c) => {
                let brk = Self::mp_optional(Self::p_expr, &c.break_level, env)?
                    .map_or(S_::Break, S_::TempBreak);
                Ok(S::new(pos, brk))
            }
            ContinueStatement(c) => {
                let ctn = Self::mp_optional(Self::p_expr, &c.continue_level, env)?
                    .map_or(S_::Continue, S_::TempContinue);
                Ok(S::new(pos, ctn))
            }
            ConcurrentStatement(c) => {
                let (lifted_awaits, S(stmt_pos, stmt)) = Self::with_new_concurrent_scrope(
                    |e: &mut Env| Self::p_stmt(&c.concurrent_statement, e),
                    env,
                )?;
                let stmt = match *stmt {
                    S_::Block(stmts) => {
                        use aast::{Expr as E, Expr_ as E_};
                        use ast_defs::Bop::Eq;
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
                                let S(p1, s_) = n;
                                if let S_::Expr(E(p2, e_)) = *s_ {
                                    if let E_::Binop(Eq(op), e1, e2) = *e_ {
                                        let tmp_n = E::mk_lvar(&e2.0, &(tv.1));
                                        if tmp_n.lvar_name() != e2.lvar_name() {
                                            let new_n = S::new(
                                                p1.clone(),
                                                S_::Expr(E::new(
                                                    p2.clone(),
                                                    E_::Binop(Eq(None), tmp_n.clone(), e2.clone()),
                                                )),
                                            );
                                            body_stmts.push(new_n);
                                        }
                                        let assign_stmt = S::new(
                                            p1,
                                            S_::Expr(E::new(p2, E_::Binop(Eq(op), e1, tmp_n))),
                                        );
                                        assign_stmts.push(assign_stmt);
                                        continue;
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
                        S::new(stmt_pos, S_::Block(body_stmts))
                    }
                    _ => Self::failwith("Unexpected concurrent stmt structure")?,
                };
                Ok(S::new(pos, S_::Awaitall((lifted_awaits, vec![stmt]))))
            }
            MarkupSection(_) => Self::p_markup(node, env),
            _ => Self::missing_syntax(
                Some(S::new(Pos::make_none(), S_::Noop)),
                "statement",
                node,
                env,
            ),
        }
    }

    fn check_mutate_class_const(e: &aast!(Expr<,>), node: &Syntax<T, V>, env: &mut Env) {
        match *(e.1) {
            aast::Expr_::ArrayGet(ref e, Some(_)) => Self::check_mutate_class_const(e, node, env),
            aast::Expr_::ClassConst(_, _) => {
                Self::raise_parsing_error(node, env, &syntax_error::const_mutation)
            }
            _ => {}
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
                Ok(aast::Stmt::new(pos, stmt_))
            }
            _ => Self::failwith("invalid node"),
        }
    }

    fn p_modifiers<F, R>(
        on_kind: F,
        mut init: R,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!((modifier::KindSet, R))
    where
        F: Fn(R, modifier::Kind, &Syntax<T, V>, &mut Env) -> R,
    {
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
        Self::p_modifiers(|_, _, _, _| {}, (), node, env).map(|r| r.0)
    }

    // TODO: change name to map_flatten after porting
    #[inline]
    fn could_map<R, F>(f: F, node: &Syntax<T, V>, env: &mut Env) -> ret!(Vec<R>)
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> ret!(R),
    {
        Self::map_flatten_(f, node, env, vec![])
    }

    #[inline]
    fn map_flatten_<R, F>(f: F, node: &Syntax<T, V>, env: &mut Env, acc: Vec<R>) -> ret!(Vec<R>)
    where
        F: Fn(&Syntax<T, V>, &mut Env) -> ret!(R),
    {
        let op = |mut v: Vec<R>, a| {
            v.push(a);
            v
        };
        Self::map_fold(&f, &op, node, env, acc)
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
        Self::p_modifiers(first_vis, None, node, env).map(|r| r.1)
    }

    fn p_visibility_or(
        node: &Syntax<T, V>,
        env: &mut Env,
        default: aast!(Visibility),
    ) -> ret_aast!(Visibility) {
        Self::p_visibility(node, env).map(|v| v.unwrap_or(default))
    }

    fn p_visibility_last_win(
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!(Option<aast!(Visibility)>) {
        let last_vis = |r, kind, _: &Syntax<T, V>, _: &mut Env| modifier::to_visibility(kind).or(r);
        Self::p_modifiers(last_vis, None, node, env).map(|r| r.1)
    }

    fn p_visibility_last_win_or(
        node: &Syntax<T, V>,
        env: &mut Env,
        default: aast!(Visibility),
    ) -> ret_aast!(Visibility) {
        Self::p_visibility_last_win(node, env).map(|v| v.unwrap_or(default))
    }

    fn has_soft(attrs: &[aast!(UserAttribute<,>)]) -> bool {
        attrs.iter().any(|attr| attr.name.1 == "__Soft")
    }

    fn soften_hint(attrs: &[aast!(UserAttribute<,>)], hint: aast!(Hint)) -> aast!(Hint) {
        if Self::has_soft(attrs) {
            aast::Hint::new(hint.0.clone(), aast::Hint_::Hsoft(hint))
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
                Self::mp_optional(Self::p_expr, &c.simple_initializer_value, env)
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
                let hint = Self::mp_optional(Self::p_hint, parameter_type, env)?
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
                    let type_name = Self::text(&c.type_name, env);
                    env.cls_reified_generics().insert(type_name);
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
                    constraints: Self::could_map(Self::p_tconstraint, &c.type_constraints, env)?,
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
                |n, e| Self::p_tparam(is_class, n, e),
                &c.type_parameters_parameters,
                env,
            ),
            _ => Self::missing_syntax(None, "type parameter", node, env),
        }
    }

    fn p_fun_hdr<F>(modifier_checker: F, node: &Syntax<T, V>, env: &mut Env) -> ret!(FunHdr)
    where
        F: Fn((), modifier::Kind, &Syntax<T, V>, &mut Env),
    {
        match &node.syntax {
            FunctionDeclarationHeader(child) => {
                let function_modifiers = &child.function_modifiers;
                let function_name = &child.function_name;
                let function_where_clause = &child.function_where_clause;
                let function_type_parameter_list = &child.function_type_parameter_list;
                let function_parameter_list = &child.function_parameter_list;
                let function_type = &child.function_type;
                let is_autoload = Self::text_str(function_name, env)
                    .eq_ignore_ascii_case(special_functions::AUTOLOAD);
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
                    Self::could_map(Self::p_fun_param, function_parameter_list, env)?;
                let fh_return_type = Self::mp_optional(Self::p_hint, function_type, env)?;
                let fh_suspension_kind =
                    Self::mk_suspension_kind_(node, env, has_async, has_coroutine);
                let fh_name = Self::pos_name(function_name, env)?;
                let fh_constrs = Self::p_where_constraint(false, node, function_where_clause, env)?;
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
            LambdaSignature(c) => {
                let mut header = FunHdr::make_empty();
                header.fh_parameters =
                    Self::could_map(Self::p_fun_param, &c.lambda_parameters, env)?;
                header.fh_return_type = Self::mp_optional(Self::p_hint, &c.lambda_type, env)?;
                Ok(header)
            }
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

    fn p_block(remove_noop: bool, node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Block<,>) {
        let aast::Stmt(p, stmt_) = Self::p_stmt(node, env)?;
        if let aast::Stmt_::Block(blk) = *stmt_ {
            if remove_noop && blk.len() == 1 {
                if let aast::Stmt_::Noop = *(blk[0].1) {
                    return Ok(vec![]);
                }
            }
            return Ok(blk);
        } else {
            Ok(vec![aast::Stmt(p, stmt_)])
        }
    }

    fn mk_noop() -> aast!(Stmt<,>) {
        aast::Stmt::noop(Pos::make_none())
    }

    fn p_function_body(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Block<,>) {
        let mk_noop_result = || Ok(vec![Self::mk_noop()]);
        let f = |e: &mut Env| -> ret_aast!(Block<,>) {
            match &node.syntax {
                Missing => Ok(vec![]),
                CompoundStatement(c) => {
                    let compound_statements = &c.compound_statements.syntax;
                    let compound_right_brace = &c.compound_right_brace.syntax;
                    match (compound_statements, compound_right_brace) {
                        (Missing, Token(_)) => mk_noop_result(),
                        (SyntaxList(t), _) if t.len() == 1 && t[0].is_yield() => {
                            e.saw_yield = true;
                            mk_noop_result()
                        }
                        _ => {
                            if !e.top_level_statements
                                && (e.file_mode() == file_info::Mode::Mdecl && !e.codegen()
                                    || e.quick_mode)
                            {
                                mk_noop_result()
                            } else {
                                Self::p_block(false /*remove noop*/, node, e)
                            }
                        }
                    }
                }
                _ => {
                    let f = |e: &mut Env| {
                        let expr = Self::p_expr(node, e)?;
                        Ok(aast::Stmt::new(
                            expr.0.clone(),
                            aast::Stmt_::Return(Some(expr)),
                        ))
                    };
                    Ok(vec![Self::lift_awaits_in_statement(f, node, e)?])
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
            |n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(Expr<,>) {
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
                Self::could_map(p_attr, &c.file_attribute_specification_attributes, env)
            }
            OldAttributeSpecification(c) => {
                Self::could_map(p_attr, &c.old_attribute_specification_attributes, env)
            }
            AttributeSpecification(c) => Self::could_map(
                |n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(UserAttribute<,>) {
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

    fn mp_yielding<F, R>(p: F, node: &Syntax<T, V>, env: &mut Env) -> ret!((R, bool))
    where
        F: FnOnce(&Syntax<T, V>, &mut Env) -> ret!(R),
    {
        let outer_saw_yield = env.saw_yield;
        env.saw_yield = false;
        let r = p(node, env);
        let saw_yield = env.saw_yield;
        env.saw_yield = outer_saw_yield;
        Ok((r?, saw_yield))
    }

    fn mk_empty_ns_env(env: &Env) -> NamespaceEnv {
        NamespaceEnv::empty(env.auto_ns_map.to_vec(), env.codegen())
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
        let str = node.leading_text(env.indexed_source_text.source_text);
        parse(str, 0, Free, 0).map(Rc::new)
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
                let type_ = Self::mp_optional(Self::p_hint, &c.const_type_specifier, env)?;
                // using map_fold can save one Vec allocation, but ocaml's behavior is that
                // if anything throw, it will discard all lowered elements. So adding to class
                // must be at the last.
                let mut class_consts = Self::could_map(
                    |n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(ClassConst<,>) {
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
                let type__ = Self::mp_optional(Self::p_hint, &c.type_const_type_specifier, env)?
                    .map(|hint| Self::soften_hint(&user_attributes, hint));
                let kinds = Self::p_kinds(&c.type_const_modifiers, env)?;
                let name = Self::pos_name(&c.type_const_name, env)?;
                let constraint =
                    Self::mp_optional(Self::p_tconstraint_ty, &c.type_const_type_constraint, env)?;
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
                let type_ = Self::mp_optional(Self::p_hint, &c.property_type, env)?
                    .map(|t| Self::soften_hint(&user_attributes, t));
                let modifier_checker = |_, _, n: &Syntax<T, V>, e: &mut Env| -> () {
                    if n.is_final() {
                        Self::raise_parsing_error(n, e, &syntax_error::declared_final("Properties"))
                    }
                };
                let kinds = Self::p_modifiers(modifier_checker, (), &c.property_modifiers, env)?.0;
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
                    |n, e| -> ret!((Pos, aast!(Sid), Option<aast!(Expr<,>)>)) {
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
                            aast::Expr::new(p.clone(), expr_)
                        };
                        let lid =
                            |s: &str| -> aast!(Lid) { aast::Lid(p.clone(), (0, s.to_string())) };
                        use aast::Expr_::*;
                        (
                            aast::Stmt::new(
                                p.clone(),
                                aast::Stmt_::Expr(e(Binop(
                                    ast_defs::Bop::Eq(None),
                                    e(ObjGet(
                                        e(Lvar(lid("$this"))),
                                        e(Id(ast_defs::Id(p.clone(), cvname.to_string()))),
                                        aast::OgNullFlavor::OGNullthrows,
                                    )),
                                    e(Lvar(lid(&param.name))),
                                ))),
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
                let hdr = Self::p_fun_hdr(|_, _, _, _| {}, header, env)?;
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
                let hdr = Self::p_fun_hdr(|_, _, _, _| {}, header, env)?;
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
                            let removed_names = Self::could_map(Self::pos_name, removed_names, e)?;
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
                                |mut acc, kind, n, e| -> Vec<aast_defs::UseAsVisibility> {
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
                let p_attr = |node: &Syntax<T, V>, env: &mut Env| -> Ret {
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
                                        Self::could_map(Self::p_expr, &c.xhp_enum_values, env)?;
                                    (None, Some((p, opt, vals)))
                                }
                                _ => (Some(Self::p_hint(ty, env)?), None),
                            };
                            let init_expr =
                                Self::mp_optional(Self::p_simple_initializer, init, env)?;
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

    fn p_where_constraint(
        is_class: bool,
        parent: &Syntax<T, V>,
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!(Vec<aast!(WhereConstraint)>) {
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
                let f = |n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(WhereConstraint) {
                    match &n.syntax {
                        WhereConstraint(c) => {
                            use ast_defs::ConstraintKind::*;
                            let l = Self::p_hint(&c.where_constraint_left_type, e)?;
                            let o = &c.where_constraint_operator;
                            let o = match Self::token_kind(o) {
                                Some(TK::Equal) => ConstraintEq,
                                Some(TK::As) => ConstraintAs,
                                Some(TK::Super) => ConstraintSuper,
                                _ => Self::missing_syntax(None, "constriant operator", o, e)?,
                            };
                            let r = Self::p_hint(&c.where_constraint_right_type, e)?;
                            Ok(aast::WhereConstraint(l, o, r))
                        }
                        _ => Self::missing_syntax(None, "where constraint", n, e),
                    }
                };
                Self::as_list(&c.where_clause_constraints)
                    .iter()
                    .map(|n| f(n, env))
                    .collect()
            }
            _ => {
                if is_class {
                    Self::missing_syntax(None, "classish declaration constraints", parent, env)
                } else {
                    Self::missing_syntax(None, "function header constraints", parent, env)
                }
            }
        }
    }

    fn p_namespace_use_kind(kind: &Syntax<T, V>, env: &mut Env) -> ret_aast!(NsKind) {
        use aast::NsKind::*;
        match &kind.syntax {
            Missing => Ok(NSClassAndNamespace),
            _ => match Self::token_kind(kind) {
                Some(TK::Namespace) => Ok(NSNamespace),
                Some(TK::Type) => Ok(NSClass),
                Some(TK::Function) => Ok(NSFun),
                Some(TK::Const) => Ok(NSConst),
                _ => Self::missing_syntax(None, "namespace use kind", kind, env),
            },
        }
    }

    fn p_namespace_use_clause(
        prefix: Option<&Syntax<T, V>>,
        kind: ret_aast!(NsKind),
        node: &Syntax<T, V>,
        env: &mut Env,
    ) -> ret!((aast!(NsKind), aast!(Sid), aast!(Sid))) {
        lazy_static! {
            static ref NAMESPACE_USE: regex::Regex = regex::Regex::new("[^\\\\]*$").unwrap();
        }

        match &node.syntax {
            NamespaceUseClause(c) => {
                let clause_kind = &c.namespace_use_clause_kind;
                let alias = &c.namespace_use_alias;
                let name = &c.namespace_use_name;
                let ast_defs::Id(p, n) = match (prefix, Self::pos_name(name, env)?) {
                    (None, id) => id,
                    (Some(prefix), ast_defs::Id(p, n)) => {
                        ast_defs::Id(p, Self::pos_name(prefix, env)?.1 + &n)
                    }
                };
                let alias = if alias.is_missing() {
                    let x = NAMESPACE_USE.find(&n).unwrap().as_str();
                    ast_defs::Id(p.clone(), x.to_string())
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
                    ast_defs::Id(
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
            _ => Self::missing_syntax(None, "namespace use clause", node, env),
        }
    }

    fn p_def(node: &Syntax<T, V>, env: &mut Env) -> ret!(Vec<aast!(Def<,>)>) {
        let doc_comment_opt = Self::extract_docblock(node, env);
        match &node.syntax {
            FunctionDeclaration(child) => {
                let function_attribute_spec = &child.function_attribute_spec;
                let function_declaration_header = &child.function_declaration_header;
                let function_body = &child.function_body;
                let mut env = Env::clone_and_unset_toplevel_if_toplevel(env);
                let env = env.as_mut();
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
                let name = Self::pos_name(&c.classish_name, env)?;
                *env.cls_reified_generics() = HashSet::new();
                let tparams = aast::ClassTparams {
                    list: Self::p_tparam_l(true, &c.classish_type_parameters, env)?,
                    constraints: s_map::SMap::new(),
                };
                let extends = Self::could_map(Self::p_hint, &c.classish_extends_list, env)?;
                // TODO: env.parent_may_reified =
                let implements = Self::could_map(Self::p_hint, &c.classish_implements_list, env)?;
                let where_constraints =
                    Self::p_where_constraint(true, node, &c.classish_where_clause, env)?;
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
                                type_: Self::mp_optional(Self::p_hint, ty, env)?,
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
            AliasDeclaration(c) => {
                let tparams = Self::p_tparam_l(false, &c.alias_generic_parameter, env)?;
                for tparam in tparams.iter() {
                    if tparam.reified != aast::ReifyKind::Erased {
                        Self::raise_parsing_error(node, env, &syntax_error::invalid_reified)
                    }
                }
                Ok(vec![aast::Def::Typedef(aast::Typedef {
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
                        Some(TK::Type) => aast::TypedefVisibility::Transparent,
                        Some(TK::Newtype) => aast::TypedefVisibility::Opaque,
                        _ => Self::missing_syntax(None, "kind", &c.alias_keyword, env)?,
                    },
                    kind: Self::p_hint(&c.alias_type, env)?,
                })])
            }
            EnumDeclaration(c) => {
                let p_enumerator = |n: &Syntax<T, V>, e: &mut Env| -> ret_aast!(ClassConst<,>) {
                    match &n.syntax {
                        Enumerator(c) => Ok(aast::ClassConst {
                            type_: None,
                            visibility: aast::Visibility::Public,
                            id: Self::pos_name(&c.enumerator_name, e)?,
                            expr: Some(Self::p_expr(&c.enumerator_value, e)?),
                            doc_comment: None,
                        }),
                        _ => Self::missing_syntax(None, "enumerator", n, e),
                    }
                };
                Ok(vec![aast::Def::Class(aast::Class_ {
                    annotation: (),
                    mode: Self::mode_annotation(env.file_mode()),
                    user_attributes: Self::p_user_attributes(&c.enum_attribute_spec, env)?,
                    file_attributes: vec![],
                    final_: false,
                    kind: ast_defs::ClassKind::Cenum,
                    is_xhp: false,
                    name: Self::pos_name(&c.enum_name, env)?,
                    tparams: aast::ClassTparams {
                        list: vec![],
                        constraints: s_map::SMap::new(),
                    },
                    extends: vec![],
                    implements: vec![],
                    where_constraints: vec![],
                    consts: Self::could_map(p_enumerator, &c.enum_enumerators, env)?,
                    namespace: Self::mk_empty_ns_env(env),
                    span: Self::p_pos(node, env),
                    enum_: Some(aast::Enum_ {
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
                })])
            }
            RecordDeclaration(c) => {
                let p_field = |n: &Syntax<T, V>, e: &mut Env| match &n.syntax {
                    RecordField(c) => Ok((
                        Self::pos_name(&c.record_field_name, e)?,
                        Self::p_hint(&c.record_field_type, e)?,
                        Self::mp_optional(Self::p_simple_initializer, &c.record_field_init, e)?,
                    )),
                    _ => Self::missing_syntax(None, "record_field", n, e),
                };
                Ok(vec![aast::Def::RecordDef(aast::RecordDef {
                    name: Self::pos_name(&c.record_name, env)?,
                    extends: Self::could_map(Self::p_hint, &c.record_extends_list, env)?
                        .into_iter()
                        .next(),
                    final_: Self::token_kind(&c.record_modifier) == Some(TK::Final),
                    user_attributes: Self::p_user_attributes(&c.record_attribute_spec, env)?,
                    fields: Self::could_map(p_field, &c.record_fields, env)?,
                    namespace: Self::mk_empty_ns_env(env),
                    span: Self::p_pos(node, env),
                    doc_comment: doc_comment_opt,
                })])
            }
            InclusionDirective(child)
                if env.file_mode() != file_info::Mode::Mdecl
                    && env.file_mode() != file_info::Mode::Mphp
                    || env.codegen() =>
            {
                let expr = Self::p_expr(&child.inclusion_expression, env)?;
                Ok(vec![aast::Def::Stmt(aast::Stmt::new(
                    Self::p_pos(node, env),
                    aast::Stmt_::Expr(expr),
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
                Ok(vec![aast::Def::Namespace(Self::pos_name(name, env)?, defs)])
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
                Ok(vec![aast::Def::NamespaceUse(uses?)])
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
                Ok(vec![aast::Def::NamespaceUse(uses?)])
            }
            FileAttributeSpecification(_) => {
                Ok(vec![aast::Def::FileAttributes(aast::FileAttribute {
                    user_attributes: Self::p_user_attribute(node, env)?,
                    namespace: Self::mk_empty_ns_env(env),
                })])
            }
            _ if env.file_mode() == file_info::Mode::Mdecl
                || env.file_mode() == file_info::Mode::Mphp && !env.codegen =>
            {
                Ok(vec![])
            }
            _ => Ok(vec![aast::Def::Stmt(Self::p_stmt(node, env)?)]),
        }
    }

    // TODO:
    fn partial_should_check_error() -> bool {
        true
    }

    fn post_process(env: &mut Env, program: aast!(Program<,>), acc: &mut aast!(Program<,>)) {
        use aast::{Def::*, Expr_::*, Stmt_::*};
        let mut saw_ns: Option<(aast!(Sid), aast!(Program<,>))> = None;
        for def in program.into_iter() {
            if let Namespace(_, _) = &def {
                if let Some((n, ns_acc)) = saw_ns {
                    acc.push(Namespace(n, ns_acc));
                    saw_ns = None;
                }
            }

            if let Namespace(n, defs) = def {
                if defs.is_empty() {
                    saw_ns = Some((n, vec![]));
                } else {
                    let mut acc_ = vec![];
                    Self::post_process(env, defs, &mut acc_);
                    acc.push(Namespace(n, acc_));
                }

                continue;
            }

            if let Stmt(s) = &def {
                if Self::is_noop(&s) {
                    continue;
                }
                let raise_error = if let Markup(_, _) = *s.1 {
                    false
                } else if let Expr(aast::Expr(_, ref e)) = *s.1 {
                    if let Import(_, _) = **e {
                        env.parser_options.po_disallow_toplevel_requires
                    } else {
                        true
                    }
                } else {
                    env.keep_errors && env.is_typechecker() && Self::partial_should_check_error()
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
            acc.push(Namespace(n, defs));
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
                    *env.saw_compiler_halt_offset() = Some(pos.end_cnum());
                }
                _ => match Self::p_def(nodes[i], env) {
                    Err(Error::APIMissingSyntax { .. }) if env.fail_open => {}
                    e @ Err(_) => return e,
                    Ok(mut def) => acc.append(&mut def),
                },
            }
        }
        let mut program = vec![];
        Self::post_process(env, acc, &mut program);
        Ok(program)
    }

    fn p_script(node: &Syntax<T, V>, env: &mut Env) -> ret_aast!(Program<,>) {
        match &node.syntax {
            Script(children) => Self::p_program(&children.script_declarations, env),
            _ => Self::missing_syntax(None, "script", node, env),
        }
    }

    fn elabarate_halt_compiler(ast: aast!(Program<,>), env: &mut Env) -> aast!(Program<,>) {
        match *env.saw_compiler_halt_offset() {
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
