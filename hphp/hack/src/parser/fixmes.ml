(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(*****************************************************************************)
(* Table containing all the HH_FIXMEs found in the source code.
 * Associates:
 *   filename =>
 *   line number guarded by HH_FIXME =>
 *   error_node_number =>
 *   position of HH_FIXME comment
 *)
(*****************************************************************************)

module HH_FIXMES = SharedMem.WithCache (SharedMem.ProfiledImmediate) (Relative_path.S) (struct
  type t = Pos.t IMap.t IMap.t
  let prefix = Prefix.make()
  let description = "HH_FIXMES"
  end)

module DECL_HH_FIXMES = SharedMem.WithCache (SharedMem.ProfiledImmediate) (Relative_path.S) (struct
  type t = Pos.t IMap.t IMap.t
  let prefix = Prefix.make()
  let description = "DECL_HH_FIXMES"
  end)

let get_fixmes_from_heap filename =
  match HH_FIXMES.get filename with
  | None -> DECL_HH_FIXMES.get filename
  | Some x -> Some x

(*****************************************************************************)
(* We register the function that can look up a position and determine if
 * a given position is affected by an HH_FIXME. We use a reference to avoid
 * a cyclic dependency: everything depends on the Errors module (the module
 * defining all the errors), because of that making the Errors module call
 * into anything that isn't in the standard library is very unwise, because
 * that code won't be able to add errors.
 *)
(*****************************************************************************)
let () =
  Errors.get_hh_fixme_pos := begin fun err_pos err_code ->
    let filename = Pos.filename err_pos in
    let err_line, _, _ = Pos.info_pos err_pos in
    get_fixmes_from_heap filename
    |> Option.value_map ~f:(IMap.get err_line) ~default:None
    |> Option.value_map ~f:(IMap.get err_code) ~default:None
    end;
  Errors.is_hh_fixme := fun err_pos err_code ->
    Option.is_some (!Errors.get_hh_fixme_pos err_pos err_code)

let fixme_was_applied applied_fixmes fn err_line err_code =
  match Relative_path.Map.get applied_fixmes fn with
  | None -> false
  | Some r ->
    match IMap.get err_line r with
    | None -> false
    | Some code_set -> ISet.mem err_code code_set

let add_applied_fixme_file m err_code err_line =
  let line_value =
    match IMap.get err_line m with
    | None -> ISet.empty
    | Some x -> x in
  IMap.add err_line (ISet.add err_code line_value) m

let add_applied_fixme applied_fixmes err_code fn err_line =
  let file_value =
    match Relative_path.Map.get applied_fixmes fn with
    | None -> IMap.empty
    | Some x -> x in
  Relative_path.Map.add applied_fixmes fn
    (add_applied_fixme_file file_value err_code err_line)

let get_unused_fixmes_for codes applied_fixme_map fn acc =
  match get_fixmes_from_heap fn with
  | None -> acc
  | Some fixme_map ->
    IMap.fold (fun line code_map acc ->
      IMap.fold (fun code fixme_pos acc ->
        if (List.mem codes code ~equal:(=) || (List.is_empty codes && code < 5000))
           && not (fixme_was_applied applied_fixme_map fn line code)
        then fixme_pos :: acc
        else acc) code_map acc) fixme_map acc

let get_unused_fixmes codes applied_fixmes fold files_info  =
  let applied_fixme_map =
    List.fold_left applied_fixmes ~init:Relative_path.Map.empty
      ~f:begin fun acc (pos,code) ->
      let fn = Pos.filename pos in
      let line, _, _ = Pos.info_pos pos in
      add_applied_fixme acc code fn line end in
  fold files_info ~init:[] ~f:(fun fn _ acc ->
    get_unused_fixmes_for codes applied_fixme_map fn acc)
