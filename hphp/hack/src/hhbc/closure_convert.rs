// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use itertools::{Either, EitherOrBoth::*, Itertools};
use std::{
    borrow::Cow,
    collections::{BTreeMap, HashSet},
    mem,
};

use ast_constant_folder_rust as ast_constant_folder;
use ast_scope_rust as ast_scope;
use decl_vars_rust as decl_vars;
use emit_fatal_rust as emit_fatal;
use env::emitter::Emitter;
use global_state::{ClosureEnclosingClassInfo, GlobalState, LazyState};
use hhbc_id::class;
use hhbc_id_rust as hhbc_id;
use hhbc_string_utils_rust as string_utils;
use instruction_sequence_rust::{unrecoverable, Error, Result};
use naming_special_names_rust::{fb, pseudo_consts, special_idents, superglobals};
use options::{CompilerFlags, HhvmFlags, Options};
use oxidized::{
    aast_defs,
    aast_visitor::{AstParams, NodeMut, VisitorMut},
    ast::*,
    ast_defs::*,
    file_info::Mode,
    local_id, namespace_env,
    s_map::SMap,
};
use rx_rust as rx;
use unique_list_rust::UniqueList;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum HoistKind {
    /// Def that is already at top-level
    TopLevel,
    /// Def that was hoisted to top-level
    Hoisted,
}

#[derive(Debug, Clone)] // TODO(hrust): Clone is used when bactracking now, can we somehow avoid it?
struct Variables {
    /// all variables declared/used in the scope
    all_vars: HashSet<String>,
    /// names of parameters if scope correspond to a function
    parameter_names: HashSet<String>,
}

#[derive(Debug, Clone)] // TODO(hrust): do we need clone
struct Env<'a> {
    /// What is the current context?
    // TODO(hrust) VisitorMut doesn't provide an interface
    // where a reference to visited NodeMut outlives the Context type (in this case Env<'a>),
    // so we have no choice but to clone in ach ScopeItem (i.e., can't se Borrowed for 'a)

    /// Span of function/method body
    pos: Pos, // TODO(hrust) change to &'a Pos after dependent Visitor/Node lifetime is fixed.
    /// What is the current context?
    scope: ast_scope::Scope<'a>,
    variable_scopes: Vec<Variables>,
    /// How many existing classes are there?
    defined_class_count: usize,
    /// How many existing records are there?
    defined_record_count: usize,
    /// How many existing functions are there?
    defined_function_count: usize,
    /// Are we immediately in a using statement?
    in_using: bool,
    /// Global compiler/hack options
    options: &'a Options,
}

#[derive(Default, Clone)]
struct PerFunctionState {
    pub has_finally: bool,
    pub has_goto: bool,
    pub labels: BTreeMap<String, bool>,
}

impl<'a> Env<'a> {
    pub fn toplevel(
        class_count: usize,
        record_count: usize,
        function_count: usize,
        defs: &Program,
        options: &'a Options,
    ) -> Result<Self> {
        let scope = ast_scope::Scope::toplevel();
        let all_vars = get_vars(&scope, false, &vec![], Either::Left(&defs))?;

        Ok(Self {
            pos: Pos::make_none(),
            scope,
            variable_scopes: vec![Variables {
                all_vars,
                parameter_names: HashSet::new(),
            }],
            defined_class_count: class_count,
            defined_record_count: record_count,
            defined_function_count: function_count,
            in_using: false,
            options,
        })
    }

    fn with_function_like_(
        &mut self,
        e: ast_scope::ScopeItem<'a>,
        is_closure_body: bool,
        params: &[FunParam],
        pos: Pos,
        body: &Block,
    ) -> Result<()> {
        self.pos = pos;
        self.scope.push_item(e);
        let all_vars = get_vars(&self.scope, is_closure_body, params, Either::Right(body))?;
        Ok(self.variable_scopes.push(Variables {
            parameter_names: get_parameter_names(params),
            all_vars,
        }))
    }

    fn with_function_like(
        &mut self,
        e: ast_scope::ScopeItem<'a>,
        is_closure_body: bool,
        fd: &Fun_,
    ) -> Result<()> {
        self.with_function_like_(
            e,
            is_closure_body,
            &fd.params,
            fd.span.clone(),
            &fd.body.ast,
        )
    }

    fn with_function(&mut self, fd: &Fun_) -> Result<()> {
        self.with_function_like(
            ast_scope::ScopeItem::Function(Cow::Owned(fd.clone())),
            false,
            fd,
        )
    }

    fn with_method(&mut self, md: &Method_) -> Result<()> {
        self.with_function_like_(
            ast_scope::ScopeItem::Method(Cow::Owned(md.clone())),
            false,
            &md.params,
            md.span.clone(),
            &md.body.ast,
        )
    }

    fn with_lambda(&mut self, fd: &Fun_) -> Result<()> {
        let is_async = fd.fun_kind.is_async();
        let rx_level = rx::Level::from_ast(&fd.user_attributes);

        let lambda = ast_scope::Lambda { is_async, rx_level };
        self.with_function_like(ast_scope::ScopeItem::Lambda(Cow::Owned(lambda)), true, fd)
    }

    fn with_longlambda(&mut self, is_static: bool, fd: &Fun_) -> Result<()> {
        let is_async = fd.fun_kind.is_async();
        let rx_level = rx::Level::from_ast(&fd.user_attributes);

        let long_lambda = ast_scope::LongLambda {
            is_static,
            is_async,
            rx_level,
        };
        self.with_function_like(
            ast_scope::ScopeItem::LongLambda(Cow::Owned(long_lambda.clone())),
            true,
            fd,
        )
    }

    fn with_class(&mut self, cd: &Class_) {
        self.scope = ast_scope::Scope::toplevel();
        self.scope
            .push_item(ast_scope::ScopeItem::Class(Cow::Owned(cd.clone())));
    }

