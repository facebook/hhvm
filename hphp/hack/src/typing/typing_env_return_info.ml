(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Return type information as gathered from explicit hints or inferred *)
type t = {
  (* Return type itself. Awaitable<t> is represented explicitly, not stripped.
   * In the case of lambdas without an explicit return type hint, the return
   * type is determined by context or is a type variable that may get resolved
   * when checking the body of the lambda.
   *)
  return_type: Typing_defs.locl_ty;
  (* Does the function or function type have the <<__ReturnDisposable>> attribute? *)
  return_disposable: bool;
}
