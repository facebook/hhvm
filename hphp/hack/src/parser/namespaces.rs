// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{ast::*, namespace_env};

use naming_special_names_rust as sn;

pub enum ElaborateKind {
    Fun,
    Class,
    Record,
    Const,
}

fn elaborate_into_ns(ns_name: &Option<&str>, id: &str) -> String {
    match ns_name {
        None => {
            let mut s = String::with_capacity(2 + id.len());
            s.push_str("\\");
            s.push_str(id);
            s
        }
        Some(ns) => {
            let mut s = String::with_capacity(4 + ns.len() + id.len());
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