    fn with_in_using<F, R>(&mut self, in_using: bool, mut f: F) -> R
    where
        F: FnMut(&mut Self) -> R,
    {
        let old_in_using = self.in_using;
        self.in_using = in_using;
        let r = f(self);
        self.in_using = old_in_using;
        r
    }

    fn check_if_in_async_context(&self) -> Result<()> {
        let check_valid_fun_kind = |name, kind: FunKind| {
            if !kind.is_async() {
                Err(emit_fatal::raise_fatal_parse(
                    &self.pos,
                    format!(
                        "Function '{}' contains 'await' but is not declared as async.",
                        string_utils::strip_global_ns(name)
                    ),
                ))
            } else {
                Ok(())
            }
        };
        let check_lambda = |is_async: bool| {
            if !is_async {
                Err(emit_fatal::raise_fatal_parse(
                    &self.pos,
                    "Await may only appear in an async function",
                ))
            } else {
                Ok(())
            }
        };
        let head = self.scope.iter().next();
        use ast_scope::ScopeItem as S;
        match head {
            None => Err(emit_fatal::raise_fatal_parse(
                &self.pos,
                "'await' can only be used inside a function",
            )),
            Some(S::Lambda(l)) => check_lambda(l.is_async),
            Some(S::LongLambda(l)) => check_lambda(l.is_async),
            Some(S::Class(_)) => Ok(()), /* Syntax error, wont get here */
            Some(S::Function(fd)) => check_valid_fun_kind(&fd.name.1, fd.fun_kind),
            Some(S::Method(md)) => check_valid_fun_kind(&md.name.1, md.fun_kind),
        }
    }
}

struct State {
    // Number of closures created in the current function
    closure_cnt_per_fun: u32,
    // Free variables computed so far
    captured_vars: UniqueList<String>,
    captured_this: bool,
    captured_generics: UniqueList<String>,
    // Closure classes and hoisted inline classes
    hoisted_classes: Vec<Class_>,
    /// Hoisted meth_caller functions
    named_hoisted_functions: SMap<Fun_>,
    // The current namespace environment
    namespace: namespace_env::Env,
    // Empty namespace as constructed by parser
    empty_namespace: namespace_env::Env,
    // information about current function
    current_function_state: PerFunctionState,
    // accumulated information about program
    global_state: GlobalState,
}

impl State {
    pub fn initial_state(empty_namespace: namespace_env::Env) -> Self {
        Self {
            namespace: empty_namespace.clone(),
            empty_namespace, // TODO(hrust) : pass in Rc?
            closure_cnt_per_fun: 0,
            captured_vars: UniqueList::new(),
            captured_this: false,
            captured_generics: UniqueList::new(),
            hoisted_classes: vec![],
            named_hoisted_functions: SMap::new(),
            current_function_state: PerFunctionState::default(),
            global_state: GlobalState::default(),
        }
    }

    pub fn reset_function_counts(&mut self) {
        self.closure_cnt_per_fun = 0;
    }

    pub fn record_function_state(
        &mut self,
        key: String,
        fun: PerFunctionState,
        rx_of_scope: rx::Level,
    ) {
        if fun.has_finally {
            self.global_state.functions_with_finally.insert(key.clone());
        }

        if !fun.labels.is_empty() {
            self.global_state
                .function_to_labels_map
                .insert(key.clone(), fun.labels);
        }

        if rx_of_scope != rx::Level::NonRx {
            self.global_state
                .lambda_rx_of_scope
                .insert(key, rx_of_scope);
        }
    }

    // Clear the variables, upon entering a lambda
    pub fn enter_lambda(&mut self) {
        self.captured_vars = UniqueList::new();
        self.captured_this = false;
        self.captured_generics = UniqueList::new();
    }
}

fn total_class_count(env: &Env, st: &State) -> usize {
    st.hoisted_classes.len() + env.defined_class_count
}

fn should_capture_var(env: &Env, var: &str) -> bool {
    // variable used in lambda should be captured if is:
    // - not contained in lambda parameter list
    if let Some(true) = env
        .variable_scopes
        .last()
        .map(|x| x.parameter_names.contains(var))
    {
        return false;
    };
    // AND
    // - it exists in one of enclosing scopes
    for x in env
        .scope
        .iter()
        .zip_longest(env.variable_scopes.iter().rev())
        .skip(1)
    {
        match x {
            Right(vars) => return vars.all_vars.contains(var),
            Both(scope, vars) => {
                if vars.all_vars.contains(var) || vars.parameter_names.contains(var) {
                    return true;
                }
                if !scope.is_in_lambda() {
                    return false;
                }
            }
            Left(_) => return false,
        }
    }
    false
}

// Add a variable to the captured variables
fn add_var(env: &Env, st: &mut State, var: &str) {
    // Don't bother if it's $this, as this is captured implicitly
    if var == special_idents::THIS {
        st.captured_this = true;
    } else if
    // If it's bound as a parameter or definite assignment, don't add it
    // Also don't add the pipe variable and superglobals
    (should_capture_var(env, var))
        && !(var == special_idents::DOLLAR_DOLLAR
            || superglobals::GLOBALS == var
            || superglobals::is_superglobal(var))
    {
        st.captured_vars.add(var.to_string())
    }
}

fn add_generic(env: &mut Env, st: &mut State, var: &str) {
    let reified_var_position = |is_fun| {
        let is_reified_var =
            |param: &Tparam| param.reified != ReifyKind::Erased && param.name.1 == var;
        if is_fun {
            env.scope.get_fun_tparams().iter().position(is_reified_var)
        } else {
            env.scope
                .get_class_tparams()
                .list
                .iter()
                .position(is_reified_var)
        }
    };

    if let Some(i) = reified_var_position(true) {
        let var = string_utils::reified::captured_name(true, i);
        st.captured_generics.add(var)
    } else if let Some(i) = reified_var_position(false) {
        let var = string_utils::reified::captured_name(false, i);
        st.captured_generics.add(var)
    }
}

