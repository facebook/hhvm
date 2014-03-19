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

let get_class_summary class_ =
  let class_name = snd class_.Nast.c_name in
  let res_list =
    (fst class_.Nast.c_name,
      snd class_.Nast.c_name, "class") :: [] in
  let res_list = List.fold_left begin fun acc method_ ->
    (fst method_.Nast.m_name,
      class_name ^ "::" ^ snd method_.Nast.m_name, "method"):: acc
    end res_list class_.Nast.c_methods in
  let res_list = List.fold_left begin fun acc method_ ->
    (fst method_.Nast.m_name,
      class_name ^ "::" ^ snd method_.Nast.m_name, "static method"):: acc
    end res_list class_.Nast.c_static_methods in
  let res_list = match class_.Nast.c_constructor with
  | None -> res_list
  | Some method_ -> (fst method_.Nast.m_name,
      class_name ^ "::" ^ snd method_.Nast.m_name, "method"):: res_list in
  res_list

let outline content =
  let ast = Parser_hack.program ~fail:false content in
  let funs, classes = List.fold_left begin fun (funs, classes) def ->
    match def with
    | Ast.Fun f ->
        let nenv = Naming.empty in
        let f = Naming.fun_ nenv f in
        (f.Nast.f_name :: funs, classes)
    | Ast.Class c ->
        let nenv = Naming.empty in
        let c = Naming.class_ nenv c in
        (funs, c :: classes)
    | _ -> (funs, classes)
  end ([], []) ast in
  let res_list = List.fold_left begin fun acc class_ ->
      List.rev_append (get_class_summary class_) acc
    end [] classes in
  List.fold_left begin fun acc f_name ->
      (fst f_name, snd f_name, "function") :: acc
    end res_list funs
