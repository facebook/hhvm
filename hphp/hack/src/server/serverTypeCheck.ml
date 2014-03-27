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
(*****************************************************************************)
open Utils
open ServerEnv

(*****************************************************************************)
(* Debugging *)
(*****************************************************************************)

let print_defs prefix defs =
  List.iter begin fun (_, fname) ->
    Printf.printf "  %s %s\n" prefix fname;
  end defs

let print_fast_pos fast_pos =
  SMap.iter begin fun x (funs, classes) ->
    Printf.printf "File: %s\n" x;
    print_defs "Fun" funs;
    print_defs "Class" classes;
  end fast_pos;
  Printf.printf "\n";
  flush stdout;
  ()

let print_fast fast =
  SMap.iter begin fun x (funs, classes) ->
    Printf.printf "File: %s\n" x;
    SSet.iter (Printf.printf "  Fun %s\n") funs;
    SSet.iter (Printf.printf "  Class %s\n") classes;
  end fast;
  Printf.printf "\n";
  flush stdout;
  ()

(*****************************************************************************)
(* Given a set of Ast.id list produce a SSet.t (got rid of the positions)    *)
(*****************************************************************************)

let set_of_idl l =
  List.fold_left (fun acc (_, x) -> SSet.add x acc) SSet.empty l

(*****************************************************************************)
(* We want add all the declarations that were present in a file *before* the
 * current modification. The scenario:
 * File foo.php was defining the class A.
 * The user gets rid of class A (in foo.php)
 * In general, the type-checker determines what must be re-declared or
 * re-chechecked, by comparing the old and the new type-definitions.
 * That's why we are adding the 'old' definitions to the file.
 * In this case, the redecl phase (typing/typing_redecl_service.ml) is going
 * to compare the 'old' definition of A with the new one. It will realize that
 * the new one is missing, and go ahead and retype everything that depends
 * on A.
 * Without a call to add_old_decls, the class A wouldn't appear anywhere,
 * and we wouldn't realize that we have to re-check the types that depend
 * on A.
 *)
(*****************************************************************************)

let add_old_decls old_files_info fast =
  SMap.fold begin fun filename info_names acc ->
    match SMap.get filename old_files_info with
    | Some {FileInfo.consider_names_just_for_autoload = true}
    | None -> acc
    | Some old_info ->
      let old_info_names = FileInfo.simplify old_info in
      let info_names = FileInfo.merge_names old_info_names info_names in
      SMap.add filename info_names acc
  end fast fast

(*****************************************************************************)
(* Reparsing helpers.
 * It's called reparse (as opposed to parse) because it retrieves the tree
 * from the datanodes where the Asts are stored in a serialized format.
 * Important: we never ever want to reparse a file that was already parsed.
 * If that was the case, an older version of the file would come and replace
 * a newer one (the one we just parsed). It would lead to very
 * subtle/terrible bugs.
 * This is why reparse takes a file->ast (fast). If we try to reparse a file
 * that we already parsed, the data is left unchanged.
 *)
(*****************************************************************************)

let reparse fast files_info additional_files =
  SSet.fold begin fun x acc ->
    match SMap.get x fast with
    | None ->
        (try
           let info = SMap.find_unsafe x files_info in
           if info.FileInfo.consider_names_just_for_autoload then acc else
           let info_names = FileInfo.simplify info in
           SMap.add x info_names acc
         with Not_found ->
           acc)
    | Some _ -> acc
  end additional_files fast

(* The version with the definitions + their position *)
let reparse_with_pos fast files_info additional_files =
  SSet.fold (
  fun x acc ->
    match SMap.get x fast with
    | None ->
        (try
          let info = SMap.find_unsafe x files_info in
          SMap.add x info acc
        with Not_found ->
          acc)
    | Some _ -> acc
 ) additional_files fast

(*****************************************************************************)
(* Removes the names that were defined in the files *)
(*****************************************************************************)

let remove_decls env fast_parsed =
  let nenv = env.nenv in
  let nenv =
    SMap.fold begin fun fn _ nenv ->
      match SMap.get fn env.files_info with
      | Some {FileInfo.consider_names_just_for_autoload = true}
      | None -> nenv
      | Some {FileInfo.
              funs = funl;
              classes = classel;
              types = typel;
              consts = constl} ->
        let funs = set_of_idl funl in
        let classes = set_of_idl classel in
        let typedefs = set_of_idl typel in
        let consts = set_of_idl constl in
        let nenv = Naming.remove_decls nenv
            (funs, classes, typedefs, consts) in
        nenv
    end fast_parsed nenv
  in
  { env with nenv = nenv }

(*****************************************************************************)
(* Removes the files that failed *)
(*****************************************************************************)

let remove_failed fast failed =
  SSet.fold SMap.remove failed fast

(*****************************************************************************)
(* Parses the set of modified files *)
(*****************************************************************************)

let rec parsing genv env =
  Parser_heap.ParserHeap.remove_batch env.failed_parsing;
  SharedMem.collect();
  let get_next = Bucket.make (SSet.elements env.failed_parsing) in
  Parsing_service.go genv.workers env.files_info ~get_next

