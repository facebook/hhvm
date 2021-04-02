(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module "naming" a program.
 *
 * The naming phase consists in several things
 * 1- get all the global names
 * 2- transform all the local names into a unique identifier
 *)

open Hh_prelude
module SN = Naming_special_names

(*****************************************************************************)
(* The types *)
(*****************************************************************************)

let category = "naming_global"

module GEnv = struct
  (** This function logs and error and raises an exception,
  ultimately causing the typechecker to terminate.
  Why? We looked for a file in the file heap, but it was deleted
  before we could get it. This occurs with highest probability when we
  have multiple large rebases in quick succession, and the typechecker
  doesn't get updates from watchman while checking. For now, we restart
  gracefully, but in future versions we'll be restarting the server on
  large rebases anyhow, so this is sufficient behavior.

  TODO(jjwu): optimize this. Instead of forcing a server restart,
  catch the exception in the recheck look and start another recheck cycle
  by adding more files to the unprocessed/partially-processed set in
  the previous loop. *)
  let file_disappeared_under_our_feet (pos, name) =
    let fn = FileInfo.get_pos_filename pos in
    Hh_logger.log "File missing: %s" (Relative_path.to_absolute fn);
    Hh_logger.log "Name missing: %s" name;
    raise File_provider.File_provider_stale

  let get_fun_full_pos ctx (pos, name) =
    match Naming_provider.get_fun_full_pos ctx (pos, name) with
    | Some pos -> (pos, name)
    | None -> file_disappeared_under_our_feet (pos, name)

  let get_type_full_pos ctx (pos, name) =
    match Naming_provider.get_type_full_pos ctx (pos, name) with
    | Some pos -> (pos, name)
    | None -> file_disappeared_under_our_feet (pos, name)

  let get_const_full_pos ctx (pos, name) =
    match Naming_provider.get_const_full_pos ctx (pos, name) with
    | Some pos -> (pos, name)
    | None -> file_disappeared_under_our_feet (pos, name)

  let type_canon_name ctx name = Naming_provider.get_type_canon_name ctx name

  let type_pos ctx name =
    match Naming_provider.get_type_pos ctx name with
    | Some pos ->
      let (p, _) = get_type_full_pos ctx (pos, name) in
      Some p
    | None -> None

  let type_info ctx name =
    match Naming_provider.get_type_pos_and_kind ctx name with
    | Some
        ( pos,
          ( ( Naming_types.TClass | Naming_types.TTypedef
            | Naming_types.TRecordDef ) as kind ) ) ->
      let (p, _) = get_type_full_pos ctx (pos, name) in
      Some (p, kind)
    | None -> None

  let fun_canon_name ctx name = Naming_provider.get_fun_canon_name ctx name

  let fun_pos ctx name =
    match Naming_provider.get_fun_pos ctx name with
    | Some pos ->
      let (p, _) = get_fun_full_pos ctx (pos, name) in
      Some p
    | None -> None

  let typedef_pos ctx name =
    match Naming_provider.get_type_pos_and_kind ctx name with
    | Some (pos, Naming_types.TTypedef) ->
      let (p, _) = get_type_full_pos ctx (pos, name) in
      Some p
    | Some (_, Naming_types.TClass)
    | Some (_, Naming_types.TRecordDef)
    | None ->
      None

  let gconst_pos ctx name =
    match Naming_provider.get_const_pos ctx name with
    | Some pos ->
      let (p, _) = get_const_full_pos ctx (pos, name) in
      Some p
    | None -> None

  let compare_pos ctx p q name =
    FileInfo.(
      match (p, q) with
      | (Full p', Full q') -> Pos.compare p' q' = 0
      | ((File (Fun, _) as p'), Full q')
      | (Full q', (File (Fun, _) as p')) ->
        let p' = fst (get_fun_full_pos ctx (p', name)) in
        Pos.compare p' q' = 0
      | ((File (Const, _) as p'), Full q')
      | (Full q', (File (Const, _) as p')) ->
        let p' = fst (get_const_full_pos ctx (p', name)) in
        Pos.compare p' q' = 0
      | ((File ((Class | Typedef | RecordDef), _) as p'), Full q')
      | (Full q', (File ((Class | Typedef | RecordDef), _) as p')) ->
        let p' = fst (get_type_full_pos ctx (p', name)) in
        Pos.compare p' q' = 0
      | (File (x, fn1), File (y, fn2)) ->
        Relative_path.equal fn1 fn2 && equal_name_type x y)