fn get_vars(
    scope: &ast_scope::Scope,
    is_closure_body: bool,
    params: &[FunParam],
    body: decl_vars::ProgramOrStmt,
) -> Result<HashSet<String>> {
    use decl_vars::{vars_from_ast, Flags};
    let mut flags = Flags::empty();
    flags.set(Flags::HAS_THIS, scope.has_this());
    flags.set(Flags::IS_TOPLEVEL, scope.is_toplevel());
    flags.set(Flags::IS_IN_STATIC_METHOD, scope.is_in_static_method());
    flags.set(Flags::IS_CLOSURE_BODY, is_closure_body);
    let res = vars_from_ast(params, body, flags).map_err(unrecoverable);
    res
}

fn get_parameter_names(params: &[FunParam]) -> HashSet<String> {
    params.iter().map(|p| p.name.to_string()).collect()
}

fn strip_id(id: &Id) -> &str {
    string_utils::strip_global_ns(&id.1)
}

fn make_class_name(cd: &Class_) -> String {
    string_utils::mangle_xhp_id(strip_id(&cd.name).to_string())
}

fn make_scope_name(scope: &ast_scope::Scope) -> String {
    let mut parts: Vec<String> = vec![];

    for sub_scope in scope.iter_subscopes() {
        match sub_scope.last().unwrap() {
            ast_scope::ScopeItem::Class(x) => {
                parts.push(make_class_name(&x));
                break;
            }
            ast_scope::ScopeItem::Function(x) => {
                let fname = strip_id(&x.name);
                parts.push(
                    ast_scope::Scope::get_subscope_class(sub_scope)
                        .map(|cd| make_class_name(cd) + "::")
                        .unwrap_or_default()
                        + &fname,
                );
                break;
            }
            ast_scope::ScopeItem::Method(x) => {
                parts.push(strip_id(&x.name).to_string());
                if !parts.last().map(|x| x.ends_with("::")).unwrap_or(false) {
                    parts.push("::".into())
                };
            }
            _ => (),
        }
    }
    parts.reverse();
    parts.join("")
}

fn make_closure_name(env: &Env, st: &State) -> String {
    let per_fun_idx = st.closure_cnt_per_fun;
    string_utils::closures::mangle_closure(&make_scope_name(&env.scope), per_fun_idx)
}

fn make_closure(
    class_num: usize,
    p: Pos,
    env: &Env,
    st: &State,
    lambda_vars: Vec<String>,
    fun_tparams: Vec<Tparam>,
    class_tparams: ClassTparams,
    is_static: bool,
    mut fd: Fun_,
) -> (Fun_, Class_) {
    let md = Method_ {
        span: fd.span.clone(),
        annotation: fd.annotation,
        final_: false,
        abstract_: false,
        static_: is_static,
        visibility: Visibility::Public,
        name: Id(fd.name.0.clone(), "__invoke".into()),
        tparams: fun_tparams,
        where_constraints: fd.where_constraints.clone(),
        variadic: fd.variadic.clone(),
        params: fd.params.clone(),
        body: fd.body.clone(),
        fun_kind: fd.fun_kind,
        user_attributes: fd.user_attributes.clone(),
        ret: fd.ret.clone(),
        external: false,
        doc_comment: fd.doc_comment.clone(),
    };

    let make_class_var = |name: &str| ClassVar {
        final_: false,
        xhp_attr: None,
        abstract_: false,
        visibility: Visibility::Private,
        type_: TypeHint((), None),
        id: Id(p.clone(), name.into()),
        expr: None,
        user_attributes: vec![],
        doc_comment: None,
        is_promoted_variadic: false,
        is_static: false,
        span: p.clone(),
    };

    let cvl = lambda_vars
        .iter()
        .map(|name| make_class_var(string_utils::locals::strip_dollar(name)));

    let cd = Class_ {
        span: p.clone(),
        annotation: fd.annotation,
        mode: fd.mode,
        user_attributes: vec![],
        file_attributes: vec![],
        final_: false,
        is_xhp: false,
        has_xhp_keyword: false,
        kind: ClassKind::Cnormal,
        name: Id(p.clone(), make_closure_name(env, st)),
        tparams: class_tparams,
        extends: vec![Hint(
            p.clone(),
            Box::new(Hint_::Happly(Id(p.clone(), "Closure".into()), vec![])),
        )],
        uses: vec![],
        use_as_alias: vec![],
        insteadof_alias: vec![],
        method_redeclarations: vec![],
        xhp_attr_uses: vec![],
        xhp_category: None,
        reqs: vec![],
        implements: vec![],
        where_constraints: vec![],
        consts: vec![],
        typeconsts: vec![],
        vars: cvl.collect(),
        methods: vec![md],
        attributes: vec![],
        xhp_children: vec![],
        xhp_attrs: vec![],
        namespace: ocamlrep::rc::RcOc::new(st.empty_namespace.clone()),
        enum_: None,
        doc_comment: None,
        pu_enums: vec![],
        emit_id: Some(EmitId::Anonymous),
    };

    // TODO(hrust): can we reconstruct fd here from the scratch?
    fd.name = Id(p.clone(), class_num.to_string());
    fd.static_ = is_static;
    (fd, cd)
}

