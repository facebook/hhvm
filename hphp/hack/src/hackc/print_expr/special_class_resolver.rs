// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use ast_scope::Scope;

pub trait SpecialClassResolver {
    fn resolve<'a>(&self, scope_opt: Option<&'a Scope<'a>>, id: &'a str) -> Cow<'a, str>;
}
