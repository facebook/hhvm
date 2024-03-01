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
    match Naming_provider.get_fun_full_pos_by_parsing_file ctx (pos, name) with
    | Some pos -> (pos, name)
    | None -> file_disappeared_under_our_feet (pos, name)

  let get_type_full_pos ctx (pos, name) =
    match Naming_provider.get_type_full_pos_by_parsing_file ctx (pos, name) with
    | Some pos -> (pos, name)
    | None -> file_disappeared_under_our_feet (pos, name)

  let get_const_full_pos ctx (pos, name) =
    match
      Naming_provider.get_const_full_pos_by_parsing_file ctx (pos, name)
    with
    | Some pos -> (pos, name)
    | None -> file_disappeared_under_our_feet (pos, name)

  let get_module_full_pos ctx (pos, name) =
    match
      Naming_provider.get_module_full_pos_by_parsing_file ctx (pos, name)
    with
    | Some pos -> (pos, name)
    | None -> file_disappeared_under_our_feet (pos, name)

  let type_pos ctx name =
    match Naming_provider.get_type_pos ctx name with
    | Some pos ->
      let (p, _) = get_type_full_pos ctx (pos, name) in
      Some p
    | None -> None

  let type_info ctx name =
    match Naming_provider.get_type_pos_and_kind ctx name with
    | Some (pos, ((Naming_types.TClass | Naming_types.TTypedef) as kind)) ->
      let (p, _) = get_type_full_pos ctx (pos, name) in
      Some (p, kind)
    | None -> None

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
    | None ->
      None

  let gconst_pos ctx name =
    match Naming_provider.get_const_pos ctx name with
    | Some pos ->
      let (p, _) = get_const_full_pos ctx (pos, name) in
      Some p
    | None -> None

  let module_pos ctx name =
    match Naming_provider.get_module_pos ctx name with
    | Some pos -> Some (fst @@ get_module_full_pos ctx (pos, name))
    | None -> None
end