// Translate special identifiers __CLASS__, __METHOD__ and __FUNCTION__ into
// literal strings. It's necessary to do this before closure conversion
// because the enclosing class will be changed.
fn convert_id(env: &Env, Id(p, s): Id) -> Expr_ {
    use ast_scope::*;
    let ret = |newstr| Expr_::mk_string(newstr);
    let name =
        |c: &Class_| Expr_::mk_string(string_utils::mangle_xhp_id(strip_id(&c.name).to_string()));

    match s {
        _ if s.eq_ignore_ascii_case(pseudo_consts::G__TRAIT__) => match env.scope.get_class() {
            Some(c) if c.kind == ClassKind::Ctrait => name(c),
            _ => ret("".into()),
        },
        _ if s.eq_ignore_ascii_case(pseudo_consts::G__CLASS__) => match env.scope.get_class() {
            Some(c) if c.kind != ClassKind::Ctrait => name(c),
            Some(_) => Expr_::mk_id(Id(p, s)),
            None => ret("".into()),
        },
        _ if s.eq_ignore_ascii_case(pseudo_consts::G__METHOD__) => {
            let (prefix, is_trait) = match env.scope.get_class() {
                None => ("".into(), false),
                Some(cd) => (
                    string_utils::mangle_xhp_id(strip_id(&cd.name).to_string()) + "::",
                    cd.kind == ClassKind::Ctrait,
                ),
            };
            // for lambdas nested in trait methods HHVM replaces __METHOD__
            // with enclosing method name - do the same and bubble up from lambdas *
            let scope = env
                .scope
                .iter()
                .skip_while(|x| is_trait && x.is_in_lambda())
                .next();

            match scope {
                Some(ScopeItem::Function(fd)) => ret(prefix + strip_id(&fd.name)),
                Some(ScopeItem::Method(md)) => ret(prefix + strip_id(&md.name)),
                Some(ScopeItem::Lambda(_)) | Some(ScopeItem::LongLambda(_)) => {
                    ret(prefix + "{closure}")
                }
                // PHP weirdness: __METHOD__ inside a class outside a method returns class name
                Some(ScopeItem::Class(cd)) => ret(strip_id(&cd.name).to_string()),
                _ => ret("".into()),
            }
        }
        _ if s.eq_ignore_ascii_case(pseudo_consts::G__FUNCTION__) => match env.scope.items.last() {
            Some(ScopeItem::Function(fd)) => ret(strip_id(&fd.name).to_string()),
            Some(ScopeItem::Method(md)) => ret(strip_id(&md.name).to_string()),
            Some(ScopeItem::Lambda(_)) | Some(ScopeItem::LongLambda(_)) => ret("{closure}".into()),
            _ => ret("".into()),
        },
        _ if s.eq_ignore_ascii_case(pseudo_consts::G__LINE__) => {
            // If the expression goes on multi lines, we return the last line
            let (_, line, _, _) = p.info_pos_extended();
            Expr_::mk_int(line.to_string())
        }
        _ => Expr_::mk_id(Id(p, s)),
    }
}

fn visit_class_id<'a>(
    env: &mut Env<'a>,
    self_: &mut ClosureConvertVisitor<'a>,
    cid: &mut ClassId,
) -> Result<()> {
    Ok(if let ClassId(_, ClassId_::CIexpr(e)) = cid {
        self_.visit_expr(env, e)?;
    })
}

fn make_info(c: &Class_) -> ClosureEnclosingClassInfo {
    ClosureEnclosingClassInfo {
        kind: c.kind,
        name: c.name.1.clone(),
        parent_class_name: match &c.extends.as_slice() {
            [x] => x.as_happly().map(|(id, _args)| id.1.clone()),
            _ => None,
        },
    }
}
// Closure-convert a lambda expression, with use_vars_opt = Some vars
// if there is an explicit `use` clause.
fn convert_lambda<'a>(
    env: &mut Env<'a>,
    self_: &mut ClosureConvertVisitor<'a>,
    mut fd: Fun_,
    use_vars_opt: Option<Vec<aast_defs::Lid>>,
) -> Result<Expr_> {
    let is_long_lambda = use_vars_opt.is_some();
    let st = &mut self_.state;

    // Remember the current capture and defined set across the lambda
    let captured_this = st.captured_this;
    let captured_vars = st.captured_vars.clone();
    let captured_generics = st.captured_generics.clone();
    let old_function_state = st.current_function_state.clone();
    let rx_of_scope = env.scope.rx_of_scope();
    st.enter_lambda();
    if let Some(user_vars) = &use_vars_opt {
        for aast_defs::Lid(p, id) in user_vars.iter() {
            if local_id::get_name(id) == special_idents::THIS {
                return Err(emit_fatal::raise_fatal_parse(
                    p,
                    "Cannot use $this as lexical variable",
                ));
            }
        }
    }
    let lambda_env = &mut env.clone();

    if use_vars_opt.is_some() {
        lambda_env.with_longlambda(false, &fd)?
    } else {
        lambda_env.with_lambda(&fd)?
    };
    let function_state = convert_function_like_body(self_, lambda_env, &mut fd.body)?;
    for param in &mut fd.params {
        self_.visit_fun_param(lambda_env, param)?;
    }
    self_.visit_type_hint(lambda_env, &mut fd.ret)?;

    let st = &mut self_.state;
    st.closure_cnt_per_fun += 1;

    let current_generics = st.captured_generics.clone();

    // TODO(hrust): produce real unique local ids
    let fresh_lid = |name: String| aast_defs::Lid(Pos::make_none(), (12345, name));

    let lambda_vars: Vec<&String> = st
        .captured_vars
        .iter()
        .chain(current_generics.iter())
        // HHVM lists lambda vars in descending order - do the same
        .sorted()
        .rev()
        .collect();

    // Remove duplicates, (not efficient, but unlikely to be large),
    // remove variables that are actually just parameters
    let use_vars_opt: Option<Vec<Lid>> = use_vars_opt.map(|x| {
        x.into_iter()
            .unique_by(|lid| lid.name().to_string())
            .filter(|x| !fd.params.iter().any(|y| x.name() == &y.name))
            .collect()
    });

    // For lambdas with explicit `use` variables, we ignore the computed
    // capture set and instead use the explicit set
    let (lambda_vars, use_vars): (Vec<String>, Vec<Lid>) = match use_vars_opt {
        None => (
            lambda_vars.iter().map(|x| x.to_string()).collect(),
            lambda_vars
                .iter()
                .map(|x| fresh_lid(x.to_string()))
                .collect(),
        ),
        Some(use_vars) => {
            // We still need to append the generics
            (
                use_vars
                    .iter()
                    .map(|x| x.name())
                    .chain(current_generics.iter())
                    .map(|x| x.to_string())
                    .collect(),
                use_vars
                    .iter()
                    .map(|x| x.clone())
                    .chain(current_generics.iter().map(|x| fresh_lid(x.to_string())))
                    .collect(),
            )
        }
    };

    let fun_tparams = lambda_env.scope.get_fun_tparams().to_vec();
    let class_tparams = lambda_env.scope.get_class_tparams();
    let class_num = total_class_count(lambda_env, st);

    let is_static = if is_long_lambda {
        // long lambdas are static if they are annotated as such
        fd.static_
    } else {
        // short lambdas can be made static if they don't capture this in
        // any form (including any nested non-static lambdas )
        !st.captured_this
    };

    // check if something can be promoted to static based on enclosing scope
    let is_static = is_static || lambda_env.scope.is_static();

    let pos = fd.span.clone();
    let lambda_vars_clone = lambda_vars.clone();
    let (inline_fundef, cd) = make_closure(
        class_num,
        pos,
        lambda_env,
        st,
        lambda_vars,
        fun_tparams,
        class_tparams.into_owned(),
        is_static,
        fd,
    );

    if is_long_lambda {
        st.global_state
            .explicit_use_set
            .insert(inline_fundef.name.1.clone());
    }

    let closure_class_name = &cd.name.1;
    if let Some(cd) = env.scope.get_class() {
        st.global_state
            .closure_enclosing_classes
            .insert(closure_class_name.clone(), make_info(cd));
    }
    // adjust captured $this information if lambda that was just processed was converted into
    // non-static one
    let captured_this = captured_this || !is_static;

    // Restore capture and defined set
    st.captured_vars = captured_vars;
    st.captured_this = captured_this;
    st.captured_generics = captured_generics;
    st.current_function_state = old_function_state;
    st.global_state
        .closure_namespaces
        .insert(closure_class_name.clone(), st.namespace.clone());
    st.record_function_state(
        env::get_unique_id_for_method(&cd, &cd.methods.first().unwrap()),
        function_state,
        rx_of_scope,
    );
    // back to using env instead of lambda_env here

    // Add lambda captured vars to current captured vars
    for var in lambda_vars_clone.iter() {
        add_var(env, st, var)
    }
    for x in current_generics.iter() {
        st.captured_generics.add(x.to_string());
    }

    st.hoisted_classes.push(cd);
    Ok(Expr_::mk_efun(inline_fundef, use_vars))
}

