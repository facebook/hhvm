(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  constant_name: string;
  constant_value: Typed_value.t option;
  constant_initializer_instrs: Instruction_sequence.t option;
}

(* Interestingly, HHAS does not represent the declared types of constants,
unlike formal parameters and return types. We might consider fixing this. *)

(* Also interestingly, abstract constants are not emitted at all. *)

let make constant_name constant_value constant_initializer_instrs =
  { constant_name; constant_value; constant_initializer_instrs }

let name hhas_constant = hhas_constant.constant_name

let value hhas_constant = hhas_constant.constant_value

let initializer_instrs hhas_constant = hhas_constant.constant_initializer_instrs

let from_ast env name expr =
  let (_, name) = name in
  let (value, init_instrs) =
    match expr with
    | None -> (None, None)
    | Some init ->
      (match
         Ast_constant_folder.expr_to_opt_typed_value
           (Emit_env.get_namespace env)
           init
       with
      | Some v -> (Some v, None)
      | None ->
        (Some Typed_value.Uninit, Some (Emit_expression.emit_expr env init)))
  in
  make name value init_instrs
