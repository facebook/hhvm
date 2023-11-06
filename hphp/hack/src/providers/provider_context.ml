(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module PositionedSyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

type entry_contents =
  | Not_yet_read_from_disk
  | Contents_from_disk of string
  | Provided_contents of string
  | Read_contents_from_disk_failed of Exception.t
  | Raise_exn_on_attempt_to_read

type entry = {
  path: Relative_path.t;
  mutable contents: entry_contents;
  mutable source_text: Full_fidelity_source_text.t option;
  mutable parser_return: Parser_return.t option;
  mutable ast_errors: Errors.t option;
  mutable cst: PositionedSyntaxTree.t option;
  mutable tast: Tast.program Tast_with_dynamic.t option;
  mutable all_errors: Errors.t option;
  mutable symbols: Relative_path.t SymbolOccurrence.t list option;
}

type entries = entry Relative_path.Map.t

type t = {
  popt: ParserOptions.t;
  tcopt: TypecheckerOptions.t;
  backend: Provider_backend.t;
  deps_mode: Typing_deps_mode.t;
  entries: entries;
}

let empty_for_tool ~popt ~tcopt ~backend ~deps_mode =
  { popt; tcopt; backend; deps_mode; entries = Relative_path.Map.empty }

let empty_for_worker ~popt ~tcopt ~deps_mode =
  {
    popt;
    tcopt;
    backend = Provider_backend.Shared_memory;
    deps_mode;
    entries = Relative_path.Map.empty;
  }

let empty_for_test ~popt ~tcopt ~deps_mode =
  {
    popt;
    tcopt;
    backend = Provider_backend.Shared_memory;
    deps_mode;
    entries = Relative_path.Map.empty;
  }

let empty_for_debugging ~popt ~tcopt ~deps_mode =
  {
    popt;
    tcopt;
    backend = Provider_backend.Shared_memory;
    deps_mode;
    entries = Relative_path.Map.empty;
  }

let make_entry ~(path : Relative_path.t) ~(contents : entry_contents) : entry =
  {
    path;
    contents;
    source_text = None;
    parser_return = None;
    ast_errors = None;
    cst = None;
    tast = None;
    all_errors = None;
    symbols = None;
  }

let add_or_overwrite_entry ~(ctx : t) (entry : entry) : t =
  {
    ctx with
    entries = Relative_path.Map.add ctx.entries ~key:entry.path ~data:entry;
  }

let add_or_overwrite_entry_contents
    ~(ctx : t) ~(path : Relative_path.t) ~(contents : string) : t * entry =
  let entry = make_entry ~path ~contents:(Provided_contents contents) in
  (add_or_overwrite_entry ~ctx entry, entry)

let add_entry_if_missing ~(ctx : t) ~(path : Relative_path.t) : t * entry =
  match Relative_path.Map.find_opt ctx.entries path with
  | Some entry -> (ctx, entry)
  | None ->
    let entry = make_entry ~path ~contents:Not_yet_read_from_disk in
    (add_or_overwrite_entry ~ctx entry, entry)

let get_popt (t : t) : ParserOptions.t = t.popt

let get_tcopt (t : t) : TypecheckerOptions.t = t.tcopt

let get_package_info (t : t) : PackageInfo.t =
  t.tcopt.GlobalOptions.tco_package_info

let map_tcopt (t : t) ~(f : TypecheckerOptions.t -> TypecheckerOptions.t) : t =
  { t with tcopt = f t.tcopt }

let get_backend (t : t) : Provider_backend.t = t.backend

let set_backend (t : t) (b : Provider_backend.t) = { t with backend = b }

let get_deps_mode (t : t) : Typing_deps_mode.t = t.deps_mode

let map_deps_mode (t : t) ~(f : Typing_deps_mode.t -> Typing_deps_mode.t) : t =
  { t with deps_mode = f t.deps_mode }

let get_entries (t : t) : entries = t.entries

let read_file_contents_exn (entry : entry) : string =
  match entry.contents with
  | Provided_contents contents
  | Contents_from_disk contents ->
    contents
  | Not_yet_read_from_disk ->
    (try
       let contents = Sys_utils.cat (Relative_path.to_absolute entry.path) in
       entry.contents <- Contents_from_disk contents;
       contents
     with
    | e ->
      (* Be sure to capture the exception and mark the entry contents as
         [Read_contents_from_disk_failed]. Otherwise, reading the contents may
         not be idempotent:

          1) We attempt to read the file from disk, but it doesn't exist, so we
          raise an exception.
          2) The file is created on disk.
          3) We attempt to read the file from disk again. Now it exists, and we
          return a different value.
      *)
      let e = Exception.wrap e in
      entry.contents <- Read_contents_from_disk_failed e;
      Exception.reraise e)
  | Raise_exn_on_attempt_to_read ->
    failwith
      (Printf.sprintf
         "Entry %s was marked as Raise_exn_on_attempt_to_read, but an attempt was made to read its contents"
         (Relative_path.to_absolute entry.path))
  | Read_contents_from_disk_failed e -> Exception.reraise e

let read_file_contents (entry : entry) : string option =
  try Some (read_file_contents_exn entry) with
  | _ -> None