fn convert_meth_caller_to_func_ptr<'a>(
    env: &Env,
    st: &mut ClosureConvertVisitor,
    pos: &Pos,
    pc: &Pos,
    cls: &String,
    pf: &Pos,
    fname: &String,
) -> Expr_ {
    fn get_scope_fmode(scope: &ast_scope::Scope) -> Mode {
        scope
            .iter()
            .find_map(|item| match item {
                ast_scope::ScopeItem::Class(cd) => Some(cd.mode),
                ast_scope::ScopeItem::Function(fd) => Some(fd.mode),
                _ => None,
            })
            .unwrap_or(Mode::Mstrict)
    }
    // TODO: Move dummy variable to tasl.rs once it exists.
    let dummy_saved_env = ();
    let pos = || pos.clone();
    let expr_id = |name: String| Expr(pos(), Expr_::mk_id(Id(pos(), name)));
    let make_fn_param = |lid: &LocalId, is_variadic: bool| -> FunParam {
        FunParam {
            annotation: pos(),
            type_hint: TypeHint((), None),
            is_variadic,
            pos: pos(),
            name: local_id::get_name(lid).clone(),
            expr: None,
            callconv: None,
            user_attributes: vec![],
            visibility: None,
        }
    };
    let cname = match env.scope.get_class() {
        Some(cd) => &cd.name.1,
        None => "",
    };
    let mangle_name = string_utils::mangle_meth_caller(cls, fname);
    let fun_handle: Expr_ = Expr_::mk_call(
        CallType::Cnormal,
        expr_id("\\__systemlib\\meth_caller".into()),
        vec![],
        vec![Expr(pos(), Expr_::mk_string(mangle_name.clone()))],
        None,
    );
    if st.state.named_hoisted_functions.contains_key(&mangle_name) {
        return fun_handle;
    }
    // AST for: invariant(is_a($o, <cls>), 'object must be an instance of <cls>');
    let obj_var = Box::new(Lid(pos(), local_id::make_unscoped("$o".into())));
    let obj_lvar = Expr(pos(), Expr_::Lvar(obj_var.clone()));
    let assert_invariant = Expr(
        pos(),
        Expr_::mk_call(
            CallType::Cnormal,
            expr_id("\\HH\\invariant".into()),
            vec![],
            vec![
                Expr(
                    pos(),
                    Expr_::mk_call(
                        CallType::Cnormal,
                        expr_id("\\is_a".into()),
                        vec![],
                        vec![
                            obj_lvar.clone(),
                            Expr(pc.clone(), Expr_::String(cls.into())),
                        ],
                        None,
                    ),
                ),
                Expr(
                    pos(),
                    Expr_::String(format!("object must be an instance of ({})", cls)),
                ),
            ],
            None,
        ),
    );
    // AST for: return $o-><func>(...$args);
    let args_var = Box::new(Lid(pos(), local_id::make_unscoped("$args".into())));
    let meth_caller_handle = Expr(
        pos(),
        Expr_::mk_call(
            CallType::Cnormal,
            Expr(
                pos(),
                Expr_::ObjGet(Box::new((
                    obj_lvar.clone(),
                    Expr(pos(), Expr_::mk_id(Id(pf.clone(), fname.clone()))),
                    OgNullFlavor::OGNullthrows,
                ))),
            ),
            vec![],
            vec![],
            Some(Expr(pos(), Expr_::Lvar(args_var.clone()))),
        ),
    );

    let variadic_param = make_fn_param(&args_var.1, true);
    let fd = Fun_ {
        span: pos(),
        annotation: dummy_saved_env,
        mode: get_scope_fmode(&env.scope),
        ret: TypeHint((), None),
        name: Id(pos(), mangle_name.clone()),
        tparams: vec![],
        where_constraints: vec![],
        variadic: FunVariadicity::FVvariadicArg(variadic_param.clone()),
        params: vec![make_fn_param(&obj_var.1, false), variadic_param.clone()],
        body: FuncBody {
            ast: vec![
                Stmt(pos(), Stmt_::Expr(Box::new(assert_invariant))),
                Stmt(pos(), Stmt_::Return(Box::new(Some(meth_caller_handle)))),
            ],
            annotation: (), // FuncBodyAnn::Named,
        },
        fun_kind: FunKind::FSync,
        user_attributes: vec![UserAttribute {
            name: Id(pos(), "__MethCaller".into()),
            params: vec![Expr(pos(), Expr_::String(cname.into()))],
        }],
        file_attributes: vec![],
        external: false,
        namespace: ocamlrep::rc::RcOc::new(st.state.empty_namespace.clone()),
        doc_comment: None,
        static_: false,
    };
    st.state.named_hoisted_functions.insert(mangle_name, fd);
    return fun_handle;
}

