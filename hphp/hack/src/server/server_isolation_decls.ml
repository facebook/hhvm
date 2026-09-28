(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let union_names (a : File_info.names) (b : File_info.names) : File_info.names =
  File_info.
    {
      n_funs = S_set.union a.n_funs b.n_funs;
      n_classes = S_set.union a.n_classes b.n_classes;
      n_types = S_set.union a.n_types b.n_types;
      n_consts = S_set.union a.n_consts b.n_consts;
      n_modules = S_set.union a.n_modules b.n_modules;
    }

let names_defined_in naming_table (files : Relative_path.t list) :
    File_info.names =
  List.fold files ~init:File_info.empty_names ~f:(fun acc file ->
      match Naming_table.get_file_info naming_table file with
      | None -> acc
      | Some file_info -> union_names acc (File_info.simplify file_info))

let remove_decls (names : File_info.names) : unit =
  let elems =
    Decl_class_elements.get_for_classes
      ~old:false
      (S_set.elements names.File_info.n_classes)
  in
  Decl_redecl_service.remove_defs names ~elems ~collect_garbage:false

(** Push a local-changes stack on each heap the backend uses, returning the pop
    that undoes exactly what was pushed.

    Each pop is recorded as its push succeeds, so a push that raises partway
    does not leave the server with stacks nobody unwinds. *)
let push_decl_stacks backend : unit -> unit =
  let pops = ref [] in
  let push p pop =
    p ();
    pops := pop :: !pops
  in
  let pop_all () =
    (* Every pop runs even if one raises, and the caches are invalidated either
       way: a heap left with a stack pushed, or a stale local cache, outlives
       the query and belongs to no one. The first failure is reraised once the
       rest have been unwound. *)
    let failure = ref None in
    (* [::] already put these in reverse order of pushing. *)
    List.iter !pops ~f:(fun pop ->
        try pop () with
        | exn ->
          let exn = Exception.wrap exn in
          if Option.is_none !failure then failure := Some exn);
    Shared_mem.invalidate_local_caches ();
    Option.iter !failure ~f:Exception.reraise
  in
  (* Which heaps a backend keeps in OCaml shared memory, mirroring
     [Provider_utils.respect_but_quarantine_unsaved_changes]. Keep the two in
     step: a heap left out here holds on to a decl built under the synthesized
     package, and nothing reports it. That function cannot be called instead —
     it ends by making the heaps read-only, and the rebuilt decls have to land.

     Under the Rust backend the shallow classes live in Rust and one push covers
     them, but member filters, ASTs and fixmes are still OCaml heaps. *)
  (try
     match backend with
     | Provider_backend.Rust_provider_backend be ->
       push
         (fun () -> Rust_provider_backend.push_local_changes be)
         (fun () -> Rust_provider_backend.pop_local_changes be);
       push
         Ast_provider.local_changes_push_sharedmem_stack
         Ast_provider.local_changes_pop_sharedmem_stack;
       push
         Decl_provider.local_changes_push_sharedmem_stack
         Decl_provider.local_changes_pop_sharedmem_stack;
       push
         Fixme_provider.local_changes_push_sharedmem_stack
         Fixme_provider.local_changes_pop_sharedmem_stack
     | Provider_backend.Shared_memory ->
       push
         Ast_provider.local_changes_push_sharedmem_stack
         Ast_provider.local_changes_pop_sharedmem_stack;
       push
         Decl_provider.local_changes_push_sharedmem_stack
         Decl_provider.local_changes_pop_sharedmem_stack;
       push
         File_provider.local_changes_push_sharedmem_stack
         File_provider.local_changes_pop_sharedmem_stack;
       push
         Fixme_provider.local_changes_push_sharedmem_stack
         Fixme_provider.local_changes_pop_sharedmem_stack;
       push
         Naming_provider.local_changes_push_sharedmem_stack
         Naming_provider.local_changes_pop_sharedmem_stack
     | backend ->
       (* No stacks pushed means nothing to restore, and the caller removes
          decls regardless. Refuse rather than leave that to be discovered. *)
       failwith
         (Printf.sprintf
            "isolation: no local-changes stacks known for backend %s"
            (Provider_backend.t_to_string backend))
   with
  | exn ->
    let exn = Exception.wrap exn in
    (* A pop that fails in turn must not replace the push failure, which is the
       one that says what went wrong. *)
    (try pop_all () with
    | pop_exn ->
      Hh_logger.log
        "isolation: unwinding a failed decl-stack push also failed: %s"
        (Exception.to_string (Exception.wrap pop_exn)));
    Exception.reraise exn);
  pop_all

let with_repackaged_decls ctx naming_table files ~allow_repackaging ~f =
  if not allow_repackaging then
    failwith
      "isolation: decl repackaging is off; set isolation_allow_decl_repackaging=true in hh.conf, on a server dedicated to this analysis";
  let backend = Provider_context.get_backend ctx in
  let names = names_defined_in naming_table files in
  let traced = !Typing_deps.trace in
  let pop_decl_stacks = push_decl_stacks backend in
  Typing_deps.trace := false;
  Utils.try_finally
    ~f:(fun () ->
      remove_decls names;
      (* Removing a decl from the shared heaps does not touch the process-local
         caches above them: [Decl_provider.Cache] holds folded classes, and a
         class folded before this point still carries the package it was folded
         under. Left in place, the typecheck reads the old package from cache
         and the candidate looks fine when it is not. *)
      Shared_mem.invalidate_local_caches ();
      f ())
    ~finally:(fun () ->
      (* Nested, so the trace flag and the stacks are restored even if the
         second deletion raises. Those two are process-wide and outlive the
         query; a decl left behind only costs the next lookup a recompute. *)
      Utils.try_finally
        ~f:(fun () -> remove_decls names)
        ~finally:(fun () ->
          Typing_deps.trace := traced;
          pop_decl_stacks ()))
