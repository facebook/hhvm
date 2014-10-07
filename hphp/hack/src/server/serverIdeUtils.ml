(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Utils

(*****************************************************************************)
(* Error. *)
(*****************************************************************************)

let report_error exn =
  let exn_str = Printexc.to_string exn in
  Printf.printf "Could not auto-complete because of errors: %s\n" exn_str;
  flush stdout;
  ()

let oldify_funs names =
  Naming_heap.FunHeap.oldify_batch names;
  Typing_env.Funs.oldify_batch names;
  ()

let oldify_classes names =
  Naming_heap.ClassHeap.oldify_batch names;
  Typing_env.Classes.oldify_batch names;
  ()

let revive funs classes =
  Naming_heap.FunHeap.revive_batch funs;
  Typing_env.Funs.revive_batch funs;
  Naming_heap.ClassHeap.revive_batch classes;
  Typing_env.Classes.revive_batch classes;
  ()

let declare content =
  Autocomplete.auto_complete := false;
  Autocomplete.auto_complete_for_global := "";
  let declared_funs = ref SSet.empty in
  let declared_classes = ref SSet.empty in
  try
    Errors.ignore_ begin fun () ->
      let {Parser_hack.is_hh_file; comments; ast} =
        Parser_hack.program content
      in
      let funs, classes = List.fold_left begin fun (funs, classes) def ->
        match def with
          | Ast.Fun f -> SSet.add (snd f.Ast.f_name) funs, classes
          | Ast.Class c -> funs, SSet.add (snd c.Ast.c_name) classes
          | _ -> funs, classes
      end (SSet.empty, SSet.empty) ast in
      oldify_funs funs;
      oldify_classes classes;
      List.iter begin fun def ->
        match def with
        | Ast.Fun f ->
            let nenv = Naming.empty in
            let f = Naming.fun_ nenv f in
            if !Find_refs.find_method_at_cursor_target <> None then
              Find_refs.process_find_refs None
                (snd f.Nast.f_name) (fst f.Nast.f_name);
            let fname = (snd f.Nast.f_name) in
            Typing.fun_decl f;
            declared_funs := SSet.add fname !declared_funs;
        | Ast.Class c ->
            let nenv = Naming.empty in
            let c = Naming.class_ nenv c in
            if !Find_refs.find_method_at_cursor_target <> None then
              Find_refs.process_class_ref (fst c.Nast.c_name)
                (snd c.Nast.c_name) None;
            let cname = snd c.Nast.c_name in
            let all_methods = c.Nast.c_methods @ c.Nast.c_static_methods in
            if !Find_refs.find_method_at_cursor_target <> None then
            List.iter begin fun method_ ->
              Find_refs.process_find_refs (Some (snd c.Nast.c_name))
                (snd method_.Nast.m_name) (fst method_.Nast.m_name)
            end all_methods;
            (match c.Nast.c_constructor with
            | Some method_ ->
                Find_refs.process_find_refs (Some (snd c.Nast.c_name))
                  "__construct" (fst method_.Nast.m_name)
            | None -> ());
            declared_classes := SSet.add cname !declared_classes;
            Typing_decl.class_decl c;
            ()
        | _ -> ()
      end ast;
      !declared_funs, !declared_classes
    end
  with e ->
    report_error e;
    SSet.empty, SSet.empty

let fix_file_and_def content = try
  Errors.ignore_ begin fun () ->
    let {Parser_hack.is_hh_file; comments; ast} = Parser_hack.program content in
    List.iter begin fun def ->
      match def with
      | Ast.Fun f ->
          let nenv = Naming.empty in
          let f = Naming.fun_ nenv f in
          let filename = Pos.filename (fst f.Nast.f_name) in
          let tenv = Typing_env.empty filename in
          Typing.fun_def tenv (snd f.Nast.f_name) f
      | Ast.Class c ->
          let nenv = Naming.empty in
          let c = Naming.class_ nenv c in
          let filename = Pos.filename (fst c.Nast.c_name) in
          let tenv = Typing_env.empty filename in
          let res = Typing.class_def tenv (snd c.Nast.c_name) c in
          res
      | _ -> ()
    end ast;
  end
with e ->
  report_error e;
  ()
    
let recheck file_names =
  SharedMem.invalidate_caches();
  Errors.ignore_ begin fun () ->
    List.iter begin fun fn ->
      match Parser_heap.ParserHeap.get fn with
      | None -> ()
      | Some defs ->
          List.iter begin function
          | Ast.Fun f ->
              (try Typing_check_service.type_fun (snd f.Ast.f_name)
              with _ -> ())
          | Ast.Class c ->
              (try Typing_check_service.type_class (snd c.Ast.c_name)
              with _ -> ())
          | Ast.Stmt _ -> ()
          | Ast.Typedef { Ast.t_id = (_, tname); _ } ->
              (try Typing_check_service.check_typedef tname
              with _ -> ()
              )
          | Ast.Constant _ -> ()
          | Ast.Namespace _
          | Ast.NamespaceUse _ -> assert false
          end defs
    end file_names
  end

let check_file_input fi =
  match fi with
    | ServerMsg.FileContent content ->
        let funs, classes = declare content in
        fix_file_and_def content;
        revive funs classes;
    | ServerMsg.FileName fn ->
        recheck [fn];
