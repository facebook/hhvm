(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

(*****************************************************************************)
(* Table containing all the HH_FIXMEs found in the source code.
 * Associates:
 *   filename =>
 *   line number guarded by HH_FIXME =>
 *   error_node_number =>
 *   position of HH_FIXME comment
 *)
(*****************************************************************************)

module HH_FIXMES = SharedMem.WithCache (Relative_path.S) (struct
  type t = Pos.t IMap.t IMap.t
  let prefix = Prefix.make()
  let description = "HH_FIXMES"
  end)

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
    HH_FIXMES.get filename
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
  match HH_FIXMES.get fn with
  | None -> acc
  | Some fixme_map ->
    IMap.fold (fun line code_map acc ->
      IMap.fold (fun code fixme_pos acc ->
        if (List.mem codes code || (List.is_empty codes && code < 5000))
           && not (fixme_was_applied applied_fixme_map fn line code)
        then fixme_pos :: acc
        else acc) code_map acc) fixme_map acc

let get_unused_fixmes codes applied_fixmes files_info  =
  let applied_fixme_map =
    List.fold_left applied_fixmes ~init:Relative_path.Map.empty
      ~f:begin fun acc (pos,code) ->
      let fn = Pos.filename pos in
      let line, _, _ = Pos.info_pos pos in
      add_applied_fixme acc code fn line end in
  Relative_path.Map.fold files_info ~f:(fun fn _ acc ->
    get_unused_fixmes_for codes applied_fixme_map fn acc) ~init:[]

let to_string fixmes =
  let folder1 line_number innermap acc =
    let folder2 error_number pos acc =
      let pos = Pos.string_no_file pos in
      (Printf.sprintf "%d %d %s" line_number error_number pos) :: acc in
    IMap.fold folder2 innermap acc in
  let results = IMap.fold folder1 fixmes ["FIXMES"] in
  String.concat "\n" (List.rev results)
