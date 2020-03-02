(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Instruction_sequence
module T = Aast

let emit_constant_cinit env (ast_constant : Tast.gconst) (c : Hhas_constant.t) :
    Hhas_function.t option =
  let (ns, name) =
    Hhas_constant.name c
    |> Hhbc_id.Const.from_ast_name
    |> Hhbc_id.Const.to_raw_string
    |> Utils.split_ns_from_name
  in
  let name = ns ^ "86cinit_" ^ name in
  let original_id = Hhbc_id.Function.from_ast_name name in
  let ret = ast_constant.T.cst_type in
  let return_type_info =
    match ret with
    | None -> None
    | Some h ->
      Some
        (Emit_type_hint.hint_to_type_info
           ~kind:Emit_type_hint.Return
           ~skipawaitable:false
           ~nullable:false
           ~tparams:[]
           h)
  in
  (* let return_type_info = ast_constant.T.cst_type in *)
  let instrs = Hhas_constant.initializer_instrs c in
  match instrs with
  | None -> None
  | Some instrs ->
    let verify_instr =
      match return_type_info with
      | None -> Instr_empty
      | Some _ -> instr_verifyRetTypeC
    in
    let instrs = gather [instrs; verify_instr; instr_retc] in
    let body =
      Emit_body.make_body
        instrs
        [] (* decl_vars *)
        false (* is_memoize_wrapper *)
        false (* is_memoize_wrapper_lsb *)
        [] (* upper_bounds *)
        [] (* shadowed_tparams *)
        [] (* params *)
        return_type_info
        None (* doc_comment *)
        (Some env)
    in
    Some
      (Hhas_function.make
         [] (* attributes *)
         original_id
         body
         (Hhas_pos.pos_to_span ast_constant.T.cst_span)
         false (* is_async *)
         false (* is_generator *)
         false (* is_pair_generator *)
         Closure_convert.TopLevel
         true (* no_injection *)
         false (* is_interceptable *)
         false (* is_memoize_impl *)
         Rx.NonRx
         false (* rx_disabled *))

let emit_constant env (ast_constant : Tast.gconst) :
    Hhas_constant.t * Hhas_function.t option =
  let c =
    Hhas_constant.from_ast
      env
      ast_constant.T.cst_name
      (Some ast_constant.T.cst_value)
  in
  let f = emit_constant_cinit env ast_constant c in
  (c, f)

let emit_constants_from_program env (ast : Tast.def list) :
    Hhas_constant.t list * Hhas_function.t list =
  let aux def =
    match def with
    | T.Constant c -> Some (emit_constant env c)
    | _ -> None
  in
  let constant_tuples = List.filter_map ast ~f:aux in
  let (constants, constant_cinits_optionals) = List.unzip constant_tuples in
  let constant_cinits = List.filter_opt constant_cinits_optionals in
  (constants, constant_cinits)
