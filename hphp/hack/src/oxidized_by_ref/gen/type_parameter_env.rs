// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<244301b8b1bd8a82e76ca10a1c83d219>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub use crate::typing_set as ty_set;

pub type TparamBounds<'a> = ty_set::TySet<'a>;

pub use oxidized::type_parameter_env::TparamInfo;

pub use oxidized::type_parameter_env::TypeParameterEnv;
