// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust as sn;
use oxidized::{ast::*, namespace_env};

use std::borrow::Cow;

#[derive(Clone, Debug, Copy, Eq, PartialEq)]
pub enum ElaborateKind {
    Fun,
    Class,
    Record,
    Const,
}

fn elaborate_into_ns(ns_name: &Option<&str>, id: &str) -> String {
    match ns_name {
        None => {
            let mut s = String::with_capacity(1 + id.len());
            s.push_str("\\");
            s.push_str(id);
            s
        }
        Some(ns) => {
            let mut s = String::with_capacity(2 + ns.len() + id.len());
            s.push_str("\\");
            s.push_str(ns);
            s.push_str("\\");
            s.push_str(id);
            s
        }
    }
}

fn elaborate_into_current_ns(nsenv: &namespace_env::Env, id: &str) -> String {
    match &nsenv.name {
        None => elaborate_into_ns(&None, id),
        Some(name) => {
            let name: &str = &name;
            elaborate_into_ns(&Some(name), id)
        }
    }
}

fn elaborate_xhp_namespace(id: &str) -> Option<String> {
    let is_xhp = !id.is_empty() && id.contains(':');
    if is_xhp {
        Some(id.replace(':', "\\"))
    } else {
        None
    }
}

/// Resolves an identifier in a given namespace environment. For example, if we
/// are in the namespace "N\O", the identifier "P\Q" is resolved to "\N\O\P\Q".
///
/// All identifiers are fully-qualified by this function; the internal
/// representation of identifiers inside the typechecker after naming is a fully
/// qualified identifier.
///
/// It's extremely important that this function is idempotent. We actually
/// normalize identifiers in two phases. Right after parsing, we need to have
/// the class hierarchy normalized so that we can recompute dependencies for
/// incremental mode properly. Other identifiers are normalized during naming.
/// However, we don't do any bookkeeping to determine which we've normalized or
/// not, just relying on the idempotence of this function to make sure everything
/// works out. (Fully qualifying identifiers is of course idempotent, but there
/// used to be other schemes here.)
fn elaborate_raw_id(nsenv: &namespace_env::Env, kind: ElaborateKind, id: &str) -> String {
    let id = if kind == ElaborateKind::Class && nsenv.disable_xhp_element_mangling {
        elaborate_xhp_namespace(id).map_or(Cow::Borrowed(id), Cow::Owned)
    } else {
        Cow::Borrowed(id)
    };
    let id = id.as_ref();
    // It is already qualified
    if id.starts_with("\\") {
        return id.to_string();
    }

    // Return early if it's a special/pseudo name
    let fqid = core_utils_rust::add_ns(id).to_string();
    match kind {
        ElaborateKind::Const => {
            if sn::pseudo_consts::is_pseudo_const(&fqid) {
                return fqid;
            }
        }
        ElaborateKind::Fun if sn::pseudo_functions::is_pseudo_function(&fqid) => {
            return fqid;
        }
        ElaborateKind::Class if sn::typehints::is_reserved_global_name(&id) => {
            return fqid;
        }
        ElaborateKind::Class if sn::typehints::is_reserved_hh_name(&id) && nsenv.is_codegen => {
            return elaborate_into_ns(&Some("HH"), &id);
        }
        ElaborateKind::Class if sn::typehints::is_reserved_hh_name(&id) => {
            return fqid;
        }
        _ => (),
    }

    let (prefix, has_bslash) = match id.find("\\") {
        Some(i) => (&id[..i], true),
        None => (&id[..], false),
    };

    if has_bslash && prefix == "namespace" {
        return elaborate_into_current_ns(nsenv, id.trim_start_matches("namespace\\"));
    }

    let uses = {
        if has_bslash {
            &nsenv.ns_uses
        } else {
            match kind {
                ElaborateKind::Class => &nsenv.class_uses,
                ElaborateKind::Fun => &nsenv.fun_uses,
                ElaborateKind::Const => &nsenv.const_uses,
                ElaborateKind::Record => &nsenv.record_def_uses,
            }
        }
    };

    match uses.get(prefix) {
        Some(used) => {
            let without_prefix = id.trim_start_matches(prefix);
            let mut s = String::with_capacity(used.len() + without_prefix.len());
            s.push_str(used);
            s.push_str(without_prefix);
            core_utils_rust::add_ns(&s).to_string()
        }
        None => elaborate_into_current_ns(nsenv, id),
    }
}