fn convert_function_like_body<'a>(
    self_: &mut ClosureConvertVisitor<'a>,
    env: &mut Env<'a>,
    body: &mut FuncBody,
) -> Result<PerFunctionState> {
    // reset has_finally/goto_state values on the state
    let old_state = std::mem::replace(
        &mut self_.state.current_function_state,
        PerFunctionState::default(),
    );
    body.recurse(env, self_.object())?;
    // restore old has_finally/goto_state values
    let function_state = std::mem::replace(&mut self_.state.current_function_state, old_state);
    Ok(function_state)
}

fn add_reified_property(tparams: &ClassTparams, vars: &mut Vec<ClassVar>) {
    if !tparams.list.iter().all(|t| t.reified == ReifyKind::Erased) {
        let p = Pos::make_none();
        // varray/vec that holds a list of type structures
        // this prop will be initilized during runtime
        let hint = Hint(
            p.clone(),
            Box::new(Hint_::Happly(Id(p.clone(), "\\HH\\varray".into()), vec![])),
        );
        vars.insert(
            0,
            ClassVar {
                final_: false,
                xhp_attr: None,
                is_promoted_variadic: false,
                doc_comment: None,
                abstract_: false,
                visibility: Visibility::Private,
                type_: TypeHint((), Some(hint)),
                id: Id(p.clone(), string_utils::reified::PROP_NAME.into()),
                expr: None,
                user_attributes: vec![],
                is_static: false,
                span: p,
            },
        )
    }
}

fn count_classes(defs: &Program) -> usize {
    defs.iter().filter(|x| x.is_class()).count()
}

fn count_records(defs: &Program) -> usize {
    defs.iter().filter(|x| x.is_record_def()).count()
}

struct ClosureConvertVisitor<'a> {
    state: State,
    phantom_lifetime_a: std::marker::PhantomData<&'a ()>,
}

impl<'a> VisitorMut for ClosureConvertVisitor<'a> {
    type P = AstParams<Env<'a>, Error>;

    fn object(&mut self) -> &mut dyn VisitorMut<P = Self::P> {
        self
    }

    fn visit_method_(&mut self, env: &mut Env<'a>, md: &mut Method_) -> Result<()> {
        let cls = env.scope.get_class().ok_or(unrecoverable(
            "unexpected scope shape - method is not inside the class",
        ))?;
        // TODO(hrust): not great to have to clone env constantly
        let mut env = env.clone();
        env.with_method(md)?;
        self.state.reset_function_counts();
        let function_state = convert_function_like_body(self, &mut env, &mut md.body)?;
        self.state.record_function_state(
            env::get_unique_id_for_method(cls, &md),
            function_state,
            rx::Level::NonRx,
        );
        for mut param in &mut md.params {
            self.visit_fun_param(&mut env, &mut param)?;
        }
        Ok(())
    }

