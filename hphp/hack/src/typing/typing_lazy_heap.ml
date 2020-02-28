(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let check_cache_consistency x expected_kind expected_result =
  if
    Relative_path.equal expected_result Relative_path.default
    && (not @@ Naming_heap.has_local_changes ())
  then (
    Hh_logger.log "WARNING: found dummy path in shared heap for %s" x;
    match Naming_heap.Types.get_pos ~bypass_cache:true x with
    | Some (pos, kind)
      when Naming_types.equal_kind_of_type kind expected_kind
           && Relative_path.equal
                (FileInfo.get_pos_filename pos)
                expected_result ->
      ()
    | _ ->
      Hh_logger.log "WARNING: get and get_no_cache returned different results"
  )

let get_type_id_filename x expected_kind =
  Counters.count_decl_accessor @@ fun () ->
  match Naming_heap.Types.get_filename_and_kind x with
  | Some (fn, kind) when Naming_types.equal_kind_of_type kind expected_kind ->
    check_cache_consistency x expected_kind fn;
    Some fn
  | _ -> None

let get_class = Typing_classes_heap.Classes.get

let get_fun ctx x =
  Counters.count_decl_accessor @@ fun () ->
  match Typing_heap.Funs.get x with
  | Some c -> Some c
  | None ->
    (match Naming_heap.Funs.get_filename x with
    | Some filename ->
      let ft =
        Errors.run_in_decl_mode filename (fun () ->
            Decl.declare_fun_in_file ctx filename x)
      in
      Some ft
    | None -> None)

let get_gconst ctx cst_name =
  Counters.count_decl_accessor @@ fun () ->
  match Typing_heap.GConsts.get cst_name with
  | Some c -> Some c
  | None ->
    (match Naming_provider.get_const_path cst_name with
    | Some filename ->
      let gconst =
        Errors.run_in_decl_mode filename (fun () ->
            Decl.declare_const_in_file ctx filename cst_name)
      in
      Some gconst
    | None -> None)

let get_record_def ctx x =
  Counters.count_decl_accessor @@ fun () ->
  match Typing_heap.RecordDefs.get x with
  | Some c -> Some c
  | None ->
    (match get_type_id_filename x Naming_types.TRecordDef with
    | Some filename ->
      let tdecl =
        Errors.run_in_decl_mode filename (fun () ->
            Decl.declare_record_def_in_file ctx filename x)
      in
      Some tdecl
    | None -> None)

let get_typedef ctx x =
  Counters.count_decl_accessor @@ fun () ->
  match Typing_heap.Typedefs.get x with
  | Some c -> Some c
  | None ->
    (match get_type_id_filename x Naming_types.TTypedef with
    | Some filename ->
      let tdecl =
        Errors.run_in_decl_mode filename (fun () ->
            Decl.declare_typedef_in_file ctx filename x)
      in
      Some tdecl
    | None -> None)
