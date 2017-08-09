(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

let make_local_changes () =
  Fixmes.HH_FIXMES.LocalChanges.push_stack();
  File_heap.FileHeap.LocalChanges.push_stack();
  Parser_heap.ParserHeap.LocalChanges.push_stack();

  Naming_heap.FunPosHeap.LocalChanges.push_stack();
  Naming_heap.FunCanonHeap.LocalChanges.push_stack();
  Naming_heap.TypeIdHeap.LocalChanges.push_stack();
  Naming_heap.TypeCanonHeap.LocalChanges.push_stack();
  Naming_heap.ConstPosHeap.LocalChanges.push_stack();

  Decl_heap.Funs.LocalChanges.push_stack();
  Decl_heap.Constructors.LocalChanges.push_stack();
  Decl_heap.Props.LocalChanges.push_stack();
  Decl_heap.StaticProps.LocalChanges.push_stack();
  Decl_heap.Methods.LocalChanges.push_stack();
  Decl_heap.StaticMethods.LocalChanges.push_stack();
  Decl_heap.Classes.LocalChanges.push_stack();
  Decl_heap.Typedefs.LocalChanges.push_stack();
  Decl_heap.GConsts.LocalChanges.push_stack();
  ()

let revert_local_changes () =
  Fixmes.HH_FIXMES.LocalChanges.pop_stack();
  File_heap.FileHeap.LocalChanges.pop_stack();
  Parser_heap.ParserHeap.LocalChanges.pop_stack();

  Naming_heap.FunPosHeap.LocalChanges.pop_stack();
  Naming_heap.FunCanonHeap.LocalChanges.pop_stack();
  Naming_heap.TypeIdHeap.LocalChanges.pop_stack();
  Naming_heap.TypeCanonHeap.LocalChanges.pop_stack();
  Naming_heap.ConstPosHeap.LocalChanges.pop_stack();

  Decl_heap.Funs.LocalChanges.pop_stack();
  Decl_heap.Constructors.LocalChanges.pop_stack();
  Decl_heap.Props.LocalChanges.pop_stack();
  Decl_heap.StaticProps.LocalChanges.pop_stack();
  Decl_heap.Methods.LocalChanges.pop_stack();
  Decl_heap.StaticMethods.LocalChanges.pop_stack();
  Decl_heap.Classes.LocalChanges.pop_stack();
  Decl_heap.Typedefs.LocalChanges.pop_stack();
  Decl_heap.GConsts.LocalChanges.pop_stack();
  ()

(** Surrounds f() with make and revert, but resilient to f throwing. Reraises
 * the exception thrown and ensures revert is always done. *)
let make_then_revert_local_changes f () =
  make_local_changes ();
  let result = try f () with
    | e ->
      revert_local_changes ();
      raise e
  in
  revert_local_changes ();
  result

let path = Relative_path.default
(* This will parse, declare and check all functions and classes in content
 * buffer.
 *
 * Declaring will overwrite definitions on shared heap, so before doing this,
 * the function will also "shelve" them (see functions above and
 * SharedMem.S.shelve_batch) - after working with local content is done,
 * original definitions can (and should) be restored using "unshelve".
 *)
let declare_and_check content ~f tcopt =
  let tcopt = TypecheckerOptions.make_permissive tcopt in
  Autocomplete.auto_complete := false;
  Autocomplete.auto_complete_for_global := "";
  Errors.ignore_ @@ make_then_revert_local_changes begin fun () ->
    Fixmes.HH_FIXMES.(remove_batch @@ KeySet.singleton path);
    let {Parser_hack.file_mode = _; comments = _; content = _; ast} =
      Parser_hack.program tcopt path content
    in
    let funs, classes, typedefs, consts =
      List.fold_left ast ~f:begin fun (funs, classes, typedefs, consts) def ->
        match def with
        | Ast.Fun { Ast.f_name; _ } ->
          (FileInfo.pos_full f_name)::funs, classes, typedefs, consts
        | Ast.Class { Ast.c_name; _ } ->
          funs, (FileInfo.pos_full c_name)::classes, typedefs, consts
        | Ast.Typedef { Ast.t_id; _ } ->
          funs, classes, (FileInfo.pos_full t_id)::typedefs, consts
        | Ast.Constant { Ast.cst_name; _ } ->
          funs, classes, typedefs, (FileInfo.pos_full cst_name)::consts
        | _ -> funs, classes, typedefs, consts
      end ~init:([], [], [], []) in

    let file_info = { FileInfo.empty_t with
                      FileInfo.funs; classes; typedefs; consts;
                    } in
    let { FileInfo.n_funs; n_classes; n_types; n_consts; } =
      FileInfo.simplify file_info in
    Parser_heap.ParserHeap.add path (ast, Parser_heap.Full);
    NamingGlobal.remove_decls
      ~funs:n_funs
      ~classes:n_classes
      ~typedefs:n_types
      ~consts:n_consts;
    NamingGlobal.make_env tcopt ~funs ~classes ~typedefs ~consts;

    (* Decl is not necessary to run typing, since typing would get
     * whatever it needs using lazy decl, but we run it anyway in order to
     * ensure that hooks attached to decl phase are executed. *)
    Decl.name_and_declare_types_program tcopt ast;

    let nast = Naming.program tcopt ast in

    List.iter nast begin function
      | Nast.Fun f -> ignore (Typing.fun_def tcopt f);
      | Nast.Class c -> ignore (Typing.class_def tcopt c);
      | Nast.Typedef t -> ignore (Typing.typedef_def tcopt t);
      | Nast.Constant cst -> ignore (Typing.gconst_def cst tcopt);
    end;
    f path file_info
  end

let declare_and_check content ~f tcopt =
  try
    declare_and_check content ~f tcopt
  with Decl_class.Decl_heap_elems_bug -> begin
    Hh_logger.log "%s" content;
    Exit_status.(exit Decl_heap_elems_bug)
  end

let recheck tcopt filetuple_l =
  SharedMem.invalidate_caches();
  List.iter filetuple_l begin fun (fn, defs) ->
    ignore @@ Typing_check_utils.check_defs tcopt fn defs
  end

let check_file_input tcopt files_info fi =
  match fi with
  | ServerUtils.FileContent content ->
      declare_and_check content ~f:(fun path _ -> path) tcopt
  | ServerUtils.FileName fn ->
      let path = Relative_path.create Relative_path.Root fn in
      let () = match Relative_path.Map.get files_info path with
        | Some fileinfo -> recheck tcopt [(path, fileinfo)]
        | None -> () in
      path
