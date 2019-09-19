(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

let make_local_changes () =
  Errors.set_allow_errors_in_default_path true;
  SharedMem.allow_hashtable_writes_by_current_process false;

  Ast_provider.local_changes_push_stack ();
  Decl_provider.local_changes_push_stack ();
  File_provider.local_changes_push_stack ();
  Fixme_provider.local_changes_push_stack ();

  Ide_parser_cache.activate ();

  Naming_table.push_local_changes ();
  ()

let revert_local_changes () =
  Errors.set_allow_errors_in_default_path false;
  SharedMem.allow_hashtable_writes_by_current_process true;

  Ast_provider.local_changes_pop_stack ();
  Decl_provider.local_changes_pop_stack ();
  File_provider.local_changes_pop_stack ();
  Fixme_provider.local_changes_pop_stack ();

  Ide_parser_cache.deactivate ();

  Naming_table.pop_local_changes ();

  SharedMem.invalidate_caches ();
  ()

(** Surrounds f() with make and revert, but resilient to f throwing. Reraises
 * the exception thrown and ensures revert is always done. *)
let make_then_revert_local_changes f () =
  make_local_changes ();
  let result =
    try f ()
    with e ->
      let stack = Caml.Printexc.get_raw_backtrace () in
      revert_local_changes ();
      Caml.Printexc.raise_with_backtrace e stack
  in
  revert_local_changes ();
  result

let path = Relative_path.default

let declare_and_check_ast ?(path = path) ?content ~make_ast ~f tcopt =
  Errors.do_
  @@ make_then_revert_local_changes (fun () ->
         Fixme_provider.remove_batch (Relative_path.Set.singleton path);
         let ast = make_ast () in
         let (funs, classes, typedefs, consts) = Nast.get_defs ast in
         let file_info =
           { FileInfo.empty_t with FileInfo.funs; classes; typedefs; consts }
         in
         let { FileInfo.n_funs; n_classes; n_types; n_consts } =
           FileInfo.simplify file_info
         in
         Ast_provider.provide_ast_hint path ast Ast_provider.Full;
         NamingGlobal.remove_decls
           ~funs:n_funs
           ~classes:n_classes
           ~typedefs:n_types
           ~consts:n_consts;
         NamingGlobal.make_env ~funs ~classes ~typedefs ~consts;

         (* Decl is not necessary to run typing, since typing would get
          * whatever it needs using lazy decl, but we run it anyway in order to
          * ensure that hooks attached to decl phase are executed. *)
         Decl.name_and_declare_types_program ast;

         let make_tast () =
           let nast = Naming.program ast in
           Typing.nast_to_tast tcopt nast
         in
         let tast =
           match content with
           | None -> make_tast ()
           | Some content -> Ide_tast_cache.get path content make_tast
         in
         f path file_info tast)

(* This will parse, declare and check all functions and classes in content
 * buffer.
 *
 * Declaring will overwrite definitions on shared heap, so before doing this,
 * the function will also "shelve" them (see functions above and
 * SharedMem.S.shelve_batch) - after working with local content is done,
 * original definitions can (and should) be restored using "unshelve".
 *)
let declare_and_check ?(path = path) content ~f tcopt =
  let make_ast () =
    let ast =
      if Ide_parser_cache.is_enabled () then
        (Ide_parser_cache.get_ast tcopt path content).Parser_return.ast
      else
        (* We need to fail open since IDE services need to be able to run over
         malformed files. *)
        (Full_fidelity_ast.defensive_program
           ~fail_open:true
           ~keep_errors:true
           ~quick:false
           tcopt
           path
           content)
          .Parser_return.ast
    in
    ast
  in
  try declare_and_check_ast ~make_ast ~f ~path ~content tcopt
  with Decl_class.Decl_heap_elems_bug ->
    Hh_logger.log "%s" content;
    Exit_status.(exit Decl_heap_elems_bug)

let get_errors path content tcopt =
  fst (declare_and_check content tcopt ~f:(fun _ _ _ -> ()) ~path)

let declare_and_check content ~f tcopt =
  snd (declare_and_check content ~f tcopt)

let recheck tcopt filetuple_l =
  SharedMem.invalidate_caches ();
  List.map filetuple_l (fun (fn, defs) ->
      (fn, fst @@ Typing_check_utils.type_file tcopt fn defs))

let check_file_input tcopt naming_table fi =
  match fi with
  | ServerCommandTypes.FileContent content ->
    declare_and_check content ~f:(fun path _ tast -> (path, tast)) tcopt
  | ServerCommandTypes.FileName fn ->
    let path = Relative_path.create Relative_path.Root fn in
    (match Naming_table.get_file_info naming_table path with
    | Some fileinfo ->
      let wrapper =
        if Ide_parser_cache.is_enabled () then
          (* Protect shared memory with local changes when using Ide_parser_cache *)
          fun f ->
        make_then_revert_local_changes f ()
        else
          fun f ->
        f ()
      in
      wrapper (fun () -> List.hd_exn (recheck tcopt [(path, fileinfo)]))
    | None -> (path, []))

let check_fileinfo tcopt path fileinfo =
  let (_path, tast) = List.hd_exn (recheck tcopt [(path, fileinfo)]) in
  tast

let check_ast tcopt ast =
  snd
  @@ declare_and_check_ast
       ~make_ast:(fun () -> ast)
       ~f:(fun _ _ tast -> tast)
       tcopt
