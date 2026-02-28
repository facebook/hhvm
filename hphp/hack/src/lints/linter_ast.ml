(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Hh_prelude

let go (ast : Nast.program) =
  List.iter ast ~f:(function
      | Stmt (_, Expr (_, p, Import ((Include | IncludeOnce), _))) ->
        Lints_diagnostics.include_use
          p
          "Prefer `require` and `require_once` to `include` and `include_once`."
      | _ -> ())
