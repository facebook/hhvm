(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let should_check_error mode code =
  match mode with
  | FileInfo.Mstrict -> true
  | FileInfo.Mpartial -> Errors.is_strict_code code
  | FileInfo.Mdecl -> false
  | FileInfo.Mphp -> false
