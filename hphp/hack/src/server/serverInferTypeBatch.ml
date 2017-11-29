(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE fn in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

(* In order to run recheck_typing, workers need access to the FileInfo for each
 * file to be typechecked, so a FileInfo is paired with each query.
 *
 * Note that this means that many queries on the same file result in needlessly
 * marshalling and unmarshalling the same FileInfo many times over. There are
 * probably ways we could avoid this, but it doesn't seem to be a major problem.
 *)
type pos = Relative_path.t * int * int
type pos_info = pos * FileInfo.t

let recheck_typing tcopt (pos_infos : pos_info list) =
  let files_to_check =
    pos_infos
    |> List.map ~f:(fun ((filename,_,_), file_info) -> filename, file_info)
    |> List.remove_consecutive_duplicates ~equal:(fun (a,_) (b,_) -> a = b)
  in
  let tcopt = TypecheckerOptions.make_permissive tcopt in
  ServerIdeUtils.recheck tcopt files_to_check

let result_to_string result (fn, line, char) =
  let open Hh_json in
  let obj = JSON_Object [
    "position", JSON_Object [
      "file", JSON_String (Relative_path.to_absolute fn);
      "line", int_ line;
      "character", int_ char;
    ];
    match result with
    | Ok ty -> "type", Option.value ty ~default:JSON_Null
    | Error e -> "error", JSON_String e
  ] in
  json_to_string obj

let helper tcopt acc pos_infos =
  let tasts =
    List.fold (recheck_typing tcopt pos_infos)
      ~init:Relative_path.Map.empty
      ~f:(fun map (key, data) -> Relative_path.Map.add map ~key ~data)
  in
  List.fold pos_infos ~init:acc ~f:begin fun acc (pos, _) ->
    let fn, line, char = pos in
    let result =
      Relative_path.Map.get tasts fn
      |> Core_result.of_option ~error:"No such file or directory"
      |> Core_result.map ~f:begin fun tast ->
        ServerInferType.type_at_pos tast line char
        |> Option.map ~f:begin fun (saved_env, ty) ->
          let env = Typing_env.empty tcopt fn ~droot:None in
          let env = Tast_expand.restore_saved_env env saved_env in
          Typing_print.to_json env ty
        end
      end
    in
    result_to_string result pos :: acc
  end

let parallel_helper workers tcopt pos_infos =
  MultiWorker.call
    workers
    ~job:(helper tcopt)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers pos_infos)

(* Entry Point *)
let go:
  Worker.t list option ->
  (string * int * int) list ->
  ServerEnv.env ->
  string list =
fun workers pos_list env ->
  let {ServerEnv.tcopt; files_info; _} = env in
  let pos_info_results =
    pos_list
    (* Sort, so that many queries on the same file will (generally) be
     * dispatched to the same worker. *)
    |> List.sort ~cmp:compare
    (* Dedup identical queries *)
    |> List.remove_consecutive_duplicates ~equal:(fun a b -> compare a b = 0)
    (* Get the FileInfo for each query *)
    |> List.map ~f:begin fun (fn, line, char) ->
      let fn = Relative_path.(create Root fn) in
      let pos = (fn, line, char) in
      match Relative_path.Map.get files_info fn with
      | Some fileinfo -> Ok (pos, fileinfo)
      | None -> Error pos
    end
  in
  let pos_infos = List.filter_map pos_info_results ~f:Core_result.ok in
  let failure_msgs =
    pos_info_results
    |> List.filter_map ~f:Core_result.error
    |> List.map ~f:(result_to_string (Error "No such file or directory"))
  in
  let results =
    if (List.length pos_infos) < 10
    then helper tcopt [] pos_infos
    else parallel_helper workers tcopt pos_infos
  in
  failure_msgs @ results
