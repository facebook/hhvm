(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type pos = Relative_path.t * int * int

let pos_to_json fn line char =
  Hh_json.(
    JSON_Object
      [
        ("file", JSON_String (Relative_path.to_absolute fn));
        ("line", int_ line);
        ("character", int_ char);
      ])

let recheck_typing ctx (pos_list : pos list) =
  let files_to_check =
    pos_list
    |> List.map ~f:(fun (path, _, _) -> path)
    |> List.remove_consecutive_duplicates ~equal:( = )
  in
  List.map files_to_check ~f:(fun path ->
      let (_ctx, entry) = Provider_context.add_entry ~ctx ~path in
      let { Tast_provider.Compute_tast.tast; _ } =
        Tast_provider.compute_tast_unquarantined ~ctx ~entry
      in
      (path, tast))

let pos_contains_line_char pos line char =
  let (l, start, end_) = Pos.info_pos pos in
  l = line && start <= char && char - 1 <= end_

type 'a walker = {
  plus: 'a -> 'a -> 'a;
  on_method: Tast_env.env -> Tast.method_ -> 'a;
  on_fun: Tast_env.env -> Tast.fun_ -> 'a;
}

let find_in_tree (walker : 'a walker) line char =
  object (self)
    inherit [_] Tast_visitor.reduce

    inherit [_] Ast_defs.option_monoid

    method merge = walker.plus

    method! on_method_ env m =
      if pos_contains_line_char (fst m.Aast.m_name) line char then
        Some (walker.on_method env m)
      else
        self#zero

    method! on_fun_ env f =
      if pos_contains_line_char (fst f.Aast.f_name) line char then
        Some (walker.on_fun env f)
      else
        self#zero
  end

type ('a, 'r, 's) handlers = {
  result_to_string:
    ('r option, string) result -> Relative_path.t * int * int -> string;
  walker: 'a walker;
  get_state: Provider_context.t -> Relative_path.t -> 's;
  map_result: Provider_context.t -> 's -> 'a -> 'r;
}

let prepare_pos_infos pos_list =
  pos_list
  (* Sort, so that many queries on the same file will (generally) be
   * dispatched to the same worker. *)
  |> List.sort ~compare
  (* Dedup identical queries *)
  |> List.remove_consecutive_duplicates ~equal:( = )
  |> List.map ~f:(fun (path, line, char) ->
         (Relative_path.create_detect_prefix path, line, char))

let helper h ctx acc pos_list =
  let tasts =
    List.fold
      (recheck_typing ctx pos_list)
      ~init:Relative_path.Map.empty
      ~f:(fun map (key, data) -> Relative_path.Map.add map ~key ~data)
  in
  List.fold pos_list ~init:acc ~f:(fun acc pos ->
      let (fn, line, char) = pos in
      let s = h.get_state ctx fn in
      let result =
        Relative_path.Map.find_opt tasts fn
        |> Result.of_option ~error:"No such file or directory"
        |> Result.map ~f:(fun tast ->
               (find_in_tree h.walker line char)#go ctx tast
               |> Option.map ~f:(h.map_result ctx s))
      in
      h.result_to_string result pos :: acc)

let parallel_helper h workers tcopt pos_list =
  MultiWorker.call
    workers
    ~job:(helper h tcopt)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers pos_list)

(* Entry Point *)
let go :
    MultiWorker.worker list option ->
    (string * int * int) list ->
    ServerEnv.env ->
    _ handlers ->
    _ =
 fun workers pos_list env h ->
  let ctx = Provider_utils.ctx_from_server_env env in
  let pos_list = prepare_pos_infos pos_list in
  let results =
    if List.length pos_list < 10 then
      helper h ctx [] pos_list
    else
      parallel_helper h workers ctx pos_list
  in
  results
