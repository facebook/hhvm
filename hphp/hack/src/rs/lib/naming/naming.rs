// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::rc::Rc;

use oxidized::aast;

use crate::sn_provider::SpecialNamesProvider;

pub struct Naming {
    sn: Rc<SpecialNamesProvider>,
}

impl Naming {
    pub fn new(sn: Rc<SpecialNamesProvider>) -> Self {
        Self { sn }
    }

    fn hint_id(
        &self,
        allow_retonly: bool,
        cls: &aast::ClassName,
        _hl: &[aast::Hint],
    ) -> Option<aast::Hint_> {
        use aast::Hint_::*;
        use aast::Tprim::*;

        // TODO(hrust): so much more
        let cls = &cls.1;
        if cls == &**self.sn.typehints().void() && allow_retonly {
            Some(Hprim(Tvoid))
        } else if cls == &**self.sn.typehints().void() {
            unimplemented!()
        } else if cls == &**self.sn.typehints().int() {
            Some(Hprim(Tint))
        } else {
            None
        }
    }

    fn hint_(&self, allow_retonly: bool, h: &mut aast::Hint_) {
        // TODO(hrust): so much more
        use aast::Hint_::*;
        match h {
            Happly(cls, hl) => {
                let new_h = self.hint_id(allow_retonly, cls, hl);

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

    fn hint(&self, allow_retonly: bool, h: &mut aast::TypeHint<()>) {
        h.1.iter_mut()
            .for_each(|h| self.hint_(allow_retonly, &mut *h.1));
    }

    fn fun_param(&self, p: &mut aast::FunParam<(), ()>) {
        // TODO(hrust): all the rest
        self.hint(false, &mut p.type_hint);
    }

    fn fun_paraml(&self, pl: &mut Vec<aast::FunParam<(), ()>>) {
        // TODO(hrust): check repetition
        // TODO(hrust): variadicity
        pl.iter_mut().for_each(|p| self.fun_param(p));
    }

    fn fun_(&self, f: &mut aast::Fun_<(), ()>) {
        // TODO(hrust): all the rest
        self.hint(true, &mut f.ret);
        self.fun_paraml(&mut f.params);
    }

    fn fun_def(&self, fd: &mut aast::FunDef<(), ()>) {
        // TODO(hrust): all the rest
        self.fun_(&mut fd.fun);
    }

    fn program_(&self, p: &mut aast::Program<(), ()>) {
        use aast::Def::*;
        for def in p {
            match def {
                Fun(fd) => self.fun_def(&mut **fd),
                Class(_) => {}
                Stmt(_) => {}
                Typedef(_) => {}
                Constant(_) => {}
                Namespace(_) => {}
                NamespaceUse(_) => {}
                SetNamespaceEnv(_) => {}
                FileAttributes(_) => {}
            }
        }
    }

    pub fn program(sn: Rc<SpecialNamesProvider>, p: &mut aast::Program<(), ()>) {
        Self::new(sn).program_(p);
    }
}
