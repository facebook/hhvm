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

(*****************************************************************************)
(* The "static" environment, initialized first and then doesn't change *)
(*****************************************************************************)

type genv = {
    options          : ServerArgs.options;
    workers          : Worker.t list option;
  }

(*****************************************************************************)
(* The environment constantly maintained by the server *)
(*****************************************************************************)

(* In addition to this environment, many functions are storing and
 * updating ASTs, NASTs, and types in a shared space
 * (see respectively Parser_heap, Naming_heap, Typing_env).
 * The Ast.id are keys to index this shared space.
 *)
type env = {
    files_info     : FileInfo.t SMap.t;
    nenv           : Naming.env;
    errorl         : error list;
    (* skip errors at initialization *)
    skip           : bool ref;
    (* the strings in those sets represent filenames *)
    failed_parsing : SSet.t;
    failed_decl    : SSet.t;
    failed_check   : SSet.t;
  }

(*****************************************************************************)
(* Killing the server  *)
(*****************************************************************************)

let die() =
  exit(0)

(*****************************************************************************)
(* Listing all the files present in the environment *)
(*****************************************************************************)

let list_files env oc =
  let acc = List.fold_right begin
    fun p acc ->
      let pos, _ = List.hd p in
      SSet.add pos.Pos.pos_file acc
  end env.errorl SSet.empty in
  SSet.iter (fun (s) -> Printf.fprintf oc "%s\n" s) acc;
  flush oc