(*****************************************************************************)
(* At any given point in time, we want to know what each file defines.
 * The datastructure that maintains this information is called file_info.
 * This code updates the file information.
 *)
(*****************************************************************************)

let update_file_info genv env fast_parsed =
  Typing_deps.update_files genv.workers fast_parsed;
  let files_info = SMap.fold SMap.add fast_parsed env.files_info in
  files_info

(*****************************************************************************)
(* Defining the global naming environment.
 * Defines an environment with the names of all the globals (classes/funs).
 *)
(*****************************************************************************)

let declare_names env files_info fast_parsed =
  let env = remove_decls env fast_parsed in
  let errorl, failed_naming, nenv =
    SMap.fold Naming.ndecl_file fast_parsed ([], SSet.empty, env.nenv) in
  let fast = remove_failed fast_parsed failed_naming in
  let fast = FileInfo.simplify_fast fast in
  let env = { env with nenv = nenv } in
  env, errorl, failed_naming, fast

(*****************************************************************************)
(* Function called after parsing, does nothing by default. *)
(*****************************************************************************)

let hook_after_parsing = ref (fun _ _ -> ())

(*****************************************************************************)
(* Where the action is! *)
(*****************************************************************************)

let type_check genv env =

  Printf.printf "******************************************\n";
  Printf.printf "Files to recompute: %d\n" (SSet.cardinal env.failed_parsing);
  flush stdout;
  (* PARSING *)
  let t = Unix.gettimeofday() in
  let fast_parsed, errorl, failed_parsing = parsing genv env in
  let t2 = Unix.gettimeofday() in
  Printf.printf "Parsing: %f\n" (t2 -. t); flush stdout;
  let t = t2 in

  (* UPDATE FILE INFO *)
  let files_info = update_file_info genv env fast_parsed in

  (* BUILDING AUTOLOADMAP *)
  !hook_after_parsing genv { env with files_info };

  (* NAMING *)
  let env, errorl', failed_naming, fast =
    declare_names env files_info fast_parsed in
  let errorl = List.rev_append errorl' errorl in
  let nenv = env.nenv in

  (* COMPUTES WHAT MUST BE REDECLARED  *)
  let fast = reparse fast files_info env.failed_decl in
  let fast = add_old_decls env.files_info fast in

  let t2 = Unix.gettimeofday() in
  Printf.printf "Naming: %f\n" (t2 -. t); flush stdout;
  let t = t2 in

  let _, _, to_redecl_phase2, to_recheck1 =
    Typing_redecl_service.redo_type_decl
      ~update_pos:true genv.workers nenv fast in
  let t2 = Unix.gettimeofday() in
  Printf.printf "Determining changes: %f\n" (t2 -. t); flush stdout;
  let t = t2 in

  let to_redecl_phase2 = Typing_deps.get_files to_redecl_phase2 in
  let to_recheck1 = Typing_deps.get_files to_recheck1 in

  let fast_redecl_phase2 = reparse fast files_info to_redecl_phase2 in

  (* DECLARING TYPES: Phase2 *)
  let errorl', failed_decl, to_redecl2, to_recheck2 =
    Typing_redecl_service.redo_type_decl
      ~update_pos:false genv.workers nenv fast_redecl_phase2 in

  let t2 = Unix.gettimeofday() in
  Printf.printf "Type-decl: %f\n" (t2 -. t); flush stdout;
  let t = t2 in

  let errorl = List.rev_append errorl' errorl in
  let to_recheck2 = Typing_deps.get_files to_recheck2 in

  (* DECLARING TYPES: merging results of the 2 phases *)
  let fast = SMap.fold SMap.add fast fast_redecl_phase2 in
  let to_recheck = SSet.union env.failed_decl to_redecl_phase2 in
  let to_recheck = SSet.union to_recheck1 to_recheck in
  let to_recheck = SSet.union to_recheck2 to_recheck in

  (* TYPE CHECKING *)
  let to_recheck = SSet.union to_recheck env.failed_check in
  let fast = reparse fast files_info to_recheck in
  let errorl', failed_check = Typing_check_service.go genv.workers fast in

  (* If we found errors during the declaration phase,
   * it's best to only show those errors, because all the others are
   * probably a consequence of the original mistake.
   *)
  let errorl = if errorl <> [] then errorl else errorl' in

  let t2 = Unix.gettimeofday() in
  Printf.printf "Type-check: %f\n" (t2 -. t); flush stdout;

  (* Done, that's the new environment *)
  { skip = ref false;
    files_info = files_info;
    nenv = nenv;
    errorl = errorl;
    failed_parsing = SSet.union failed_naming failed_parsing;
    failed_decl = failed_decl;
    failed_check = failed_check;
  }

(*****************************************************************************)
(* Checks that the working directory is clean *)
(*****************************************************************************)

let check genv env =
  if !debug then begin
    Printf.printf "****************************************\n";
    Printf.printf "Start Check\n";
    flush stdout;
  end;
  type_check genv env

