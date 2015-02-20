(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open ServerEnv
open Utils

type result = Pos.absolute Errors.error_ list

let lint _acc fnl =
  List.fold_left begin fun acc fn ->
    let errs, ()  =
      Lint.do_ begin fun () ->
        let {Parser_hack.file_mode; comments; ast} =
          Parser_hack.from_file fn in
        let funs, classes, typedefs, consts = Ast_utils.get_defs ast in
        Parser_heap.ParserHeap.add fn ast;
        (* naming and typing currently don't produce any lint errors *)
        let fi =
          {FileInfo.file_mode; funs; classes; typedefs; consts; comments;
           consider_names_just_for_autoload = false} in
        Linter.lint fi
      end in
    errs @ acc
  end [] fnl

let go genv fnl oc =
  let fnl = List.fold_left begin fun acc fn ->
    match realpath fn with
    | Some path -> path :: acc
    | None ->
        Printf.fprintf stderr "Could not find file '%s'" fn;
        acc
  end [] fnl in
  let fnl = rev_rev_map (Relative_path.create Relative_path.Root) fnl in
  let errs =
    if List.length fnl > 10
    then
      MultiWorker.call
        genv.workers
        ~job:lint
        ~merge:List.rev_append
        ~neutral:[]
        ~next:(Bucket.make fnl)
    else
      lint [] fnl in
  let errs = rev_rev_map Errors.to_absolute errs in
  Marshal.to_channel oc (errs : result) [];
  flush oc