pub fn elaborate_id(nsenv: &namespace_env::Env, kind: ElaborateKind, Id(p, id): &Id) -> Id {
    Id(p.clone(), elaborate_raw_id(nsenv, kind, id))
}

pub mod toplevel_elaborator {
    use ocamlrep::rc::RcOc;
    use oxidized::{global_options::GlobalOptions, namespace_env, uast::*};

    fn elaborate_xhp_namespace(id: &mut String) {
        if let Some(s) = super::elaborate_xhp_namespace(id.as_str()) {
            *id = s
        }
    }

    fn elaborate_into_current_ns(nsenv: &namespace_env::Env, id: &mut String) {
        *id = super::elaborate_into_current_ns(nsenv, id.as_str());
    }

    fn elaborate_defined_id(nsenv: &namespace_env::Env, sid: &mut Sid) {
        if nsenv.disable_xhp_element_mangling && !sid.1.is_empty() && sid.1.contains(':') {
            elaborate_xhp_namespace(&mut sid.1);
            if sid.1.as_bytes()[0] != b'\\' {
                elaborate_into_current_ns(nsenv, &mut sid.1);
            }
        } else {
            elaborate_into_current_ns(nsenv, &mut sid.1);
        }
    }

    fn on_hint(nsenv: &namespace_env::Env, hint: &mut Hint) {
        if let Hint_::Happly(ref mut id, _) = hint.1.as_mut() {
            *id = super::elaborate_id(nsenv, super::ElaborateKind::Class, id);
        }
    }

