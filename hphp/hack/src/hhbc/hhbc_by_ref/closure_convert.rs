// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::mem;
use std::path::PathBuf;

use itertools::{Either, EitherOrBoth::*, Itertools};

use decl_provider::DeclProvider;
use hash::HashSet;
use hhbc_by_ref_ast_scope::{
    self as ast_scope, Lambda, LongLambda, Scope as AstScope, ScopeItem as AstScopeItem,
};
use hhbc_by_ref_decl_vars as decl_vars;
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_global_state::{ClosureEnclosingClassInfo, GlobalState};
use hhbc_by_ref_hhas_coeffects::HhasCoeffects;
use hhbc_by_ref_hhbc_id as hhbc_id;
use hhbc_by_ref_hhbc_id::class;
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::{unrecoverable, Error, Result};
use hhbc_by_ref_options::{CompilerFlags, HhvmFlags, LangFlags, Options};
use hhbc_by_ref_unique_id_builder::*;
use hhbc_by_ref_unique_list::UniqueList;
use naming_special_names_rust::{fb, pseudo_consts, special_idents, superglobals};
use ocamlrep::rc::RcOc;
use oxidized::{
    aast_defs,
    aast_visitor::{visit_mut, AstParams, NodeMut, VisitorMut},
    ast::*,
    ast_defs::*,
    file_info::Mode,
    local_id, namespace_env,
    relative_path::{Prefix, RelativePath},
    s_map::SMap,
};

type Scope<'a> = AstScope<'a>;
type ScopeItem<'a> = AstScopeItem<'a>;

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
    scope: Scope<'a>,
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
    /// For debugger eval
    for_debugger_eval: bool,
}

#[derive(Default, Clone)]
struct PerFunctionState {
    pub has_finally: bool,
}

impl<'a> Env<'a> {
    pub fn toplevel(
        class_count: usize,
        record_count: usize,
        function_count: usize,
        defs: &[Def],
        options: &'a Options,
        for_debugger_eval: bool,
    ) -> Result<Self> {
        let scope = Scope::toplevel();
        let all_vars = get_vars(&[], Either::Left(defs))?;

        Ok(Self {
            pos: Pos::make_none(),
            scope,
            variable_scopes: vec![Variables {
                all_vars,
                parameter_names: HashSet::default(),
            }],
            defined_class_count: class_count,
            defined_record_count: record_count,
            defined_function_count: function_count,
            in_using: false,
            options,
            for_debugger_eval,
        })
    }

    fn with_function_like_(
        &mut self,
        e: ScopeItem<'a>,
        _is_closure_body: bool,
        params: &[FunParam],
        pos: Pos,
        body: &[Stmt],
    ) -> Result<()> {
        self.pos = pos;
        self.scope.push_item(e);
        let all_vars = get_vars(params, Either::Right(body))?;
        Ok(self.variable_scopes.push(Variables {
            parameter_names: get_parameter_names(params),
            all_vars,
        }))
    }

    fn with_function_like(
        &mut self,
        e: ScopeItem<'a>,
        is_closure_body: bool,
        fd: &Fun_,
    ) -> Result<()> {
        self.with_function_like_(
            e,
            is_closure_body,
            &fd.params,
            fd.span.clone(),
            &fd.body.ast.as_slice(),
        )
    }

    fn with_function(&mut self, fd: &FunDef) -> Result<()> {
        self.with_function_like(
            ScopeItem::Function(ast_scope::Fun::new_rc(fd)),
            false,
            &fd.fun,
        )
    }

    fn with_method(&mut self, md: &Method_) -> Result<()> {
        self.with_function_like_(
            ScopeItem::Method(ast_scope::Method::new_rc(md)),
            false,
            &md.params,
            md.span.clone(),
            &md.body.ast,
        )
    }

    fn with_lambda(&mut self, fd: &Fun_) -> Result<()> {
        let is_async = fd.fun_kind.is_async();
        let coeffects = HhasCoeffects::from_ast(&fd.ctxs, &fd.params, vec![], vec![]);

        let lambda = Lambda {
            is_async,
            coeffects,
        };
        self.with_function_like(ScopeItem::Lambda(lambda), true, fd)
    }

    fn with_longlambda(&mut self, fd: &Fun_) -> Result<()> {
        let is_async = fd.fun_kind.is_async();
        let coeffects = HhasCoeffects::from_ast(&fd.ctxs, &fd.params, vec![], vec![]);

        let long_lambda = LongLambda {
            is_async,
            coeffects,
        };
        self.with_function_like(ScopeItem::LongLambda(long_lambda), true, fd)
    }

