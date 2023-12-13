(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shallow_decl_defs

val diff_class :
  Provider_context.t ->
  PackageInfo.t ->
  shallow_class ->
  shallow_class ->
  ClassDiff.t option
