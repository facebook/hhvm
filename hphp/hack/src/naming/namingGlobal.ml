(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Module "naming" a program.
 *
 * The naming phase consists in several things
 * 1- get all the global names
 * 2- transform all the local names into a unique identifier
 *)
open Core
open Utils
open Naming_heap
module SN = Naming_special_names

(*****************************************************************************)
(* The types *)
(*****************************************************************************)

let canon_key = String.lowercase

module GEnv = struct
  let get_full_pos popt (pos, name) =
    try
    match pos with
      | FileInfo.Full p -> p, name
      | FileInfo.File (FileInfo.Class, fn) ->
        let res = unsafe_opt (Parser_heap.find_class_in_file popt fn name) in
        let (p', _) = res.Ast.c_name in
        p', name
      | FileInfo.File (FileInfo.Typedef, fn) ->
        let res = unsafe_opt (Parser_heap.find_typedef_in_file popt fn name) in
        let (p', _) = res.Ast.t_id in
        p', name
      | FileInfo.File (FileInfo.Const, fn) ->
        let res = unsafe_opt (Parser_heap.find_const_in_file popt fn name) in
        let (p', _) = res.Ast.cst_name in
        p', name
      | FileInfo.File (FileInfo.Fun, fn) ->
        let res = unsafe_opt (Parser_heap.find_fun_in_file popt fn name) in
        let (p', _) = res.Ast.f_name in
        p', name
    with Invalid_argument _ ->
      (* We looked for a file in the file heap, but it was deleted
        before we could get it. This occurs with highest probability when we
        have multiple large rebases in quick succession, and the typechecker
         doesn't get updates from watchman while checking. For now, we restart
        gracefully, but in future versions we'll be restarting the server on
        large rebases anyhow, so this is sufficient behavior.

        TODO(jjwu): optimize this. Instead of forcing a server restart,
        catch the exception in the recheck look and start another recheck cycle
        by adding more files to the unprocessed/partially-processed set in
        the previous loop.
      *)
      let fn = FileInfo.get_pos_filename pos in
      Hh_logger.log "File missing: %s" (Relative_path.to_absolute fn);
      Hh_logger.log "Name missing: %s" (name);
      raise File_heap.File_heap_stale

  let type_canon_name name = TypeCanonHeap.get (canon_key name)
  let type_pos popt name =
    let name = Option.value (type_canon_name name) ~default:name in
    match TypeIdHeap.get name with
      | Some (pos, `Class) ->
          let p, _ = get_full_pos popt (pos, name) in
          Some p
      | Some (pos, `Typedef) ->
          let p, _ = get_full_pos popt (pos, name) in
          Some p
      | None -> None

  let type_canon_pos popt name =
    let name = Option.value (type_canon_name name) ~default:name in
    type_pos popt name

  let type_info popt name = match TypeIdHeap.get name with
    | Some (pos, `Class) ->
        let p, _ = get_full_pos popt (pos, name) in
        Some (p, `Class)
    | Some (pos, `Typedef) ->
        let p, _ = get_full_pos popt (pos, name) in
        Some (p, `Typedef)
    | None -> None

  let fun_canon_name name = FunCanonHeap.get (canon_key name)

  let fun_pos popt name =
    match FunPosHeap.get name with
    | Some pos ->
        let p, _ = get_full_pos popt (pos, name) in
        Some p
    | None -> None

  let fun_canon_pos popt name =
    let name = Option.value (fun_canon_name name) ~default:name in
    fun_pos popt name

  let typedef_pos popt name = match TypeIdHeap.get name with
    | Some (pos, `Typedef) ->
        let p, _ = get_full_pos popt (pos, name) in
        Some p
    | Some (_, `Class)
    | None -> None

  let gconst_pos popt name =
    match ConstPosHeap.get name with
    | Some pos ->
      let p, _ = get_full_pos popt (pos, name) in
      Some p
    | None -> None


  let compare_pos p q =
    let open FileInfo in
    match p, q with
    | Full p', Full q' -> Pos.compare p' q' = 0
    | Full q', File(_, fn)
    | File (_, fn), Full q' ->
      let qf = Pos.filename q' in
      if fn = qf then
        assert_false_log_backtrace(
          Some "Compared file with full pos in same file"
        )
      else false
    | File (x, fn1), File (y, fn2) ->
      fn1 = fn2 && x = y


end

(* The primitives to manipulate the naming environment *)
module Env = struct
  let check_not_typehint popt (p, name) =
    let x = canon_key (Utils.strip_all_ns name) in
    if SN.Typehints.is_reserved_hh_name x ||
       SN.Typehints.is_reserved_global_name x
    then
      let p, name = GEnv.get_full_pos popt (p, name) in
      Errors.name_is_reserved name p; false
    else true

  (* Dont check for errors, just add to canonical heap *)
  let new_fun_fast fn name =
    let name_key = canon_key name in
    match FunCanonHeap.get name_key with
      | Some _ -> ()
      | None ->
        FunCanonHeap.add name_key name;
        FunPosHeap.add name (FileInfo.File (FileInfo.Fun, fn))

  let new_cid_fast fn name cid_kind =
    let name_key = canon_key name in
    let mode = match cid_kind with
    | `Class -> FileInfo.Class
    | `Typedef -> FileInfo.Typedef in
    match TypeCanonHeap.get name_key with
      | Some _ -> ()
      | None ->
        TypeCanonHeap.add name_key name;
        (* We store redundant info in this case, but if the position is a *)
        (* Full position, we don't store the kind, so this is necessary *)
        TypeIdHeap.write_through name ((FileInfo.File (mode, fn)), cid_kind)

  let new_class_fast fn name = new_cid_fast fn name `Class
  let new_typedef_fast fn name = new_cid_fast fn name `Typedef

  let new_global_const_fast fn name =
    ConstPosHeap.add name (FileInfo.File (FileInfo.Const, fn))

  let new_fun popt (p, name) =
    let name_key = canon_key name in
    match FunCanonHeap.get name_key with
    | Some canonical ->
      let p' = FunPosHeap.find_unsafe canonical in
      if not @@ GEnv.compare_pos p' p
      then
        let p, name = GEnv.get_full_pos popt (p, name) in
        let p', canonical = GEnv.get_full_pos popt (p', canonical) in
        Errors.error_name_already_bound name canonical p p'
    | None ->
      FunPosHeap.add name p;
      FunCanonHeap.add name_key name;
      ()

  let new_cid popt cid_kind (p, name) =
    if not (check_not_typehint popt (p, name)) then () else
    let name_key = canon_key name in
    match TypeCanonHeap.get name_key with
    | Some canonical ->
      let (p', _) = unsafe_opt @@ TypeIdHeap.get canonical in
      if not @@ GEnv.compare_pos p' p
      then
      let p, name = GEnv.get_full_pos popt (p, name) in
      let p', canonical = GEnv.get_full_pos popt (p', canonical) in
      Errors.error_name_already_bound name canonical p p'
    | None ->
      TypeIdHeap.write_through name (p, cid_kind);
      TypeCanonHeap.add name_key name;
      ()

  let new_class popt = new_cid popt `Class

  let new_typedef popt = new_cid popt `Typedef

  let new_global_const popt (p, x) =
    match ConstPosHeap.get x with
    | Some p' ->
      if not @@ GEnv.compare_pos p' p
      then
      let p, x = GEnv.get_full_pos popt (p, x) in
      let p', x = GEnv.get_full_pos popt (p', x) in
      Errors.error_name_already_bound x x p p'
    | None ->
      ConstPosHeap.add x p
end

(*****************************************************************************)
(* Updating the environment *)
(*****************************************************************************)
let remove_decls ~funs ~classes ~typedefs ~consts =
  let canonicalize_set = (fun elt acc -> SSet.add (canon_key elt) acc) in
  let types = SSet.union classes typedefs in
  let canon_types = SSet.fold canonicalize_set types SSet.empty in
  TypeCanonHeap.remove_batch canon_types;
  TypeIdHeap.remove_batch types;

  let fun_namekeys = SSet.fold canonicalize_set funs SSet.empty in
  FunCanonHeap.remove_batch fun_namekeys;
  FunPosHeap.remove_batch funs;

  ConstPosHeap.remove_batch consts

(*****************************************************************************)
(* The entry point to build the naming environment *)
(*****************************************************************************)

let make_env popt ~funs ~classes ~typedefs ~consts =
  List.iter funs (Env.new_fun popt);
  List.iter classes (Env.new_class popt);
  List.iter typedefs (Env.new_typedef popt);
  List.iter consts (Env.new_global_const popt)


let make_env_from_fast fn ~funs ~classes ~typedefs ~consts =
  SSet.iter (Env.new_fun_fast fn) funs;
  SSet.iter (Env.new_class_fast fn) classes;
  SSet.iter (Env.new_typedef_fast fn) typedefs;
  SSet.iter (Env.new_global_const_fast fn) consts


(*****************************************************************************)
(* Declaring the names in a list of files *)
(*****************************************************************************)

let add_files_to_rename failed defl defs_in_env =
  List.fold_left ~f:begin fun failed (_, def) ->
    match defs_in_env def with
    | None -> failed
    | Some previous_definition_position ->
      let filename = Pos.filename previous_definition_position in
      Relative_path.Set.add failed filename
  end ~init:failed defl

let ndecl_file_fast fn ~funs ~classes ~typedefs ~consts =
  make_env_from_fast fn ~funs ~classes ~typedefs ~consts

let ndecl_file popt fn
              { FileInfo.file_mode = _; funs; classes; typedefs; consts;
                comments = _} =
  let errors, _, _ = Errors.do_ begin fun () ->
    dn ("Naming decl: "^Relative_path.to_absolute fn);
    make_env popt ~funs ~classes ~typedefs ~consts
  end in
  if Errors.is_empty errors
  then errors, Relative_path.Set.empty
  else
  (* IMPORTANT:
   * If a file has name collisions, we MUST add the list of files that
   * were previously defining the type to the set of "failed" files.
   * If we fail to do so, we will be in a phony state, where a name could
   * be missing.
   *
   * Example:
   * A.php defines class A
   * B.php defines class B
   * Save the state, now let's introduce a new file (foo.php):
   * foo.php defines class A and class B.
   *
   * 2 things happen (cf serverTypeCheck.ml):
   * We remove the names A and B from the global environment.
   * We report the error.
   *
   * But this is clearly not enough. If the user removes the file foo.php,
   * both class A and class B are now missing from the naming environment.
   * If the user has a file using class A (in strict), he now gets the
   * error "Unbound name class A".
   *
   * The solution consist in adding all the files that were previously
   * defining the same things as foo.php to the set of files to recheck.
   *
   * This way, when the user removes foo.php, A.php and B.php are recomputed
   * and the naming environment is in a sane state.
   *
   * XXX (jezng): we can probably be less conservative about this -- instead
   * of adding all the declarations in the file, why not just add those that
   * were actually duplicates?
   *)
  let failed = Relative_path.Set.singleton fn in
  let failed = add_files_to_rename failed funs (GEnv.fun_canon_pos popt) in
  let failed = add_files_to_rename failed classes (GEnv.type_canon_pos popt) in
  let failed = add_files_to_rename failed typedefs (GEnv.type_canon_pos popt) in
  let failed = add_files_to_rename failed consts (GEnv.gconst_pos popt) in
  errors, failed
