// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::collections::BTreeSet;
use std::ffi::OsStr;
use std::os::unix::ffi::OsStrExt;
use std::path::Path;
use std::sync::Arc;

use ast_scope::Scope;
use decl_provider::DeclProvider;
use decl_provider::MemoProvider;
use global_state::GlobalState;
use hhbc::ClassName;
use hhbc::ConstName;
use hhbc::FunctionName;
use hhbc::IncludePath;
use hhbc::IncludePathSet;
use hhbc::Local;
use hhbc::StringId;
use hhbc::StringIdIndexSet;
use hhbc::SymbolRefs;
use options::Options;
use oxidized::ast;
use oxidized::ast_defs;
use oxidized::pos::Pos;
use relative_path::RelativePath;
use statement_state::StatementState;

use crate::adata_state::AdataState;
use crate::ClassExpr;
use crate::IterGen;
use crate::LabelGen;
use crate::LocalGen;

#[derive(Debug)]
pub struct Emitter<'decl> {
    /// Options are frozen/const after emitter is constructed
    opts: Options,

    /// systemlib is part of context, changed externally
    systemlib: bool,

    // the rest is being mutated during emittance
    label_gen: LabelGen,
    local_gen: LocalGen,
    iterator: IterGen,
    named_locals: StringIdIndexSet,

    pub filepath: RelativePath,

    pub for_debugger_eval: bool,

    pub adata_state: AdataState,
    pub statement_state_: Option<StatementState>,
    symbol_refs_state: SymbolRefsState,

    /// State is also frozen and set after closure conversion
    pub global_state_: Option<GlobalState>,

    /// Controls whether we call the decl provider for testing purposes.
    /// Some(provider) => use the given DeclProvider, which may return NotFound.
    /// None => do not look up any decls. For now this is the same as as a
    /// DeclProvider that always returns NotFound, but this behavior may later
    /// diverge from None provider behavior.
    pub decl_provider: Option<Arc<dyn DeclProvider<'decl> + 'decl>>,
}

