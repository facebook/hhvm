(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let decls_to_fileinfo
    fn
    ((decls, file_mode, hash, symbol_decl_hashes) :
      Direct_decl_parser.decls
      * FileInfo.mode option
      * Int64.t option
      * Int64.t option list) : FileInfo.t =
  let add acc ((name, decl), decl_hash) =
    match decl with
    | Shallow_decl_defs.Class c ->
      {
        acc with
        FileInfo.classes =
          ( FileInfo.Full
              (fst c.Shallow_decl_defs.sc_name
              |> Pos_or_decl.fill_in_filename fn),
            name,
            decl_hash )
          :: acc.FileInfo.classes;
      }
    | Shallow_decl_defs.Fun f ->
      {
        acc with
        FileInfo.funs =
          ( FileInfo.Full (Pos_or_decl.fill_in_filename fn f.Typing_defs.fe_pos),
            name,
            decl_hash )
          :: acc.FileInfo.funs;
      }
    | Shallow_decl_defs.Typedef tf ->
      {
        acc with
        FileInfo.typedefs =
          ( FileInfo.Full (Pos_or_decl.fill_in_filename fn tf.Typing_defs.td_pos),
            name,
            decl_hash )
          :: acc.FileInfo.typedefs;
      }
    | Shallow_decl_defs.Const c ->
      {
        acc with
        FileInfo.consts =
          ( FileInfo.Full (Pos_or_decl.fill_in_filename fn c.Typing_defs.cd_pos),
            name,
            decl_hash )
          :: acc.FileInfo.consts;
      }
    | Shallow_decl_defs.Record r ->
      {
        acc with
        FileInfo.record_defs =
          ( FileInfo.Full (Pos_or_decl.fill_in_filename fn r.Typing_defs.rdt_pos),
            name,
            decl_hash )
          :: acc.FileInfo.record_defs;
      }
  in
  List.fold
    ~f:add
    ~init:FileInfo.{ empty_t with hash; file_mode }
    (List.zip_exn decls symbol_decl_hashes)

let parse
    (ctx : Provider_context.t)
    ~(trace : bool)
    ~(cache_decls : bool)
    (acc : FileInfo.t Relative_path.Map.t)
    (fn : Relative_path.t) : FileInfo.t Relative_path.Map.t =
  if not (FindUtils.path_filter fn) then
    acc
  else
    let start_parse_time = Unix.gettimeofday () in
    match
      Direct_decl_utils.direct_decl_parse
        ~file_decl_hash:true
        ~symbol_decl_hashes:true
        ctx
        fn
    with
    | None -> acc
    | Some ((decls, _, _, _) as decl_and_mode_and_hash) ->
      let end_parse_time = Unix.gettimeofday () in
      if cache_decls then Direct_decl_utils.cache_decls ctx decls;
      let fileinfo = decls_to_fileinfo fn decl_and_mode_and_hash in
      if trace then
        Hh_logger.log
          "[%.1fms] %s - %s"
          ((end_parse_time -. start_parse_time) *. 1000.0)
          (Relative_path.suffix fn)
          (FileInfo.to_string fileinfo);
      Relative_path.Map.add acc ~key:fn ~data:fileinfo

let go
    (ctx : Provider_context.t)
    ~(trace : bool)
    ~(cache_decls : bool)
    (workers : MultiWorker.worker list option)
    ~(ide_files : Relative_path.Set.t)
    ~(get_next : Relative_path.t list MultiWorker.Hh_bucket.next) :
    FileInfo.t Relative_path.Map.t =
  let acc =
    MultiWorker.call
      workers
      ~job:(fun init -> List.fold ~init ~f:(parse ctx ~trace ~cache_decls))
      ~neutral:Relative_path.Map.empty
      ~merge:Relative_path.Map.union
      ~next:get_next
  in
  Relative_path.Set.fold ide_files ~init:acc ~f:(fun fn acc ->
      parse ctx ~trace ~cache_decls acc fn)
