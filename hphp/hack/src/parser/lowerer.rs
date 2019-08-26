// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized as o;
use oxidized::file_info;
use oxidized::parser_options::ParserOptions;
use oxidized::pos::Pos;
use oxidized::prim_defs::Comment;
use oxidized::relative_path::RelativePath;
use oxidized::s_set::SSet;
use parser::lexable_token::LexablePositionedToken;
use parser::positioned_syntax::PositionedSyntaxTrait;
use parser::source_text::SourceText;
use parser_core_types::syntax::{Syntax, SyntaxVariant};
use parser_rust as parser;

use std::result::Result::{Err, Ok};

macro_rules! aast {
    ($ty:ident) =>  {o::aast::$ty};
    // NOTE: In <,> pattern, comma prevents rustfmt eating <>
    ($ty:ident<,>) =>  {o::aast::$ty<Pos, (), (), ()>}
}

macro_rules! ret {
    ($ty:ident) => { std::result::Result<$ty, Error<Syntax<T, V>>> }
}

macro_rules! ret_aast {
    ($ty:ident) => { std::result::Result<aast!($ty), Error<Syntax<T, V>>> };
    ($ty:ident<,>) => { std::result::Result<aast!($ty<,>), Error<Syntax<T, V>>> }
}

#[derive(Debug, Clone)]
enum LiftedAwaitKind {
    LiftedFromStatement,
    LiftedFromConcurrent,
}

#[derive(Debug, Clone)]
pub struct LiftedAwaits {
    pub awaits: Vec<(Option<o::aast_defs::Id>, aast!(Expr<,>))>,
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

    pub source_text: &'a SourceText<'a>,
}

impl<'a> Env<'a> {
    fn is_hh_file(&self) -> bool {
        self.is_hh_file
    }

    fn fi_mode(&self) -> file_info::Mode {
        self.fi_mode.clone()
    }

    fn should_surface_error(&self) -> bool {
        (!self.quick_mode || self.show_all_errors) && self.keep_errors
    }

    fn is_typechecker(&self) -> bool {
        self.codegen
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

pub trait Ast<'a, T, V>
where
    T: LexablePositionedToken<'a>,
    Syntax<T, V>: PositionedSyntaxTrait,
{
    fn p_pos(node: &Syntax<T, V>, env: &Env) -> Pos {
        //TODO:
        if env.ignore_pos {
            Pos::make_none()
        } else {
            Pos::make_none()
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

    fn p_program(node: &Syntax<T, V>, mut env: &Env) -> ret_aast!(Program<,>) {
        //TODO:
        Ok(vec![])
    }

    fn p_script(node: &Syntax<T, V>, mut env: &Env) -> ret_aast!(Program<,>) {
        //TODO:
        match &node.syntax {
            SyntaxVariant::Script(children) => Self::p_program(&children.script_declarations, env),
            _ => Err(Error::LowererInvariantFailure(
                String::from(""),
                String::from(""),
            )),
        }
    }

    fn lower(
        mut env: &Env<'a>,
        script: &Syntax<T, V>,
        comments: Vec<(Pos, Comment)>,
    ) -> Result<'a> {
        //TODO:
        let ast = vec![];
        Result {
            fi_mode: env.fi_mode(),
            is_hh_file: env.is_hh_file(),
            ast,
            content: env.source_text.text(),
            file: env.file.clone(),
            comments,
        }
    }
}
