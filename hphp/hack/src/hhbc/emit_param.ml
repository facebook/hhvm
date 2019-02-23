(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Instruction_sequence
open Ast_class_expr
module A = Ast
module SU = Hhbc_string_utils

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

let from_variadic_param_hint_opt ho =
  let p = Pos.none in
  match ho with
  | None -> Some (p, A.Happly ((p, "array"), []))
  | Some h -> Some (p, A.Happly ((p, "array"), [h]))

let resolve_class_id ~scope cid =
  let cexpr = expr_to_class_expr ~resolve_self:false scope cid in
  match cexpr with
  | Class_id ((p, _) as cid) -> p, A.Id cid
  | Class_special _ | Class_expr _ | Class_reified _ -> cid

let resolver_visitor =
object(_)
  inherit [_] Ast.endo

  method! on_Class_get scope _ cid id =
    let cid = resolve_class_id ~scope cid in
    A.Class_get (cid, id)

  method! on_Class_const scope _ cid id =
    let cid = resolve_class_id ~scope cid in
    A.Class_const (cid, id)

end

let get_hint_display_name hint =
  Option.map hint (fun h ->
    match h with
    |  "HH\\bool"                ->   "bool"
    |  "array"                   ->   "array"
    |  "HH\\varray"              ->   "HH\\varray"
    |  "HH\\darray"              ->   "HH\\darray"
    |  "HH\\varray_or_darray"    ->   "HH\\varray_or_darray"
    |  "HH\\vec_or_dict"         ->   "HH\\vec_or_dict"
    |  "HH\\arraylike"           ->   "HH\\arraylike"
    |  "HH\\int"                 ->   "int"
    |  "HH\\num"                 ->   "num"
    |  "HH\\arraykey"            ->   "arraykey"
    |  "HH\\float"               ->   "float"
    |  "HH\\string"              ->   "string"
    |   _                        ->   "class" )


(* By now only check default type for bool, array, int, float and string.
   Return None when hint_type and default_value matches (in hh mode,
   "class" type matches anything). If not, return default_value type string
   for printing fatal parse error *)
let match_default_and_hint is_hh_file hint_type param_expr =
  let accept_any = match hint_type with
                  | "class" -> is_hh_file
                  | _       -> false
  in
  if accept_any
  then None
  else match param_expr with
      | Some (_, (A.True | A.False)) ->
        begin match hint_type with
               | "bool"   -> None
               | _        -> Some "Boolean"
        end
      | Some (_, A.Array _ ) ->
        begin match hint_type with
             | "array"
             | "HH\\varray"
             | "HH\\darray"
             | "HH\\varray_or_darray" -> None
             | _        -> Some "Array"
        end
      | Some (_, A.Int _ )  ->
        begin match hint_type with
             | "int"
             | "num"
             | "arraykey"
             | "float"  -> None
             | _        -> Some "Int64"
        end
      | Some (_, A.Float _ ) ->
        begin match hint_type with
             | "float"
             | "num"    -> None
             | _        -> Some "Double"
        end
      | Some (_, A.String _ ) ->
        begin match hint_type with
             | "string"
             | "arraykey" -> None
             | _          -> Some "String"
        end

      | _  -> None

(* Return None if it passes type check, otherwise return error msg *)
let default_type_check param_name param_type_info param_expr =
  let hint =
    Option.bind param_type_info Hhas_type_info.user_type
  in
  let hint_type = get_hint_display_name hint in
  (* If matches, return None, otherwise return default_type *)
  let default_type = Option.bind hint_type (function hint_type ->
      match_default_and_hint
      (Emit_env.is_hh_syntax_enabled ())
      hint_type param_expr)
  in
  let param_true_name = Hhbc_string_utils.Locals.strip_dollar param_name
  in
    Option.bind default_type (function t ->
      match hint_type with
      |  Some "class" ->  Some ("Default value for parameter "
      ^ param_true_name ^ " with a class type hint can only be NULL")
      |  Some h       ->  Some ("Default value for parameter "
      ^ param_true_name ^ " with type " ^ t ^
      " needs to have the same type as the type hint " ^ h)
      |  _            ->  None )

let from_ast ~tparams ~namespace ~generate_defaults ~scope p =
  let param_name = snd p.A.param_id in
  let param_is_variadic = p.Ast.param_is_variadic in
  let param_is_inout = p.Ast.param_callconv = Some A.Pinout in
  let param_user_attributes =
    Emit_attribute.from_asts namespace p.Ast.param_user_attributes in
  let param_hint =
    if param_is_variadic
    then from_variadic_param_hint_opt p.Ast.param_hint
    else p.Ast.param_hint
  in
  let tparams = if param_is_variadic then "array"::tparams else tparams in
  let nullable =
    match p.A.param_expr with
    | Some (_, A.Null) -> true
    | _ -> false in
  let param_type_info = Option.map param_hint
    Emit_type_hint.(hint_to_type_info
      ~kind:Param ~skipawaitable:false ~nullable ~namespace ~tparams) in
  (* Do the type check for default value type and hint type *)
  let _ =
    if not nullable then
    match (default_type_check param_name param_type_info p.A.param_expr) with
    | None    -> ()
    | Some s  -> Emit_fatal.raise_fatal_parse (fst p.A.param_id) s
  in
  let param_expr =
    Option.map p.Ast.param_expr ~f:(resolver_visitor#on_expr scope)
  in
  let param_default_value =
    if generate_defaults
    then Option.map param_expr ~f:(fun e -> Label.next_default_arg (), e)
    else None
  in
  if param_is_variadic && param_name = "..." then None else
  Some (Hhas_param.make param_name p.A.param_is_reference param_is_variadic
    param_is_inout param_user_attributes param_type_info param_default_value)

let rename_params params =
  let names = Hh_core.List.fold_left params
    ~init:SSet.empty ~f:(fun n p -> SSet.add (Hhas_param.name p) n) in
  let rec rename param_counts param =
    let name = Hhas_param.name param in
    match SMap.get name param_counts with
    | None ->
      (SMap.add name 0 param_counts, param)
    | Some count ->
      let param_counts = SMap.add name (count + 1) param_counts in
      let newname = name ^ string_of_int count in
      if SSet.mem newname names
      then rename param_counts param
      else param_counts, (Hhas_param.with_name newname param)
  in
    List.rev (snd (Hh_core.List.map_env SMap.empty (List.rev params) rename))

let from_asts ~namespace ~tparams ~generate_defaults ~scope ast_params =
  let hhas_params = List.filter_map ast_params
    (from_ast ~tparams ~namespace ~generate_defaults ~scope) in
  rename_params hhas_params

let emit_param_default_value_setter ?(is_native = false) env pos params =
  let setters = List.filter_map params (fun p ->
    let param_name = Hhas_param.name p in
    let dvo = Hhas_param.default_value p in
    let nop_requirements e = is_native && snd e = A.Null && begin
      let is_optional, is_mixed, is_callable =
        match Hhas_param.type_info p with
        | Some {Hhas_type_info.type_info_user_type = Some s; _} ->
          let is_optional = String_utils.string_starts_with s "?" in
          let s = String_utils.lstrip s "?" in
          is_optional, s = "HH\\mixed", s = "callable"
        | _ -> false, false, false in
      is_mixed ||
      Hhas_param.is_reference p ||
      (is_optional && is_callable)
      end
    in
    Option.map dvo (fun (l, e) ->
      gather [
        instr_label l;
        if nop_requirements e then
          instr_nop
        else gather [
          Emit_expression.emit_expr ~need_ref:false env e;
        Emit_pos.emit_pos pos;
          instr_setl (Local.Named param_name);
          instr_popc;
        ];
      ]) )
  in
  if List.is_empty setters
  then empty, empty
  else
    let l = Label.next_regular () in
    instr_label l, gather [gather setters; instr_jmpns l]

let emit_reified_params tparams =
  let convert_reified_params = function
    | (_, _, _, false) -> None
    | (_, (_, s), _, true) ->
      let param_name = SU.Reified.mangle_reified_param s in
      let type_name = Some "HH\\darray" in
      let tc =
        Hhas_type_constraint.make type_name [Hhas_type_constraint.HHType] in
      let ti = Some (Hhas_type_info.make type_name tc) in
      Some (Hhas_param.make param_name false false false [] ti None) in
  List.filter_map tparams ~f:convert_reified_params
