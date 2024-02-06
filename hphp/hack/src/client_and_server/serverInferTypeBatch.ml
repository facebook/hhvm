(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type pos = Relative_path.t * int * int * (int * int) option

type spos = string * int * int * (int * int) option [@@deriving eq, ord]

let recheck_typing ctx (path_list : Relative_path.t list) =
  let files_to_check =
    List.sort path_list ~compare:Relative_path.compare
    |> List.remove_consecutive_duplicates ~equal:Relative_path.equal
  in
  let (ctx, paths_and_tasts) =
    List.fold
      files_to_check
      ~init:(ctx, [])
      ~f:(fun (ctx, paths_and_tasts) path ->
        let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        let { Tast_provider.Compute_tast.tast; _ } =
          Tast_provider.compute_tast_unquarantined ~ctx ~entry
        in
        (ctx, (path, tast) :: paths_and_tasts))
  in
  (ctx, paths_and_tasts)

let get_tast_map ctx path_list =
  let (ctx, paths_and_tasts) = recheck_typing ctx path_list in
  let tasts =
    List.fold
      paths_and_tasts
      ~init:Relative_path.Map.empty
      ~f:(fun map (key, data) -> Relative_path.Map.add map ~key ~data)
  in
  (ctx, tasts)

let result_to_string result (fn, line, char, range_end) =
  Hh_json.(
    let obj =
      JSON_Object
        [
          ( "position",
            JSON_Object
              ([("file", JSON_String (Relative_path.to_absolute fn))]
              @
              match range_end with
              | None -> [("line", int_ line); ("character", int_ char)]
              | Some (end_line, end_char) ->
                let pos l c =
                  JSON_Object [("line", int_ l); ("character", int_ c)]
                in
                [("start", pos line char); ("end", pos end_line end_char)]) );
          (match result with
          | Ok ty -> ("type", Option.value ty ~default:JSON_Null)
          | Error e -> ("error", JSON_String e));
        ]
    in
    json_to_string obj)

let helper ctx acc pos_list =
  let empty_map = Relative_path.Map.empty in
  let (at_pos_map, at_range_map) =
    List.fold
      pos_list
      ~init:(empty_map, empty_map)
      ~f:
        begin
          fun (at_pos_map, at_range_map) (fn, line, char, range_end) ->
            let add map ~data =
              Relative_path.Map.update
                fn
                (function
                  | None -> Some [data]
                  | Some acc -> Some (data :: acc))
                map
            in
            match range_end with
            | Some (end_line, end_char) ->
              let data = (line, char, end_line, end_char) in
              (at_pos_map, add at_range_map ~data)
            | None ->
              let data = (line, char) in
              (add at_pos_map ~data, at_range_map)
        end
  in
  let path_list =
    Relative_path.Map.keys at_pos_map @ Relative_path.Map.keys at_range_map
  in
  let (ctx, tasts) = get_tast_map ctx path_list in
  let type_at_range_results =
    Relative_path.Map.mapi at_range_map ~f:(fun fn pos_list ->
        match Relative_path.Map.find_opt tasts fn with
        | None ->
          List.map pos_list ~f:(fun (line, char, end_line, end_char) ->
              let result = "No such file or directory" in
              let pos = (fn, line, char, Some (end_line, end_char)) in
              result_to_string (Result.Error result) pos)
        | Some tast ->
          let results = ServerInferType.type_at_range_fused ctx tast pos_list in
          List.map2_exn
            pos_list
            results
            ~f:(fun (line, char, end_line, end_char) env_and_ty ->
              let result =
                env_and_ty
                |> Option.map ~f:(fun (env, ty) -> Tast_env.ty_to_json env ty)
              in
              let pos = (fn, line, char, Some (end_line, end_char)) in
              result_to_string (Result.Ok result) pos))
  in
  let acc =
    List.concat (Relative_path.Map.values type_at_range_results) @ acc
  in
  let type_at_pos_results =
    Relative_path.Map.mapi at_pos_map ~f:(fun fn pos_list ->
        match Relative_path.Map.find_opt tasts fn with
        | None ->
          List.map pos_list ~f:(fun (line, char) ->
              let result = "No such file or directory" in
              let pos = (fn, line, char, None) in
              result_to_string (Result.Error result) pos)
        | Some tast ->
          let results = ServerInferType.type_at_pos_fused ctx tast pos_list in
          List.map2_exn pos_list results ~f:(fun (line, char) env_and_ty ->
              let result =
                env_and_ty
                |> Option.map ~f:(fun (env, ty) -> Tast_env.ty_to_json env ty)
              in
              let pos = (fn, line, char, None) in
              result_to_string (Result.Ok result) pos))
  in
  let acc = List.concat (Relative_path.Map.values type_at_pos_results) @ acc in
  acc

(** This divides files amongst all the workers.
No file is handled by more than one worker. *)
let parallel_helper
    (workers : MultiWorker.worker list option)
    (ctx : Provider_context.t)
    (pos_list : pos list) : string list =
  let add_pos_to_map map pos =
    let (path, _, _, _) = pos in
    Relative_path.Map.update
      path
      (function
        | None -> Some [pos]
        | Some others -> Some (pos :: others))
      map
  in
  let pos_by_file =
    List.fold ~init:Relative_path.Map.empty ~f:add_pos_to_map pos_list
    |> Relative_path.Map.values
  in
  (* pos_by_file is a list-of-lists [[posA1;posA2;...];[posB1;...];...]
     where each inner list [posA1;posA2;...] is all for the same file.
     This is so that a given file is only ever processed by a single worker. *)
  MultiWorker.call
    workers
    ~job:(fun acc pos_by_file -> helper ctx acc (List.concat pos_by_file))
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers pos_by_file)

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
    |> List.sort ~compare:compare_spos
    (* Dedup identical queries *)
    |> List.remove_consecutive_duplicates ~equal:equal_spos
    |> List.map ~f:(fun (fn, line, char, range_end) ->
           let fn = Relative_path.create_detect_prefix fn in
           (fn, line, char, range_end))
  in
  let num_files =
    pos_list
    |> List.map ~f:(fun (path, _, _, _) -> path)
    |> Relative_path.Set.of_list
    |> Relative_path.Set.cardinal
  in
  let num_positions = List.length pos_list in
  let ctx = Provider_utils.ctx_from_server_env env in
  let start_time = Unix.gettimeofday () in
  (* Just for now, as a rollout telemetry defense against crashes, we'll log at the start *)
  HackEventLogger.type_at_pos_batch
    ~start_time
    ~num_files
    ~num_positions
    ~results:None;
  let results =
    if num_positions < 10 then
      helper ctx [] pos_list
    else
      parallel_helper workers ctx pos_list
  in
  HackEventLogger.type_at_pos_batch
    ~start_time
    ~num_files
    ~num_positions
    ~results:(Some (List.length results));
  results