    fn with_class(&mut self, cd: &Class_) {
        self.scope = Scope::toplevel();
        self.scope
            .push_item(ScopeItem::Class(ast_scope::Class::new_rc(cd)))
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
        use ScopeItem as S;
        match head {
            None => Err(emit_fatal::raise_fatal_parse(
                &self.pos,
                "'await' can only be used inside a function",
            )),
            Some(S::Lambda(l)) => check_lambda(l.is_async),
            Some(S::LongLambda(l)) => check_lambda(l.is_async),
            Some(S::Class(_)) => Ok(()), /* Syntax error, wont get here */
            Some(S::Function(fd)) => check_valid_fun_kind(&fd.get_name().1, fd.get_fun_kind()),
            Some(S::Method(md)) => check_valid_fun_kind(&md.get_name().1, md.get_fun_kind()),
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
    // Closure classes
    closures: Vec<Class_>,
    /// Hoisted meth_caller functions
    named_hoisted_functions: SMap<FunDef>,
    // The current namespace environment
    namespace: RcOc<namespace_env::Env>,
    // Empty namespace as constructed by parser
    empty_namespace: RcOc<namespace_env::Env>,
    // information about current function
    current_function_state: PerFunctionState,
    // accumulated information about program
    global_state: GlobalState,
}

impl State {
    pub fn initial_state(empty_namespace: RcOc<namespace_env::Env>) -> Self {
        Self {
            namespace: RcOc::clone(&empty_namespace),
            empty_namespace,
            closure_cnt_per_fun: 0,
            captured_vars: UniqueList::new(),
            captured_this: false,
            captured_generics: UniqueList::new(),
            closures: vec![],
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
        coeffects_of_scope: HhasCoeffects,
        num_closures: u32,
    ) {
        if fun.has_finally {
            self.global_state.functions_with_finally.insert(key.clone());
        }

        if !coeffects_of_scope.get_static_coeffects().is_empty() {
            self.global_state
                .lambda_coeffects_of_scope
                .insert(key.clone(), coeffects_of_scope);
        }
        if num_closures > 0 {
            self.global_state.num_closures.insert(key, num_closures);
        }
    }

    // Clear the variables, upon entering a lambda
    pub fn enter_lambda(&mut self) {
        self.captured_vars = UniqueList::new();
        self.captured_this = false;
        self.captured_generics = UniqueList::new();
    }

    pub fn set_namespace(&mut self, namespace: RcOc<namespace_env::Env>) {
        self.namespace = namespace;
    }
}

fn total_class_count(env: &Env, st: &State) -> usize {
    st.closures.len() + env.defined_class_count
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
        && !(var == special_idents::DOLLAR_DOLLAR || superglobals::is_superglobal(var))
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

fn get_vars(params: &[FunParam], body: hhbc_by_ref_ast_body::AstBody) -> Result<HashSet<String>> {
    decl_vars::vars_from_ast(params, &body).map_err(unrecoverable)
}

fn get_parameter_names(params: &[FunParam]) -> HashSet<String> {
    params.iter().map(|p| p.name.to_string()).collect()
}

fn strip_id(id: &Id) -> &str {
    string_utils::strip_global_ns(&id.1)
}

fn make_class_name(cd: &ast_scope::Class) -> String {
    string_utils::mangle_xhp_id(strip_id(cd.get_name()).to_string())
}

fn make_scope_name(ns: &RcOc<namespace_env::Env>, scope: &ast_scope::Scope) -> String {
    let mut parts: Vec<String> = vec![];
    for sub_scope in scope.iter_subscopes() {
        match sub_scope.last() {
            None => {
                return match &ns.name {
                    None => String::new(),
                    Some(n) => format!("{}\\", n),
                };
            }
            Some(ast_scope::ScopeItem::Class(x)) => {
                parts.push(make_class_name(&x));
                break;
            }
            Some(ast_scope::ScopeItem::Function(x)) => {
                let fname = strip_id(x.get_name());
                parts.push(
                    Scope::get_subscope_class(sub_scope)
                        .map(|cd| make_class_name(cd) + "::")
                        .unwrap_or_default()
                        + fname,
                );
                break;
            }
            Some(ast_scope::ScopeItem::Method(x)) => {
                parts.push(strip_id(x.get_name()).to_string());
                if !parts.last().map_or(false, |x| x.ends_with("::")) {
                    parts.push("::".into())
                };
            }
            _ => {}
        }
    }
    parts.reverse();
    parts.join("")
}

fn make_closure_name(env: &Env, st: &State) -> String {
    let per_fun_idx = st.closure_cnt_per_fun;
    string_utils::closures::mangle_closure(&make_scope_name(&st.namespace, &env.scope), per_fun_idx)
}

fn make_closure(
    class_num: usize,
    p: Pos,
    env: &Env,
    st: &State,
    lambda_vars: Vec<String>,
    fun_tparams: Vec<Tparam>,
    class_tparams: Vec<Tparam>,
    is_static: bool,
    mode: Mode,
    mut fd: Fun_,
) -> (Fun_, Class_) {
    let md = Method_ {
        span: fd.span.clone(),
        annotation: fd.annotation,
        final_: false,
        abstract_: false,
        static_: is_static,
        readonly_this: false, // readonly on closure_convert
        visibility: Visibility::Public,
        name: Id(fd.name.0.clone(), "__invoke".into()),
        tparams: fun_tparams,
        where_constraints: fd.where_constraints.clone(),
        variadic: fd.variadic.clone(),
        params: fd.params.clone(),
        ctxs: fd.ctxs.clone(),
        unsafe_ctxs: None, // TODO(T70095684)
        body: fd.body.clone(),
        fun_kind: fd.fun_kind,
        user_attributes: fd.user_attributes.clone(),
        readonly_ret: None, // readonly_ret on closure_convert
        ret: fd.ret.clone(),
        external: false,
        doc_comment: fd.doc_comment.clone(),
    };

    let make_class_var = |name: &str| ClassVar {
        final_: false,
        xhp_attr: None,
        abstract_: false,
        readonly: false, // readonly on closure_convert
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
        mode,
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
        xhp_attr_uses: vec![],
        xhp_category: None,
        reqs: vec![],
        implements: vec![],
        support_dynamic_type: false,
        where_constraints: vec![],
        consts: vec![],
        typeconsts: vec![],
        vars: cvl.collect(),
        methods: vec![md],
        attributes: vec![],
        xhp_children: vec![],
        xhp_attrs: vec![],
        namespace: RcOc::clone(&st.empty_namespace),
        enum_: None,
        doc_comment: None,
        emit_id: Some(EmitId::Anonymous),
    };

    // TODO(hrust): can we reconstruct fd here from the scratch?
    fd.name = Id(p.clone(), class_num.to_string());
    (fd, cd)
}

// Translate special identifiers __CLASS__, __METHOD__ and __FUNCTION__ into
// literal strings. It's necessary to do this before closure conversion
// because the enclosing class will be changed.
fn convert_id(env: &Env, Id(p, s): Id) -> Expr_ {
    let ret = |newstr| Expr_::mk_string(newstr);
    let name = |c: &ast_scope::Class| {
        Expr_::mk_string(string_utils::mangle_xhp_id(strip_id(c.get_name()).to_string()).into())
    };

    match s {
        _ if s.eq_ignore_ascii_case(pseudo_consts::G__TRAIT__) => match env.scope.get_class() {
            Some(c) if c.get_kind() == ClassKind::Ctrait => name(c),
            _ => ret("".into()),
        },
        _ if s.eq_ignore_ascii_case(pseudo_consts::G__CLASS__) => match env.scope.get_class() {
            Some(c) if c.get_kind() != ClassKind::Ctrait => name(c),
            Some(_) => Expr_::mk_id(Id(p, s)),
            None => ret("".into()),
        },
        _ if s.eq_ignore_ascii_case(pseudo_consts::G__METHOD__) => {
            let (prefix, is_trait) = match env.scope.get_class() {
                None => ("".into(), false),
                Some(cd) => (
                    string_utils::mangle_xhp_id(strip_id(cd.get_name()).to_string()) + "::",
                    cd.get_kind() == ClassKind::Ctrait,
                ),
            };
            // for lambdas nested in trait methods HHVM replaces __METHOD__
            // with enclosing method name - do the same and bubble up from lambdas *
            let scope = env.scope.iter().find(|x| !(is_trait && x.is_in_lambda()));

            match scope {
                Some(ScopeItem::Function(fd)) => ret((prefix + strip_id(fd.get_name())).into()),
                Some(ScopeItem::Method(md)) => ret((prefix + strip_id(md.get_name())).into()),
                Some(ScopeItem::Lambda(_)) | Some(ScopeItem::LongLambda(_)) => {
                    ret((prefix + "{closure}").into())
                }
                // PHP weirdness: __METHOD__ inside a class outside a method returns class name
                Some(ScopeItem::Class(cd)) => ret(strip_id(cd.get_name()).into()),
                _ => ret("".into()),
            }
        }
        _ if s.eq_ignore_ascii_case(pseudo_consts::G__FUNCTION__) => match env.scope.items.last() {
            Some(ScopeItem::Function(fd)) => ret(strip_id(fd.get_name()).into()),
            Some(ScopeItem::Method(md)) => ret(strip_id(md.get_name()).into()),
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

fn make_info(c: &ast_scope::Class) -> ClosureEnclosingClassInfo {
    ClosureEnclosingClassInfo {
        kind: c.get_kind(),
        name: c.get_name().1.clone(),
        parent_class_name: match c.get_extends() {
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
    let coeffects_of_scope = env.scope.coeffects_of_scope();
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
        lambda_env.with_longlambda(&fd)?
    } else {
        lambda_env.with_lambda(&fd)?
    };
    let function_state = convert_function_like_body(self_, lambda_env, &mut fd.body)?;
    for param in &mut fd.params {
        self_.visit_type_hint(lambda_env, &mut param.type_hint)?;
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
    let use_vars_opt: Option<Vec<Lid>> = use_vars_opt.map(|use_vars| {
        use_vars
            .into_iter()
            .rev()
            .unique_by(|lid| lid.name().to_string())
            .filter(|x| !fd.params.iter().any(|y| x.name() == &y.name))
            .collect::<Vec<_>>()
            .into_iter()
            .rev()
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
                    .cloned()
                    .chain(current_generics.iter().map(|x| fresh_lid(x.to_string())))
                    .collect(),
            )
        }
    };

    let fun_tparams = lambda_env.scope.get_fun_tparams().to_vec();
    let class_tparams = lambda_env.scope.get_class_tparams().to_vec();
    let class_num = total_class_count(lambda_env, st);

    let is_static = if is_long_lambda {
        // long lambdas are never static
        false
    } else {
        // short lambdas can be made static if they don't capture this in
        // any form (including any nested lambdas)
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
        class_tparams,
        is_static,
        get_scope_fmode(&env.scope),
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
        get_unique_id_for_method(&cd.name.1, &cd.methods.first().unwrap().name.1),
        function_state,
        coeffects_of_scope,
        0,
    );
    // back to using env instead of lambda_env here

    // Add lambda captured vars to current captured vars
    for var in lambda_vars_clone.iter() {
        add_var(env, st, var)
    }
    for x in current_generics.iter() {
        st.captured_generics.add(x.to_string());
    }

    st.closures.push(cd);
    Ok(Expr_::mk_efun(inline_fundef, use_vars))
}

fn make_fn_param(pos: Pos, lid: &LocalId, is_variadic: bool, is_inout: bool) -> FunParam {
    FunParam {
        annotation: pos.clone(),
        type_hint: TypeHint((), None),
        is_variadic,
        pos,
        name: local_id::get_name(lid).clone(),
        expr: None,
        callconv: if is_inout {
            Some(ParamKind::Pinout)
        } else {
            None
        },
        readonly: None, // TODO
        user_attributes: vec![],
        visibility: None,
    }
}

fn get_scope_fmode(scope: &Scope) -> Mode {
    scope
        .iter()
        .find_map(|item| match item {
            ScopeItem::Class(cd) => Some(cd.get_mode()),
            ScopeItem::Function(fd) => Some(fd.get_mode()),
            _ => None,
        })
        .unwrap_or(Mode::Mstrict)
}

fn convert_meth_caller_to_func_ptr(
    env: &Env,
    st: &mut ClosureConvertVisitor,
    pos: &Pos,
    pc: &Pos,
    cls: &str,
    pf: &Pos,
    fname: &str,
) -> Expr_ {
    fn get_scope_fmode(scope: &Scope) -> Mode {
        scope
            .iter()
            .find_map(|item| match item {
                ScopeItem::Class(cd) => Some(cd.get_mode()),
                ScopeItem::Function(fd) => Some(fd.get_mode()),
                _ => None,
            })
            .unwrap_or(Mode::Mstrict)
    }
    // TODO: Move dummy variable to tasl.rs once it exists.
    let dummy_saved_env = ();
    let pos = || pos.clone();
    let expr_id = |name: String| Expr(pos(), Expr_::mk_id(Id(pos(), name)));
    let cname = match env.scope.get_class() {
        Some(cd) => &cd.get_name().1,
        None => "",
    };
    let mangle_name = string_utils::mangle_meth_caller(cls, fname);
    let fun_handle: Expr_ = Expr_::mk_call(
        expr_id("\\__systemlib\\meth_caller".into()),
        vec![],
        vec![Expr(pos(), Expr_::mk_string(mangle_name.clone().into()))],
        None,
    );
    if st.state.named_hoisted_functions.contains_key(&mangle_name) {
        return fun_handle;
    }
    // AST for: invariant(is_a($o, <cls>), 'object must be an instance of <cls>');
    let obj_var = Box::new(Lid(pos(), local_id::make_unscoped("$o")));
    let obj_lvar = Expr(pos(), Expr_::Lvar(obj_var.clone()));
    let assert_invariant = Expr(
        pos(),
        Expr_::mk_call(
            expr_id("\\HH\\invariant".into()),
            vec![],
            vec![
                Expr(
                    pos(),
                    Expr_::mk_call(
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
                    Expr_::String(format!("object must be an instance of ({})", cls).into()),
                ),
            ],
            None,
        ),
    );
    // AST for: return $o-><func>(...$args);
    let args_var = Box::new(Lid(pos(), local_id::make_unscoped("$args")));
    let variadic_param = make_fn_param(pos(), &args_var.1, true, false);
    let meth_caller_handle = Expr(
        pos(),
        Expr_::mk_call(
            Expr(
                pos(),
                Expr_::ObjGet(Box::new((
                    obj_lvar,
                    Expr(pos(), Expr_::mk_id(Id(pf.clone(), fname.to_owned()))),
                    OgNullFlavor::OGNullthrows,
                    false,
                ))),
            ),
            vec![],
            vec![],
            Some(Expr(pos(), Expr_::Lvar(args_var))),
        ),
    );

    let f = Fun_ {
        span: pos(),
        annotation: dummy_saved_env,
        readonly_this: None, // TODO(readonly): readonly_this in closure_convert
        readonly_ret: None,
        ret: TypeHint((), None),
        name: Id(pos(), mangle_name.clone()),
        tparams: vec![],
        where_constraints: vec![],
        variadic: FunVariadicity::FVvariadicArg(variadic_param.clone()),
        params: vec![
            make_fn_param(pos(), &obj_var.1, false, false),
            variadic_param,
        ],
        ctxs: Some(Contexts(pos(), vec![])),
        unsafe_ctxs: None,
        body: FuncBody {
            ast: vec![
                Stmt(pos(), Stmt_::Expr(Box::new(assert_invariant))),
                Stmt(pos(), Stmt_::Return(Box::new(Some(meth_caller_handle)))),
            ],
            annotation: (),
        },
        fun_kind: FunKind::FSync,
        user_attributes: vec![UserAttribute {
            name: Id(pos(), "__MethCaller".into()),
            params: vec![Expr(pos(), Expr_::String(cname.into()))],
        }],
        external: false,
        doc_comment: None,
    };
    let fd = FunDef {
        file_attributes: vec![],
        namespace: RcOc::clone(&st.state.empty_namespace),
        mode: get_scope_fmode(&env.scope),
        fun: f,
    };
    st.state.named_hoisted_functions.insert(mangle_name, fd);
    fun_handle
}

fn make_dyn_meth_caller_lambda(pos: &Pos, cexpr: &Expr, fexpr: &Expr, force: bool) -> Expr_ {
    // TODO: Move dummy variable to tasl.rs once it exists.
    let dummy_saved_env = ();
    let pos = || pos.clone();
    let obj_var = Box::new(Lid(pos(), local_id::make_unscoped("$o")));
    let meth_var = Box::new(Lid(pos(), local_id::make_unscoped("$m")));
    let obj_lvar = Expr(pos(), Expr_::Lvar(obj_var.clone()));
    let meth_lvar = Expr(pos(), Expr_::Lvar(meth_var.clone()));
    // AST for: return $o-><func>(...$args);
    let args_var = Box::new(Lid(pos(), local_id::make_unscoped("$args")));
    let variadic_param = make_fn_param(pos(), &args_var.1, true, false);
    let invoke_method = Expr(
        pos(),
        Expr_::mk_call(
            Expr(
                pos(),
                Expr_::ObjGet(Box::new((
                    obj_lvar,
                    meth_lvar,
                    OgNullFlavor::OGNullthrows,
                    false,
                ))),
            ),
            vec![],
            vec![],
            Some(Expr(pos(), Expr_::Lvar(args_var))),
        ),
    );
    let attrs = if force {
        vec![UserAttribute {
            name: Id(pos(), "__DynamicMethCallerForce".into()),
            params: vec![],
        }]
    } else {
        vec![]
    };

    let fd = Fun_ {
        span: pos(),
        annotation: dummy_saved_env,
        readonly_this: None, // TODO: readonly_this in closure_convert
        readonly_ret: None,  // TODO: readonly_ret in closure convert
        ret: TypeHint((), None),
        name: Id(pos(), ";anonymous".to_string()),
        tparams: vec![],
        where_constraints: vec![],
        variadic: FunVariadicity::FVvariadicArg(variadic_param.clone()),
        params: vec![
            make_fn_param(pos(), &obj_var.1, false, false),
            make_fn_param(pos(), &meth_var.1, false, false),
            variadic_param,
        ],
        ctxs: Some(Contexts(pos(), vec![])),
        unsafe_ctxs: None,
        body: FuncBody {
            ast: vec![Stmt(pos(), Stmt_::Return(Box::new(Some(invoke_method))))],
            annotation: (),
        },
        fun_kind: FunKind::FSync,
        user_attributes: attrs,
        external: false,
        doc_comment: None,
    };
    let expr_id = |name: String| Expr(pos(), Expr_::mk_id(Id(pos(), name)));
    let force_val = if force { Expr_::True } else { Expr_::False };
    let fun_handle: Expr_ = Expr_::mk_call(
        expr_id("\\__systemlib\\dynamic_meth_caller".into()),
        vec![],
        vec![
            cexpr.clone(),
            fexpr.clone(),
            Expr(pos(), Expr_::mk_efun(fd, vec![])),
            Expr(pos(), force_val),
        ],
        None,
    );
    fun_handle
}

fn convert_function_like_body<'a>(
    self_: &mut ClosureConvertVisitor<'a>,
    env: &mut Env<'a>,
    body: &mut FuncBody,
) -> Result<PerFunctionState> {
    // reset has_finally values on the state
    let old_state = std::mem::take(&mut self_.state.current_function_state);
    body.recurse(env, self_.object())?;
    // restore old has_finally values
    let function_state = std::mem::replace(&mut self_.state.current_function_state, old_state);
    Ok(function_state)
}

fn add_reified_property(tparams: &[Tparam], vars: &mut Vec<ClassVar>) {
    if !tparams.iter().all(|t| t.reified == ReifyKind::Erased) {
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
                readonly: false,
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

fn count_classes(defs: &[Def]) -> usize {
    defs.iter().filter(|x| x.is_class()).count()
}

fn count_records(defs: &[Def]) -> usize {
    defs.iter().filter(|x| x.is_record_def()).count()
}

struct ClosureConvertVisitor<'a> {
    state: State,
    phantom_lifetime_a: std::marker::PhantomData<&'a ()>,
}

impl<'ast, 'a> VisitorMut<'ast> for ClosureConvertVisitor<'a> {
    type P = AstParams<Env<'a>, Error>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_method_(&mut self, env: &mut Env<'a>, md: &mut Method_) -> Result<()> {
        let cls = env.scope.get_class().ok_or_else(|| {
            unrecoverable("unexpected scope shape - method is not inside the class")
        })?;
        // TODO(hrust): not great to have to clone env constantly
        let mut env = env.clone();
        env.with_method(md)?;
        self.state.reset_function_counts();
        let function_state = convert_function_like_body(self, &mut env, &mut md.body)?;
        self.state.record_function_state(
            get_unique_id_for_method(cls.get_name_str(), &md.name.1),
            function_state,
            HhasCoeffects::default(),
            self.state.closure_cnt_per_fun,
        );
        visit_mut(self, &mut env, &mut md.params)
    }

    fn visit_class_<'b>(&mut self, env: &mut Env<'a>, cd: &'b mut Class_) -> Result<()> {
        let mut env = env.clone();
        env.with_class(cd);
        self.state.reset_function_counts();
        visit_mut(self, &mut env, &mut cd.methods)?;
        visit_mut(self, &mut env, &mut cd.consts)?;
        visit_mut(self, &mut env, &mut cd.vars)?;
        visit_mut(self, &mut env, &mut cd.xhp_attrs)?;
        visit_mut(self, &mut env, &mut cd.user_attributes)?;
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
                let function_state = convert_function_like_body(self, &mut env, &mut x.fun.body)?;
                self.state.record_function_state(
                    get_unique_id_for_function(&x.fun.name.1),
                    function_state,
                    HhasCoeffects::default(),
                    self.state.closure_cnt_per_fun,
                );
                visit_mut(self, &mut env, &mut x.fun.params)?;
                visit_mut(self, &mut env, &mut x.fun.user_attributes)
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
                env.with_in_using(false, |env| visit_mut(self, env, b))?;
                self.visit_expr(env, e)
            }
            Stmt_::While(x) => {
                let (e, b) = (&mut x.0, &mut x.1);
                self.visit_expr(env, e)?;
                env.with_in_using(false, |env| visit_mut(self, env, b))
            }
            Stmt_::Foreach(x) => {
                if x.1.is_await_as_v() || x.1.is_await_as_kv() {
                    env.check_if_in_async_context()?
                }
                x.recurse(env, self.object())
            }
            Stmt_::For(x) => {
                let (e1, e2, e3, b) = (&mut x.0, &mut x.1, &mut x.2, &mut x.3);

                for e in e1 {
                    self.visit_expr(env, e)?;
                }
                if let Some(e) = e2 {
                    self.visit_expr(env, e)?;
                }
                env.with_in_using(false, |env| visit_mut(self, env, b))?;
                for e in e3 {
                    self.visit_expr(env, e)?;
                }
                Ok(())
            }
            Stmt_::Switch(x) => {
                let (e, cl) = (&mut x.0, &mut x.1);
                self.visit_expr(env, e)?;
                env.with_in_using(false, |env| visit_mut(self, env, cl))
            }
            Stmt_::Try(x) => {
                let (b1, cl, b2) = (&mut x.0, &mut x.1, &mut x.2);
                visit_mut(self, env, b1)?;
                visit_mut(self, env, cl)?;
                visit_mut(self, env, b2)?;
                Ok(self.state.current_function_state.has_finally |= !x.2.is_empty())
            }
            Stmt_::Using(x) => {
                if x.has_await {
                    env.check_if_in_async_context()?;
                }
                for e in &mut x.exprs.1 {
                    self.visit_expr(env, e)?;
                }
                env.with_in_using(true, |env| visit_mut(self, env, &mut x.block))?;
                Ok(self.state.current_function_state.has_finally = true)
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
            Expr_::Lvar(id_orig) => {
                let id = if env.for_debugger_eval
                    && local_id::get_name(&id_orig.1) == special_idents::THIS
                    && env.scope.is_in_debugger_eval_fun()
                {
                    Box::new(Lid(id_orig.0, (0, "$__debugger$this".to_string())))
                } else {
                    id_orig
                };
                add_var(env, &mut self.state, local_id::get_name(&id.1));
                Expr_::Lvar(id)
            }
            Expr_::Id(id) if id.name().starts_with('$') => {
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
                    if let Expr_::Id(ref id) = (x.0).1 {
                        strip_id(id).eq_ignore_ascii_case("hh\\dynamic_meth_caller")
                            || strip_id(id).eq_ignore_ascii_case("hh\\dynamic_meth_caller_force")
                    } else {
                        false
                    }
                } =>
            {
                let force = if let Expr_::Id(ref id) = (x.0).1 {
                    strip_id(id).eq_ignore_ascii_case("hh\\dynamic_meth_caller_force")
                } else {
                    false
                };
                if let [cexpr, fexpr] = &mut *x.2 {
                    let mut res = make_dyn_meth_caller_lambda(&*pos, &cexpr, &fexpr, force);
                    res.recurse(env, self.object())?;
                    res
                } else {
                    let mut res = Expr_::Call(x);
                    res.recurse(env, self.object())?;
                    res
                }
            }
            Expr_::Call(mut x)
                if {
                    if let Expr_::Id(ref id) = (x.0).1 {
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
                let strict = env
                    .options
                    .hhvm
                    .hack_lang
                    .flags
                    .contains(LangFlags::DISALLOW_DYNAMIC_METH_CALLER_ARGS);
                if let [Expr(pc, cls), Expr(pf, func)] = &mut *x.2 {
                    match (&cls, func.as_string()) {
                        (Expr_::ClassConst(cc), Some(fname))
                            if string_utils::is_class(&(cc.1).1) =>
                        {
                            let mut cls_const = cls.as_class_const_mut();
                            let (cid, _) = match cls_const {
                                None => unreachable!(),
                                Some((ref mut cid, (_, cs))) => (cid, cs),
                            };
                            use hhbc_id::Id;
                            visit_class_id(env, self, cid)?;
                            match &cid.1 {
                                cid if cid.as_ciexpr().and_then(|x| x.as_id()).map_or(
                                    false,
                                    |id| {
                                        !(string_utils::is_self(id)
                                            || string_utils::is_parent(id)
                                            || string_utils::is_static(id))
                                    },
                                ) =>
                                {
                                    let alloc = bumpalo::Bump::new();
                                    let id = cid.as_ciexpr().unwrap().as_id().unwrap();
                                    let mangled_class_name =
                                        class::Type::from_ast_name(&alloc, id.as_ref());
                                    let mangled_class_name = mangled_class_name.to_raw_string();
                                    convert_meth_caller_to_func_ptr(
                                        env,
                                        self,
                                        &*pos,
                                        pc,
                                        mangled_class_name,
                                        pf,
                                        // FIXME: This is not safe--string literals are binary strings.
                                        // There's no guarantee that they're valid UTF-8.
                                        unsafe { std::str::from_utf8_unchecked(fname.as_slice()) },
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
                        (Expr_::ClassConst(cc), None)
                            if string_utils::is_class(&(cc.1).1) && strict =>
                        {
                            return Err(emit_fatal::raise_fatal_parse(
                                pf,
                                "Method name must be a literal string",
                            ));
                        }
                        (Expr_::ClassConst(_), _) if strict => {
                            return Err(emit_fatal::raise_fatal_parse(
                                pc,
                                "Class must be a Class or string type",
                            ));
                        }
                        (Expr_::String(_), None) if strict => {
                            return Err(emit_fatal::raise_fatal_parse(
                                pf,
                                "Method name must be a literal string",
                            ));
                        }
                        (Expr_::String(cls_name), Some(fname)) => convert_meth_caller_to_func_ptr(
                            env,
                            self,
                            &*pos,
                            pc,
                            // FIXME: This is not safe--string literals are binary strings.
                            // There's no guarantee that they're valid UTF-8.
                            unsafe { std::str::from_utf8_unchecked(cls_name.as_slice()) },
                            pf,
                            // FIXME: This is not safe--string literals are binary strings.
                            // There's no guarantee that they're valid UTF-8.
                            unsafe { std::str::from_utf8_unchecked(fname.as_slice()) },
                        ),

                        (_, _) if strict => {
                            return Err(emit_fatal::raise_fatal_parse(
                                pf,
                                "Method name must be a literal string",
                            ));
                        }
                        // For other cases, fallback to create __SystemLib\MethCallerHelper
                        _ => Expr_::Call(x),
                    }
                } else {
                    let mut res = Expr_::Call(x);
                    res.recurse(env, self.object())?;
                    res
                }
            }
            Expr_::Call(mut x)
                if {
                    if let Expr_::Id(ref id) = (x.0).1 {
                        let name = strip_id(id);
                        ["hh\\meth_caller", "meth_caller"]
                            .iter()
                            .any(|n| n.eq_ignore_ascii_case(name))
                            && env
                                .options
                                .hhvm
                                .hack_lang
                                .flags
                                .contains(LangFlags::DISALLOW_DYNAMIC_METH_CALLER_ARGS)
                    } else {
                        false
                    }
                } =>
            {
                if let [Expr(pc, cls), Expr(pf, func)] = &mut *x.2 {
                    match (&cls, func.as_string()) {
                        (Expr_::ClassConst(cc), Some(_)) if string_utils::is_class(&(cc.1).1) => {
                            let mut cls_const = cls.as_class_const_mut();
                            let cid = match cls_const {
                                None => unreachable!(),
                                Some((ref mut cid, (_, _))) => cid,
                            };
                            #[allow(clippy::blocks_in_if_conditions)]
                            if cid.as_ciexpr().and_then(|x| x.as_id()).map_or(false, |id| {
                                !(string_utils::is_self(id)
                                    || string_utils::is_parent(id)
                                    || string_utils::is_static(id))
                            }) {
                                let mut res = Expr_::Call(x);
                                res.recurse(env, self.object())?;
                                res
                            } else {
                                return Err(emit_fatal::raise_fatal_parse(pc, "Invalid class"));
                            }
                        }
                        (Expr_::String(_), Some(_)) => {
                            let mut res = Expr_::Call(x);
                            res.recurse(env, self.object())?;
                            res
                        }
                        (Expr_::ClassConst(cc), None) if string_utils::is_class(&(cc.1).1) => {
                            return Err(emit_fatal::raise_fatal_parse(
                                pf,
                                "Method name must be a literal string",
                            ));
                        }
                        (Expr_::String(_), None) => {
                            return Err(emit_fatal::raise_fatal_parse(
                                pf,
                                "Method name must be a literal string",
                            ));
                        }
                        (_, _) => {
                            return Err(emit_fatal::raise_fatal_parse(
                                pc,
                                "Class must be a Class or string type",
                            ));
                        }
                    }
                } else {
                    let mut res = Expr_::Call(x);
                    res.recurse(env, self.object())?;
                    res
                }
            }
            Expr_::Call(x)
                if (x.0)
                    .as_class_get()
                    .and_then(|(id, _, _)| id.as_ciexpr())
                    .and_then(|x| x.as_id())
                    .map_or(false, string_utils::is_parent)
                    || (x.0)
                        .as_class_const()
                        .and_then(|(id, _)| id.as_ciexpr())
                        .and_then(|x| x.as_id())
                        .map_or(false, string_utils::is_parent) =>
            {
                add_var(env, &mut self.state, "$this");
                let mut res = Expr_::Call(x);
                res.recurse(env, self.object())?;
                res
            }
            Expr_::Call(mut x)
                if x.0
                    .as_id()
                    .map(|id| id.1.eq_ignore_ascii_case("tuple"))
                    .unwrap_or_default() =>
            {
                // replace tuple with varray
                let call_args = mem::replace(&mut x.2, vec![]);
                let mut res = Expr_::mk_varray(None, call_args);
                res.recurse(env, self.object())?;
                res
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
            Expr_::ReadonlyExpr(mut x) => {
                x.recurse(env, self.object())?;
                Expr_::ReadonlyExpr(x)
            }
            Expr_::ExpressionTree(mut x) => {
                x.runtime_expr.recurse(env, self.object())?;
                Expr_::ExpressionTree(x)
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

fn extract_debugger_main(
    empty_namespace: &RcOc<namespace_env::Env>,
    all_defs: &mut Program,
) -> std::result::Result<(), String> {
    let (stmts, mut defs): (Vec<Def>, Vec<Def>) = all_defs.drain(..).partition(|x| x.is_stmt());
    let mut vars = decl_vars::vars_from_ast(&[], &Either::Left(&stmts))?
        .into_iter()
        .collect::<Vec<_>>();
    // TODO(hrust) sort is only required when comparing Rust/Ocaml, remove sort after emitter shipped
    vars.sort();
    let mut stmts = stmts
        .into_iter()
        .filter_map(|x| x.as_stmt_into())
        .collect::<Vec<_>>();
    let stmts =
        if defs.is_empty() && stmts.len() == 2 && stmts[0].1.is_markup() && stmts[1].1.is_expr() {
            let Stmt(p, s) = stmts.pop().unwrap();
            let e = s.as_expr_into().unwrap();
            let m = stmts.pop().unwrap();
            vec![m, Stmt::new(p, Stmt_::mk_return(Some(e)))]
        } else {
            stmts
        };
    let p = || Pos::make_none();
    let id = |n: &str| Expr(p(), Expr_::mk_id(Id(p(), n.into())));
    let lv = |n: &String| Expr(p(), Expr_::mk_lvar(Lid(p(), local_id::make_unscoped(n))));
    let mut unsets: Vec<_> = vars
        .iter()
        .map(|name| {
            let unset = Stmt(
                p(),
                Stmt_::mk_expr(Expr(
                    p(),
                    Expr_::mk_call(id("unset"), vec![], vec![lv(&name)], None),
                )),
            );
            Stmt(
                p(),
                Stmt_::mk_if(
                    Expr(
                        p(),
                        Expr_::mk_is(
                            lv(&name),
                            Hint::new(
                                p(),
                                Hint_::mk_happly(Id(p(), "__uninitSentinel".into()), vec![]),
                            ),
                        ),
                    ),
                    vec![unset],
                    vec![],
                ),
            )
        })
        .collect();
    let sets: Vec<_> = vars
        .iter()
        .map(|name| {
            let checkfunc = id("\\__systemlib\\__debugger_is_uninit");
            let isuninit = Expr(p(), Expr_::mk_call(checkfunc, vec![], vec![lv(name)], None));
            let obj = Expr(
                p(),
                Expr_::mk_new(
                    ClassId(p(), ClassId_::CI(Id(p(), "__uninitSentinel".into()))),
                    vec![],
                    vec![],
                    None,
                    p(),
                ),
            );
            let set = Stmt(
                p(),
                Stmt_::mk_expr(Expr(p(), Expr_::mk_binop(Bop::mk_eq(None), lv(name), obj))),
            );
            Stmt(p(), Stmt_::mk_if(isuninit, vec![set], vec![]))
        })
        .collect();
    vars.push("$__debugger$this".into());
    vars.push("$__debugger_exn$output".into());
    let params: Vec<_> = vars
        .iter()
        .map(|var| make_fn_param(p(), &local_id::make_unscoped(var), false, true))
        .collect();
    let exnvar = Lid(p(), local_id::make_unscoped("$__debugger_exn$output"));
    let catch = Stmt(
        p(),
        Stmt_::mk_try(
            stmts,
            vec![Catch(Id(p(), "Throwable".into()), exnvar, vec![])],
            sets,
        ),
    );
    unsets.push(catch);
    let body = unsets;
    let pos = Pos::from_line_cols_offset(
        RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(""))),
        1,
        0..0,
        0,
    );
    let f = Fun_ {
        span: pos,
        annotation: (),
        readonly_this: None, // TODO(readonly): readonly_this in closure_convert
        readonly_ret: None,  // TODO(readonly): readonly_ret in closure_convert
        ret: TypeHint((), None),
        name: Id(Pos::make_none(), "include".into()),
        tparams: vec![],
        where_constraints: vec![],
        variadic: FunVariadicity::FVnonVariadic,
        params,
        ctxs: None,        // TODO(T70095684)
        unsafe_ctxs: None, // TODO(T70095684)
        body: FuncBody {
            ast: body,
            annotation: (),
        },
        fun_kind: FunKind::FSync,
        user_attributes: vec![UserAttribute {
            name: Id(Pos::make_none(), "__DebuggerMain".into()),
            params: vec![],
        }],
        external: false,
        doc_comment: None,
    };
    let fd = FunDef {
        namespace: RcOc::clone(empty_namespace),
        file_attributes: vec![],
        mode: Mode::Mstrict,
        fun: f,
    };
    let mut new_defs = vec![Def::mk_fun(fd)];
    new_defs.append(&mut defs);
    *all_defs = new_defs;
    Ok(())
}

pub fn convert_toplevel_prog<'local_arena, 'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'local_arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    defs: &mut Program,
    namespace_env: RcOc<namespace_env::Env>,
) -> Result<()> {
    if e.options()
        .hack_compiler_flags
        .contains(CompilerFlags::CONSTANT_FOLDING)
    {
        hhbc_by_ref_ast_constant_folder::fold_program(defs, alloc, e)
            .map_err(|e| unrecoverable(format!("{}", e)))?;
    }

    let mut env = Env::toplevel(
        count_classes(defs.as_slice()),
        count_records(defs.as_slice()),
        1,
        defs.as_slice(),
        e.options(),
        e.for_debugger_eval,
    )?;
    *defs = flatten_ns(defs);
    if e.for_debugger_eval {
        extract_debugger_main(&namespace_env, defs).map_err(unrecoverable)?;
    }

    let mut visitor = ClosureConvertVisitor {
        state: State::initial_state(namespace_env),
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
            Def::SetNamespaceEnv(x) => {
                visitor.state.set_namespace(RcOc::clone(&*x));
                new_defs.push(Def::SetNamespaceEnv(x))
            }
            def => new_defs.push(def),
        }
    }

    visitor.state.record_function_state(
        get_unique_id_for_main(),
        visitor.state.current_function_state.clone(),
        HhasCoeffects::default(),
        0,
    );
    hoist_toplevel_functions(&mut new_defs);
    let named_fun_defs = visitor
        .state
        .named_hoisted_functions
        .into_iter()
        .map(|(_, fd)| Def::mk_fun(fd));
    *defs = named_fun_defs.collect();
    defs.extend(new_defs.drain(..));
    for class in visitor.state.closures.into_iter() {
        defs.push(Def::mk_class(class));
    }
    *e.emit_global_state_mut() = visitor.state.global_state;
    Ok(())
}
