(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type pos = Relative_path.t * int * int * (int * int) option

let recheck_typing ctx (pos_list : pos list) =
  let files_to_check =
    pos_list |> List.remove_consecutive_duplicates ~equal:( = )
  in
  let tasts =
    List.map files_to_check ~f:(fun (path, _, _, _) ->
        let (_ctx, entry) = Provider_context.add_entry ~ctx ~path in
        let { Tast_provider.Compute_tast.tast; _ } =
          Tast_provider.compute_tast_unquarantined ~ctx ~entry
        in
        (path, tast))
  in
  tasts

let result_to_string result (fn, line, char, range_end) =
  Hh_json.(
    let obj =
      JSON_Object
        [
          ( "position",
            JSON_Object
              ( [("file", JSON_String (Relative_path.to_absolute fn))]
              @
              match range_end with
              | None -> [("line", int_ line); ("character", int_ char)]
              | Some (end_line, end_char) ->
                let pos l c =
                  JSON_Object [("line", int_ l); ("character", int_ c)]
                in
                [("start", pos line char); ("end", pos end_line end_char)] ) );
          (match result with
          | Ok ty -> ("type", Option.value ty ~default:JSON_Null)
          | Error e -> ("error", JSON_String e));
        ]
    in
    json_to_string obj)

let helper ctx acc pos_list =
  let tasts =
    List.fold
      (recheck_typing ctx pos_list)
      ~init:Relative_path.Map.empty
      ~f:(fun map (key, data) -> Relative_path.Map.add map ~key ~data)
  in
  List.fold pos_list ~init:acc ~f:(fun acc pos ->
      let (fn, line, char, range_end) = pos in
      let result =
        Relative_path.Map.find_opt tasts fn
        |> Result.of_option ~error:"No such file or directory"
        |> Result.map ~f:(fun tast ->
               let env_and_ty =
                 match range_end with
                 | None -> ServerInferType.type_at_pos ctx tast line char
                 | Some (end_line, end_char) ->
                   ServerInferType.type_at_range
                     ctx
                     tast
                     line
                     char
                     end_line
                     end_char
               in
               Option.map env_and_ty ~f:(fun (env, ty) ->
                   Tast_env.ty_to_json env ty))
      in
      result_to_string result pos :: acc)

let parallel_helper workers ctx pos_list =
  MultiWorker.call
    workers
    ~job:(helper ctx)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers pos_list)

(* Entry Point *)
let go :
    MultiWorker.worker list option ->
    (string * int * int * (int * int) option) list ->
    ServerEnv.env ->
    string list =
 fun workers pos_list env ->
  let pos_list =
    pos_list
    (* Sort, so that many queries on the same file will (generally) be
     * dispatched to the same worker. *)
    |> List.sort ~compare
    (* Dedup identical queries *)
    |> List.remove_consecutive_duplicates ~equal:( = )
    |> List.map ~f:(fun (fn, line, char, range_end) ->
           let fn = Relative_path.create_detect_prefix fn in
           (fn, line, char, range_end))
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  let results =
    if List.length pos_list < 10 then
      helper ctx [] pos_list
    else
      parallel_helper workers ctx pos_list
  in
  results
