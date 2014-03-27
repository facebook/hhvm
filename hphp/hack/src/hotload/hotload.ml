(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Module saving/loading a pre-populated cache *)
(*****************************************************************************)
open Utils

(* The files with an error *)
type failed = SSet.t

(* The files with a parsing error *)
type failed_parsing = SSet.t

type filename = string

let filename () =
  let user = Sys.getenv "USER" in
  let filename = "/tmp/hh_server/state_" ^ user in
  filename

let save env ~failed_parsing ~failed filename =
  Shared.save (filename^"_shared");
  let oc = open_out (filename^"_env") in
  let igraph = !(Typing_deps.igraph) in
  let iclasses = !(Typing_deps.iclasses) in
  let ifuns = !(Typing_deps.ifuns) in
  Marshal.to_channel oc 
    (env, igraph, iclasses, ifuns, (failed_parsing, failed)) [];
  close_out oc;
  Printf.printf "State saved in %s\n" filename; flush stdout

let is_ready () =
  let filename = filename() in
  Sys.file_exists (filename^"_env")

let load workerl filename =
  Printf.printf "Loading State from: %s\n" filename; flush stdout;
  Shared.load (filename^"_shared");
    let ic = open_in (filename^"_env") in
  let (env, igraph, iclasses, ifuns, (failed_parsing, failed)) = 
    Marshal.from_channel ic in
  close_in ic;
  Typing_deps.igraph := !igraph;
  Typing_deps.iclasses := !iclasses;
  Typing_deps.ifuns := !ifuns;
  (env, failed_parsing, failed)
