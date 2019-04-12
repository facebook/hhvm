(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

val extract_as_json:
  php5_compat_mode: bool ->
  hhvm_compat_mode: bool ->
  force_hh: bool ->
  enable_xhp: bool ->
  filename: Relative_path.t ->
  text:string
  -> Hh_json.json option

val from_text:
  php5_compat_mode: bool ->
  hhvm_compat_mode: bool ->
  force_hh: bool ->
  enable_xhp: bool ->
  filename: Relative_path.t ->
  text:string
  -> Facts.facts option
