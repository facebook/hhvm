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
open Reordered_argument_collections

(*****************************************************************************)
(* Error. *)
(*****************************************************************************)

let canon_set names =
  names
  |> SSet.elements
  |> List.map ~f:NamingGlobal.canon_key
  |> List.fold_left ~f:SSet.add ~init:SSet.empty

let shelve_funs names =
  Naming_heap.FunPosHeap.shelve_batch names;
  Naming_heap.FunCanonHeap.shelve_batch @@ canon_set names;
  Decl_heap.Funs.shelve_batch names;
  ()

let shelve_classes names =
  Naming_heap.TypeIdHeap.shelve_batch names;
  Naming_heap.TypeCanonHeap.shelve_batch @@ canon_set names;
  Decl_class_elements.(
    names |> SSet.elements |> get_for_classes ~old:false |> shelve_all
  );
  Decl_heap.Classes.shelve_batch names;
  ()

let shelve_typedefs names =
  Naming_heap.TypeIdHeap.shelve_batch names;
  Naming_heap.TypeCanonHeap.shelve_batch @@ canon_set names;
  Decl_heap.Typedefs.shelve_batch names

let shelve_consts names =
  Naming_heap.ConstPosHeap.shelve_batch names;
  Decl_heap.GConsts.shelve_batch names

let shelve_file name =
  Parser_heap.ParserHeap.shelve_batch @@
    Parser_heap.ParserHeap.KeySet.singleton name

let shelve_file_info path file_info =
  shelve_file path;
  let {
    FileInfo.n_funs; n_classes; n_types; n_consts
  } = FileInfo.simplify file_info in
  shelve_funs n_funs;
  shelve_classes n_classes;
  shelve_typedefs n_types;
  shelve_consts n_consts

let unshelve funs classes typedefs consts file_name =
  Decl_heap.Funs.unshelve_batch funs;
  Naming_heap.FunPosHeap.unshelve_batch funs;
  Naming_heap.FunCanonHeap.unshelve_batch @@ canon_set funs;

  (* It is important we unshelve the class elements first before we unshelve
   * the class itself. We already unshelved all the class elements that were
   * removed in declare_and_check. To restore the world to the original state
   * we need to unshelve all the remaining class elements.
   *)
  Decl_class_elements.(
    classes |> SSet.elements |> get_for_classes ~old:false |> unshelve_all
  );
  Decl_heap.Classes.unshelve_batch classes;
  Naming_heap.TypeIdHeap.unshelve_batch classes;
  Naming_heap.TypeCanonHeap.unshelve_batch @@ canon_set classes;

  Naming_heap.TypeIdHeap.unshelve_batch typedefs;
  Naming_heap.TypeCanonHeap.unshelve_batch @@ canon_set typedefs;
  Decl_heap.Typedefs.unshelve_batch typedefs;

  Naming_heap.ConstPosHeap.unshelve_batch consts;
  Decl_heap.GConsts.unshelve_batch consts;

  Fixmes.HH_FIXMES.unshelve_batch @@
    Fixmes.HH_FIXMES.KeySet.singleton file_name;
  Parser_heap.ParserHeap.unshelve_batch @@
    Parser_heap.ParserHeap.KeySet.singleton file_name

let unshelve_file_info path file_info =
  let {
    FileInfo.n_funs; n_classes; n_types; n_consts
  } = FileInfo.simplify file_info in
  unshelve n_funs n_classes n_types n_consts path

(** Surrounds f() with shelve and unshelve, but resilient to f throwing. Reraises
 * the exception thrown and ensures unshelve is always done. *)
let shelve_then_unshelve path file_info f =
  let () = shelve_file_info path file_info in
  let result = try f () with
  | e ->
    let () = unshelve_file_info path file_info in
    raise e
  in
  let () = unshelve_file_info path file_info in
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
  let result =
    Errors.ignore_ begin fun () ->
      Fixmes.HH_FIXMES.shelve_batch @@
        Fixmes.HH_FIXMES.KeySet.singleton path;
      let {Parser_hack.file_mode = _; comments = _; content = _; ast} =
        Parser_hack.program tcopt path content
      in
      let funs, classes, typedefs, consts =
        List.fold_left ast ~f:begin fun (funs, classes, typedefs, consts) def ->
        match def with
          | Ast.Fun { Ast.f_name; _ } ->
            f_name::funs, classes, typedefs, consts
          | Ast.Class { Ast.c_name; _ } ->
            funs, c_name::classes, typedefs, consts
          | Ast.Typedef { Ast.t_id; _ } ->
            funs, classes, t_id::typedefs, consts
          | Ast.Constant { Ast.cst_name; _ } ->
            funs, classes, typedefs, cst_name::consts
          | _ -> funs, classes, typedefs, consts
      end ~init:([], [], [], []) in

      let file_info = { FileInfo.empty_t with
        FileInfo.funs; classes; typedefs; consts;
      } in

      shelve_then_unshelve path file_info begin fun () ->
        Parser_heap.ParserHeap.add path (ast, Parser_heap.Full);
        NamingGlobal.make_env ~funs ~classes ~typedefs ~consts;
        let nast = Naming.program tcopt ast in
        List.iter nast begin function
          | Nast.Fun f -> Decl.fun_decl f
          | Nast.Class c -> Decl.class_decl tcopt c
          | Nast.Typedef t -> Decl.typedef_decl t
          | Nast.Constant cst -> Decl.const_decl cst
        end;

        (* If we remove a class member, there may still be child classes that
         * refer to that member. We can either invalidate all the extends_deps
         * of the classes we just declared, or unshelve the types of the class
         * elements back into the new heap. We choose to unshelve since it should
         * be faster, even though it is technically incorrect.
         *)
        classes
        |> List.map ~f:snd
        |> Decl_class_elements.unshelve_removed_elems;
        (* We must run all the declaration steps first to ensure that the
         * typechecking below sees all the new declarations. Lazy decl
         * won't work in this case because we haven't put the new ASTs into
         * the parsing heap. *)
        List.iter nast begin function
          | Nast.Fun f -> Typing.fun_def tcopt f;
          | Nast.Class c -> Typing.class_def tcopt c;
          | Nast.Typedef t -> Typing.typedef_def tcopt t;
          | Nast.Constant cst -> Typing.gconst_def cst tcopt;
        end;
        f path file_info
      end
    end
  in
  result

let recheck tcopt filetuple_l =
  SharedMem.invalidate_caches();
  List.iter filetuple_l begin fun (fn, defs) ->
    ignore @@ Typing_check_utils.check_defs tcopt fn defs
  end

let check_file_input tcopt files_info fi =
  match fi with
  | ServerUtils.FileContent content ->
      begin
        try
          declare_and_check content ~f:(fun path _ -> path) tcopt
        with Decl_class.Decl_heap_elems_bug -> begin
          Hh_logger.log "%s" content;
          Exit_status.(exit Decl_heap_elems_bug)
        end
      end
  | ServerUtils.FileName fn ->
      let path = Relative_path.create Relative_path.Root fn in
      let () = match Relative_path.Map.get files_info path with
        | Some fileinfo -> recheck tcopt [(path, fileinfo)]
        | None -> () in
      path
