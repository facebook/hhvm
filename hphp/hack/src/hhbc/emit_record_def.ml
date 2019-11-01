open Core_kernel
open Aast
open Hhbc_string_utils

let valid_tc_for_record_field tc =
  match Hhas_type_constraint.name tc with
  | None -> true
  | Some name ->
    (not (is_self name))
    && (not (is_parent name))
    && (not (String.lowercase name = "hh\\this"))
    && (not (String.lowercase name = "callable"))
    && (not (String.lowercase name = "hh\\nothing"))
    && not (String.lowercase name = "hh\\noreturn")

let emit_field namespace (f : Aast.sid * Aast.hint * Tast.expr option) =
  let ((pos, name), hint, expr) = f in
  let otv =
    match expr with
    | Some expr -> Ast_constant_folder.expr_to_opt_typed_value namespace expr
    | None -> None
  in
  let tc =
    Emit_type_hint.hint_to_type_info
      ~kind:Emit_type_hint.Property
      ~nullable:false
      ~skipawaitable:false
      ~tparams:[]
      hint
  in
  if valid_tc_for_record_field (Hhas_type_info.type_constraint tc) then
    (name, tc, otv)
  else
    Emit_fatal.raise_fatal_parse
      pos
      (Printf.sprintf
         "Invalid record field type hint for '%s'"
         (Utils.strip_ns name))

let emit_record_def rd =
  let elaborate (_, name) = Hhbc_id.Record.from_ast_name name in
  let name = elaborate rd.rd_name in
  let parent_name =
    match rd.rd_extends with
    | Some (_, Aast.Happly (name, _)) -> Some name
    | _ -> None
  in
  let parent_name = Option.map ~f:elaborate parent_name in
  Hhas_record_def.make
    name
    rd.rd_abstract
    parent_name
    (List.map rd.rd_fields ~f:(emit_field rd.rd_namespace))

let emit_record_defs_from_program
    (ast : (Closure_convert.hoist_kind * Tast.def) list) =
  let aux (_, d) =
    match d with
    | RecordDef rd -> Some (emit_record_def rd)
    | _ -> None
  in
  List.filter_map ~f:aux ast
