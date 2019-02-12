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
type pos = Relative_path.t * int * int
type pos_info = pos * FileInfo.t

module T = Tast
module SN = Naming_special_names

let pos_to_json fn line char =
  let open Hh_json in
  JSON_Object [
      "file", JSON_String (Relative_path.to_absolute fn);
      "line", int_ line;
      "character", int_ char
  ]

let recheck_typing tcopt (pos_infos : pos_info list) =
  let files_to_check =
    pos_infos
    |> List.map ~f:(fun ((filename,_, _), file_info) -> filename, file_info)
    |> List.remove_consecutive_duplicates ~equal:(fun (a,_) (b,_) -> a = b)
  in
  ServerIdeUtils.recheck tcopt files_to_check

let pos_contains_line_char pos line char =
  let l, start, end_ = Pos.info_pos pos in
  l = line && start <= char && char - 1 <= end_

type 'a walker = {
  plus: 'a -> 'a -> 'a;
  on_method: Tast_env.env -> Tast.constructor -> 'a;
  on_fun: Tast_env.env -> Tast.fun_ -> 'a;
}

let find_in_tree (walker: 'a walker) line char = object(self)
  inherit [_] Tast_visitor.reduce
  inherit [_] Ast.option_monoid

  method merge = walker.plus

  method! on_method_ env m =
    if pos_contains_line_char (fst m.Tast.m_name) line char
    then Some (walker.on_method env m)
    else self#zero

  method! on_fun_ env f =
    if pos_contains_line_char (fst f.Tast.f_name) line char
    then Some (walker.on_fun env f)
    else self#zero
end

type ('a, 'r, 's) handlers = {
  result_to_string: ('r option, string) result -> (Relative_path.t * int * int) -> string;
  walker: 'a walker;
  get_state: Relative_path.t -> 's;
  map_result: 's -> 'a -> 'r
}

let prepare_pos_infos h pos_list naming_table =
  let pos_info_results =
    pos_list
    (* Sort, so that many queries on the same file will (generally) be
     * dispatched to the same worker. *)
    |> List.sort ~compare
    (* Dedup identical queries *)
    |> List.remove_consecutive_duplicates ~equal:(=)
    (* Get the FileInfo for each query *)
    |> List.map ~f:begin fun (fn, line, char) ->
      let fn = Relative_path.create_detect_prefix fn in
      let pos = (fn, line, char) in
      match Naming_table.get_file_info naming_table fn with
      | Some fileinfo -> Ok (pos, fileinfo)
      | None -> Error pos
    end
  in
  let pos_infos = List.filter_map pos_info_results ~f:Result.ok in
  let failure_msgs =
    pos_info_results
    |> List.filter_map ~f:Result.error
    |> List.map ~f:(h.result_to_string (Error "No such file or directory")) in
  pos_infos, failure_msgs

let helper h tcopt acc pos_infos =
  let tasts =
    List.fold (recheck_typing tcopt pos_infos)
      ~init:Relative_path.Map.empty
      ~f:(fun map (key, data) -> Relative_path.Map.add map ~key ~data)
  in
  List.fold pos_infos ~init:acc ~f:begin fun acc (pos, _) ->
    let fn, line, char = pos in
    let s = h.get_state fn in
    let result =
      Relative_path.Map.get tasts fn
      |> Result.of_option ~error:"No such file or directory"
      |> Result.map ~f:begin fun tast ->
        (find_in_tree h.walker line char)#go tast
        |> Option.map ~f:(h.map_result s)
      end
    in
    h.result_to_string result pos :: acc
  end

let parallel_helper h workers tcopt pos_infos =
  MultiWorker.call
    workers
    ~job:(helper h tcopt)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers pos_infos)

(* Entry Point *)
let go:
  MultiWorker.worker list option ->
  (string * int * int) list ->
  ServerEnv.env ->
  (_ handlers) ->
  _ =
fun workers pos_list env h ->
  let {ServerEnv.tcopt; naming_table; _} = env in
  let pos_infos, failure_msgs = prepare_pos_infos h pos_list naming_table in
  let results =
    if (List.length pos_infos) < 10
    then helper h tcopt [] pos_infos
    else parallel_helper h workers tcopt pos_infos
  in
  failure_msgs @ results
