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
(* File checking that the errors are "sane".
 * I know this is going to come as a surprise, but sometimes, we have bugs
 * in Hack ;-)
 * What happens is that for one reason or another, the server didn't properly
 * update one of the types. When this happens, it gets frustrating for the
 * user. We end up in a situation where we are showing errors on files that
 * no longer exist, or with an older version of the file.
 * This module checks the consistency of our view of the world for any given
 * error. That is, given an error, it takes the files that where involved,
 * checks that we have the correct version of all the types involved (using
 * time-stamps).
 *)
(*****************************************************************************)
open Utils

(*****************************************************************************)
(* The working environment *)
(*****************************************************************************)

type env = {
    (* The files we have already visited. *)
    visited   : SSet.t ref;

    (* The report (A list of messages describing the problems encountered). *)
    report    : string list ref;

    (* The file we are currently working on. *)
    file      : string;

    (* The time-stamp (last modified) of the file we are currenlty working on*)
    mtime     : float;

    (* True if we want to log what has been visited *)
    log       : bool;

    (* Log level (to pretty print the right amount of spaces) *)
    log_level : int ref;
  }

(*****************************************************************************)
(* Reporting primitive *)
(*****************************************************************************)

let add_report env msg =
  let msg = "File "^env.file^": "^msg in
  env.report := msg :: !(env.report)

(*****************************************************************************)
(* Given a class, retrieves the files where this class could be defined. *)
(*****************************************************************************)

(* Remember that our inverted graph of dependencies is an approximation.
 * We don't know exactly where a class is defined, we know all the places
 * where it "could" be defined. So really, we are doing too much work, 
 * but it is good enough in practice.
 *)
let files_from_class class_name =
  let dep = Typing_deps.Dep.Class class_name in
  let dset = Typing_deps.DSet.singleton dep in
  Typing_deps.get_additional_files dset

(*****************************************************************************)
(* Logs (when turned on) *)
(*****************************************************************************)

let log env s =
  if not env.log then () else
  begin
    for i = 0 to !(env.log_level) do
      output_string stdout " ";
    done;
    Printf.printf "%s\n" s;
    flush stdout;
  end

(*****************************************************************************)
(* Checking primitives *)
(*****************************************************************************)

let rec check_file env fn =
  if SSet.mem fn !(env.visited) then () else
  begin
    env.visited := SSet.add fn !(env.visited);
    log env ("Checking file: "^fn^string_of_int (SSet.cardinal !(env.visited)));
    let env = 
      { env with 
        file  = fn;
        mtime = (Unix.stat fn).Unix.st_mtime; }
    in
    match Parser_heap.ParserHeap.get fn with
    | None      -> add_report env " file should have been removed"
    | Some defs -> List.iter (check_definition env) defs
  end
          
and check_definition env = function
  | Ast.Fun f   -> check_function env f
  | Ast.Class c -> check_class env c
  | Ast.Typedef _ -> (* TODO *) ()
  | Ast.Stmt _  -> ()

and check_function env f =
  let mtime = f.Ast.f_mtime in
  if mtime <> env.mtime
  then add_report env ("function "^env.file^" has an old AST")
  else ()

and check_class env c =
  let cname = snd c.Ast.c_name in
  log env ("Checking class: "^cname);
  let mtime = c.Ast.c_mtime in  
  if mtime <> env.mtime
  then add_report env ("class "^cname^" has an old AST");
  match Typing_env.Classes.get cname with
  | None -> ()
  | Some tc -> check_class_type env cname tc

and check_class_type env cname tc =
  let mtime = tc.Typing_defs.tc_mtime in
  if mtime <> env.mtime
  then add_report env ("class "^cname^" has on old type");
  SMap.iter begin fun parent_name _ ->
    SSet.iter (check_file env) (files_from_class parent_name)
  end tc.Typing_defs.tc_ancestors

(*****************************************************************************)
(* Initialization *)
(*****************************************************************************)

let initial_env() =
  (* We empty the cache to avoid cache inconsistencies *)
  Typing_env.Classes.invalidate_cache();
  Typing_env.Funs.invalidate_cache();
  Typing_env.Typedefs.invalidate_cache();
  Typing_env.GConsts.invalidate_cache();
  {
   visited   = ref SSet.empty;
   report    = ref [];
   file      = "";
   mtime     = 0.0;
   log       = false;
   log_level = ref 0;
 }

(*****************************************************************************)
(* Entry points *)
(*****************************************************************************)

let check_error env pos_msg_l =
  List.iter begin fun (pos, _) ->
    check_file env (Pos.filename pos)
  end pos_msg_l

let check_errors errorl =
  let env = initial_env() in
  List.iter (check_error env) errorl;
  !(env.report)

let check_first_error errorl =
  match errorl with
  | [] -> []
  | x :: _ ->
      let env = initial_env() in
      check_error env x;
      !(env.report)

let check_files files =
  let env = initial_env() in
  SSet.iter (check_file env) files;
  ()
