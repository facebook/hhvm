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

module FixmeMap = Provider_backend.FixmeMap

module HH_FIXMES =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (Relative_path.S)
    (struct
      type t = FixmeMap.t

      let description = "Fixme_HH_FIXMES"
    end)
    (struct
      let capacity = 1000
    end)

module IGNORES =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (Relative_path.S)
    (struct
      type t = FixmeMap.t

      let description = "Fixme_IGNORES"
    end)
    (struct
      let capacity = 1000
    end)

module DECL_HH_FIXMES =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (Relative_path.S)
    (struct
      type t = FixmeMap.t

      let description = "Fixme_DECL_HH_FIXMES"
    end)
    (struct
      let capacity = 1000
    end)

module DISALLOWED_FIXMES =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (Relative_path.S)
    (struct
      type t = FixmeMap.t

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

let get_ignores filename =
  match Provider_backend.get () with
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    IGNORES.get filename
  | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
    Fixme_store.get fixmes.ignores filename

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

let provide_ignores filename fixme_map =
  if not (IMap.is_empty fixme_map) then
    match Provider_backend.get () with
    | Provider_backend.Analysis
    | Provider_backend.Rust_provider_backend _
    | Provider_backend.Pessimised_shared_memory _
    | Provider_backend.Shared_memory ->
      IGNORES.add filename fixme_map
    | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
      Fixme_store.add fixmes.ignores filename fixme_map

let remove_batch paths =
  match Provider_backend.get () with
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    HH_FIXMES.remove_batch paths;
    DECL_HH_FIXMES.remove_batch paths;
    DISALLOWED_FIXMES.remove_batch paths;
    IGNORES.remove_batch paths;
    ()
  | Provider_backend.Local_memory { Provider_backend.fixmes; _ } ->
    Fixme_store.remove_batch fixmes.hh_fixmes paths;
    Fixme_store.remove_batch fixmes.decl_hh_fixmes paths;
    Fixme_store.remove_batch fixmes.disallowed_fixmes paths;
    Fixme_store.remove_batch fixmes.ignores paths;
    ()

let local_changes_push_sharedmem_stack () =
  HH_FIXMES.LocalChanges.push_stack ();
  DECL_HH_FIXMES.LocalChanges.push_stack ();
  DISALLOWED_FIXMES.LocalChanges.push_stack ();
  IGNORES.LocalChanges.push_stack ();
  ()

let local_changes_pop_sharedmem_stack () =
  HH_FIXMES.LocalChanges.pop_stack ();
  DECL_HH_FIXMES.LocalChanges.pop_stack ();
  DISALLOWED_FIXMES.LocalChanges.pop_stack ();
  IGNORES.LocalChanges.pop_stack ();
  ()

module UnusedFixmes = struct
  module LineToCodesMap = struct
    (** Mapping error lines to sets of codes *)
    type t = ISet.t IMap.t

    let find_or_default line m =
      IMap.find_opt line m |> Option.value ~default:ISet.empty

    let add (line : int) (code : int) (m : t) =
      IMap.add line (ISet.add code (find_or_default line m)) m
  end

  module FileToLineToCodesMap = struct
    type t = LineToCodesMap.t Relative_path.Map.t

    let find_or_default fn m =
      Relative_path.Map.find_opt m fn |> Option.value ~default:IMap.empty

    let mem fn line code m =
      match Relative_path.Map.find_opt m fn with
      | None -> false
      | Some m ->
        (match IMap.find_opt line m with
        | None -> false
        | Some s -> ISet.mem code s)

    let add (fn : Relative_path.t) (line : int) (code : int) (m : t) =
      Relative_path.Map.add
        m
        ~key:fn
        ~data:(LineToCodesMap.add line code (find_or_default fn m))

    let empty : t = Relative_path.Map.empty

    let make (pos_codes : (Pos.t * int) list) =
      List.fold pos_codes ~init:empty ~f:(fun acc (pos, code) ->
          let fn = Pos.filename pos in
          let (line, _, _) = Pos.info_pos pos in
          add fn line code acc)
  end

  let fixme_was_applied
      (applied_fixmes : FileToLineToCodesMap.t) fn err_line err_code =
    FileToLineToCodesMap.mem fn err_line err_code applied_fixmes

  let code_is_concerned codes code =
    List.mem codes code ~equal:Int.equal
    || List.is_empty codes
       && (code < 5000 || Error_codes.Warning.of_enum code |> Option.is_some)

  let get_unused_fixmes_in_file
      get_fixmes
      codes
      (applied_fixmes : FileToLineToCodesMap.t)
      (fn : Relative_path.t)
      acc =
    match get_fixmes fn with
    | None -> acc
    | Some fixme_map ->
      FixmeMap.fold fixme_map ~init:acc ~f:(fun acc err_line code fixme_pos ->
          if
            code_is_concerned codes code
            && not (fixme_was_applied applied_fixmes fn err_line code)
          then
            fixme_pos :: acc
          else
            acc)

  let get
      ~(codes : int list)
      ~(applied_fixmes : (Pos.t * int) list)
      ~(fold :
         'files ->
         init:Pos.t list ->
         f:(Relative_path.t -> 'unused -> Pos.t list -> Pos.t list) ->
         Pos.t list)
      ~(files : 'files) : Pos.t list =
    let applied_fixmes = FileToLineToCodesMap.make applied_fixmes in
    fold files ~init:[] ~f:(fun fn _ acc ->
        acc
        |> get_unused_fixmes_in_file get_fixmes codes applied_fixmes fn
        |> get_unused_fixmes_in_file get_ignores codes applied_fixmes fn)
end

(*****************************************************************************)
(* We register the function that can look up a position and determine if
 * a given position is affected by an HH_FIXME. We use a reference to avoid
 * a cyclic dependency: everything depends on the Errors module (the module
 * defining all the errors), because of that making the Errors module call
 * into anything that isn't in the standard library is very unwise, because
 * that code won't be able to add errors.
 *)
(*****************************************************************************)

let get_entries get_map pos =
  let filename = Pos.filename pos in
  let (line, _, _) = Pos.info_pos pos in
  get_map filename
  |> Option.value ~default:IMap.empty
  |> IMap.find_opt line
  |> Option.value ~default:IMap.empty

let get_fixmes_for_pos pos = get_entries get_fixmes pos

let get_fixme_codes_for_pos pos =
  get_fixmes_for_pos pos |> IMap.keys |> ISet.of_list

let get_entry get_map pos code = get_entries get_map pos |> IMap.find_opt code

let get_disallowed_fixme_pos pos code = get_entry get_disallowed_fixmes pos code

let get_ignore_pos pos code = get_entry get_ignores pos code

let inf_err_codes = [4110; 4323; 4324]

let is_inf_err_code =
  let inf_err_codes = ISet.of_list inf_err_codes in
  (fun err_code -> ISet.mem err_code inf_err_codes)

let any_inf_err_code imap =
  let rec aux = function
    | [] -> None
    | code :: codes ->
      let fixme_opt = IMap.find_opt code imap in
      if Option.is_none fixme_opt then
        aux codes
      else
        fixme_opt
  in
  aux inf_err_codes

let () =
  (Diagnostics.get_hh_fixme_pos :=
     fun err_pos err_code ->
       get_fixmes_for_pos err_pos |> fun imap ->
       if !Diagnostics.code_agnostic_fixme then
         if IMap.is_empty imap then
           None
         else
           Some err_pos
       else
         match IMap.find_opt err_code imap with
         | None when is_inf_err_code err_code -> any_inf_err_code imap
         | x -> x);
  Diagnostics.get_disallowed_fixme_pos := get_disallowed_fixme_pos;
  Diagnostics.get_ignore_pos := get_ignore_pos;
  ()
