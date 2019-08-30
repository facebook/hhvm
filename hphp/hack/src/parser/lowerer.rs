// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::map::Map;
use oxidized::namespace_env::Env as NamespaceEnv;
use oxidized::parser_options::ParserOptions;
use oxidized::pos::Pos;
use oxidized::prim_defs::Comment;
use oxidized::relative_path::RelativePath;
use oxidized::s_map::SMap;
use oxidized::s_set::SSet;
use oxidized::{aast, file_info};
use parser::indexed_source_text::IndexedSourceText;
use parser::lexable_token::LexablePositionedToken;
use parser::positioned_syntax::PositionedSyntaxTrait;
use parser::source_text::SourceText;
use parser_core_types::syntax::*;

use parser_rust as parser;

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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
pub struct Env<'a> {
    is_hh_file: bool,
    codegen: bool,
    php5_compat_mode: bool,
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
    parser_options: &'a ParserOptions,
    fi_mode: file_info::Mode,
    file: RelativePath,
    hacksperimental: bool,
    top_level_statements: bool, /* Whether we are (still) considering TLSs*/
    /* Changing parts; should disappear in future. `mutable` saves allocations. */
    pub ignore_pos: bool,
    pub max_depth: isize, /* Filthy hack around OCaml bug */
    pub saw_yield: bool,  /* Information flowing back up */
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
    pub recursion_depth: isize,
    cls_reified_generics: SSet,
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
}

impl<'a> Env<'a> {
    fn is_hh_file(&self) -> bool {
        self.is_hh_file
    }

    fn fi_mode(&self) -> file_info::Mode {
        self.fi_mode
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
}

pub enum Error<Node> {
    APIMissingSyntax(String, Node),
    LowererInvariantFailure(String, String),
}

pub struct Result<'a> {
    fi_mode: file_info::Mode,
    is_hh_file: bool,
    ast: aast!(Program<,>),
    content: &'a [u8],
    file: RelativePath,
    comments: Vec<(Pos, Comment)>,
}

use parser_core_types::syntax::SyntaxVariant::*;

pub trait Ast<'a, T, V>
where
    T: LexablePositionedToken<'a>,
    Syntax<T, V>: PositionedSyntaxTrait,
{
    fn mode_annotation(mode: file_info::Mode) -> file_info::Mode {
        match mode {
            file_info::Mode::Mphp => file_info::Mode::Mdecl,
            m => m,
        }
    }

    fn p_pos(node: &Syntax<T, V>, env: &Env) -> Pos {
        if env.ignore_pos {
            Pos::make_none()
        } else {
            node.position_exclusive(env.indexed_source_text)
                .unwrap_or(Pos::make_none())
        }
    }

    fn missing_syntax<N>(
        fallback: Option<N>,
        expecting: &'static str,
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

    fn mp_optional<S>(
        p: &Fn(&Syntax<T, V>, &Env) -> ret!(S),
        node: &Syntax<T, V>,
        env: &Env,
    ) -> ret!(Option<S>) {
        match &node.syntax {
            Missing => Ok(None),
            _ => p(node, env).map(Some),
        }
    }

    fn pos_name(node: &Syntax<T, V>, env: &Env) -> aast!(Sid) {
        not_impl!()
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

    fn p_hint(node: &Syntax<T, V>, env: &Env) -> ret_aast!(Hint) {
        not_impl!()
    }

    fn p_simple_initializer(node: &Syntax<T, V>, env: &Env) -> ret_aast!(Expr<,>) {
        match &node.syntax {
            SimpleInitializer(child) => Self::p_expr(&child.simple_initializer_value, env),
            _ => Self::missing_syntax(None, "simple initializer", node, env),
        }
    }

    fn p_expr(node: &Syntax<T, V>, env: &Env) -> ret_aast!(Expr<,>) {
        not_impl!()
    }

    fn p_def(node: &Syntax<T, V>, env: &Env) -> ret!(Vec<aast!(Def<,>)>) {
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
                                mode: Self::mode_annotation(env.fi_mode()),
                                name: Self::pos_name(name, env),
                                type_: Self::mp_optional(&Self::p_hint, ty, env)?,
                                value: Self::p_simple_initializer(init, env)?,
                                namespace: namespace_env_empty(&env.parser_options),
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
                if env.fi_mode() != file_info::Mode::Mdecl
                    && env.fi_mode() != file_info::Mode::Mphp
                    || env.codegen() =>
            {
                let expr = Self::p_expr(&child.inclusion_expression, env)?;
                Ok(vec![aast::Def::Stmt(aast::Stmt(
                    Self::p_pos(node, env),
                    Box::new(aast::Stmt_::Expr(expr)),
                ))])
            }
            _ => not_impl!(),
        }
    }

    fn p_program(node: &Syntax<T, V>, mut env: &Env) -> ret_aast!(Program<,>) {
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

    fn p_script(node: &Syntax<T, V>, mut env: &Env) -> ret_aast!(Program<,>) {
        match &node.syntax {
            Script(children) => Self::p_program(&children.script_declarations, env),
            _ => Self::missing_syntax(None, "script", node, env),
        }
    }

    fn lower(
        mut env: &Env<'a>,
        script: &Syntax<T, V>,
        comments: Vec<(Pos, Comment)>,
    ) -> ::std::result::Result<Result<'a>, String> {
        let ast_result = Self::p_script(script, env);
        // TODO: handle error
        let ast = match ast_result {
            Ok(ast) => ast,
            // TODO: add msg
            Err(_) => return Err(String::from("ERROR")),
        };
        Ok(Result {
            fi_mode: env.fi_mode(),
            is_hh_file: env.is_hh_file(),
            ast,
            content: env.source_text().text(),
            file: env.file.clone(),
            comments,
        })
    }
}

// Move to namespace_env.rs
fn namespace_env_empty(parser_options: &ParserOptions) -> NamespaceEnv {
    NamespaceEnv {
        ns_uses: Map::empty(),
        class_uses: Map::empty(),
        fun_uses: Map::empty(),
        const_uses: Map::empty(),
        name: None,
        // TODO: avoid clone
        popt: parser_options.clone(),
    }
}
