(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* TODO(jakebailey): Can/should this be done with the direct decl parser? *)
val lambda_decl_in_env : Decl_env.env -> Nast.fun_ -> Typing_defs.fun_elt
