// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use im::HashSet;
use oxidized::aast;
use special_names as sn;

use crate::naming::naming_elaborate_namespaces;

#[derive(Clone, Debug, Default)]
struct NamingEnv {
    type_params: HashSet<String>,
}

fn hint_id(
    env: &NamingEnv,
    allow_retonly: bool,
    cls: &aast::ClassName,
    hl: &[aast::Hint],
) -> Option<aast::Hint_> {
    use aast::Hint_::*;
    use aast::Tprim::*;

    let x = &cls.1;

    if x == sn::typehints::void.as_str() {
        if allow_retonly {
            Some(Hprim(Tvoid))
        } else {
            rupro_todo!(Naming);
        }
    } else if x == sn::typehints::int.as_str() {
        Some(Hprim(Tint))
    } else if x == sn::typehints::string.as_str() {
        Some(Hprim(Tstring))
    } else if env.type_params.contains(x) {
        rupro_todo_assert!(hl.is_empty(), HKD);
        Some(Habstr(x.clone(), vec![]))
    } else {
        rupro_todo_mark!(Naming, "hintl/hint has special logic, maybe trait-ise this");
        Some(Happly(
            cls.clone(),
            hl.iter()
                .map(|h| {
                    rupro_todo_mark!(Naming, "we should fix cloning here");
                    let mut h = h.clone();
                    hint(env, true, &mut h);
                    h
                })
                .collect(),
        ))
    }
}

fn hint_(env: &NamingEnv, allow_retonly: bool, h: &mut aast::Hint_) {
    rupro_todo_mark!(AST);
    use aast::Hint_::*;
    match h {
        Happly(cls, hl) => {
            let new_h = hint_id(env, allow_retonly, cls, hl);

            if !hl.is_empty() {
                match new_h.as_ref().unwrap_or(h) {
                    Hprim(..) | Hmixed | Hnonnull | Hdynamic | Hnothing => rupro_todo!(Naming),
                    _ => {}
                }
            }

            if let Some(new_h) = new_h {
                *h = new_h;
            }
        }
        _ => {}
    }
}

fn hint(env: &NamingEnv, allow_retonly: bool, h: &mut aast::Hint) {
    hint_(env, allow_retonly, &mut h.1);
}

fn type_hint(env: &NamingEnv, allow_retonly: bool, h: &mut aast::TypeHint<()>) {
    h.1.iter_mut().for_each(|h| hint(env, allow_retonly, h))
}

fn fun_param(env: &NamingEnv, p: &mut aast::FunParam<(), ()>) {
    rupro_todo_mark!(AST);
    type_hint(env, false, &mut p.type_hint);
}

fn fun_paraml(env: &NamingEnv, pl: &mut [aast::FunParam<(), ()>]) {
    rupro_todo_mark!(MissingError, "check repitition");
    pl.iter_mut().for_each(|p| fun_param(env, p));
}

fn fun_(env: &NamingEnv, f: &mut aast::Fun_<(), ()>) {
    rupro_todo_mark!(AST);
    type_hint(env, true, &mut f.ret);
    fun_paraml(env, &mut f.params);
}

fn fun_def(env: &NamingEnv, fd: &mut aast::FunDef<(), ()>) {
    rupro_todo_mark!(AST);
    naming_elaborate_namespaces::elaborate_fun_def(&env.type_params, fd);
    fun_(env, &mut fd.fun);
}

fn method_(env: &NamingEnv, m: &mut aast::Method_<(), ()>) {
    rupro_todo_mark!(AST);
    let mut env = env.clone();
    env.extend_tparams(m.tparams.iter());
    type_hint(&env, true, &mut m.ret);
    fun_paraml(&env, &mut m.params);
}

fn class_prop(env: &NamingEnv, p: &mut aast::ClassVar<(), ()>) {
    rupro_todo_mark!(AST);
    type_hint(env, false, &mut p.type_);
}

fn interface(c: &mut aast::Class_<(), ()>) {
    rupro_todo_mark!(AST);
    if matches!(c.kind, oxidized::ast_defs::ClassishKind::Cinterface) {
        c.methods.iter_mut().for_each(|m| m.abstract_ = true);
    }
}

fn class_(env: &NamingEnv, c: &mut aast::Class_<(), ()>) {
    rupro_todo_mark!(AST);

    let mut env = env.clone();
    env.initialize_class_env(c);

    naming_elaborate_namespaces::elaborate_class(&env.type_params, c);
    c.methods.iter_mut().for_each(|m| method_(&env, m));
    c.vars.iter_mut().for_each(|p| class_prop(&env, p));
    interface(c);
}

pub fn program(p: &mut aast::Program<(), ()>) {
    let env = NamingEnv::default();

    use aast::Def::*;
    for def in p {
        match def {
            Fun(fd) => fun_def(&env, &mut **fd),
            Class(c) => class_(&env, &mut **c),
            Stmt(_) => {}
            Typedef(_) => {}
            Constant(_) => {}
            Namespace(_) => {}
            NamespaceUse(_) => {}
            SetNamespaceEnv(_) => {}
            FileAttributes(_) => {}
            Module(_) => {}
            SetModule(_) => {}
        }
    }
}

impl NamingEnv {
    fn initialize_class_env(&mut self, c: &aast::Class_<(), ()>) {
        self.type_params = Default::default();
        self.extend_tparams(c.tparams.iter());
    }

    fn extend_tparams<'aast>(&mut self, tps: impl Iterator<Item = &'aast aast::Tparam<(), ()>>) {
        self.type_params.extend(tps.map(|t| t.name.1.clone()));
    }
}
