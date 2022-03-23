// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::aast;

use crate::naming::naming_elaborate_namespaces;
use crate::special_names as sn;

fn hint_id(allow_retonly: bool, cls: &aast::ClassName, _hl: &[aast::Hint]) -> Option<aast::Hint_> {
    use aast::Hint_::*;
    use aast::Tprim::*;

    // TODO(hrust): so much more
    let cls = &cls.1;
    if cls == sn::typehints::void.as_str() {
        if allow_retonly {
            Some(Hprim(Tvoid))
        } else {
            unimplemented!()
        }
    } else if cls == sn::typehints::int.as_str() {
        Some(Hprim(Tint))
    } else {
        None
    }
}

fn hint_(allow_retonly: bool, h: &mut aast::Hint_) {
    // TODO(hrust): so much more
    use aast::Hint_::*;
    match h {
        Happly(cls, hl) => {
            let new_h = hint_id(allow_retonly, cls, hl);

            if !hl.is_empty() {
                match new_h.as_ref().unwrap_or(h) {
                    Hprim(..) | Hmixed | Hnonnull | Hdynamic | Hnothing => unimplemented!(),
                    _ => {}
                }
            }

            if let Some(new_h) = new_h {
                *h = new_h;
            }
        }
        _ => {
            // TODO(hrust)
        }
    }
}

fn hint(allow_retonly: bool, h: &mut aast::TypeHint<()>) {
    h.1.iter_mut().for_each(|h| hint_(allow_retonly, &mut *h.1));
}

fn fun_param(p: &mut aast::FunParam<(), ()>) {
    // TODO(hrust): all the rest
    hint(false, &mut p.type_hint);
}

fn fun_paraml(pl: &mut Vec<aast::FunParam<(), ()>>) {
    // TODO(hrust): check repetition
    // TODO(hrust): variadicity
    pl.iter_mut().for_each(fun_param);
}

fn fun_(f: &mut aast::Fun_<(), ()>) {
    // TODO(hrust): all the rest
    hint(true, &mut f.ret);
    fun_paraml(&mut f.params);
}

fn fun_def(fd: &mut aast::FunDef<(), ()>) {
    // TODO(hrust): all the rest
    fun_(&mut fd.fun);
}

fn method_(m: &mut aast::Method_<(), ()>) {
    // TODO(hrust): all the rest
    hint(true, &mut m.ret);
    fun_paraml(&mut m.params);
}

fn class_prop(p: &mut aast::ClassVar<(), ()>) {
    // TODO(hrust): all the rest
    hint(false, &mut p.type_);
}

fn interface(c: &mut aast::Class_<(), ()>) {
    // TODO(hrust): all the rest
    if matches!(c.kind, oxidized::ast_defs::ClassishKind::Cinterface) {
        c.methods.iter_mut().for_each(|m| m.abstract_ = true);
    }
}

fn class_(c: &mut aast::Class_<(), ()>) {
    // TODO(hrust): all the rest
    naming_elaborate_namespaces::elaborate_class(c);
    c.methods.iter_mut().for_each(method_);
    c.vars.iter_mut().for_each(class_prop);
    interface(c);
}

pub fn program(p: &mut aast::Program<(), ()>) {
    use aast::Def::*;
    for def in p {
        match def {
            Fun(fd) => fun_def(&mut **fd),
            Class(c) => class_(&mut **c),
            Stmt(_) => {}
            Typedef(_) => {}
            Constant(_) => {}
            Namespace(_) => {}
            NamespaceUse(_) => {}
            SetNamespaceEnv(_) => {}
            FileAttributes(_) => {}
            Module(_) => {}
        }
    }
}
