(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Glean_schema.Hack
module Fact_acc = Predicate.Fact_acc

let call_handler ~path fa_ref (pos_map : Xrefs.pos_map) =
  object (_self)
    inherit Tast_visitor.handler_base

    method! at_Call _env call =
      let Aast.{ func = (_, callee_pos, callee_exp); args; _ } = call in
      let f (_, (_, arg_pos, exp)) =
        let arg =
          match exp with
          | Aast.String s
            when String.for_all ~f:(fun c -> Stdlib.Char.code c < 127) s ->
            (* TODO make this more general *)
            Some Argument.(Lit (StringLiteral.Key s))
          | Aast.Id (id_pos, _)
          | Aast.Class_const (_, (id_pos, _)) ->
            (match Xrefs.PosMap.find_opt id_pos pos_map with
            | Some (Xrefs.{ target; _ } :: _) ->
              (* there shouldn't be more than one target for a symbol in that
                 position *)
              Some (Argument.XRef target)
            | _ -> None)
          | _ -> None
        in
        (arg, arg_pos)
      in
      let call_args = Build_fact.call_arguments (List.map args ~f) in
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
    ctx member pos mem_decl_fun ref_fun ?receiver_type (xrefs, fa) =
  let Sym_def.{ name; full_name; kind; _ } = member in
  match Str.split (Str.regexp "::") full_name with
  | [] -> (xrefs, fa)
  | con_name :: _mem_name ->
    let con_name_with_ns = Utils.add_ns con_name in
    (match Sym_def.get_class_by_name ctx con_name_with_ns with
    | `None ->
      Hh_logger.log
        "WARNING: could not find parent container %s processing reference to %s"
        con_name_with_ns
        full_name;
      (xrefs, fa)
    | `Enum ->
      (match kind with
      | SymbolDefinition.ClassConst ->
        let (enum_id, fa) = Add_fact.enum_decl con_name fa in
        process_xref
          (Add_fact.enumerator enum_id)
          (fun x -> Declaration.Enumerator (Enumerator.Id x))
          name
          pos
          (xrefs, fa)
      (* This includes references to built-in enum methods *)
      | _ -> (xrefs, fa))
    | `Class cls ->
      let con_kind = Predicate.get_parent_kind cls.Aast.c_kind in
      let decl_pred = Predicate.parent_decl_predicate con_kind in
      let (con_decl_id, fa) = Add_fact.container_decl decl_pred con_name fa in
      process_xref
        (mem_decl_fun con_kind con_decl_id)
        ref_fun
        name
        pos
        ?receiver_type
        (xrefs, fa))

let process_container_xref (con_type, decl_pred) symbol_name pos (xrefs, fa) =
  process_xref
    (Add_fact.container_decl decl_pred)
    (Predicate.container_ref con_type)
    symbol_name
    pos
    (xrefs, fa)

let process_attribute_xref ctx File_info.{ occ; def } opt_info (xrefs, fa) =
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
    | `Class cls ->
      let parent_kind = Predicate.get_parent_kind cls.Aast.c_kind in
      Some (parent_kind, Predicate.(parent_decl_predicate parent_kind))
  in
  (* Process <<__Override>>, for which we write a MethodOverrides fact
     instead of a cross-reference *)
  let SymbolOccurrence.{ name; pos; _ } = occ in
  if String.equal name "__Override" then
    match opt_info with
    | None ->
      Hh_logger.log "WARNING: no override info for <<__Override>> instance";
      (xrefs, fa)
    | Some SymbolOccurrence.{ class_name; method_name; _ } ->
      (match get_con_preds_from_name class_name with
      | None -> (xrefs, fa)
      | Some override_con_pred_types ->
        (match def with
        | None -> (xrefs, fa)
        | Some Sym_def.{ full_name; _ } ->
          (match Str.split (Str.regexp "::") full_name with
          | [] -> (xrefs, fa)
          | base_con_name :: _mem_name ->
            (match get_con_preds_from_name base_con_name with
            | None ->
              Hh_logger.log
                "WARNING: could not compute parent container type for override %s::%s"
                class_name
                method_name;
              (xrefs, fa)
            | Some base_con_pred_types ->
              let (_fid, fa) =
                Add_fact.method_overrides
                  method_name
                  base_con_name
                  (fst base_con_pred_types)
                  class_name
                  (fst override_con_pred_types)
                  fa
              in
              (* Cross-references for overrides could be added to xefs by calling
                 'process_member_xref' here with 'sym_def' and 'occ.pos' *)
              (xrefs, fa)))))
  (* Ignore other built-in attributes *)
  else if String.is_prefix name ~prefix:"__" then
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

let receiver_type occ =
  let open SymbolOccurrence in
  match occ.type_ with
  | Method (ClassName receiver, _) ->
    Some
      (Declaration.Container
         (ContainerDeclaration.Class_
            (ClassDeclaration.Key
               { ClassDeclaration.name = QName.Key (QName.of_string receiver) })))
  | _ -> None

(* given symbols occurring in a file, compute the maps of xrefs *)
let process_xrefs ctx symbols fa : Xrefs.t * Fact_acc.t =
  let open SymbolOccurrence in
  List.fold
    symbols
    ~init:(Xrefs.empty, fa)
    ~f:(fun (xrefs, fa) (File_info.{ occ; def } as sym) ->
      if occ.is_declaration then
        (xrefs, fa)
      else
        let pos = occ.pos in
        match occ.type_ with
        | Attribute info -> process_attribute_xref ctx sym info (xrefs, fa)
        | _ ->
          (match def with
          | None ->
            (* no symbol info - likely dynamic *)
            (match occ.type_ with
            | Method (receiver_class, name) ->
              let (target_id, fa) =
                Add_fact.method_occ receiver_class name fa
              in
              let receiver_type = receiver_type occ in
              let target =
                XRefTarget.Occurrence
                  Occurrence.{ method_ = MethodOccurrence.Id target_id }
              in
              let xrefs =
                Xrefs.add xrefs target_id pos Xrefs.{ target; receiver_type }
              in
              (xrefs, fa)
            | _ -> (xrefs, fa))
          | Some (Sym_def.{ name; kind; _ } as sym_def) ->
            let open SymbolDefinition in
            let proc_mem = process_member_xref ctx sym_def pos in
            (match kind with
            | Class ->
              process_container_xref
                Predicate.(ClassContainer, Hack ClassDeclaration)
                name
                pos
                (xrefs, fa)
            | ClassConst ->
              let ref_fun x =
                Declaration.ClassConst (ClassConstDeclaration.Id x)
              in
              proc_mem Add_fact.class_const_decl ref_fun (xrefs, fa)
            | GlobalConst -> process_gconst_xref name pos (xrefs, fa)
            | Enum -> process_enum_xref name pos (xrefs, fa)
            | Function -> process_function_xref name pos (xrefs, fa)
            | Interface ->
              process_container_xref
                Predicate.(InterfaceContainer, Hack InterfaceDeclaration)
                name
                pos
                (xrefs, fa)
            | Method ->
              let ref_fun x = Declaration.Method (MethodDeclaration.Id x) in
              process_member_xref
                ctx
                sym_def
                pos
                Add_fact.method_decl
                ref_fun (* TODO just pass the occurrence here *)
                ?receiver_type:(receiver_type occ)
                (xrefs, fa)
            | Property ->
              let ref_fun x =
                Declaration.Property_ (PropertyDeclaration.Id x)
              in
              proc_mem Add_fact.property_decl ref_fun (xrefs, fa)
            | Typeconst ->
              let ref_fun x =
                Declaration.TypeConst (TypeConstDeclaration.Id x)
              in
              proc_mem Add_fact.type_const_decl ref_fun (xrefs, fa)
            | Typedef -> process_typedef_xref name pos (xrefs, fa)
            | Trait ->
              process_container_xref
                Predicate.(TraitContainer, Hack TraitDeclaration)
                name
                pos
                (xrefs, fa)
            | _ -> (xrefs, fa))))

let process_xrefs_and_calls ctx fa File_info.{ path; tast; symbols; _ } =
  Fact_acc.set_ownership_unit fa (Some path);
  let ((Xrefs.{ pos_map; _ } as xrefs), fa) = process_xrefs ctx symbols fa in
  let fa = process_calls ctx path tast pos_map fa in
  (fa, xrefs)