    fn visit_class_<'b>(&mut self, env: &mut Env<'a>, cd: &'b mut Class_) -> Result<()> {
        let mut env = env.clone();
        env.with_class(cd);
        self.state.reset_function_counts();
        cd.recurse(&mut env, self.object())?;
        Ok(add_reified_property(&cd.tparams, &mut cd.vars))
    }

    fn visit_def(&mut self, env: &mut Env<'a>, def: &mut Def) -> Result<()> {
        match def {
            // need to handle it ourselvses, because visit_fun_ is
            // called both for toplevel functions and lambdas
            Def::Fun(x) => {
                let mut env = env.clone();
                env.with_function(&x)?;
                self.state.reset_function_counts();
                let function_state = convert_function_like_body(self, &mut env, &mut x.body)?;
                self.state.record_function_state(
                    env::get_unique_id_for_function(&x),
                    function_state,
                    rx::Level::NonRx,
                );
                for mut param in &mut x.params {
                    self.visit_fun_param(&mut env, &mut param)?
                }
                for mut ua in &mut x.user_attributes {
                    self.visit_user_attribute(&mut env, &mut ua)?
                }
                Ok(())
            }
            _ => def.recurse(env, self.object()),
        }
    }

    fn visit_hint_(&mut self, env: &mut Env<'a>, hint: &mut Hint_) -> Result<()> {
        if let Hint_::Happly(id, _) = hint {
            add_generic(env, &mut self.state, id.name())
        };
        hint.recurse(env, self.object())
    }

    fn visit_stmt_(&mut self, env: &mut Env<'a>, stmt: &mut Stmt_) -> Result<()> {
        match stmt {
            Stmt_::Awaitall(x) => {
                env.check_if_in_async_context()?;
                x.recurse(env, self.object())
            }
            Stmt_::Do(x) => {
                let (b, e) = (&mut x.0, &mut x.1);
                env.with_in_using(false, |env| {
                    Ok(for stmt in b.iter_mut() {
                        self.visit_stmt(env, stmt)?;
                    })
                })?;
                self.visit_expr(env, e)
            }
            Stmt_::While(x) => {
                let (e, b) = (&mut x.0, &mut x.1);
                self.visit_expr(env, e)?;
                env.with_in_using(false, |env| {
                    Ok(for stmt in b.iter_mut() {
                        self.visit_stmt(env, stmt)?;
                    })
                })
            }
            Stmt_::Foreach(x) => {
                if x.1.is_await_as_v() || x.1.is_await_as_kv() {
                    env.check_if_in_async_context()?
                }
                x.recurse(env, self.object())
            }
            Stmt_::For(x) => {
                let (e1, e2, e3, b) = (&mut x.0, &mut x.1, &mut x.2, &mut x.3);
                self.visit_expr(env, e1)?;
                self.visit_expr(env, e2)?;
                env.with_in_using(false, |env| {
                    Ok(for stmt in b.iter_mut() {
                        self.visit_stmt(env, stmt)?;
                    })
                })?;
                self.visit_expr(env, e3)
            }
            Stmt_::Switch(x) => {
                let (e, cl) = (&mut x.0, &mut x.1);
                self.visit_expr(env, e)?;
                env.with_in_using(false, |env| {
                    Ok(for c in cl.iter_mut() {
                        self.visit_case(env, c)?;
                    })
                })
            }
            Stmt_::Try(x) => {
                let (b1, cl, b2) = (&mut x.0, &mut x.1, &mut x.2);
                for stmt in b1.iter_mut() {
                    self.visit_stmt(env, stmt)?;
                }
                for c in cl.iter_mut() {
                    self.visit_catch(env, c)?;
                }
                env.with_in_using(false, |env| {
                    Ok(for stmt in b2.iter_mut() {
                        self.visit_stmt(env, stmt)?;
                    })
                })?;
                Ok(self.state.current_function_state.has_finally |= !x.2.is_empty())
            }
            Stmt_::Using(x) => {
                if x.has_await {
                    env.check_if_in_async_context()?;
                }
                self.visit_expr(env, &mut x.expr)?;
                env.with_in_using(true, |env| {
                    Ok(for stmt in x.block.iter_mut() {
                        self.visit_stmt(env, stmt)?;
                    })
                })?;
                Ok(self.state.current_function_state.has_finally = true)
            }
            Stmt_::GotoLabel(x) => {
                let label = &x.1;
                // record known label in function
                self.state
                    .current_function_state
                    .labels
                    .insert(label.clone(), env.in_using);
                Ok(())
            }
            _ => stmt.recurse(env, self.object()),
        }
    }

    //TODO(hrust): do we need special handling for Awaitall?
    fn visit_expr(&mut self, env: &mut Env<'a>, Expr(pos, e): &mut Expr) -> Result<()> {
        let null = Expr_::mk_null();
        let e_owned = std::mem::replace(e, null);
        *e = match e_owned {
            Expr_::Efun(x) => convert_lambda(env, self, x.0, Some(x.1))?,
            Expr_::Lfun(x) => convert_lambda(env, self, x.0, None)?,
            Expr_::Lvar(id) => {
                add_var(env, &mut self.state, local_id::get_name(&id.1));
                Expr_::Lvar(id)
            }
            Expr_::Id(id) if id.name().starts_with("$") => {
                add_var(env, &mut self.state, id.name());
                add_generic(env, &mut self.state, id.name());
                Expr_::Id(id)
            }
            Expr_::Id(id) => {
                add_generic(env, &mut self.state, id.name());
                convert_id(env, *id)
            }
            Expr_::Call(mut x)
                if {
                    if let Expr_::Id(ref id) = (x.1).1 {
                        let name = strip_id(id);
                        ["hh\\meth_caller", "meth_caller"]
                            .iter()
                            .any(|n| n.eq_ignore_ascii_case(name))
                            && env
                                .options
                                .hhvm
                                .flags
                                .contains(HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS)
                    } else {
                        false
                    }
                } =>
            {
                let (pc, cls, pf, func) = {
                    if let [Expr(pc, cls), Expr(pf, fname)] = &mut *x.3 {
                        (pc, cls, pf, fname)
                    } else {
                        return Err(emit_fatal::raise_fatal_parse(
                            pos,
                            format!(
                                "meth_caller must have exactly two arguments. Received {}",
                                x.3.len()
                            ),
                        ));
                    }
                };
                match (&cls, func.as_string()) {
                    (Expr_::ClassConst(cc), Some(fname)) if string_utils::is_class(&(cc.1).1) => {
                        let mut cls_const = cls.as_class_const_mut();
                        let (cid, _) = match cls_const {
                            None => unreachable!(),
                            Some((ref mut cid, (_, cs))) => (cid, cs),
                        };
                        use hhbc_id::Id;
                        visit_class_id(env, self, cid)?;
                        match &cid.1 {
                            cid if cid
                                .as_ciexpr()
                                .and_then(|x| x.as_id())
                                .map(|id| {
                                    !(string_utils::is_self(id)
                                        || string_utils::is_parent(id)
                                        || string_utils::is_static(id))
                                })
                                .unwrap_or(false) =>
                            {
                                let id = cid.as_ciexpr().unwrap().as_id().unwrap();
                                let mangled_class_name = class::Type::from_ast_name(id.as_ref())
                                    .to_raw_string()
                                    .into();
                                convert_meth_caller_to_func_ptr(
                                    env,
                                    self,
                                    &*pos,
                                    pc,
                                    &mangled_class_name,
                                    pf,
                                    fname,
                                )
                            }
                            _ => {
                                return Err(emit_fatal::raise_fatal_parse(pc, "Invalid class"));
                            }
                        }
                    }
                    (Expr_::ClassConst(_), Some(_)) => {
                        return Err(emit_fatal::raise_fatal_parse(
                            pc,
                            "Class must be a Class or string type",
                        ));
                    }
                    (Expr_::String(cls_name), Some(fname)) => {
                        convert_meth_caller_to_func_ptr(env, self, &*pos, pc, &cls_name, pf, fname)
                    }

                    // For other cases, fallback to create __SystemLib\MethCallerHelper
                    _ => Expr_::Call(x),
                }
            }
            Expr_::Call(x)
                if (x.1)
                    .as_class_get()
                    .and_then(|(id, _)| id.as_ciexpr())
                    .and_then(|x| x.as_id())
                    .map(string_utils::is_parent)
                    .unwrap_or(false)
                    || (x.1)
                        .as_class_const()
                        .and_then(|(id, _)| id.as_ciexpr())
                        .and_then(|x| x.as_id())
                        .map(string_utils::is_parent)
                        .unwrap_or(false) =>
            {
                add_var(env, &mut self.state, "$this");
                let mut res = Expr_::Call(x);
                res.recurse(env, self.object())?;
                res
            }
            Expr_::Call(mut x)
                if x.1
                    .as_id()
                    .map(|id| id.1.eq_ignore_ascii_case("tuple"))
                    .unwrap_or_default() =>
            {
                // replace tuple with varray
                let call_args = mem::replace(&mut x.3, vec![]);
                let mut res = Expr_::mk_varray(None, call_args);
                res.recurse(env, self.object())?;
                res
            }
            Expr_::BracedExpr(mut x) => {
                x.recurse(env, self.object())?;
                match x.1 {
                    Expr_::Lvar(_) => x.1,
                    Expr_::String(_) => x.1,
                    _ => Expr_::BracedExpr(x),
                }
            }
            Expr_::As(x) if (x.1).is_hlike() => {
                let mut res = x.0;
                res.recurse(env, self.object())?;
                *pos = res.0;
                res.1
            }
            Expr_::As(x)
                if (x.1)
                    .as_happly()
                    .map(|(id, args)| {
                        (id.name() == fb::INCORRECT_TYPE || id.name() == fb::INCORRECT_TYPE_NO_NS)
                            && args.len() == 1
                    })
                    .unwrap_or_default() =>
            {
                let mut res = x.0;
                res.recurse(env, self.object())?;
                *pos = res.0;
                res.1
            }
            Expr_::ClassGet(mut x) => {
                if let ClassGetExpr::CGstring(id) = &x.1 {
                    // T43412864 claims that this does not need to be added into the closure and can be removed
                    // There are no relevant HHVM tests checking for it, but there are flib test failures when you try
                    // to remove it.
                    add_var(env, &mut self.state, &id.1);
                };
                x.recurse(env, self.object())?;
                Expr_::ClassGet(x)
            }
            Expr_::Await(mut x) => {
                env.check_if_in_async_context()?;
                x.recurse(env, self.object())?;
                Expr_::Await(x)
            }
            mut x => {
                x.recurse(env, self.object())?;
                x
            }
        };
        Ok(())
    }
}

