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

let get_first_param_name params =
  match params with
  | p :: _ -> p.T.param_name
  | _ -> failwith "native generator requires params"

let emit_generator_method name params =
  gather
    [
      begin
        match name with
        | "send" ->
          let local = Local.Named (get_first_param_name params) in
          gather [instr_contcheck_check; instr_pushl local; instr_contenter]
        | "raise"
        | "throw" ->
          let local = Local.Named (get_first_param_name params) in
          gather [instr_contcheck_check; instr_pushl local; instr_contraise]
        | "next"
        | "rewind" ->
          gather [instr_contcheck_ignore; instr_null; instr_contenter]
        | "valid" -> instr_contvalid
        | "current" -> instr_contcurrent
        | "key" -> instr_contkey
        | "getReturn" -> instr_contgetreturn
        | _ -> failwith "incorrect native generator function"
      end;
      instr_retc;
    ]

let emit_native_opcode_impl name params ua =
  match ua with
  | [
      {
        T.ua_name = (_, "__NativeData");
        T.ua_params = [(_, T.String "HH\\AsyncGenerator")];
      };
    ]
  | [
      {
        T.ua_name = (_, "__NativeData");
        T.ua_params = [(_, T.String "Generator")];
      };
    ] ->
    emit_generator_method name params
  | _ ->
    Emit_fatal.raise_fatal_runtime
      Pos.none
      ("OpCodeImpl attribute is not applicable to " ^ name)

let emit_body scope namespace class_attrs name params ret =
  let body_instrs = emit_native_opcode_impl (snd name) params class_attrs in
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun t -> snd t.T.tp_name)
  in
  let params =
    Emit_param.from_asts
      ~namespace
      ~tparams
      ~generate_defaults:false
      ~scope
      params
  in
  let return_type_info =
    Emit_body.emit_return_type_info ~scope ~skipawaitable:false ret
  in
  Hhas_body.make
    body_instrs
    [] (* decl vars *)
    0
    false
    false
    []
    []
    params
    (Some return_type_info)
    None
    None
