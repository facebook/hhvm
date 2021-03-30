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
    ((decls, file_mode, hash) :
      Direct_decl_parser.decls * FileInfo.mode option * Int64.t option) :
    FileInfo.t =
  let add acc (name, decl) =
    match decl with
    | Shallow_decl_defs.Class c ->
      {
        acc with
        FileInfo.classes =
          ( FileInfo.Full
              ( fst c.Shallow_decl_defs.sc_name
              |> Pos_or_decl.fill_in_filename fn ),
            name )
          :: acc.FileInfo.classes;
      }
    | Shallow_decl_defs.Fun f ->
      {
        acc with
        FileInfo.funs =
          ( FileInfo.Full (Pos_or_decl.fill_in_filename fn f.Typing_defs.fe_pos),
            name )
          :: acc.FileInfo.funs;
      }
    | Shallow_decl_defs.Typedef tf ->
      {
        acc with
        FileInfo.typedefs =
          ( FileInfo.Full (Pos_or_decl.fill_in_filename fn tf.Typing_defs.td_pos),
            name )
          :: acc.FileInfo.typedefs;
      }
    | Shallow_decl_defs.Const c ->
      {
        acc with
        FileInfo.consts =
          ( FileInfo.Full (Pos_or_decl.fill_in_filename fn c.Typing_defs.cd_pos),
            name )
          :: acc.FileInfo.consts;
      }
    | Shallow_decl_defs.Record r ->
      {
        acc with
        FileInfo.record_defs =
          ( FileInfo.Full (Pos_or_decl.fill_in_filename fn r.Typing_defs.rdt_pos),
            name )
          :: acc.FileInfo.record_defs;
      }
  in
  List.fold ~f:add ~init:FileInfo.{ empty_t with hash; file_mode } decls

let parse_batch
    (ctx : Provider_context.t)
    (acc : FileInfo.t Relative_path.Map.t)
    (fnl : Relative_path.t list) : FileInfo.t Relative_path.Map.t =
  let parse acc fn =
    if not (FindUtils.path_filter fn) then
      acc
    else
      match
        Direct_decl_utils.direct_decl_parse_and_cache ~decl_hash:true ctx fn
      with
      | None -> acc
      | Some decl_and_mode_and_hash ->
        Relative_path.Map.add
          acc
          ~key:fn
          ~data:(decls_to_fileinfo fn decl_and_mode_and_hash)
  in
  List.fold ~f:parse ~init:acc fnl

let go
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (get_next : Relative_path.t list MultiWorker.Hh_bucket.next) :
    FileInfo.t Relative_path.Map.t =
  MultiWorker.call
    workers
    ~job:(parse_batch ctx)
    ~neutral:Relative_path.Map.empty
    ~merge:Relative_path.Map.union
    ~next:get_next
