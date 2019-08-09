// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<42cb5f2bed3e6d689289e2115487fa0e>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#[derive(Clone, Debug)]
pub enum Prefix {
    Root,
    Hhi,
    Dummy,
    Tmp,
}

#[derive(Clone, Debug)]
pub struct RelativePath(pub Prefix, pub String);
