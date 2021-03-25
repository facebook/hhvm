// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod fun_set {
    pub type FunSet = std::collections::BTreeSet<String>;
}

pub mod type_set {
    pub type TypeSet = std::collections::BTreeSet<String>;
}

pub mod const_set {
    pub type ConstSet = std::collections::BTreeSet<String>;
}

pub mod i_fun {
    pub type IFun = String;
}

pub mod i_type {
    pub type IType = String;
}

pub mod i_const {
    pub type IConst = String;
}
