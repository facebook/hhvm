// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod fun_set {
    pub type FunSet<'a> = arena_collections::set::Set<'a, &'a str>;
}

pub mod type_set {
    pub type TypeSet<'a> = arena_collections::set::Set<'a, &'a str>;
}

pub mod const_set {
    pub type ConstSet<'a> = arena_collections::set::Set<'a, &'a str>;
}

pub mod i_fun {
    pub type IFun<'a> = &'a str;
}

pub mod i_type {
    pub type IType<'a> = &'a str;
}

pub mod i_const {
    pub type IConst<'a> = &'a str;
}
