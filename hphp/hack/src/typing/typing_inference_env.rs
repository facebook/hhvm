// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use arena_trait::Arena;

use oxidized::aast;
use oxidized::ident::Ident;
use oxidized::pos::Pos;

use typing_collections_rust::{IMap, ISet, SMap};
use typing_defs_rust::typing_make_type::TypeBuilder;
use typing_defs_rust::{ITySet, PReason, Ty, Ty_};

#[derive(Clone)]
pub struct TyvarConstraints<'a> {
    /// Does this type variable appear covariantly in the type of the expression?
    pub appears_covariantly: bool,
    /// Does this type variable appear contravariantly in the type of the expression?
    /// If it appears in an invariant position then both will be true; if it doesn't
    /// appear at all then both will be false.
    pub appears_contravariantly: bool,
    pub lower_bounds: ITySet<'a>,
    pub upper_bounds: ITySet<'a>,
    /// Map associating a type to each type constant id of this variable.
    /// Whenever we localize "T1::T" in a constraint, we add a fresh type variable
    /// indexed by "T" in the type_constants of the type variable representing T1.
    /// This allows to properly check constraints on "T1::T".
    pub type_constants: SMap<'a, (&'a aast::Sid, Ty<'a>)>,
    /// Map associating PU information to each instance of
    /// C:@E:@#v:@T
    /// when the type variable #v is not resolved yet. We introduce a new type
    /// variable to 'postpone' the checking of this expression until the end,
    /// when #v will be known.
    pub pu_accesses: SMap<'a, (Ty<'a>, &'a aast::Sid, Ty<'a>, &'a aast::Sid)>,
}

impl<'a> TyvarConstraints<'a> {
    fn new() -> TyvarConstraints<'a> {
        TyvarConstraints {
            appears_covariantly: false,
            appears_contravariantly: false,
            lower_bounds: ITySet::empty(),
            upper_bounds: ITySet::empty(),
            type_constants: SMap::empty(),
            pu_accesses: SMap::empty(),
        }
    }
}

#[derive(Clone)]
pub enum SolvingInfo<'a> {
    /// when the type variable is bound to a type
    Type(Ty<'a>),
    /// when the type variable is still unsolved
    Constraints(TyvarConstraints<'a>),
}

#[derive(Clone)]
pub struct TyvarInfo_<'a> {
    /// Where was the type variable introduced? (e.g. generic method invocation,
    /// new object construction)
    pub tyvar_pos: Option<&'a Pos>,
    pub global_reason: Option<PReason<'a>>,
    pub eager_solve_failed: bool,
    pub solving_info: SolvingInfo<'a>,
}

pub type TyvarInfo<'a> = &'a TyvarInfo_<'a>;

pub type Tvenv<'a> = IMap<'a, TyvarInfo<'a>>;

#[derive(Clone)]
pub struct InferenceEnv<'a> {
    pub bld: &'a TypeBuilder<'a>,
    pub tvenv: Tvenv<'a>,
    pub tyvars_stack: Vec<(&'a Pos, Vec<Ident>)>,
    pub allow_solve_globals: bool,
    var_id_counter: Ident,
}

impl<'a> InferenceEnv<'a> {
    pub fn new(bld: &'a TypeBuilder) -> Self {
        InferenceEnv {
            bld,
            tvenv: IMap::empty(),
            tyvars_stack: vec![],
            allow_solve_globals: false,
            var_id_counter: 0,
        }
    }

    fn new_var_id(&mut self) -> Ident {
        let id = self.var_id_counter;
        self.var_id_counter += 1;
        id
    }

    pub fn expand_type(&mut self, ty: Ty<'a>) -> Ty<'a> {
        let (r, ty_) = ty.unpack();
        match ty_ {
            Ty_::Tvar(v) => match self.expand_var(r, *v) {
                None => ty,
                Some(ty) => ty,
            },
            _ => ty,
        }
    }

    fn expand_var(&mut self, r: PReason<'a>, v: Ident) -> Option<Ty<'a>> {
        let ty = self.get_type(r, v);
        // TODO(hrust) port eager_solve_fail logic
        ty
    }

    fn get_type(&mut self, r: PReason<'a>, v: Ident) -> Option<Ty<'a>> {
        let mut aliases = ISet::empty();
        let mut v = v;
        let mut r = r;
        loop {
            match self.tvenv.find(&v) {
                None => {
                    // TODO: This should actually be an error
                    return None;
                }
                Some(tvinfo) => match &tvinfo.solving_info {
                    SolvingInfo::Type(ty) => {
                        let (r0, ty_) = ty.unpack();
                        r = r0;
                        match ty_ {
                            Ty_::Tvar(v0) => {
                                if aliases.mem(v0) {
                                    panic!("Two type variables are aliasing each other!");
                                }
                                aliases = aliases.add(self.bld, *v0);
                                v = *v0;
                                continue;
                            }
                            _ => {
                                for alias in aliases.into_iter() {
                                    self.add(*alias, *ty);
                                }
                                return Some(*ty);
                            }
                        }
                    }
                    SolvingInfo::Constraints(_) => {
                        let ty = self.bld.tyvar(r, v);
                        for alias in aliases.into_iter() {
                            self.add(*alias, ty);
                        }
                        return Some(ty);
                    }
                },
            }
        }
    }

    fn add(&mut self, v: Ident, ty: Ty<'a>) -> () {
        self.bind(v, ty);
        // TODO(hrust) update tyvar occurrences
    }

    fn bind(&mut self, v: Ident, ty: Ty<'a>) -> () {
        /* TODO(hrust): the following indexing might panic,
        as somehow sometimes we're missing variables in the environment,
        The OCaml error, would create an empty tyvar_info in this case
        to avoid throwing. */

        let mut tvinfo = self.tvenv.find(&v).unwrap().clone();
        tvinfo.solving_info = SolvingInfo::Type(ty);
        self.tvenv = self.tvenv.add(self.bld, v, self.bld.alloc(tvinfo));
    }

    pub fn fresh_type_reason(&mut self, r: PReason<'a>) -> Ty<'a> {
        let v = self.new_var_id();
        self.add_current_tyvar(r.pos, v);
        self.bld.tyvar(r, v)
    }

    fn add_current_tyvar(&mut self, pos: Option<&'a Pos>, v: Ident) -> () {
        self.fresh_unsolved_tyvar(v, pos);
        // TODO(hrust) update tyvar stack
    }

    fn fresh_unsolved_tyvar(&mut self, v: Ident, tyvar_pos: Option<&'a Pos>) -> () {
        // TODO(hrust) variance
        let solving_info = SolvingInfo::Constraints(TyvarConstraints::new());
        let tyvar_info = self.bld.alloc(TyvarInfo_ {
            tyvar_pos,
            global_reason: None,
            eager_solve_failed: false,
            solving_info,
        });
        self.tvenv = self.tvenv.add(self.bld, v, tyvar_info)
    }
}
