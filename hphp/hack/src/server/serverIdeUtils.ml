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

let report_error errl =
  let err_str = Utils.pmsg_l errl in
  Printf.printf "Could not auto-complete because of errors: %s\n" err_str;
  flush stdout;
  ()

let oldify_fun (_, fname) =
  let names = SSet.singleton fname in
  Naming_heap.FunHeap.oldify_batch names;
  Typing_env.Funs.oldify_batch names;
  ()

let oldify_class (_, cname) =
  let names = SSet.singleton cname in
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
  Silent.is_silent_mode := true;
  Autocomplete.auto_complete_for_global := "";
  Autocomplete.auto_complete_result := SMap.empty;
  let declared_funs = ref SSet.empty in
  let declared_classes = ref SSet.empty in
  try
    let ast = Parser_hack.program ~fail:false content in
    List.iter begin fun def ->
      match def with
      | Ast.Fun f ->
          oldify_fun f.Ast.f_name;
          let nenv = Naming.empty in
          let f = Naming.fun_ nenv f in
          if !Find_refs.find_method_at_cursor_target <> None then
            Find_refs.process_find_refs None
              (snd f.Nast.f_name) (fst f.Nast.f_name);
          let fname = (snd f.Nast.f_name) in
          Typing.fun_decl f;
          declared_funs := SSet.add fname !declared_funs;
      | Ast.Class c ->
          oldify_class c.Ast.c_name;
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
  with
  | Error l ->
      report_error l;
      SSet.empty, SSet.empty
  | _ ->
      SSet.empty, SSet.empty

let fix_file_and_def content =
  try
    let ast = Parser_hack.program ~fail:false content in
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
    ()
  with
  | Error l ->
      report_error l;
      ()
  | _ -> ()

let recheck file_names =
  SharedMem.invalidate_caches();
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
        | Ast.Typedef { Ast.t_id = (_, tname) } ->
            (try Typing_check_service.check_typedef tname
            with _ -> ()
            )
        | Ast.Constant _ -> ()
        | Ast.Namespace _
        | Ast.NamespaceUse _ -> assert false
        end defs
  end file_names
