// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use itertools::Either;
use oxidized::ast::*;

pub type AstBody<'a> = Either<&'a [Def], &'a [Stmt]>;
