(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

type result = Pos.absolute Errors.error_ list

let go fnl oc =
  let fnl = List.fold_left begin fun acc fn ->
    match realpath fn with
    | Some path -> path :: acc
    | None ->
        Printf.fprintf stderr "Could not find file '%s'" fn;
        acc
  end [] fnl in
  let fnl = rev_rev_map (Relative_path.create Relative_path.Root) fnl in
  (* XXX(jezng): multiprocess this? *)
  let errs = List.fold_left begin fun acc fn ->
    let errs, ast = Lint.do_ begin fun () ->
      (* All lint errors are currently from parsing, but we could add more
       * stuff here later *)
      Parser_hack.from_file fn
    end in
    errs @ acc
  end [] fnl in
  let errs = rev_rev_map Errors.to_absolute errs in
  Marshal.to_channel oc (errs : result) [];
  flush oc
