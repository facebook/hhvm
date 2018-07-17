(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module WithSyntax : functor (Syntax : Syntax_sig.Syntax_S) -> sig
module WithSmartConstructors : functor (SmartConstructors : SmartConstructors.SmartConstructors_S
  with type r = Syntax.t
  with module Token = Syntax.Token
) -> sig
  type error_level = Minimum | Typical | Maximum

  type hhvm_compat_mode = NoCompat | HHVMCompat | SystemLibCompat

  type env
  val make_env :
    (* Optional parts *)
       ?level:error_level
    -> ?hhvm_compat_mode:hhvm_compat_mode
    -> ?enable_hh_syntax:bool
    -> ?hhi_mode:bool
    (* Required parts *)
    -> Full_fidelity_syntax_tree.WithSyntax(Syntax).WithSmartConstructors(SmartConstructors).t
    -> codegen:bool
    -> env

  val parse_errors : env -> Full_fidelity_syntax_error.t list

end (* WithSmartConstructors *)

include module type of WithSmartConstructors(SyntaxSmartConstructors.WithSyntax(Syntax))

end (* WithSyntax *)
