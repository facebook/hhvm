(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 * This module contains smart constructors implementation that can be used to
 * build AST.

 *)

open Core_kernel

module WithSyntax (Syntax : Syntax_sig.Syntax_S) = struct
  module Token = Syntax.Token

  type t = Syntax.t list [@@deriving show]

  type r = Syntax.t [@@deriving show]

  exception NotEquals of string * Syntax.t list * Syntax.t list * Syntax.t list

  exception
    NotPhysicallyEquals of
      string * Syntax.t list * Syntax.t list * Syntax.t list

  let verify ~stack params args cons_name =
    let equals e1 e2 =
      if not (phys_equal e1 e2) then
        if e1 = e2 then
          raise @@ NotPhysicallyEquals (cons_name, List.rev stack, params, args)
        else
          raise @@ NotEquals (cons_name, List.rev stack, params, args)
    in
    List.iter2_exn ~f:equals params args

  let rust_parse = Syntax.rust_parse_with_verify_sc

  let initial_state _ = []
end
