// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub use oxidized::typing_local_types::ExpressionId;
use typing_defs_rust::Ty;

use crate::typing_env_types::LocalIdMap;

#[derive(Debug, Copy, Clone)]
pub struct Local<'a>(pub Ty<'a>, pub ExpressionId);

pub type TypingLocalTypes<'a> = LocalIdMap<'a, Local<'a>>;
