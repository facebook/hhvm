// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_defs_core::*;
use crate::*;

impl From<Option<ast::ParamKind>> for ParamMode {
    fn from(callconv: Option<ast::ParamKind>) -> Self {
        match callconv {
            Some(ast::ParamKind::Pinout) => ParamMode::FPinout,
            None => ParamMode::FPnormal,
        }
    }
}