fn hoist_toplevel_functions(defs: &mut Program) {
    // Reorder the functions so that they appear first.
    let (funs, nonfuns): (Vec<Def>, Vec<Def>) = defs.drain(..).partition(|x| x.is_fun());
    defs.extend(funs);
    defs.extend(nonfuns);
}

fn flatten_ns(defs: &mut Program) -> Program {
    defs.drain(..)
        .flat_map(|x| match x {
            Def::Namespace(mut x) => flatten_ns(&mut x.1),
            _ => vec![x],
        })
        .collect()
}

pub fn convert_toplevel_prog(e: &mut Emitter, defs: &mut Program) -> Result<Vec<HoistKind>> {
    let empty_namespace = namespace_env::Env::empty(vec![], false, false);
    if e.options()
        .hack_compiler_flags
        .contains(CompilerFlags::CONSTANT_FOLDING)
    {
        ast_constant_folder::fold_program(defs, e, &empty_namespace);
    }

    let mut env = Env::toplevel(
        count_classes(defs),
        count_records(defs),
        1,
        defs,
        e.options(),
    )?;
    *defs = flatten_ns(defs);

    let mut visitor = ClosureConvertVisitor {
        state: State::initial_state(empty_namespace),
        phantom_lifetime_a: std::marker::PhantomData,
    };

    let mut new_defs = vec![];
    let mut class_count = 0;
    let mut record_count = 0;
    let mut typedef_count = 0;
    let mut const_count = 0;

    for mut def in defs.drain(..) {
        visitor.visit_def(&mut env, &mut def)?;
        match def {
            Def::Class(mut x) => {
                x.emit_id = Some(EmitId::EmitId(class_count));
                class_count += 1;
                new_defs.push(Def::Class(x));
            }
            Def::RecordDef(mut x) => {
                x.emit_id = Some(EmitId::EmitId(record_count));
                record_count += 1;
                new_defs.push(Def::RecordDef(x));
            }
            Def::Typedef(mut x) => {
                x.emit_id = Some(EmitId::EmitId(typedef_count));
                typedef_count += 1;
                new_defs.push(Def::Typedef(x));
            }
            Def::Constant(mut x) => {
                x.emit_id = Some(EmitId::EmitId(const_count));
                const_count += 1;
                new_defs.push(Def::Constant(x));
            }
            Def::Namespace(x) => new_defs.extend_from_slice((*x).1.as_slice()),
            Def::SetNamespaceEnv(_) => panic!("TODO, can't test yet without porting namespaces.ml"),
            def => new_defs.push(def),
        }
    }

    visitor.state.record_function_state(
        env::get_unique_id_for_main(),
        visitor.state.current_function_state.clone(),
        rx::Level::NonRx,
    );
    hoist_toplevel_functions(&mut new_defs);
    let named_fun_defs = visitor
        .state
        .named_hoisted_functions
        .into_iter()
        .map(|(_, fd)| Def::mk_fun(fd));
    *defs = named_fun_defs.collect();
    defs.extend(new_defs.drain(..));
    let mut hoist_kinds = defs.iter().map(|_| HoistKind::TopLevel).collect::<Vec<_>>();
    for class in visitor.state.hoisted_classes.into_iter() {
        defs.push(Def::mk_class(class));
        hoist_kinds.push(HoistKind::Hoisted);
    }
    *e.emit_state_mut() = visitor.state.global_state;
    Ok(hoist_kinds)
}
