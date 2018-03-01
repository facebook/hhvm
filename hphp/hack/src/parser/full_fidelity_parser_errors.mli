(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module WithSyntax : functor (Syntax : Syntax_sig.Syntax_S) -> sig
  type error_level = Minimum | Typical | Maximum

  type hhvm_compat_mode = NoCompat | HHVMCompat | SystemLibCompat

  type env
  val make_env :
    (* Optional parts *)
       ?level:error_level
    -> ?hhvm_compat_mode:hhvm_compat_mode
    -> ?enable_hh_syntax:bool
    (* Required parts *)
    -> Full_fidelity_syntax_tree.WithSyntax(Syntax).t
    -> env

  val parse_errors : env -> Full_fidelity_syntax_error.t list
end
