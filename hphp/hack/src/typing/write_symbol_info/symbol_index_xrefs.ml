(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Add_fact = Symbol_add_fact
module Fact_acc = Symbol_predicate.Fact_acc
module Build = Symbol_build_json
module Predicate = Symbol_predicate
module File_info = Symbol_file_info
module XRefs = Symbol_xrefs
module Sym_def = Symbol_sym_def

let call_handler ~path progress_ref (pos_map : XRefs.pos_map) =
  object (_self)
    inherit Tast_visitor.handler_base

    method! at_Call _env (_, callee_pos, callee_exp) _targs args _unpack_arg =
      let f (_, (_, arg_pos, exp)) =
        let exp_json =
          match exp with
          | Aast.String s
            when String.for_all ~f:(fun c -> Caml.Char.code c < 127) s ->
            (* TODO make this more general *)
            Some (Symbol_build_json.build_argument_lit_json s)
          | Aast.Id (id_pos, _)
          | Aast.Class_const (_, (id_pos, _)) ->
            Option.map
              ~f:(fun XRefs.{ target; _ } ->
                Symbol_build_json.build_argument_xref_json target)
              (XRefs.PosMap.find_opt id_pos pos_map)
          | _ -> None
        in
        (exp_json, arg_pos)
      in
      let call_args =
        Symbol_build_json.build_call_arguments_json (List.map args ~f)
      in
      let (id_pos, receiver_span) =
        match callee_exp with
        | Aast.Id (id_pos, _) -> (Some id_pos, None)
        | Aast.Class_const (_, (id_pos, _)) -> (Some id_pos, None)
        | Aast.(Obj_get ((_, receiver_span, _), (_, _, Id (id_pos, _)), _, _))
          ->
          (Some id_pos, Some receiver_span)
        | _ -> (None, None)
      in
      let dispatch_arg =
        Option.(
          receiver_span >>= fun pos ->
          match Symbol_build_json.build_call_arguments_json [(None, pos)] with
          | [arg] -> Some arg
          | _ -> None)
      in
      let callee_xref_receiver_type_opt =
        Option.(id_pos >>= fun pos -> XRefs.PosMap.find_opt pos pos_map)
      in
      let (callee_xref, receiver_type) =
        match callee_xref_receiver_type_opt with
        | None -> (None, None)
        | Some XRefs.{ target; receiver_type } -> (Some target, receiver_type)
      in
      let (_fact_id, prog) =
        Add_fact.file_call
          ~path
          callee_pos
          ~callee_xref
          ~call_args
          ~dispatch_arg
          ~receiver_type
          !progress_ref
      in
      progress_ref := prog
  end

let process_calls ctx path tast map_pos_decl progress =
  let progress_ref = ref progress in
  let visitor =
    Tast_visitor.iter_with [call_handler ~path progress_ref map_pos_decl]
  in
  visitor#go ctx tast;
  !progress_ref

let process_xref
    decl_fun decl_ref_fun symbol_name pos ?receiver_type (xrefs, prog) =
  let (target_id, prog) = decl_fun symbol_name prog in
  let xref_json = decl_ref_fun target_id in
  let target = Build.build_decl_target_json xref_json in
  let xrefs = XRefs.add xrefs target_id pos XRefs.{ target; receiver_type } in
  (xrefs, prog)

let process_enum_xref symbol_name pos (xrefs, prog) =
  process_xref
    Add_fact.enum_decl
    Build.build_enum_decl_json_ref
    symbol_name
    pos
    (xrefs, prog)

let process_typedef_xref symbol_name pos (xrefs, prog) =
  process_xref
    Add_fact.typedef_decl
    Build.build_typedef_decl_json_ref
    symbol_name
    pos
    (xrefs, prog)

let process_function_xref symbol_name pos (xrefs, prog) =
  process_xref
    Add_fact.func_decl
    Build.build_func_decl_json_ref
    symbol_name
    pos
    (xrefs, prog)

let process_gconst_xref symbol_def pos (xrefs, prog) =
  process_xref
    Add_fact.gconst_decl
    Build.build_gconst_decl_json_ref
    symbol_def
    pos
    (xrefs, prog)

let process_member_xref
    ctx member pos mem_decl_fun ref_fun ?receiver_type (xrefs, prog) =
  let Sym_def.{ name; full_name; kind; _ } = member in
  match Str.split (Str.regexp "::") full_name with
  | [] -> (xrefs, prog)
  | con_name :: _mem_name ->
    let con_name_with_ns = Utils.add_ns con_name in
    (match Sym_def.get_class_by_name ctx con_name_with_ns with
    | `None ->
      Hh_logger.log
        "WARNING: could not find parent container %s processing reference to %s"
        con_name_with_ns
        full_name;
      (xrefs, prog)
    | `Enum ->
      (match kind with
      | SymbolDefinition.ClassConst ->
        let (enum_id, prog) = Add_fact.enum_decl con_name prog in
        process_xref
          (Add_fact.enumerator enum_id)
          Build.build_enumerator_decl_json_ref
          name
          pos
          (xrefs, prog)
      (* This includes references to built-in enum methods *)
      | _ -> (xrefs, prog))
    | `Class cls ->
      let con_kind = Predicate.get_parent_kind cls in
      let (con_type, decl_pred) = Predicate.parent_decl_predicate con_kind in
      let (con_decl_id, prog) =
        Add_fact.container_decl decl_pred con_name prog
      in
      process_xref
        (mem_decl_fun con_type con_decl_id)
        ref_fun
        name
        pos
        ?receiver_type
        (xrefs, prog))

let process_container_xref (con_type, decl_pred) symbol_name pos (xrefs, prog) =
  process_xref
    (Add_fact.container_decl decl_pred)
    (Build.build_container_decl_json_ref con_type)
    symbol_name
    pos
    (xrefs, prog)

let process_attribute_xref ctx File_info.{ occ; def } opt_info (xrefs, prog) =
  let get_con_preds_from_name con_name =
    let con_name_with_ns = Utils.add_ns con_name in
    match Sym_def.get_class_by_name ctx con_name_with_ns with
    | `None ->
      Hh_logger.log
        "WARNING: could not find declaration container %s for attribute reference to %s"
        con_name_with_ns
        con_name;
      None
    | `Enum ->
      Hh_logger.log
        "WARNING: unexpected enum %s processing attribute reference %s"
        con_name_with_ns
        con_name;
      None
    | `Class cls -> Some Predicate.(parent_decl_predicate (get_parent_kind cls))
  in
  (* Process <<__Override>>, for which we write a MethodOverrides fact
     instead of a cross-reference *)
  let SymbolOccurrence.{ name; pos; _ } = occ in
  if String.equal name "__Override" then
    match opt_info with
    | None ->
      Hh_logger.log "WARNING: no override info for <<__Override>> instance";
      (xrefs, prog)
    | Some SymbolOccurrence.{ class_name; method_name; _ } ->
      (match get_con_preds_from_name class_name with
      | None -> (xrefs, prog)
      | Some override_con_pred_types ->
        (match def with
        | None -> (xrefs, prog)
        | Some Sym_def.{ full_name; _ } ->
          (match Str.split (Str.regexp "::") full_name with
          | [] -> (xrefs, prog)
          | base_con_name :: _mem_name ->
            (match get_con_preds_from_name base_con_name with
            | None ->
              Hh_logger.log
                "WARNING: could not compute parent container type for override %s::%s"
                class_name
                method_name;
              (xrefs, prog)
            | Some base_con_pred_types ->
              let (_fid, prog) =
                Add_fact.method_overrides
                  method_name
                  base_con_name
                  (fst base_con_pred_types)
                  class_name
                  (fst override_con_pred_types)
                  prog
              in
              (* Cross-references for overrides could be added to xefs by calling
                 'process_member_xref' here with 'sym_def' and 'occ.pos' *)
              (xrefs, prog)))))
  (* Ignore other built-in attributes *)
  else if String.is_prefix name ~prefix:"__" then
    (xrefs, prog)
  (* Process user-defined attributes *)
  else
    try
      (* Look for a container declaration with the same name as the attribute,
         which will be where it is defined *)
      match get_con_preds_from_name name with
      | None -> (xrefs, prog)
      | Some con_pred_types ->
        process_container_xref con_pred_types name pos (xrefs, prog)
    with
    | e ->
      Hh_logger.log
        "WARNING: error processing reference to attribute %s\n: %s\n"
        name
        (Exn.to_string e);
      (xrefs, prog)

let receiver_type occ =
  let open SymbolOccurrence in
  match occ.type_ with
  | Method (ClassName receiver, _) ->
    Some (Build.build_class_decl_json_nested receiver)
  | _ -> None

(* given symbols occurring in a file, compute the maps of xrefs *)
let process_xrefs ctx symbols prog : XRefs.t * Fact_acc.t =
  let open SymbolOccurrence in
  List.fold
    symbols
    ~init:(XRefs.empty, prog)
    ~f:(fun (xrefs, prog) (File_info.{ occ; def } as sym) ->
      if occ.is_declaration then
        (xrefs, prog)
      else
        let pos = occ.pos in
        match occ.type_ with
        | Attribute info -> process_attribute_xref ctx sym info (xrefs, prog)
        | _ ->
          (match def with
          | None ->
            (* no symbol info - likely dynamic *)
            (match occ.type_ with
            | Method (receiver_class, name) ->
              let (target_id, prog) =
                Add_fact.method_occ receiver_class name prog
              in
              let xref_json = Build.build_method_occ_json_ref target_id in
              let target = Build.build_occ_target_json xref_json in
              let receiver_type = receiver_type occ in
              let xrefs =
                XRefs.add xrefs target_id pos XRefs.{ target; receiver_type }
              in
              (xrefs, prog)
            | _ -> (xrefs, prog))
          | Some (Sym_def.{ name; kind; _ } as sym_def) ->
            let open SymbolDefinition in
            let proc_mem = process_member_xref ctx sym_def pos in
            (match kind with
            | Class ->
              let con_kind =
                Predicate.parent_decl_predicate Predicate.ClassContainer
              in
              process_container_xref con_kind name pos (xrefs, prog)
            | ClassConst ->
              let ref_fun = Build.build_class_const_decl_json_ref in
              proc_mem Add_fact.class_const_decl ref_fun (xrefs, prog)
            | GlobalConst -> process_gconst_xref name pos (xrefs, prog)
            | Enum -> process_enum_xref name pos (xrefs, prog)
            | Function -> process_function_xref name pos (xrefs, prog)
            | Interface ->
              let con_kind =
                Predicate.parent_decl_predicate Predicate.InterfaceContainer
              in
              process_container_xref con_kind name pos (xrefs, prog)
            | Method ->
              let ref_fun = Build.build_method_decl_json_ref in
              process_member_xref
                ctx
                sym_def
                pos
                Add_fact.method_decl
                ref_fun (* TODO just pass the occurrence here *)
                ?receiver_type:(receiver_type occ)
                (xrefs, prog)
            | Property ->
              let ref_fun = Build.build_property_decl_json_ref in
              proc_mem Add_fact.property_decl ref_fun (xrefs, prog)
            | Typeconst ->
              let ref_fun = Build.build_type_const_decl_json_ref in
              proc_mem Add_fact.type_const_decl ref_fun (xrefs, prog)
            | Typedef -> process_typedef_xref name pos (xrefs, prog)
            | Trait ->
              let con_kind =
                Predicate.parent_decl_predicate Predicate.TraitContainer
              in
              process_container_xref con_kind name pos (xrefs, prog)
            | _ -> (xrefs, prog))))

let process_xrefs_and_calls ctx prog File_info.{ path; tast; symbols; _ } =
  Fact_acc.set_ownership_unit prog (Some path);
  let ((XRefs.{ pos_map; _ } as xrefs), prog) =
    process_xrefs ctx symbols prog
  in
  let prog = process_calls ctx path tast pos_map prog in
  (prog, xrefs)
