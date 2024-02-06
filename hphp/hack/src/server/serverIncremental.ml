(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** Removes the names that were defined in the files from the reverse naming table *)
let remove_defs_from_reverse_naming_table
    naming_table (defs_per_file_parsed : _ Relative_path.Map.t) =
  Relative_path.Map.iter defs_per_file_parsed ~f:(fun fn _ ->
      match Naming_table.get_file_info naming_table fn with
      | None -> ()
      | Some
          {
            FileInfo.ids = { FileInfo.funs; classes; typedefs; consts; modules };
            file_mode = _;
            comments = _;
            position_free_decl_hash = _;
          } ->
        (* we use [snd] to strip away positions *)
        let snd (_, x, _) = x in
        Naming_global.remove_decls
          ~backend:(Provider_backend.get ())
          ~funs:(List.map funs ~f:snd)
          ~classes:(List.map classes ~f:snd)
          ~typedefs:(List.map typedefs ~f:snd)
          ~consts:(List.map consts ~f:snd)
          ~modules:(List.map modules ~f:snd))

let get_old_and_new_defs_in_files
    (old_naming_table : Naming_table.t)
    (new_naming_table : Naming_table.t)
    (files : Relative_path.Set.t) :
    Decl_compare.VersionedNames.t Relative_path.Map.t =
  Relative_path.Set.fold
    files
    ~f:
      begin
        fun path acc ->
          let old_names =
            Naming_table.get_file_info old_naming_table path
            |> Option.map ~f:FileInfo.simplify
          in
          let new_names =
            Naming_table.get_file_info new_naming_table path
            |> Option.map ~f:FileInfo.simplify
          in
          match (old_names, new_names) with
          | (None, None) -> acc
          | _ ->
            Relative_path.Map.add
              acc
              ~key:path
              ~data:
                {
                  Decl_compare.VersionedNames.old_names =
                    Option.value old_names ~default:FileInfo.empty_names;
                  new_names =
                    Option.value new_names ~default:FileInfo.empty_names;
                }
      end
    ~init:Relative_path.Map.empty

(** If the only things that would change about file analysis are positions,
    we're not going to recheck it, and positions in its error list might
    become stale. Look if any of those positions refer to files that have
    actually changed and add them to files to recheck.

  @param reparsed   Set of files that were reparsed (so their ASTs and positions
                    in them could have changed.

  @param errors     Current global error list
*)
let add_files_with_stale_errors ctx ~reparsed errors files_acc =
  Errors.fold_errors errors ~init:files_acc ~f:(fun source error acc ->
      if
        List.exists (User_error.to_list_ error) ~f:(fun e ->
            Relative_path.Set.mem
              reparsed
              (fst e |> Naming_provider.resolve_position ctx |> Pos.filename))
      then
        Relative_path.Set.add acc source
      else
        acc)

let resolve_files ctx env fanout =
  Server_progress.with_message "resolving files" @@ fun () ->
  let { Fanout.changed = _; to_recheck; to_recheck_if_errors } = fanout in
  let files_to_recheck = Naming_provider.get_files ctx to_recheck in
  let files_with_errors_to_recheck =
    let files_with_errors = Errors.get_failed_files env.ServerEnv.errorl in
    let files_to_recheck_if_errors =
      Naming_provider.get_files ctx to_recheck_if_errors
    in
    Relative_path.Set.inter files_with_errors files_to_recheck_if_errors
  in
  Relative_path.Set.union files_to_recheck files_with_errors_to_recheck

let get_files_to_recheck ctx env fanout ~reparsed ~errors =
  resolve_files ctx env fanout
  (* We want to also recheck files that have typing errors referring to files that were
   * reparsed, since positions in those errors can be now stale. *)
  |> add_files_with_stale_errors ctx ~reparsed errors
