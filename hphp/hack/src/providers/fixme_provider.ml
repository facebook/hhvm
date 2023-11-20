(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Fixme_store = Provider_backend.Fixme_store
open Provider_backend.Fixmes

(*****************************************************************************)
(* Table containing all the HH_FIXMEs found in the source code.
 * Associates:
 *   filename =>
 *   line number guarded by HH_FIXME =>
 *   error_node_number =>
 *   position of HH_FIXME comment
 *)
(*****************************************************************************)

type fixme_map = Provider_backend.fixme_map

module HH_FIXMES =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (Relative_path.S)
    (struct
      type t = fixme_map

      let description = "Fixme_HH_FIXMES"
    end)
    (struct
      let capacity = 1000
    end)

module DECL_HH_FIXMES =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (Relative_path.S)
    (struct
      type t = fixme_map

      let description = "Fixme_DECL_HH_FIXMES"
    end)
    (struct
      let capacity = 1000
    end)

module DISALLOWED_FIXMES =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (Relative_path.S)
    (struct
      type t = fixme_map

      let description = "Fixme_DISALLOWED_FIXMES"
    end)
    (struct
      let capacity = 1000
    end)

let get_fixmes filename =
  match Provider_backend.get () with
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    (match HH_FIXMES.get filename with
    | None -> DECL_HH_FIXMES.get filename
    | Some x -> Some x)
  | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
    (match Fixme_store.get fixmes.hh_fixmes filename with
    | None -> Fixme_store.get fixmes.decl_hh_fixmes filename
    | Some x -> Some x)

let get_hh_fixmes filename =
  match Provider_backend.get () with
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    HH_FIXMES.get filename
  | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
    Fixme_store.get fixmes.hh_fixmes filename

let get_decl_hh_fixmes filename =
  match Provider_backend.get () with
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    DECL_HH_FIXMES.get filename
  | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
    Fixme_store.get fixmes.decl_hh_fixmes filename

let get_disallowed_fixmes filename =
  match Provider_backend.get () with
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    DISALLOWED_FIXMES.get filename
  | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
    Fixme_store.get fixmes.disallowed_fixmes filename

let provide_hh_fixmes filename fixme_map =
  if not (IMap.is_empty fixme_map) then
    match Provider_backend.get () with
    | Provider_backend.Analysis
    | Provider_backend.Rust_provider_backend _
    | Provider_backend.Pessimised_shared_memory _
    | Provider_backend.Shared_memory ->
      HH_FIXMES.add filename fixme_map
    | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
      Fixme_store.add fixmes.hh_fixmes filename fixme_map

let provide_decl_hh_fixmes filename fixme_map =
  if not (IMap.is_empty fixme_map) then
    match Provider_backend.get () with
    | Provider_backend.Analysis
    | Provider_backend.Rust_provider_backend _
    | Provider_backend.Pessimised_shared_memory _
    | Provider_backend.Shared_memory ->
      DECL_HH_FIXMES.add filename fixme_map
    | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
      Fixme_store.add fixmes.decl_hh_fixmes filename fixme_map

let provide_disallowed_fixmes filename fixme_map =
  if not (IMap.is_empty fixme_map) then
    match Provider_backend.get () with
    | Provider_backend.Analysis
    | Provider_backend.Rust_provider_backend _
    | Provider_backend.Pessimised_shared_memory _
    | Provider_backend.Shared_memory ->
      DISALLOWED_FIXMES.add filename fixme_map
    | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
      Fixme_store.add fixmes.disallowed_fixmes filename fixme_map

let remove_batch paths =
  match Provider_backend.get () with
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    HH_FIXMES.remove_batch paths;
    DECL_HH_FIXMES.remove_batch paths;
    DISALLOWED_FIXMES.remove_batch paths
  | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
    Fixme_store.remove_batch fixmes.hh_fixmes paths;
    Fixme_store.remove_batch fixmes.decl_hh_fixmes paths;
    Fixme_store.remove_batch fixmes.disallowed_fixmes paths

let local_changes_push_sharedmem_stack () =
  HH_FIXMES.LocalChanges.push_stack ();
  DECL_HH_FIXMES.LocalChanges.push_stack ();
  DISALLOWED_FIXMES.LocalChanges.push_stack ()

let local_changes_pop_sharedmem_stack () =
  HH_FIXMES.LocalChanges.pop_stack ();
  DECL_HH_FIXMES.LocalChanges.pop_stack ();
  DISALLOWED_FIXMES.LocalChanges.pop_stack ()

let fixme_was_applied applied_fixmes fn err_line err_code =
  match Relative_path.Map.find_opt applied_fixmes fn with
  | None -> false
  | Some r ->
    (match IMap.find_opt err_line r with
    | None -> false
    | Some code_set -> ISet.mem err_code code_set)

let add_applied_fixme_file m err_code err_line =
  let line_value =
    match IMap.find_opt err_line m with
    | None -> ISet.empty
    | Some x -> x
  in
  IMap.add err_line (ISet.add err_code line_value) m

let add_applied_fixme applied_fixmes err_code fn err_line =
  let file_value =
    match Relative_path.Map.find_opt applied_fixmes fn with
    | None -> IMap.empty
    | Some x -> x
  in
  Relative_path.Map.add
    applied_fixmes
    ~key:fn
    ~data:(add_applied_fixme_file file_value err_code err_line)

let get_unused_fixmes_for codes applied_fixme_map fn acc =
  match get_fixmes fn with
  | None -> acc
  | Some fixme_map ->
    IMap.fold
      (fun line code_map acc ->
        IMap.fold
          (fun code fixme_pos acc ->
            if
              (List.mem codes code ~equal:( = )
              || (List.is_empty codes && code < 5000))
              && not (fixme_was_applied applied_fixme_map fn line code)
            then
              fixme_pos :: acc
            else
              acc)
          code_map
          acc)
      fixme_map
      acc

let get_unused_fixmes ~codes ~applied_fixmes ~fold ~files_info =
  let applied_fixme_map =
    List.fold_left
      applied_fixmes
      ~init:Relative_path.Map.empty
      ~f:(fun acc (pos, code) ->
        let fn = Pos.filename pos in
        let (line, _, _) = Pos.info_pos pos in
        add_applied_fixme acc code fn line)
  in
  fold files_info ~init:[] ~f:(fun fn _ acc ->
      get_unused_fixmes_for codes applied_fixme_map fn acc)

(*****************************************************************************)
(* We register the function that can look up a position and determine if
 * a given position is affected by an HH_FIXME. We use a reference to avoid
 * a cyclic dependency: everything depends on the Errors module (the module
 * defining all the errors), because of that making the Errors module call
 * into anything that isn't in the standard library is very unwise, because
 * that code won't be able to add errors.
 *)
(*****************************************************************************)
let get_fixmes_for_pos pos =
  let filename = Pos.filename pos in
  let (line, _, _) = Pos.info_pos pos in
  get_fixmes filename
  |> Option.value ~default:IMap.empty
  |> IMap.find_opt line
  |> Option.value ~default:IMap.empty

let get_fixme_codes_for_pos pos =
  get_fixmes_for_pos pos |> IMap.keys |> ISet.of_list

let is_disallowed pos code =
  let filename = Pos.filename pos in
  let (line, _, _) = Pos.info_pos pos in
  let fixme_map_opt =
    match Provider_backend.get () with
    | Provider_backend.Analysis
    | Provider_backend.Rust_provider_backend _
    | Provider_backend.Pessimised_shared_memory _
    | Provider_backend.Shared_memory ->
      DISALLOWED_FIXMES.get filename
    | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
      Fixme_store.get fixmes.disallowed_fixmes filename
  in
  fixme_map_opt
  |> Option.value ~default:IMap.empty
  |> IMap.find_opt line
  |> Option.value ~default:IMap.empty
  |> IMap.find_opt code

let () =
  (Errors.get_hh_fixme_pos :=
     fun err_pos err_code ->
       get_fixmes_for_pos err_pos |> fun imap ->
       if !Errors.code_agnostic_fixme then
         if IMap.is_empty imap then
           None
         else
           Some err_pos
       else
         IMap.find_opt err_code imap);
  (Errors.is_hh_fixme :=
     fun err_pos err_code ->
       Option.is_some (!Errors.get_hh_fixme_pos err_pos err_code));
  Errors.is_hh_fixme_disallowed :=
    (fun err_pos err_code -> Option.is_some (is_disallowed err_pos err_code))
