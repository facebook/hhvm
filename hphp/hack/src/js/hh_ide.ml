(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils
open Hh_json

(*****************************************************************************)
(* Globals *)
(*****************************************************************************)

let () = Ide.is_ide_mode := true

let (files: (Relative_path.t, string) Hashtbl.t) = Hashtbl.create 23

let globals = Hashtbl.create 23
let parse_errors = Hashtbl.create 23

(*****************************************************************************)
(* helpers *)
(*****************************************************************************)

let root = "/"
let make_path fn = Relative_path.create Relative_path.Root (root ^ fn)

let rec to_js_object json =
  match json with
  | JSON_Array l ->
      let l = List.map l to_js_object in
      let l = Array.of_list l in
      Js.Unsafe.inject (Js.array l)
  | JSON_Object l ->
      let l = List.map l begin fun (k, v) ->
        k, to_js_object v
      end in
      let l = Array.of_list l in
      Js.Unsafe.obj l
  | JSON_Bool b -> Js.Unsafe.inject (Js.bool b)
  | JSON_String s -> Js.Unsafe.inject (Js.string s)
  | JSON_Null -> Js.Unsafe.inject Js.null
  | JSON_Number i -> Js.Unsafe.inject (Js.number_of_float (float_of_string i))


let error el =
  let res =
    if el = [] then
      JSON_Object [ "passed",         JSON_Bool true;
               "errors",         JSON_Array [];
               "internal_error", JSON_Bool false;
             ]
    else
      let errors_json = List.map el (compose Errors.to_json Errors.to_absolute)
      in JSON_Object [ "passed",         JSON_Bool false;
                  "errors",         JSON_Array errors_json;
                  "internal_error", JSON_Bool false;
                ]
  in
  to_js_object res

(*****************************************************************************)

let type_fun tcopt x fn =
  match Parser_heap.find_fun_in_file fn x with
  | Some f ->
    let fun_ = Naming.fun_ tcopt f in
    Typing.fun_def tcopt fun_
  | None -> ()

let type_class tcopt x fn =
  match Parser_heap.find_class_in_file fn x with
  | Some cls ->
    let class_ = Naming.class_ tcopt cls in
    Typing.class_def tcopt class_
  | None -> ()

let make_funs_classes ast =
  List.fold_left ast ~f:begin fun (funs, classes, typedefs, consts) def ->
    match def with
    | Ast.Fun f -> f.Ast.f_name :: funs, classes, typedefs, consts
    | Ast.Class c -> funs, c.Ast.c_name :: classes, typedefs, consts
    | Ast.Typedef td -> funs, classes, td.Ast.t_id :: typedefs, consts
    | Ast.Constant cst -> funs, classes, typedefs, cst.Ast.cst_name :: consts
    | _ -> funs, classes, typedefs, consts
  end ~init:([], [], [], [])

let rec get_sub_classes classes =
  let sub_classes = SMap.fold (fun x _ acc -> SSet.add x acc) classes SSet.empty in
  SSet.fold get_sub_class sub_classes sub_classes

and get_sub_class cname acc =
  let sub_classes = try
    Hashtbl.find Typing_deps.extends_igraph cname
  with Not_found -> SSet.empty in
  SSet.fold begin fun sub_cname acc ->
    if SSet.mem sub_cname acc
    then acc
    else
      let acc = SSet.add sub_cname acc in
      get_sub_class sub_cname acc
  end sub_classes acc

let declare_file fn content =
  let _, old_funs, old_classes =
    try Hashtbl.find globals fn
    with Not_found -> true, [], []
  in
  List.iter old_funs begin fun (_, fname) ->
    Naming_heap.FunPosHeap.remove fname;
    Naming_heap.FunCanonHeap.remove (NamingGlobal.canon_key fname);
    Decl_heap.Funs.remove fname;
  end;
  List.iter old_classes begin fun (_, cname) ->
    Naming_heap.TypeIdHeap.remove cname;
    Naming_heap.TypeCanonHeap.remove (NamingGlobal.canon_key cname);
    Decl_heap.Classes.remove cname;
  end;
  try
    Autocomplete.auto_complete := false;
    let {Parser_hack.file_mode; comments; ast} =
      (* FIXME: Don't use default tcopt *)
      Parser_hack.program TypecheckerOptions.default fn content
    in
    let is_php = file_mode = None in
    Parser_heap.ParserHeap.add fn ast;
    if not is_php
    then begin
      let funs, classes, typedefs, consts = make_funs_classes ast in
      Hashtbl.replace globals fn (is_php, funs, classes);
      let tcopt = TypecheckerOptions.permissive in
      NamingGlobal.make_env ~funs ~classes ~typedefs ~consts;
      let all_classes = List.fold_right classes ~f:begin fun (_, cname) acc ->
        SMap.add cname (Relative_path.Set.singleton fn) acc
      end ~init:SMap.empty in
      Decl.make_env tcopt fn;
      let sub_classes = get_sub_classes all_classes in
      SSet.iter begin fun cname ->
        match Naming_heap.TypeIdHeap.get cname with
        | Some (p, `Class) ->
          let filename = Pos.filename p in
          Decl.declare_class_in_file tcopt filename cname
        | _ -> ()
      end sub_classes
    end
    else Hashtbl.replace globals fn (false, [], [])
  with _ ->
    Hashtbl.replace globals fn (true, [], []);
    ()

let rec last_error errors =
  match errors with
    | [] -> None
    | [e] -> Some e
    | _ :: tail -> last_error tail

let hh_add_file fn content =
  let fn = make_path fn in
  Hashtbl.replace files fn content;
  try
    let errors, _, _ = Errors.do_ begin fun () ->
      declare_file fn content
    end in
    Hashtbl.replace parse_errors fn (last_error (Errors.get_error_list errors))
  with e ->
    ()

let hh_add_dep fn content =
  let fn = make_path fn in
  Typing_deps.is_dep := true;
  (try
    Errors.ignore_ begin fun () ->
      declare_file fn content;
      Parser_heap.ParserHeap.remove fn
    end
  with e ->
    ()
  );
  Typing_deps.is_dep := false

let hh_check fn =
  let fn = make_path fn in
  match Hashtbl.find parse_errors fn with
    | Some e -> error [e]
    | None ->
      Autocomplete.auto_complete := false;
      Errors.try_
        begin fun () ->
        let ast = Parser_heap.ParserHeap.find_unsafe fn in
        let funs, classes, typedefs, consts = make_funs_classes ast in
        let tcopt = TypecheckerOptions.permissive in
        NamingGlobal.make_env ~funs ~classes ~typedefs ~consts;
        Decl.make_env tcopt fn;
        List.iter funs (fun (_, fname) -> type_fun tcopt fname fn);
        List.iter classes (fun (_, cname) -> type_class tcopt cname fn);
        error []
        end
        begin fun l ->
          error [l]
        end

let hh_auto_complete fn =
  let tcopt = TypecheckerOptions.permissive in
  let fn = make_path fn in
  AutocompleteService.attach_hooks();
  try
    let ast = Parser_heap.ParserHeap.find_unsafe fn in
    Errors.ignore_ begin fun () ->
      List.iter ast begin fun def ->
        match def with
        | Ast.Fun f ->
          let f = Naming.fun_ tcopt f in
          Typing.fun_def tcopt f
        | Ast.Class c ->
          let c = Naming.class_ tcopt c in
          Decl.class_decl tcopt c;
          Typing.class_def tcopt c
        | _ -> ()
      end;
    end;
    let completion_type_str =
      match !Autocomplete.argument_global_type with
      | Some Autocomplete.Acid -> "id"
      | Some Autocomplete.Acnew -> "new"
      | Some Autocomplete.Actype -> "type"
      | Some Autocomplete.Acclass_get -> "class_get"
      | Some Autocomplete.Acprop -> "var"
      | None -> "none" in
    let result = AutocompleteService.get_results tcopt SSet.empty SSet.empty in
    let result =
      List.map result AutocompleteService.autocomplete_result_to_json
    in
    AutocompleteService.detach_hooks();
    to_js_object (JSON_Object [ "completions",    JSON_Array result;
                          "completion_type", JSON_String completion_type_str;
                          "internal_error",  JSON_Bool false;
                        ])
  with _ ->
    AutocompleteService.detach_hooks();
    to_js_object (JSON_Object [ "internal_error", JSON_Bool true;
                        ])

(* Helpers to turn JavaScript strings into OCaml strings *)
let js_wrap_string_1 func =
  let f str = begin
    let str = Js.to_string str in
    func str
  end in
  Js.wrap_callback f

let js_wrap_string_2 func =
  let f str1 str2 = begin
    let str1 = Js.to_string str1 in
    let str2 = Js.to_string str2 in
    func str1 str2
  end in
  Js.wrap_callback f

let () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  Js.Unsafe.set Js.Unsafe.global "hh_check_file" (js_wrap_string_1 hh_check);
  Js.Unsafe.set Js.Unsafe.global "hh_add_file" (js_wrap_string_2 hh_add_file);
  Js.Unsafe.set Js.Unsafe.global "hh_add_dep" (js_wrap_string_2 hh_add_dep);
  Js.Unsafe.set Js.Unsafe.global "hh_auto_complete" (js_wrap_string_1 hh_auto_complete)
