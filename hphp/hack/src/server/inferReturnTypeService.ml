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
open Nast
open Result

type t =
| Function of string
| Method of string * string

type result = (string, string) Result.t

let add_ns name =
  if String_utils.string_starts_with name "\\" then name else "\\" ^ name

let get_type_from_hint tcopt file ret =
  let tenv = map file (Typing_env.empty tcopt ~droot:None) in
  let decl_ty =
    map tenv (fun tenv -> Typing_instantiability.instantiable_hint tenv ret)
  in
  map decl_ty Typing_print.suggest

let infer_return_type tcopt c_or_f name =
  let def = map c_or_f (fun c_or_f -> Typing_suggest_service.Def c_or_f) in
  let types = map def (Typing_suggest_service.get_inferred_types tcopt) in
  let tyopt =
    map types
      (List.find
        ~f:(fun (_, p1, _, _) ->
          let p2 = map name fst in
          match map p2 (Pos.compare p1) with
          | Ok 0 -> true
          | _ -> false))
  in
  let ty = join @@ map tyopt (fun t -> of_option t "Could not infer type") in
  map ty (fun (tenv, _, _, ty) -> Typing_print.full tenv ty)

let get_fun_return_ty tcopt popt fun_name =
  let fun_name = add_ns fun_name in
  let pos =
    of_option (Naming_heap.FunPosHeap.get fun_name) "Could not find function"
  in
  let file = map pos FileInfo.get_pos_filename in
  let funopt =
    map file (fun file -> Parser_heap.find_fun_in_file popt file fun_name)
  in
  let f = join @@ map funopt (of_option ~error:"Could not find function") in
  let f = map f (Naming.fun_ tcopt) in
  let ret = map f (fun f -> f.f_ret) in
  bind ret
    begin function
    | None ->
      infer_return_type tcopt
        (map f (fun f -> Fun f))
        (map f (fun f -> f.f_name))
    | Some h -> get_type_from_hint tcopt file h
    end

let get_meth_return_ty tcopt popt class_name meth_name =
  let class_name = add_ns class_name in
  let pos =
    match Naming_heap.TypeIdHeap.get class_name with
    | Some (pos, `Class) -> Ok pos
    | _ -> Error "Could not find class"
  in
  let file = map pos FileInfo.get_pos_filename in
  let classopt =
     map file (fun file -> Parser_heap.find_class_in_file popt file class_name)
  in
  let c = join @@ map classopt (of_option ~error:"Could not find class") in
  let c = map c (Naming.class_ tcopt) in
  let meth_list = map c (fun c -> c.c_static_methods @ c.c_methods) in
  let mopt = map meth_list (List.find ~f:(fun m -> snd m.m_name = meth_name)) in
  let m = join @@ map mopt (of_option ~error:"Could not find method") in
  let ret = map m (fun m -> m.m_ret) in
  bind ret
    begin function
    | None ->
      infer_return_type tcopt
        (map c (fun c -> Class c))
        (map m (fun m -> m.m_name))
    | Some h -> get_type_from_hint tcopt file h
    end
