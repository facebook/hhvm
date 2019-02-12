(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* In order to run recheck_typing, workers need access to the FileInfo for each
 * file to be typechecked, so a FileInfo is paired with each query.
 *
 * Note that this means that many queries on the same file result in needlessly
 * marshalling and unmarshalling the same FileInfo many times over. There are
 * probably ways we could avoid this, but it doesn't seem to be a major problem.
 *)
type pos = Relative_path.t * int * int * (int * int) option
type pos_info = pos * FileInfo.t

let recheck_typing tcopt (pos_infos : pos_info list) =
  let files_to_check =
    pos_infos
    |> List.map ~f:(fun ((filename,_,_,_), file_info) -> filename, file_info)
    |> List.remove_consecutive_duplicates ~equal:(fun (a,_) (b,_) -> a = b)
  in
  ServerIdeUtils.recheck tcopt files_to_check

let result_to_string result (fn, line, char, range_end) =
  let open Hh_json in
  let obj = JSON_Object [
    "position", JSON_Object (
      ["file", JSON_String (Relative_path.to_absolute fn)] @
      begin
        match range_end with
        | None -> ["line", int_ line; "character", int_ char]
        | Some (end_line, end_char) ->
          let pos l c = JSON_Object ["line", int_ l; "character", int_ c] in
          [
            "start", pos line char;
            "end", pos end_line end_char;
          ]
      end
    );
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
    let fn, line, char, range_end = pos in
    let result =
      Relative_path.Map.get tasts fn
      |> Result.of_option ~error:"No such file or directory"
      |> Result.map ~f:begin fun tast ->
        let env_and_ty =
          match range_end with
          | None ->
            ServerInferType.type_at_pos tast line char
          | Some (end_line, end_char) ->
            ServerInferType.type_at_range tast line char end_line end_char
        in
        Option.map env_and_ty ~f:(fun (env, ty) -> Tast_env.ty_to_json env ty)
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
  MultiWorker.worker list option ->
  (string * int * int * (int * int) option) list ->
  ServerEnv.env ->
  string list =
fun workers pos_list env ->
  let {ServerEnv.tcopt; naming_table; _} = env in
  let pos_info_results =
    pos_list
    (* Sort, so that many queries on the same file will (generally) be
     * dispatched to the same worker. *)
    |> List.sort ~compare
    (* Dedup identical queries *)
    |> List.remove_consecutive_duplicates ~equal:(=)
    (* Get the FileInfo for each query *)
    |> List.map ~f:begin fun (fn, line, char, range_end) ->
      let fn = Relative_path.create_detect_prefix fn in
      let pos = (fn, line, char, range_end) in
      match Naming_table.get_file_info naming_table fn with
      | Some fileinfo -> Ok (pos, fileinfo)
      | None -> Error pos
    end
  in
  let pos_infos = List.filter_map pos_info_results ~f:Result.ok in
  let failure_msgs =
    pos_info_results
    |> List.filter_map ~f:Result.error
    |> List.map ~f:(result_to_string (Error "No such file or directory"))
  in
  let results =
    if (List.length pos_infos) < 10
    then helper tcopt [] pos_infos
    else parallel_helper workers tcopt pos_infos
  in
  failure_msgs @ results
