(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Checks if a function/method can be memoized. If the function cannot be
 * memoized this will add an error to the gloabl error list
 *)
val check_function: Typing_env.env -> Nast.fun_ -> unit
val check_method: Typing_env.env -> Nast.method_ -> unit
