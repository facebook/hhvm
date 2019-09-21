(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
(** Module "naming" a program.
 *
 * The naming phase consists in several things
 * 1- get all the global names
 * 2- transform all the local names into a unique identifier
 *)

open Utils
module SN = Naming_special_names

(*****************************************************************************)
(* The types *)
(*****************************************************************************)

let canon_key = String.lowercase

module GEnv = struct
  let get_full_pos (pos, name) =
    try
      match pos with
      | FileInfo.Full p -> (p, name)
      | FileInfo.File (FileInfo.Class, fn) ->
        let res = unsafe_opt (Ast_provider.find_class_in_file fn name) in
        let (p', _) = res.Aast.c_name in
        (p', name)
      | FileInfo.File (FileInfo.RecordDef, fn) ->
        let res = unsafe_opt (Ast_provider.find_record_def_in_file fn name) in
        let (p', _) = res.Aast.rd_name in
        (p', name)
      | FileInfo.File (FileInfo.Typedef, fn) ->
        let res = unsafe_opt (Ast_provider.find_typedef_in_file fn name) in
        let (p', _) = res.Aast.t_name in
        (p', name)
      | FileInfo.File (FileInfo.Const, fn) ->
        let res = unsafe_opt (Ast_provider.find_gconst_in_file fn name) in
        let (p', _) = res.Aast.cst_name in
        (p', name)
      | FileInfo.File (FileInfo.Fun, fn) ->
        let res = unsafe_opt (Ast_provider.find_fun_in_file fn name) in
        let (p', _) = res.Aast.f_name in
        (p', name)
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
      Hh_logger.log "Name missing: %s" name;
      raise File_provider.File_provider_stale

  let type_canon_name name = Naming_table.Types.get_canon_name (canon_key name)

  let type_pos name =
    let name = Option.value (type_canon_name name) ~default:name in
    match Naming_table.Types.get_pos name with
    | Some (pos, Naming_table.TClass)
    | Some (pos, Naming_table.TTypedef)
    | Some (pos, Naming_table.TRecordDef) ->
      let (p, _) = get_full_pos (pos, name) in
      Some p
    | None -> None

  let type_canon_pos name =
    let name = Option.value (type_canon_name name) ~default:name in
    type_pos name

  let type_info name =
    match Naming_table.Types.get_pos name with
    | Some (pos, Naming_table.TClass) ->
      let (p, _) = get_full_pos (pos, name) in
      Some (p, Naming_table.TClass)
    | Some (pos, Naming_table.TTypedef) ->
      let (p, _) = get_full_pos (pos, name) in
      Some (p, Naming_table.TTypedef)
    | Some (pos, Naming_table.TRecordDef) ->
      let (p, _) = get_full_pos (pos, name) in
      Some (p, Naming_table.TRecordDef)
    | None -> None

  let fun_canon_name name = Naming_table.Funs.get_canon_name (canon_key name)

  let fun_pos name =
    match Naming_table.Funs.get_pos name with
    | Some pos ->
      let (p, _) = get_full_pos (pos, name) in
      Some p
    | None -> None

  let fun_canon_pos name =
    let name = Option.value (fun_canon_name name) ~default:name in
    fun_pos name

  let typedef_pos name =
    match Naming_table.Types.get_pos name with
    | Some (pos, Naming_table.TTypedef) ->
      let (p, _) = get_full_pos (pos, name) in
      Some p
    | Some (_, Naming_table.TClass)
    | Some (_, Naming_table.TRecordDef)
    | None ->
      None

  let gconst_pos name =
    match Naming_table.Consts.get_pos name with
    | Some pos ->
      let (p, _) = get_full_pos (pos, name) in
      Some p
    | None -> None

  let compare_pos p q name =
    FileInfo.(
      match (p, q) with
      | (Full p', Full q') -> Pos.compare p' q' = 0
      | (Full q', (File _ as p'))
      | ((File _ as p'), Full q') ->
        let p' = fst (get_full_pos (p', name)) in
        Pos.compare p' q' = 0
      | (File (x, fn1), File (y, fn2)) -> fn1 = fn2 && x = y)
end

(* The primitives to manipulate the naming environment *)
module Env = struct
  let check_not_typehint (p, name) =
    let x = canon_key (Utils.strip_all_ns name) in
    if
      SN.Typehints.is_reserved_hh_name x
      || SN.Typehints.is_reserved_global_name x
    then (
      let (p, name) = GEnv.get_full_pos (p, name) in
      Errors.name_is_reserved name p;
      false
    ) else
      true

  (* Dont check for errors, just add to canonical heap *)
  let new_fun_fast fn name =
    let name_key = canon_key name in
    match Naming_table.Funs.get_canon_name name_key with
    | Some _ -> ()
    | None -> Naming_table.Funs.add name (FileInfo.File (FileInfo.Fun, fn))

  let new_cid_fast fn name cid_kind =
    let name_key = canon_key name in
    let mode =
      match cid_kind with
      | Naming_table.TClass -> FileInfo.Class
      | Naming_table.TTypedef -> FileInfo.Typedef
      | Naming_table.TRecordDef -> FileInfo.RecordDef
    in
    match Naming_table.Types.get_canon_name name_key with
    | Some _ -> ()
    | None ->
      (* We store redundant info in this case, but if the position is a *)
      (* Full position, we don't store the kind, so this is necessary *)
      Naming_table.Types.add name (FileInfo.File (mode, fn), cid_kind)

  let new_class_fast fn name = new_cid_fast fn name Naming_table.TClass

  let new_record_decl_fast fn name =
    new_cid_fast fn name Naming_table.TRecordDef

  let new_typedef_fast fn name = new_cid_fast fn name Naming_table.TTypedef

  let new_global_const_fast fn name =
    Naming_table.Consts.add name (FileInfo.File (FileInfo.Const, fn))

  let new_fun (p, name) =
    let name_key = canon_key name in
    match Naming_table.Funs.get_canon_name name_key with
    | Some canonical ->
      let p' = Option.value_exn (Naming_table.Funs.get_pos canonical) in
      if not @@ GEnv.compare_pos p' p canonical then
        let (p, name) = GEnv.get_full_pos (p, name) in
        let (p', canonical) = GEnv.get_full_pos (p', canonical) in
        Errors.error_name_already_bound name canonical p p'
    | None -> Naming_table.Funs.add name p

  let (attr_prefix, attr_prefix_len) =
    let a = "\\__attribute__" in
    (* lowercase because canon_key call *)
    (a, String.length a)

  let new_cid cid_kind (p, name) =
    let validate canonical error =
      let (p', _) =
        match Naming_table.Types.get_pos canonical with
        | Some x -> x
        | None ->
          failwith
            ( "Failed to get canonical pos for name "
            ^ name
            ^ " vs canonical "
            ^ canonical )
      in
      if not @@ GEnv.compare_pos p' p canonical then
        let (p, name) = GEnv.get_full_pos (p, name) in
        let (p', canonical) = GEnv.get_full_pos (p', canonical) in
        error name canonical p p'
    in
    if not (check_not_typehint (p, name)) then
      ()
    else
      let name_key = canon_key name in
      match Naming_table.Types.get_canon_name name_key with
      | Some canonical -> validate canonical Errors.error_name_already_bound
      | None ->
        (* Check to prevent collision with attribute classes
         * If we are checking \A, check \__Attribute__A and vice versa *)
        let name_len = String.length name_key in
        let alt_name_key =
          if
            name_len > attr_prefix_len
            && String.equal attr_prefix (String.sub name_key 0 attr_prefix_len)
          then
            "\\"
            ^ String.sub name_key attr_prefix_len (name_len - attr_prefix_len)
          else
            attr_prefix ^ String.sub name_key 1 (name_len - 1)
        in
        begin
          match Naming_table.Types.get_canon_name alt_name_key with
          | Some alt_canonical ->
            validate alt_canonical Errors.error_class_attribute_already_bound
          | None -> ()
        end;
        Naming_table.Types.add name (p, cid_kind)

  let new_class = new_cid Naming_table.TClass

  let new_record_decl = new_cid Naming_table.TRecordDef

  let new_typedef = new_cid Naming_table.TTypedef

  let new_global_const (p, x) =
    match Naming_table.Consts.get_pos x with
    | Some p' ->
      if not @@ GEnv.compare_pos p' p x then
        let (p, x) = GEnv.get_full_pos (p, x) in
        let (p', x) = GEnv.get_full_pos (p', x) in
        Errors.error_name_already_bound x x p p'
    | None -> Naming_table.Consts.add x p
end

(*****************************************************************************)
(* Updating the environment *)
(*****************************************************************************)
let remove_decls ~funs ~classes ~record_defs ~typedefs ~consts =
  let types = SSet.union classes typedefs in
  let types = SSet.union types record_defs in
  Naming_table.Types.remove_batch types;
  Naming_table.Funs.remove_batch funs;
  Naming_table.Consts.remove_batch consts

(*****************************************************************************)
(* The entry point to build the naming environment *)
(*****************************************************************************)

let make_env ~funs ~classes ~record_defs ~typedefs ~consts =
  List.iter funs Env.new_fun;
  List.iter classes Env.new_class;
  List.iter record_defs Env.new_record_decl;
  List.iter typedefs Env.new_typedef;
  List.iter consts Env.new_global_const

let make_env_from_fast fn ~funs ~classes ~record_defs ~typedefs ~consts =
  SSet.iter (Env.new_fun_fast fn) funs;
  SSet.iter (Env.new_class_fast fn) classes;
  SSet.iter (Env.new_record_decl_fast fn) record_defs;
  SSet.iter (Env.new_typedef_fast fn) typedefs;
  SSet.iter (Env.new_global_const_fast fn) consts

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

let ndecl_file_fast fn ~funs ~classes ~record_defs ~typedefs ~consts =
  make_env_from_fast fn ~funs ~classes ~record_defs ~typedefs ~consts

let ndecl_file
    fn
    {
      FileInfo.file_mode = _;
      funs;
      classes;
      record_defs;
      typedefs;
      consts;
      comments = _;
      hash = _;
    } =
  let (errors, _) =
    Errors.do_with_context fn Errors.Naming (fun () ->
        dn ("Naming decl: " ^ Relative_path.to_absolute fn);
        make_env ~funs ~classes ~record_defs ~typedefs ~consts)
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
    let failed = Relative_path.Set.singleton fn in
    let failed = add_files_to_rename failed funs GEnv.fun_canon_pos in
    let failed = add_files_to_rename failed classes GEnv.type_canon_pos in
    let failed = add_files_to_rename failed record_defs GEnv.type_canon_pos in
    let failed = add_files_to_rename failed typedefs GEnv.type_canon_pos in
    let failed = add_files_to_rename failed consts GEnv.gconst_pos in
    (errors, failed)
