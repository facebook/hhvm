(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Hh_core
open Instruction_sequence

let autoprime_generators () =
  Hhbc_options.autoprime_generators !Hhbc_options.compiler_options

let get_first_param_name = function
  | p :: _ -> snd p.A.param_id
  | _ -> failwith "native generator requires params"

let emit_generator_method name params is_async =
  let name = match name with
    | "rewind" when not @@ autoprime_generators () -> "next"
    | "throw" -> "raise"
    | _ -> name in
  let prep = if not is_async && autoprime_generators () then
      let label = Label.next_regular () in
      gather [
        (* Check if the generator has started yet *)
        instr_contstarted;
        instr_jmpnz label;
        (* If it hasn't started, perform one "next" operation before
          the actual operation (auto-priming) *)
        instr_contcheck_ignore;
        instr_null;
        instr_contenter;
        instr_popc;
        instr_label label
      ]
    else empty in
  let body =
    match name with
    | "send" ->
      let local = Local.Named (get_first_param_name params) in
      gather [
        instr_contcheck_check;
        instr_pushl local;
        instr_contenter;
        instr_nop;
        instr_retc
      ]
    | "raise" ->
      let local = Local.Named (get_first_param_name params) in
      gather [
        instr_contcheck_check;
        instr_pushl local;
        instr_contraise;
        instr_nop;
        instr_retc
      ]
    | "next" ->
      gather [
        instr_contcheck_ignore;
        instr_null;
        instr_contenter;
        instr_nop;
        instr_retc
      ]
    | _ ->
      gather [
        begin match name with
          | "valid" -> instr_contvalid
          | "current" -> instr_contcurrent
          | "key" -> instr_contkey
          | "rewind" -> instr_null
          | "getReturn" -> instr_contgetreturn
          | _ -> failwith "incorrect native generator function"
        end;
        instr_retc
      ]
  in
  gather [ prep; body ]

let emit_native_opcode_impl name params = function
  | [{ A.ua_name = (_, "__NativeData");
       A.ua_params = [_, A.String "HH\\AsyncGenerator"]}] ->
    emit_generator_method name params true
  | [{ A.ua_name = (_, "__NativeData");
       A.ua_params = [_, A.String "Generator"]}] ->
    emit_generator_method name params false
  | _ ->
    Emit_fatal.raise_fatal_runtime
      Pos.none
      ("OpCodeImpl attribute is not applicable to " ^ name)

let emit_body scope namespace class_attrs name params ret =
  let body_instrs = emit_native_opcode_impl (snd name) params class_attrs in
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _, _) -> s) in
  let params =
    Emit_param.from_asts
      ~namespace ~tparams ~generate_defaults:false ~scope params
  in
  let return_type_info =
    Emit_body.emit_return_type_info
      ~scope ~skipawaitable:false ~namespace ret in
  Hhas_body.make
    body_instrs
    [] (* decl vars *)
    0
    0
    false
    false
    params
    (Some return_type_info)
    []
    None
    None