impl<'decl> Emitter<'decl> {
    pub fn new(
        opts: Options,
        systemlib: bool,
        for_debugger_eval: bool,
        decl_provider: Option<Arc<dyn DeclProvider<'decl> + 'decl>>,
        filepath: RelativePath,
    ) -> Emitter<'decl> {
        Emitter {
            opts,
            systemlib,
            for_debugger_eval,
            decl_provider: decl_provider
                .map(|p| Arc::new(MemoProvider::new(p)) as Arc<dyn DeclProvider<'decl> + 'decl>),

            label_gen: LabelGen::new(),
            local_gen: LocalGen::new(),
            iterator: Default::default(),
            named_locals: Default::default(),
            filepath,

            adata_state: Default::default(),
            statement_state_: None,
            symbol_refs_state: Default::default(),
            global_state_: None,
        }
    }

    pub fn options(&self) -> &Options {
        &self.opts
    }

    pub fn iterator(&self) -> &IterGen {
        &self.iterator
    }

    pub fn iterator_mut(&mut self) -> &mut IterGen {
        &mut self.iterator
    }

    pub fn label_gen_mut(&mut self) -> &mut LabelGen {
        &mut self.label_gen
    }

    pub fn local_gen_mut(&mut self) -> &mut LocalGen {
        &mut self.local_gen
    }

    pub fn local_gen(&self) -> &LocalGen {
        &self.local_gen
    }

    /// Initialize the named locals table. Canonical HHBC numbering
    /// puts the parameters first in left-to-right order, then local varables
    /// that have names from the source text. In HHAS those names must appear
    /// in the `.decl_vars` directive.
    pub fn init_named_locals(&mut self, names: impl IntoIterator<Item = StringId>) {
        assert!(self.named_locals.is_empty());
        self.named_locals = names.into_iter().collect();
    }

    pub fn clear_named_locals(&mut self) {
        self.named_locals = Default::default();
    }

    /// Given a name, return corresponding local. Panic if the local is unknown,
    /// indicating a logic bug in the compiler; all params and named locals must
    /// be provided in advance to init_named_locals().
    pub fn named_local(&self, name: impl AsRef<str>) -> Local {
        let name = name.as_ref();
        match StringId::get_interned(name)
            .and_then(|name| self.named_locals.get_index_of(&name).map(Local::new))
        {
            Some(local) => local,
            None => panic!(
                "{}: local not found among {:#?}",
                name,
                self.named_locals
                    .iter()
                    .map(|name| name.as_str())
                    .collect::<Vec<_>>()
            ),
        }
    }

    /// Given a name, return corresponding local. Panic if the local is unknown,
    /// indicating a logic bug in the compiler; all params and named locals must
    /// be provided in advance to init_named_locals().
    pub fn interned_local(&self, name: StringId) -> Local {
        match self.named_locals.get_index_of(&name).map(Local::new) {
            Some(local) => local,
            None => panic!(
                "{}: local not found among {:#?}",
                name,
                self.named_locals
                    .iter()
                    .map(|name| name.as_str())
                    .collect::<Vec<_>>()
            ),
        }
    }

    /// Given a named local, return its name. Panic for unnamed locals
    /// indicating a logic bug in the compiler.
    pub fn local_name(&self, local: Local) -> StringId {
        *self.named_locals.get_index(local.idx as usize).unwrap()
    }

    pub fn local_scope<R, F: FnOnce(&mut Self) -> R>(&mut self, f: F) -> R {
        let counter = self.local_gen.counter;
        self.local_gen.dedicated.temp_map.push();
        let r = f(self);
        self.local_gen.counter = counter;
        self.local_gen.dedicated.temp_map.pop();
        r
    }

    pub fn systemlib(&self) -> bool {
        self.systemlib
    }

    pub fn statement_state(&self) -> &StatementState {
        self.statement_state_
            .as_ref()
            .expect("uninit'd statement_state")
    }
    pub fn statement_state_mut(&mut self) -> &mut StatementState {
        self.statement_state_
            .get_or_insert_with(StatementState::init)
    }

    pub fn global_state(&self) -> &GlobalState {
        self.global_state_.as_ref().expect("uninit'd global_state")
    }
    pub fn global_state_mut(&mut self) -> &mut GlobalState {
        self.global_state_.get_or_insert_with(GlobalState::init)
    }

    pub fn add_include_ref(&mut self, inc: IncludePath) {
        match inc {
            IncludePath::SearchPathRelative(p)
            | IncludePath::DocRootRelative(p)
            | IncludePath::Absolute(p) => {
                let path = Path::new(OsStr::from_bytes(&p));
                if path.exists() {
                    self.symbol_refs_state.includes.insert(inc);
                }
            }
            IncludePath::IncludeRootRelative(_, _) => {}
        };
    }

    pub fn add_constant_ref(&mut self, s: ConstName) {
        if !s.is_empty() {
            self.symbol_refs_state.constants.insert(s);
        }
    }

    pub fn add_class_ref(&mut self, s: ClassName) {
        if !s.is_empty() {
            self.symbol_refs_state.classes.insert(s);
        }
    }

    pub fn add_function_ref(&mut self, s: FunctionName) {
        if !s.is_empty() {
            self.symbol_refs_state.functions.insert(s);
        }
    }

    pub fn finish_symbol_refs(&mut self) -> SymbolRefs {
        let state = std::mem::take(&mut self.symbol_refs_state);
        state.to_hhas()
    }
}

impl<'decl> print_expr::SpecialClassResolver for Emitter<'decl> {
    fn resolve<'a>(&self, scope_opt: Option<&'a Scope<'_>>, id: &'a str) -> Cow<'a, str> {
        let class_expr = ClassExpr::expr_to_class_expr(
            self,
            scope_opt.unwrap_or(&ast_scope::Scope::default()),
            true,
            true,
            ast::Expr(
                (),
                Pos::NONE,
                ast::Expr_::mk_id(ast_defs::Id(Pos::NONE, id.into())),
            ),
        );
        match class_expr {
            ClassExpr::Id(ast_defs::Id(_, name)) => Cow::Owned(name),
            _ => Cow::Borrowed(id),
        }
    }
}

#[derive(Clone, Debug, Default)]
struct SymbolRefsState {
    includes: IncludePathSet,
    constants: BTreeSet<ConstName>,
    functions: BTreeSet<FunctionName>,
    classes: BTreeSet<ClassName>,
}

impl SymbolRefsState {
    fn to_hhas(self) -> SymbolRefs {
        SymbolRefs {
            includes: Vec::from_iter(self.includes).into(),
            constants: Vec::from_iter(self.constants).into(),
            functions: Vec::from_iter(self.functions).into(),
            classes: Vec::from_iter(self.classes).into(),
        }
    }
}
