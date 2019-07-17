(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hack_bucket = Bucket
open Core_kernel
open Hh_json
open Tast
open Typing_symbol_json_builder
module Bucket = Hack_bucket

let get_decls
  (tast: (Relative_path.t * Tast.program) list)
  : decls =
  let empty_decls = Typing_symbol_json_builder.default_decls in
  List.fold tast ~init:empty_decls ~f:begin fun acc (_,prog) ->
    List.fold prog ~init:acc ~f:begin fun acc2 def ->
      let {funs=acc_funs; classes=acc_clss; typedefs=acc_typedef; consts=acc_consts} = acc2 in
      match def with
      | Fun f -> { acc2 with funs = f::acc_funs }
      | Class c  -> { acc2 with classes = c::acc_clss }
      | Typedef t  -> { acc2 with typedefs = t::acc_typedef }
      | Constant g -> { acc2 with consts = g::acc_consts }
      | _ -> acc2
    end
  end

let write_json
    (tcopt: TypecheckerOptions.t)
    (file_dir: string)
    (tast_lst: (Relative_path.t * Tast.program) list)
  : unit =

  let decls = get_decls tast_lst in
  let json_chunks = Typing_symbol_json_builder.build_json tcopt decls in
  let _, channel = Filename.open_temp_file ~temp_dir:file_dir (file_dir ^ "_chunk_") ".json" in
  let json = JSON_Array(json_chunks) in
  Out_channel.output_string channel (json_to_string ~pretty:true json);
  Out_channel.close channel

let recheck_job
    (tcopt: TypecheckerOptions.t)
    (out_dir: string)
    ()
    (progress: (Relative_path.t * FileInfo.t) list)
  : unit =
  let results = ServerIdeUtils.recheck tcopt progress in
  write_json tcopt out_dir results

let go
    (workers: MultiWorker.worker list option)
    (tcopt: TypecheckerOptions.t)
    (out_dir: string)
    (file_tuples: (Relative_path.t * FileInfo.t) list)
  : unit =
  let num_workers = (match workers with Some w -> List.length w | None -> 1) in

  MultiWorker.call
    workers
    ~job:(recheck_job tcopt out_dir)
    ~merge:( fun () () -> () )
    ~next:(Bucket.make ~num_workers:num_workers file_tuples)
    ~neutral:()
