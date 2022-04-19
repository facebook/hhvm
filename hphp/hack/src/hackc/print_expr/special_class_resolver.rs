// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::hhas_body::HhasBodyEnv;
use std::borrow::Cow;

pub trait SpecialClassResolver {
    fn resolve<'a>(&self, env: Option<&'a HhasBodyEnv<'_>>, id: &'a str) -> Cow<'a, str>;
}