(** Given name-and-position [id], compared to the [canonical_id] name-and-position,
this judges whether we should report [id] as "already bound".

This is surprising. Notionally you'd expect that the mere existence of
a canonical_id means that we should report [id] as already-bound.
But this function has some historical quirks:
- The function was written to support [id] being a file-only
  position. But in practice it's always a full position that comes
  from parsing. If we encounter a file-only position, we log this
  anomalous path.
- The function was written so that if you declare [id], but it's
  in the exact same file+position as [canonical_id], then you must
  be declaring the same name and hence shouldn't report it as
  already-bound. This path should never arise (since a file's
  previous names all get removed before we decl that file);
  we log this anomalous path.
- From the way this function is invoked, [canonical_id] should
  always be either in a different file (meaning that the user declared
  a symbol in the currently-being-declared file which conflicts with another
  file), or in the same file (meaning that the user declared two conflicting
  symbols in the same file). The parameter [current_file_symbols_acc]
  gathers all the symbol positions we've declared so far from the current file.
  If we encounter a [canonical_id] that's in the same file but not already in
  [current_file_symbols_acc], that's anomalous, and we log it.
*)
let should_report_duplicate
    (ctx : Provider_context.t)
    (fi : FileInfo.ids)
    (current_file_symbols_acc : FileInfo.pos list)
    ~(id : FileInfo.id)
    ~(canonical_id : FileInfo.id) : bool =
  let open FileInfo in
  let p = id.pos in
  let name = id.name in
  let pc = canonical_id.pos in
  let canonical = canonical_id.name in
  (* helper, for the various paths below which want to log a bug *)
  let bug ~(desc : string) : unit =
    let () =
      Hh_logger.log
        "INVARIANT_VIOLATION_BUG [%s] %s %s"
        desc
        name
        (FileInfo.show_pos p)
    in
    Printf.eprintf
      "%s\n%!"
      (Exception.get_current_callstack_string 99 |> Exception.clean_stack);
    HackEventLogger.invariant_violation_bug
      desc
      ~path:(FileInfo.get_pos_filename p)
      ~telemetry:
        (Telemetry.create ()
        |> Telemetry.string_ ~key:"name" ~value:name
        |> Telemetry.string_ ~key:"canonical_name" ~value:canonical
        |> Telemetry.string_
             ~key:"canonical_path"
             ~value:(FileInfo.get_pos_filename pc |> Relative_path.to_absolute)
        |> Telemetry.string_ ~key:"fileinfo" ~value:(FileInfo.show_ids fi))
  in
  (* Detect anomaly where we're given a file-only [id] *)
  begin
    match p with
    | Full _ -> ()
    | File _ -> bug ~desc:"naming_duplicate_file_only_p"
  end;
  let is_same_pos =
    match (pc, p) with
    | (Full a, Full b) -> Pos.compare a b = 0
    | ((File (Fun, _) as a), Full b)
    | (Full b, (File (Fun, _) as a)) ->
      let a = fst (GEnv.get_fun_full_pos ctx (a, canonical)) in
      Pos.compare a b = 0
    | ((File (Const, _) as a), Full b)
    | (Full b, (File (Const, _) as a)) ->
      let a = fst (GEnv.get_const_full_pos ctx (a, canonical)) in
      Pos.compare a b = 0
    | ((File ((Class | Typedef), _) as a), Full b)
    | (Full b, (File ((Class | Typedef), _) as a)) ->
      let a = fst (GEnv.get_type_full_pos ctx (a, canonical)) in
      Pos.compare a b = 0
    | ((File (Module, _) as a), Full b)
    | (Full b, (File (Module, _) as a)) ->
      let a = fst (GEnv.get_module_full_pos ctx (a, canonical)) in
      Pos.compare a b = 0
    | (File (a, fna), File (b, fnb)) ->
      Relative_path.equal fna fnb && equal_name_type a b
  in
  (* Detect anomaly if [id] and [canonical_id] are identical positions *)
  if is_same_pos then bug ~desc:"naming_duplicate_same";
  (* Detect anomaly where [canonical_id] is in the same file but not found in [current_file_symbols_acc] *)
  if
    (not is_same_pos)
    && Relative_path.equal
         (FileInfo.get_pos_filename pc)
         (FileInfo.get_pos_filename p)
    && not (List.mem current_file_symbols_acc pc ~equal:FileInfo.equal_pos)
  then
    bug ~desc:"naming_duplicate_same_file_not_acc";
  (* Finally, should we report duplicates? Generally yes, except in that same anomalous case! *)
  not is_same_pos

(* The primitives to manipulate the naming environment *)
module Env = struct
  let new_fun_skip_if_already_bound ctx fn id =
    match Naming_provider.get_fun_canon_name ctx id.FileInfo.name with
    | Some _ -> ()
    | None ->
      let backend = Provider_context.get_backend ctx in
      Naming_provider.add_fun
        backend
        id.FileInfo.name
        (FileInfo.File (FileInfo.Fun, fn))

  let new_type_skip_if_already_bound ctx fn ~kind id =
    let name_type = Naming_types.type_kind_to_name_type kind in
    match Naming_provider.get_type_canon_name ctx id.FileInfo.name with
    | Some _ -> ()
    | None ->
      let backend = Provider_context.get_backend ctx in
      (* We store redundant info in this case, but if the position is a *)
      (* Full position, we don't store the kind, so this is necessary *)
      Naming_provider.add_type
        backend
        id.FileInfo.name
        (FileInfo.File (name_type, fn))
        kind

  let new_global_const_skip_if_already_bound ctx fn id =
    let backend = Provider_context.get_backend ctx in
    Naming_provider.add_const
      backend
      id.FileInfo.name
      (FileInfo.File (FileInfo.Const, fn))

  let new_fun_error_if_already_bound
      (ctx : Provider_context.t)
      (fi : FileInfo.ids)
      ((current_file_symbols_acc, is_okay_acc) : FileInfo.pos list * bool)
      (id : FileInfo.id) : FileInfo.pos list * bool =
    let p = id.FileInfo.pos in
    let name = id.FileInfo.name in
    match Naming_provider.get_fun_canon_name ctx name with
    | Some canonical ->
      let pos = Option.value_exn (Naming_provider.get_fun_pos ctx canonical) in
      let name = canonical in
      let decl_hash = None in
      let is_error =
        should_report_duplicate
          ctx
          fi
          current_file_symbols_acc
          ~id
          ~canonical_id:FileInfo.{ pos; name; decl_hash }
      in
      (current_file_symbols_acc, is_okay_acc && not is_error)
    | None ->
      let backend = Provider_context.get_backend ctx in
      Naming_provider.add_fun backend name p;
      (p :: current_file_symbols_acc, is_okay_acc)

  let new_type_error_if_already_bound
      (ctx : Provider_context.t)
      (fi : FileInfo.ids)
      ~(kind : Naming_types.kind_of_type)
      ((current_file_symbols_acc, is_okay_acc) : FileInfo.pos list * bool)
      (id : FileInfo.id) : FileInfo.pos list * bool =
    let p = id.FileInfo.pos in
    let name = id.FileInfo.name in
    match Naming_provider.get_type_canon_name ctx name with
    | Some canonical ->
      let pos = Option.value_exn (Naming_provider.get_type_pos ctx canonical) in
      let name = canonical in
      let decl_hash = None in
      let is_error =
        should_report_duplicate
          ctx
          fi
          current_file_symbols_acc
          ~id
          ~canonical_id:FileInfo.{ pos; name; decl_hash }
      in
      (current_file_symbols_acc, is_okay_acc && not is_error)
    | None ->
      let backend = Provider_context.get_backend ctx in
      Naming_provider.add_type backend name p kind;
      (p :: current_file_symbols_acc, is_okay_acc)

  let new_global_const_error_if_already_bound
      (ctx : Provider_context.t)
      (fi : FileInfo.ids)
      ((current_file_symbols_acc, is_okay_acc) : FileInfo.pos list * bool)
      (id : FileInfo.id) : FileInfo.pos list * bool =
    let p = id.FileInfo.pos in
    let name = id.FileInfo.name in
    match Naming_provider.get_const_pos ctx name with
    | Some pos ->
      let decl_hash = None in
      let is_error =
        should_report_duplicate
          ctx
          fi
          current_file_symbols_acc
          ~id
          ~canonical_id:FileInfo.{ pos; name; decl_hash }
      in
      (current_file_symbols_acc, is_okay_acc && not is_error)
    | None ->
      let backend = Provider_context.get_backend ctx in
      Naming_provider.add_const backend name p;
      (p :: current_file_symbols_acc, is_okay_acc)

  let new_module_skip_if_already_bound ctx fn id =
    let backend = Provider_context.get_backend ctx in
    Naming_provider.add_module
      backend
      id.FileInfo.name
      (FileInfo.File (FileInfo.Module, fn))

  let new_module_error_if_already_bound
      (ctx : Provider_context.t)
      (fi : FileInfo.ids)
      ((current_file_symbols_acc, is_okay_acc) : FileInfo.pos list * bool)
      (id : FileInfo.id) : FileInfo.pos list * bool =
    let p = id.FileInfo.pos in
    let name = id.FileInfo.name in
    match Naming_provider.get_module_pos ctx name with
    | Some pos ->
      let decl_hash = None in
      let is_error =
        should_report_duplicate
          ctx
          fi
          current_file_symbols_acc
          ~id
          ~canonical_id:FileInfo.{ pos; name; decl_hash }
      in
      (current_file_symbols_acc, is_okay_acc && not is_error)
    | None ->
      let backend = Provider_context.get_backend ctx in
      Naming_provider.add_module backend name p;
      (p :: current_file_symbols_acc, is_okay_acc)
end

(** Remove defs from reverse naming table *)
let remove_decls ~backend ~funs ~classes ~typedefs ~consts ~modules =
  Naming_provider.remove_type_batch backend (typedefs @ classes);
  Naming_provider.remove_fun_batch backend funs;
  Naming_provider.remove_const_batch backend consts;
  Naming_provider.remove_module_batch backend modules

let remove_decls_using_file_info backend ids =
  let id_name id = id.FileInfo.name in
  let { FileInfo.funs; classes; typedefs; consts; modules } = ids in
  remove_decls
    ~backend
    ~funs:(List.map ~f:id_name funs)
    ~classes:(List.map ~f:id_name classes)
    ~typedefs:(List.map ~f:id_name typedefs)
    ~consts:(List.map ~f:id_name consts)
    ~modules:(List.map ~f:id_name modules)

(*****************************************************************************)
(* The entry point to build the naming environment *)
(*****************************************************************************)

(** return true if no names were already bound; returns false if some were *)
let make_env_and_check_not_already_bound ctx ids =
  let { FileInfo.classes; funs; modules; typedefs; consts } = ids in
  let is_okay = true in
  (* funs *)
  let (_, is_okay) =
    List.fold
      funs
      ~init:([], is_okay)
      ~f:(Env.new_fun_error_if_already_bound ctx ids)
  in
  (* types *)
  let (current_file_symbols_acc, is_okay) =
    List.fold
      classes
      ~init:([], is_okay)
      ~f:(Env.new_type_error_if_already_bound ctx ids ~kind:Naming_types.TClass)
  in
  let (_, is_okay) =
    List.fold
      typedefs
      ~init:(current_file_symbols_acc, is_okay)
      ~f:
        (Env.new_type_error_if_already_bound
           ctx
           ids
           ~kind:Naming_types.TTypedef)
  in
  (* consts *)
  let (_, is_okay) =
    List.fold
      consts
      ~init:([], is_okay)
      ~f:(Env.new_global_const_error_if_already_bound ctx ids)
  in
  (* modules *)
  let (_, is_okay) =
    List.fold
      modules
      ~init:([], is_okay)
      ~f:(Env.new_module_error_if_already_bound ctx ids)
  in
  is_okay

let make_env_skip_if_already_bound ctx fn fileinfo =
  List.iter fileinfo.FileInfo.funs ~f:(Env.new_fun_skip_if_already_bound ctx fn);
  List.iter
    fileinfo.FileInfo.classes
    ~f:(Env.new_type_skip_if_already_bound ctx fn ~kind:Naming_types.TClass);
  List.iter
    fileinfo.FileInfo.typedefs
    ~f:(Env.new_type_skip_if_already_bound ctx fn ~kind:Naming_types.TTypedef);
  List.iter
    fileinfo.FileInfo.consts
    ~f:(Env.new_global_const_skip_if_already_bound ctx fn);
  List.iter
    fileinfo.FileInfo.modules
    ~f:(Env.new_module_skip_if_already_bound ctx fn);
  ()

(*****************************************************************************)
(* Declaring the names in a list of files *)
(*****************************************************************************)

let add_files_to_rename failed defl defs_in_env =
  List.fold_left
    ~f:
      begin
        fun failed id ->
          match defs_in_env id.FileInfo.name with
          | None -> failed
          | Some previous_definition_position ->
            let filename = Pos.filename previous_definition_position in
            Relative_path.Set.add failed filename
      end
    ~init:failed
    defl

let ndecl_file_skip_if_already_bound ctx fn fileinfo =
  make_env_skip_if_already_bound ctx fn fileinfo

let ndecl_file_and_get_conflict_files ctx fn ids =
  Hh_logger.debug ~category "Naming decl: %s" (Relative_path.to_absolute fn);
  let is_okay = make_env_and_check_not_already_bound ctx ids in
  if is_okay then
    Relative_path.Set.empty
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
      Naming_provider.get_type_canon_name ctx name
      |> Option.bind ~f:(GEnv.type_pos ctx)
    in
    let fun_canon_pos name =
      Naming_provider.get_fun_canon_name ctx name
      |> Option.bind ~f:(GEnv.fun_pos ctx)
    in

    let { FileInfo.classes; typedefs; funs; consts; modules } = ids in
    let failed = Relative_path.Set.singleton fn in
    let failed = add_files_to_rename failed funs fun_canon_pos in
    let failed = add_files_to_rename failed classes type_canon_pos in
    let failed = add_files_to_rename failed typedefs type_canon_pos in
    let failed = add_files_to_rename failed consts (GEnv.gconst_pos ctx) in
    let failed = add_files_to_rename failed modules (GEnv.module_pos ctx) in
    failed