end

(* The primitives to manipulate the naming environment *)
module Env = struct
  let check_type_not_typehint ctx (p, name) =
    let x = String.lowercase (Utils.strip_all_ns name) in
    if
      SN.Typehints.is_reserved_hh_name x
      || SN.Typehints.is_reserved_global_name x
    then (
      let (p, name) = GEnv.get_type_full_pos ctx (p, name) in
      Errors.name_is_reserved name p;
      false
    ) else
      true

  let new_fun_skip_if_already_bound ctx fn (_p, name) =
    match Naming_provider.get_fun_canon_name ctx name with
    | Some _ -> ()
    | None ->
      let backend = Provider_context.get_backend ctx in
      Naming_provider.add_fun backend name (FileInfo.File (FileInfo.Fun, fn))

  let new_cid_skip_if_already_bound ctx fn (_p, name) cid_kind =
    let mode =
      match cid_kind with
      | Naming_types.TClass -> FileInfo.Class
      | Naming_types.TTypedef -> FileInfo.Typedef
      | Naming_types.TRecordDef -> FileInfo.RecordDef
    in
    match Naming_provider.get_type_canon_name ctx name with
    | Some _ -> ()
    | None ->
      let backend = Provider_context.get_backend ctx in
      (* We store redundant info in this case, but if the position is a *)
      (* Full position, we don't store the kind, so this is necessary *)
      Naming_provider.add_type backend name (FileInfo.File (mode, fn)) cid_kind

  let new_class_skip_if_already_bound ctx fn cid =
    new_cid_skip_if_already_bound ctx fn cid Naming_types.TClass

  let new_record_decl_skip_if_already_bound ctx fn cid =
    new_cid_skip_if_already_bound ctx fn cid Naming_types.TRecordDef

  let new_typedef_skip_if_already_bound ctx fn cid =
    new_cid_skip_if_already_bound ctx fn cid Naming_types.TTypedef

  let new_global_const_skip_if_already_bound ctx fn (_p, name) =
    let backend = Provider_context.get_backend ctx in
    Naming_provider.add_const backend name (FileInfo.File (FileInfo.Const, fn))

  let new_fun_error_if_already_bound ctx (p, name) =
    match Naming_provider.get_fun_canon_name ctx name with
    | Some canonical ->
      let p' = Option.value_exn (Naming_provider.get_fun_pos ctx canonical) in
      if not @@ GEnv.compare_pos ctx p' p canonical then
        let (p, name) = GEnv.get_fun_full_pos ctx (p, name) in
        let (p', canonical) = GEnv.get_fun_full_pos ctx (p', canonical) in
        Errors.error_name_already_bound name canonical p p'
    | None ->
      let backend = Provider_context.get_backend ctx in
      Naming_provider.add_fun backend name p

  let new_cid_error_if_already_bound ctx cid_kind (p, name) =
    if not (check_type_not_typehint ctx (p, name)) then
      ()
    else
      match Naming_provider.get_type_canon_name ctx name with
      | Some canonical ->
        let p' =
          Option.value_exn (Naming_provider.get_type_pos ctx canonical)
        in
        if not @@ GEnv.compare_pos ctx p' p canonical then
          let (p, name) = GEnv.get_type_full_pos ctx (p, name) in
          let (p', canonical) = GEnv.get_type_full_pos ctx (p', canonical) in
          Errors.error_name_already_bound name canonical p p'
      | None ->
        let backend = Provider_context.get_backend ctx in
        Naming_provider.add_type backend name p cid_kind

  let new_class_error_if_already_bound ctx =
    new_cid_error_if_already_bound ctx Naming_types.TClass

  let new_record_decl_error_if_already_bound ctx =
    new_cid_error_if_already_bound ctx Naming_types.TRecordDef

  let new_typedef_error_if_already_bound ctx =
    new_cid_error_if_already_bound ctx Naming_types.TTypedef

  let new_global_const_error_if_already_bound ctx (p, x) =
    match Naming_provider.get_const_pos ctx x with
    | Some p' ->
      if not @@ GEnv.compare_pos ctx p' p x then
        let (p, x) = GEnv.get_const_full_pos ctx (p, x) in
        let (p', x) = GEnv.get_const_full_pos ctx (p', x) in
        Errors.error_name_already_bound x x p p'
    | None ->
      let backend = Provider_context.get_backend ctx in
      Naming_provider.add_const backend x p
end

(*****************************************************************************)
(* Updating the environment *)
(*****************************************************************************)
let remove_decls ~backend ~funs ~classes ~record_defs ~typedefs ~consts =
  Naming_provider.remove_type_batch backend (record_defs @ typedefs @ classes);
  Naming_provider.remove_fun_batch backend funs;
  Naming_provider.remove_const_batch backend consts

(*****************************************************************************)
(* The entry point to build the naming environment *)
(*****************************************************************************)

let make_env_error_if_already_bound ctx fileinfo =
  List.iter fileinfo.FileInfo.funs (Env.new_fun_error_if_already_bound ctx);
  List.iter fileinfo.FileInfo.classes (Env.new_class_error_if_already_bound ctx);
  List.iter
    fileinfo.FileInfo.record_defs
    (Env.new_record_decl_error_if_already_bound ctx);
  List.iter
    fileinfo.FileInfo.typedefs
    (Env.new_typedef_error_if_already_bound ctx);
  List.iter
    fileinfo.FileInfo.consts
    (Env.new_global_const_error_if_already_bound ctx)

let make_env_skip_if_already_bound ctx fn fileinfo =
  List.iter fileinfo.FileInfo.funs (Env.new_fun_skip_if_already_bound ctx fn);
  List.iter
    fileinfo.FileInfo.classes
    (Env.new_class_skip_if_already_bound ctx fn);
  List.iter
    fileinfo.FileInfo.record_defs
    (Env.new_record_decl_skip_if_already_bound ctx fn);
  List.iter
    fileinfo.FileInfo.typedefs
    (Env.new_typedef_skip_if_already_bound ctx fn);
  List.iter
    fileinfo.FileInfo.consts
    (Env.new_global_const_skip_if_already_bound ctx fn);
  ()

(*****************************************************************************)
(* Declaring the names in a list of files *)
(*****************************************************************************)

let add_files_to_rename failed defl defs_in_env =
  List.fold_left
    ~f:
      begin
        fun failed (_, def) ->
        match defs_in_env def with
        | None -> failed
        | Some previous_definition_position ->
          let filename = Pos.filename previous_definition_position in
          Relative_path.Set.add failed filename
      end
    ~init:failed
    defl

let ndecl_file_skip_if_already_bound ctx fn fileinfo =
  make_env_skip_if_already_bound ctx fn fileinfo

let ndecl_file_error_if_already_bound ctx fn fileinfo =
  let (errors, ()) =
    Errors.do_with_context fn Errors.Naming (fun () ->
        Hh_logger.debug
          ~category
          "Naming decl: %s"
          (Relative_path.to_absolute fn);
        make_env_error_if_already_bound ctx fileinfo)
  in
  if Errors.is_empty errors then
    (errors, Relative_path.Set.empty)
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
    let type_canon_pos name =
      GEnv.type_canon_name ctx name |> Option.bind ~f:(GEnv.type_pos ctx)
    in
    let fun_canon_pos name =
      GEnv.fun_canon_name ctx name |> Option.bind ~f:(GEnv.fun_pos ctx)
    in

    let failed = Relative_path.Set.singleton fn in
    let failed =
      add_files_to_rename failed fileinfo.FileInfo.funs fun_canon_pos
    in
    let failed =
      add_files_to_rename failed fileinfo.FileInfo.classes type_canon_pos
    in
    let failed =
      add_files_to_rename failed fileinfo.FileInfo.record_defs type_canon_pos
    in
    let failed =
      add_files_to_rename failed fileinfo.FileInfo.typedefs type_canon_pos
    in
    let failed =
      add_files_to_rename failed fileinfo.FileInfo.consts (GEnv.gconst_pos ctx)
    in
    (errors, failed)
