<?hh
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

<<file: __EnableUnstableFeatures('require_class')>>

interface PageRoles {};

interface IDefinesConst {
  const type TPermissionRole = PageRoles;
}

trait ExtendPajouxTrait implements IDefinesConst {
  require class Pajoux;
}

final class Pajoux implements IDefinesConst {
  use ExtendPajouxTrait;
  const type TPermissionRole = PageRoles;
}