    fn on_def<A: ClonableAnnot>(
        nsenv: &mut RcOc<namespace_env::Env>,
        acc: &mut Vec<Def<A>>,
        def: Def<A>,
    ) {
        use oxidized::uast::Def as D;
        match def {
            // The default namespace in php is the global namespace specified by
            // the empty string. In the case of an empty string, we model it as
            // the global namespace.
            //
            // We remove namespace and use nodes and replace them with
            // SetNamespaceEnv nodes that contain the namespace environment
            D::<A>::Namespace(ns) => {
                let (Id(_, nsname), defs) = *ns;
                let nsname: Option<String> = if nsname.is_empty() {
                    None
                } else {
                    match &nsenv.as_ref().name {
                        None => Some(nsname),
                        Some(p) => Some(p.to_string() + "\\" + &nsname[..]),
                    }
                };
                let mut new_nsenv = nsenv.as_ref().clone();
                new_nsenv.name = nsname;
                let new_nsenv = RcOc::new(new_nsenv);

                acc.push(Def::<A>::mk_set_namespace_env(RcOc::clone(&new_nsenv)));
                on_program_::<A>(new_nsenv, defs, acc);
            }
            D::<A>::NamespaceUse(nsu) => {
                let mut new_nsenv = nsenv.as_ref().clone();

                for (kind, Id(_, id1), Id(_, id2)) in nsu.into_iter() {
                    match kind {
                        NsKind::NSNamespace => new_nsenv.ns_uses.insert(id2, id1),
                        NsKind::NSClass => new_nsenv.class_uses.insert(id2, id1),
                        NsKind::NSClassAndNamespace => {
                            new_nsenv.ns_uses.insert(id2.clone(), id1.clone());
                            new_nsenv.class_uses.insert(id2, id1)
                        }
                        NsKind::NSFun => new_nsenv.fun_uses.insert(id2, id1),
                        NsKind::NSConst => new_nsenv.const_uses.insert(id2, id1),
                    };
                }

                let new_nsenv = RcOc::new(new_nsenv);
                std::mem::replace(nsenv, RcOc::clone(&new_nsenv));
                acc.push(Def::<A>::mk_set_namespace_env(new_nsenv));
            }
            D::<A>::Class(mut x) => {
                let c = x.as_mut();
                elaborate_defined_id(nsenv, &mut c.name);
                c.extends.iter_mut().for_each(|ref mut x| on_hint(nsenv, x));
                c.reqs
                    .iter_mut()
                    .for_each(|ref mut x| on_hint(nsenv, &mut x.0));
                c.implements
                    .iter_mut()
                    .for_each(|ref mut x| on_hint(nsenv, x));
                c.uses.iter_mut().for_each(|ref mut x| on_hint(nsenv, x));
                c.xhp_attr_uses
                    .iter_mut()
                    .for_each(|ref mut x| on_hint(nsenv, x));
                c.namespace = RcOc::clone(nsenv);
                acc.push(Def::<A>::Class(x));
            }
            D::<A>::RecordDef(mut x) => {
                let r = x.as_mut();
                elaborate_defined_id(nsenv, &mut r.name);
                r.namespace = RcOc::clone(nsenv);
                acc.push(Def::<A>::RecordDef(x));
            }
            D::<A>::Fun(mut x) => {
                let f = x.as_mut();
                elaborate_defined_id(nsenv, &mut f.name);
                f.namespace = RcOc::clone(nsenv);
                acc.push(Def::<A>::Fun(x));
            }
            D::<A>::Typedef(mut x) => {
                let t = x.as_mut();
                elaborate_defined_id(nsenv, &mut t.name);
                t.namespace = RcOc::clone(nsenv);
                acc.push(Def::<A>::Typedef(x));
            }
            D::<A>::Constant(mut x) => {
                let c = x.as_mut();
                elaborate_defined_id(nsenv, &mut c.name);
                c.namespace = RcOc::clone(nsenv);
                acc.push(Def::<A>::Constant(x));
            }
            D::<A>::FileAttributes(mut f) => {
                f.as_mut().namespace = RcOc::clone(nsenv);
                acc.push(Def::<A>::FileAttributes(f));
            }
            x => acc.push(x),
        }
    }

    fn attach_file_attributes<A: ClonableAnnot>(p: &mut Program<A>) {
        let file_attrs: Vec<FileAttribute<A>> = p
            .iter()
            .filter_map(|x| x.as_file_attributes())
            .cloned()
            .collect();

        p.iter_mut().for_each(|x| match x {
            Def::<A>::Class(c) => c.file_attributes = file_attrs.clone(),
            Def::<A>::Fun(f) => f.file_attributes = file_attrs.clone(),
            _ => (),
        });
    }

    fn on_program_<A: ClonableAnnot>(
        mut nsenv: RcOc<namespace_env::Env>,
        p: Program<A>,
        acc: &mut Vec<Def<A>>,
    ) {
        let mut new_acc = vec![];
        for def in p.into_iter() {
            on_def::<A>(&mut nsenv, &mut new_acc, def)
        }

        attach_file_attributes::<A>(&mut new_acc);
        acc.append(&mut new_acc);
    }

    fn on_program<A: ClonableAnnot>(nsenv: RcOc<namespace_env::Env>, p: Program<A>) -> Program<A> {
        let mut acc = vec![];
        on_program_::<A>(nsenv, p, &mut acc);
        acc
    }

    pub fn elaborate_toplevel_defs<A: ClonableAnnot>(
        popt: &GlobalOptions,
        defs: Program<A>,
    ) -> Program<A> {
        on_program::<A>(RcOc::new(namespace_env::Env::empty_from_popt(popt)), defs)
    }
}