let get_file_contents_if_present (entry : entry) : string option =
  match entry.contents with
  | Provided_contents contents
  | Contents_from_disk contents ->
    Some contents
  | Not_yet_read_from_disk
  | Raise_exn_on_attempt_to_read
  | Read_contents_from_disk_failed _ ->
    None

(** ref_is_quarantined stores the stack at which it was last changed,
so we can give better failwith error messages where appropriate. *)
let ref_is_quarantined : (bool * Utils.callstack) ref =
  ref (false, Utils.Callstack "init")

let is_quarantined () : bool = !ref_is_quarantined |> fst

let set_is_quarantined_internal () : unit =
  match !ref_is_quarantined with
  | (true, Utils.Callstack stack) ->
    failwith ("set_is_quarantined: was already quarantined at\n" ^ stack)
  | (false, _) ->
    ref_is_quarantined :=
      (true, Utils.Callstack (Exception.get_current_callstack_string 99))

let unset_is_quarantined_internal () : unit =
  match !ref_is_quarantined with
  | (true, _) ->
    ref_is_quarantined :=
      (false, Utils.Callstack (Exception.get_current_callstack_string 99))
  | (false, Utils.Callstack stack) ->
    failwith
      ("unset_is_quarantined: but quarantine had already been released at\n"
      ^ stack)

let get_telemetry (t : t) : Telemetry.t =
  let telemetry =
    Telemetry.create ()
    |> Telemetry.object_
         ~key:"entries"
         ~value:
           (Telemetry.create ()
           |> Telemetry.int_
                ~key:"count"
                ~value:(Relative_path.Map.cardinal t.entries)
           |> Telemetry.int_
                ~key:"size"
                ~value:
                  (Relative_path.Map.fold
                     t.entries
                     ~init:0
                     ~f:(fun _path entry acc ->
                       let contents =
                         get_file_contents_if_present entry
                         |> Option.value ~default:""
                       in
                       acc + String.length contents)))
    |> Telemetry.string_
         ~key:"backend"
         ~value:(t.backend |> Provider_backend.t_to_string)
    |> Telemetry.object_
         ~key:"SharedMem"
         ~value:(SharedMem.SMTelemetry.get_telemetry ())
    (* We get SharedMem telemetry for all providers, not just the SharedMem
       provider, just in case there are code paths which use SharedMem despite
       it not being the intended provider. *)
  in
  match t.backend with
  | Provider_backend.Local_memory
      {
        Provider_backend.shallow_decl_cache;
        decl_cache;
        folded_class_cache;
        reverse_naming_table_delta;
        fixmes;
        naming_db_path_ref = _;
      } ->
    let open Provider_backend in
    telemetry
    |> Decl_cache.get_telemetry decl_cache ~key:"decl_cache"
    |> Shallow_decl_cache.get_telemetry
         shallow_decl_cache
         ~key:"shallow_decl_cache"
    |> Folded_class_cache.get_telemetry
         folded_class_cache
         ~key:"folded_class_cache"
    |> Reverse_naming_table_delta.get_telemetry
         reverse_naming_table_delta
         ~key:"reverse_naming_table_delta"
    |> Fixmes.get_telemetry fixmes ~key:"fixmes"
  | _ -> telemetry

let reset_telemetry (t : t) : unit =
  match t.backend with
  | Provider_backend.Local_memory
      {
        Provider_backend.shallow_decl_cache;
        decl_cache;
        folded_class_cache;
        reverse_naming_table_delta = _;
        fixmes = _;
        naming_db_path_ref = _;
      } ->
    Provider_backend.Decl_cache.reset_telemetry decl_cache;
    Provider_backend.Shallow_decl_cache.reset_telemetry shallow_decl_cache;
    Provider_backend.Folded_class_cache.reset_telemetry folded_class_cache;
    ()
  | _ -> ()

let ctx_with_pessimisation_info_exn ctx info =
  match ctx.backend with
  | Provider_backend.Pessimised_shared_memory _ ->
    { ctx with backend = Provider_backend.Pessimised_shared_memory info }
  | _ ->
    failwith
      "This operation is only supported on contexts with a
      Provider_backend.Pessimised_shared_memory backend."

let noautodynamic this_class =
  match this_class with
  | None -> false
  | Some sc ->
    Typing_defs.Attributes.mem
      Naming_special_names.UserAttributes.uaNoAutoDynamic
      sc.Shallow_decl_defs.sc_user_attributes

let implicit_sdt_for_class ctx this_class =
  TypecheckerOptions.everything_sdt (get_tcopt ctx)
  && not (noautodynamic this_class)

let implicit_sdt_for_fun ctx fe =
  TypecheckerOptions.everything_sdt (get_tcopt ctx)
  && not fe.Typing_defs.fe_no_auto_dynamic

let no_auto_likes_for_fun fe = fe.Typing_defs.fe_no_auto_likes

let with_tcopt_for_autocomplete t =
  let tcopt =
    t.tcopt
    |> TypecheckerOptions.set_tco_autocomplete_mode
    |> TypecheckerOptions.set_skip_check_under_dynamic
  in
  { t with tcopt }
