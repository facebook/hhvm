// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use super::syn_helper::*;
use crate::common::Result;
use quote::format_ident;
use std::{
    collections::{HashMap, HashSet, VecDeque},
    iter::once,
};
use syn::*;

pub struct Context<'a> {
    pub defs: HashMap<String, &'a Item>,
    pub root: &'a str,
    pub root_ty_params: Vec<String>,
    pub types: Vec<String>,
    pub context: String,
}
impl<'a> Context<'a> {
    pub fn new(files: &'a [syn::File], root: &'a str) -> Result<Self> {
        let mut defs = HashMap::new();
        for f in files.iter() {
            for i in f.items.iter() {
                if let Ok(name) = get_ty_def_name(i) {
                    if let Some(old) = defs.insert(name, i) {
                        return Err(format!("Type {:?} already exists, file {:?}", old, f).into());
                    }
                }
            }
        }
        let root_item = defs
            .get(root)
            .ok_or_else(|| format!("Root {} not found", root))?;
        let root_ty_params = get_ty_params(root_item)?;
        let types = Self::get_all_tys(&defs, root)?;
        Ok(Self {
            defs,
            root,
            root_ty_params,
            types,
            context: "Context".into(),
        })
    }

    pub fn visitor_context(&self) -> Ident {
        format_ident!("{}", self.context)
    }

    pub fn is_root_ty_param(&self, ty_param: &str) -> bool {
        self.root_ty_params.iter().any(|t| t == ty_param)
    }

    pub fn root_ty_params_raw(&'a self) -> impl Iterator<Item = &'a String> {
        self.root_ty_params.iter()
    }

    pub fn root_ty_params_(&'a self) -> impl Iterator<Item = Ident> + 'a {
        self.root_ty_params_raw().map(|t| format_ident!("{}", t))
    }

    pub fn root_ty_params_with_context_raw(&'a self) -> impl Iterator<Item = &'a String> {
        once(&self.context).chain(self.root_ty_params.iter())
    }

    pub fn root_ty_params_with_context(&'a self) -> impl Iterator<Item = Ident> + 'a {
        self.root_ty_params_with_context_raw()
            .map(|t| format_ident!("{}", t))
    }

    fn get_ty_names_<'b>(defs: &'b HashMap<String, &'b Item>) -> HashSet<&'b str> {
        defs.keys().map(|s| s.as_str()).collect()
    }

    fn get_all_tys(defs: &HashMap<String, &Item>, root: &'a str) -> Result<Vec<String>> {
        let defined_types = Self::get_ty_names_(defs);
        let mut visited = HashSet::<String>::new();
        let mut q = VecDeque::new();
        q.push_back(root.into());
        while let Some(ty) = q.pop_front() {
            let item = defs
                .get(&ty)
                .ok_or_else(|| format!("Type {} not found", ty))?;
            visited.insert(get_ty_def_name(&item)?);
            let deps = get_dep_tys(&defined_types, item)?;
            for d in deps.into_iter() {
                if !visited.contains(&d) {
                    q.push_back(d);
                }
            }
        }
        let mut types: Vec<String> = visited.into_iter().collect();
        types.sort();
        Ok(types)
    }
}
