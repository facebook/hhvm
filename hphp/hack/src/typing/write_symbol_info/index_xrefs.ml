(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hack
module Fact_acc = Predicate.Fact_acc

let process_arg pos_map (exp, arg_pos) =
  let arg =
    match exp with
    | Aast.String s when String.for_all ~f:(fun c -> Stdlib.Char.code c < 127) s
      ->
      (* TODO make this more general *)
      Some Argument.(Lit (StringLiteral.Key s))
    | Aast.Id (id_pos, _)
    | Aast.Class_const (_, (id_pos, _)) ->
      (match Xrefs.PosMap.find_opt id_pos pos_map with
      | Some (Xrefs.{ target; _ } :: _) ->
        (* there shouldn't be more than one target for a symbol in that
           position *)
        Some (Argument.Xref target)
      | _ -> None)
    | _ -> None
  in
  (arg, arg_pos)

let process_call
    ~path
    ~pos_map
    ~fa_ref
    ~callee_pos
    ~id_pos
    ~receiver_span
    ~args
    ~arg_processor =
  let call_args = Build_fact.call_arguments (List.map args ~f:arg_processor) in
  let dispatch_arg =
    Option.(
      receiver_span >>= fun pos ->
      match Build_fact.call_arguments [(None, pos)] with
      | [arg] -> Some arg
      | _ -> None)
  in
  let callee_infos =
    match id_pos with
    | None -> []
    | Some pos ->
      (match Xrefs.PosMap.find_opt pos pos_map with
      | Some l -> l
      | None -> [])
  in
  fa_ref :=
    Add_fact.file_call
      ~path
      callee_pos
      ~callee_infos
      ~call_args
      ~dispatch_arg
      !fa_ref
    |> snd

let call_handler ~path fa_ref (pos_map : Xrefs.pos_map) =
  object (_self)
    inherit Tast_visitor.handler_base

    method! at_expr _env expr =
      match expr with
      | (_, _, Aast.New ((_, callee_pos, _), _, args, _, _)) ->
        let arg_processor arg =
          let (_, arg_pos, exp) = Aast_utils.arg_to_expr arg in
          process_arg pos_map (exp, arg_pos)
        in
        process_call
          ~path
          ~pos_map
          ~fa_ref
          ~callee_pos
          ~id_pos:(Some callee_pos)
          ~receiver_span:None
          ~args
          ~arg_processor
      | _ -> ()

    method! at_Call _env call =
      let Aast.{ func = (_, callee_pos, callee_exp); args; _ } = call in
      let arg_processor arg =
        let (_, arg_pos, exp) = Aast_utils.arg_to_expr arg in
        process_arg pos_map (exp, arg_pos)
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
      process_call
        ~path
        ~pos_map
        ~fa_ref
        ~callee_pos
        ~id_pos
        ~receiver_span
        ~args
        ~arg_processor
  end

let process_calls ctx path tast map_pos_decl fa =
  let fa_ref = ref fa in
  let visitor =
    Tast_visitor.iter_with [call_handler ~path fa_ref map_pos_decl]
  in
  let tast = List.map ~f:fst tast in
  visitor#go ctx tast;
  !fa_ref

let process_xref decl_fun decl_ref_fun symbol_name pos ?receiver_type (xrefs, fa)
    =
  let (target_id, fa) = decl_fun symbol_name fa in
  let target = XRefTarget.Declaration (decl_ref_fun target_id) in
  let xrefs = Xrefs.add xrefs target_id pos Xrefs.{ target; receiver_type } in
  (xrefs, fa)

let process_enum_xref symbol_name pos (xrefs, fa) =
  process_xref
    Add_fact.enum_decl
    (fun x ->
      Declaration.Container (ContainerDeclaration.Enum_ (EnumDeclaration.Id x)))
    symbol_name
    pos
    (xrefs, fa)

let process_typedef_xref symbol_name pos (xrefs, fa) =
  process_xref
    Add_fact.typedef_decl
    (fun x -> Declaration.Typedef_ (TypedefDeclaration.Id x))
    symbol_name
    pos
    (xrefs, fa)

let process_function_xref symbol_name pos (xrefs, fa) =
  process_xref
    Add_fact.func_decl
    (fun x -> Declaration.Function_ (FunctionDeclaration.Id x))
    symbol_name
    pos
    (xrefs, fa)

let process_gconst_xref symbol_def pos (xrefs, fa) =
  process_xref
    Add_fact.gconst_decl
    (fun x -> Declaration.GlobalConst (GlobalConstDeclaration.Id x))
    symbol_def
    pos
    (xrefs, fa)

let process_member_xref
    ctx
    ~class_name
    ~name
    pos
    mem_decl_fun
    ref_fun
    ?receiver_type
    ~class_const
    (xrefs, fa) =
  match Sym_def.get_kind ctx class_name with
  | None ->
    Hh_logger.log
      "WARNING: could not find parent container %s processing reference to %s"
      class_name
      name;
    (xrefs, fa)
  | Some Ast_defs.Cenum
  | Some (Ast_defs.Cenum_class _) ->
    if class_const then
      let (enum_id, fa) = Add_fact.enum_decl class_name fa in
      process_xref
        (Add_fact.enumerator enum_id)
        (fun x -> Declaration.Enumerator (Enumerator.Id x))
        name
        pos
        (xrefs, fa)
    else
      (xrefs, fa)
  | Some cls ->
    let (class_kind, decl_pred) = Predicate.classish_to_predicate cls in
    let (class_decl_id, fa) = Add_fact.container_decl decl_pred class_name fa in
    process_xref
      (mem_decl_fun class_kind class_decl_id)
      ref_fun
      name
      pos
      ?receiver_type
      (xrefs, fa)

let process_container_xref (con_type, decl_pred) symbol_name pos (xrefs, fa) =
  process_xref
    (Add_fact.container_decl decl_pred)
    (Predicate.container_ref con_type)
    symbol_name
    pos
    (xrefs, fa)

let process_attribute_xref ctx File_info.{ occ; _ } (xrefs, fa) =
  let get_con_preds_from_name con_name =
    match Sym_def.get_kind ctx con_name with
    | None ->
      Hh_logger.log
        "WARNING: could not find declaration container %s for attribute reference"
        con_name;
      None
    | Some (Ast_defs.Cenum_class _)
    | Some Ast_defs.Cenum ->
      Hh_logger.log "WARNING: unexpected enum %s" con_name;
      None
    | Some cls ->
      let parent_kind = Predicate.get_parent_kind cls in
      Some (parent_kind, Predicate.(parent_decl_predicate parent_kind))
  in
  (* Process <<__Override>>, for which we write a MethodOverrides fact
     instead of a cross-reference *)
  let SymbolOccurrence.{ name; pos; _ } = occ in
  if String.is_prefix name ~prefix:"__" then
    (xrefs, fa)
  (* Process user-defined attributes *)
  else
    try
      (* Look for a container declaration with the same name as the attribute,
         which will be where it is defined *)
      match get_con_preds_from_name name with
      | None -> (xrefs, fa)
      | Some con_pred_types ->
        process_container_xref con_pred_types name pos (xrefs, fa)
    with
    | e ->
      Hh_logger.log
        "WARNING: error processing reference to attribute %s\n: %s\n"
        name
        (Exn.to_string e);
      (xrefs, fa)

let receiver_type ctx occ =
  let open SymbolOccurrence in
  match occ.type_ with
  | Method (ClassName receiver, _) ->
    let qname = Util.make_qname receiver in
    (match Sym_def.get_kind ctx receiver with
    | None -> None
    | Some Ast_defs.Cenum
    | Some (Ast_defs.Cenum_class _) ->
      Some
        (Declaration.Container
           (ContainerDeclaration.Enum_
              (EnumDeclaration.Key { EnumDeclaration.name = qname })))
    | Some Ast_defs.Cinterface ->
      Some
        (Declaration.Container
           (ContainerDeclaration.Interface_
              (InterfaceDeclaration.Key { InterfaceDeclaration.name = qname })))
    | Some Ast_defs.Ctrait ->
      Some
        (Declaration.Container
           (ContainerDeclaration.Trait
              (TraitDeclaration.Key { TraitDeclaration.name = qname })))
    | Some (Ast_defs.Cclass _) ->
      Some
        (Declaration.Container
           (ContainerDeclaration.Class_
              (ClassDeclaration.Key { ClassDeclaration.name = qname }))))
  | _ -> None

(* given symbols occurring in a file, compute the maps of xrefs *)
let process_xrefs ctx symbols fa : Xrefs.t * Fact_acc.t =
  let open SymbolOccurrence in
  List.fold
    symbols
    ~init:(Xrefs.empty, fa)
    ~f:(fun (xrefs, fa) (File_info.{ occ; def } as sym) ->
      if Option.is_some occ.is_declaration then
        (xrefs, fa)
      else
        let pos = occ.pos in
        match occ.type_ with
        | Attribute _info -> process_attribute_xref ctx sym (xrefs, fa)
        | _ ->
          (match def with
          | None ->
            (* no symbol info - likely dynamic *)
            (match occ.type_ with
            | Method (receiver_class, name) ->
              let (target_id, fa) =
                Add_fact.method_occ receiver_class name fa
              in
              let receiver_type = receiver_type ctx occ in
              let target =
                XRefTarget.Occurrence
                  (Occurrence.Method (MethodOccurrence.Id target_id))
              in
              let xrefs =
                Xrefs.add xrefs target_id pos Xrefs.{ target; receiver_type }
              in
              (xrefs, fa)
            | _ -> (xrefs, fa))
          | Some sym_def ->
            let open Sym_def in
            (match sym_def with
            | Class { kind = Ast_defs.Cenum; name }
            | Class { kind = Ast_defs.Cenum_class _; name } ->
              process_enum_xref name pos (xrefs, fa)
            | Class { kind; name } ->
              process_container_xref
                (Predicate.classish_to_predicate kind)
                name
                pos
                (xrefs, fa)
            | ClassConst { class_name; name } ->
              let ref_fun x =
                Declaration.ClassConst (ClassConstDeclaration.Id x)
              in
              process_member_xref
                ctx
                ~class_name
                ~name
                pos
                Add_fact.class_const_decl
                ref_fun
                ~class_const:true
                (xrefs, fa)
            | GlobalConst { name } -> process_gconst_xref name pos (xrefs, fa)
            | Function { name } -> process_function_xref name pos (xrefs, fa)
            | Method { class_name; name } ->
              let ref_fun x = Declaration.Method (MethodDeclaration.Id x) in
              process_member_xref
                ctx
                ~class_name
                ~name
                pos
                Add_fact.method_decl
                ref_fun (* TODO just pass the occurrence here *)
                ?receiver_type:(receiver_type ctx occ)
                ~class_const:false
                (xrefs, fa)
            | Property { class_name; name } ->
              let ref_fun x =
                Declaration.Property_ (PropertyDeclaration.Id x)
              in
              process_member_xref
                ctx
                ~class_name
                ~name
                pos
                Add_fact.property_decl
                ref_fun
                ~class_const:false
                (xrefs, fa)
            | Typeconst { class_name; name } ->
              let ref_fun x =
                Declaration.TypeConst (TypeConstDeclaration.Id x)
              in
              process_member_xref
                ctx
                ~class_name
                ~name
                pos
                Add_fact.type_const_decl
                ref_fun
                ~class_const:false
                (xrefs, fa)
            | Typedef { name } -> process_typedef_xref name pos (xrefs, fa)
            | _ -> (xrefs, fa))))

let process_xrefs_and_calls ctx fa File_info.{ path; tast; symbols; _ } =
  Fact_acc.set_ownership_unit fa (Some path);
  let ((Xrefs.{ pos_map; _ } as xrefs), fa) = process_xrefs ctx symbols fa in
  let fa = process_calls ctx path tast pos_map fa in
  (fa, xrefs)
