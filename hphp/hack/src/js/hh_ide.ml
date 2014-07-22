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
open Hh_json

(*****************************************************************************)
(* Globals *)
(*****************************************************************************)

let () = Ide.is_ide_mode := true

let (files: (string, string) Hashtbl.t) = Hashtbl.create 23

let globals = Hashtbl.create 23

(*****************************************************************************)
(* helpers *)
(*****************************************************************************)

(* Javascript function that turns a Mlstring object into a javscript string *)
external to_byte_jsstring: string -> Js.js_string Js.t = "caml_js_from_byte_string"

let output_json json =
  to_byte_jsstring (json_to_string json)

let error el =
  let res =
    if el = [] then
      JAssoc [ "passed",         JBool true;
               "errors",         JList [];
               "internal_error", JBool false;
             ]
    else
      let errors_json = List.map Errors.to_json el in
      JAssoc [ "passed",         JBool false;
               "errors",         JList errors_json;
               "internal_error", JBool false;
             ]
  in
  output_json res

(*****************************************************************************)

let type_fun x fn =
  try
    let tenv = Typing_env.empty fn in
    let fun_ = Naming_heap.FunHeap.find_unsafe x in
    Typing.fun_def tenv x fun_;
  with Not_found ->
    ()

let type_class x fn =
  try
    let class_ = Naming_heap.ClassHeap.find_unsafe x in
    let tenv = Typing_env.empty fn in
    Typing.class_def tenv x class_
  with Not_found ->
    ()

let make_funs_classes ast =
  List.fold_left begin fun (funs, classes, typedefs, consts) def ->
    match def with
    | Ast.Fun f -> f.Ast.f_name :: funs, classes, typedefs, consts
    | Ast.Class c -> funs, c.Ast.c_name :: classes, typedefs, consts
    | Ast.Typedef td -> funs, classes, td.Ast.t_id :: typedefs, consts
    | Ast.Constant cst -> funs, classes, typedefs, cst.Ast.cst_name :: consts
    | _ -> funs, classes, typedefs, consts
  end ([], [], [], []) ast

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
  List.iter begin fun (_, fname) ->
      Naming_heap.FunHeap.remove fname;
      Typing_env.Funs.remove fname;
  end old_funs;
  List.iter begin fun (_, cname) ->
    Naming_heap.ClassHeap.remove cname;
    Typing_env.Classes.remove cname;
  end old_classes;
  try
    Pos.file := fn ;
    Autocomplete.auto_complete := false;
    let {Parser_hack.is_hh_file; comments; ast} =
      Parser_hack.program content
    in
    let is_php = not is_hh_file in
    if is_hh_file
    then begin
      Parser_heap.ParserHeap.add fn ast;
      let funs, classes, typedefs, consts = make_funs_classes ast in
      Hashtbl.replace globals fn (is_php, funs, classes);
      let nenv = Naming.make_env Naming.empty ~funs ~classes ~typedefs ~consts in
      let all_classes = List.fold_right begin fun (_, cname) acc ->
        SMap.add cname (SSet.singleton fn) acc
      end classes SMap.empty in
      Typing_decl.make_env nenv all_classes fn;
      let sub_classes = get_sub_classes all_classes in
      SSet.iter begin fun cname ->
        match Naming_heap.ClassHeap.get cname with
        | None -> ()
        | Some c -> Typing_decl.class_decl c
      end sub_classes
    end
    else Hashtbl.replace globals fn (false, [], [])
  with _ ->
    Hashtbl.replace globals fn (true, [], []);
    ()

let hh_add_file fn content =
  Hashtbl.replace files fn content;
  try
    declare_file fn content
  with e ->
    ()

let hh_check ?(check_mode=true) fn =
  declare_file fn (Hashtbl.find files fn);
  Pos.file := fn;
  Autocomplete.auto_complete := false;
  let content = Hashtbl.find files fn in
  Errors.try_
    begin fun () ->
(*    let builtins = Parser.program lexer (Lexing.from_string Ast.builtins) in *)
    let {Parser_hack.is_hh_file; comments; ast} =
      Parser_hack.program content
    in
    let ast = (*builtins @ *) ast in
    Parser_heap.ParserHeap.add fn ast;
    let funs, classes, typedefs, consts = make_funs_classes ast in
    let nenv = Naming.make_env Naming.empty ~funs ~classes ~typedefs ~consts in
    let all_classes = List.fold_right begin fun (_, cname) acc ->
      SMap.add cname (SSet.singleton fn) acc
    end classes SMap.empty in
    Typing_decl.make_env nenv all_classes fn;
    List.iter (fun (_, fname) -> type_fun fname fn) funs;
    List.iter (fun (_, cname) -> type_class cname fn) classes;
    error []
    end
    begin fun l ->
      error [l]
    end

let hh_auto_complete fn =
  AutocompleteService.attach_hooks();
  let content = Hashtbl.find files fn in
  try
    let {Parser_hack.is_hh_file; comments; ast} =
      Parser_hack.program content
    in
    List.iter begin fun def ->
      match def with
      | Ast.Fun f ->
          let nenv = Naming.empty in
          let tenv = Typing_env.empty fn in
          let f = Naming.fun_ nenv f in
          Typing.fun_def tenv (snd f.Nast.f_name) f
      | Ast.Class c ->
          let nenv = Naming.empty in
          let tenv = Typing_env.empty fn in
          let c = Naming.class_ nenv c in
          Typing_decl.class_decl c;
          let res = Typing.class_def tenv (snd c.Nast.c_name) c in
          res
      | _ -> ()
    end ast;
    let completion_type_str =
      match !Autocomplete.argument_global_type with
      | Some Autocomplete.Acid -> "id"
      | Some Autocomplete.Acnew -> "new"
      | Some Autocomplete.Actype -> "type"
      | Some Autocomplete.Acclass_get -> "class_get"
      | Some Autocomplete.Acvar -> "var"
      | None -> "none" in
    let result = AutocompleteService.get_results [] [] in
    let result =
      List.map AutocompleteService.autocomplete_result_to_json result
    in
    AutocompleteService.detach_hooks();
    output_json (JAssoc [ "completions",     JList result;
                          "completion_type", JString completion_type_str;
                          "internal_error",  JBool false;
                        ])
  with _ ->
    AutocompleteService.detach_hooks();
    output_json (JAssoc [ "internal_error", JBool true;
                        ])

let hh_get_method_at_position fn line char =
  Find_refs.find_method_at_cursor_result := None;
  Autocomplete.auto_complete := false;
  Find_refs.find_method_at_cursor_target := Some (line, char);
  let content = Hashtbl.find files fn in
  try
    let {Parser_hack.is_hh_file; comments; ast} =
      Parser_hack.program content
    in
    List.iter begin fun def ->
      match def with
      | Ast.Fun f ->
          let nenv = Naming.empty in
          let tenv = Typing_env.empty fn in
          let f = Naming.fun_ nenv f in
          Find_refs.process_find_refs None
              (snd f.Nast.f_name) (fst f.Nast.f_name);
          Typing.fun_def tenv (snd f.Nast.f_name) f
      | Ast.Class c ->
          let nenv = Naming.empty in
          let tenv = Typing_env.empty fn in
          let c = Naming.class_ nenv c in
          if !Find_refs.find_method_at_cursor_target <> None
          then begin
            Find_refs.process_class_ref (fst c.Nast.c_name)
              (snd c.Nast.c_name) None
          end;
          let all_methods = c.Nast.c_methods @ c.Nast.c_static_methods in
          List.iter begin fun method_ ->
            Find_refs.process_find_refs (Some (snd c.Nast.c_name))
              (snd method_.Nast.m_name) (fst method_.Nast.m_name)
          end all_methods;
          (match c.Nast.c_constructor with
          | None -> ()
          | Some method_ ->
              Find_refs.process_find_refs (Some (snd c.Nast.c_name))
                  "__construct" (fst method_.Nast.m_name));
          let res = Typing.class_def tenv (snd c.Nast.c_name) c in
          res
      | _ -> ()
    end ast;
    let result =
      match !Find_refs.find_method_at_cursor_result with
      | Some res ->
          let result_type =
            match res.Find_refs.type_ with
            | Find_refs.Class -> "class"
            | Find_refs.Method -> "method"
            | Find_refs.Function -> "function"
            | Find_refs.LocalVar -> "local" in
          JAssoc [ "name",           JString res.Find_refs.name;
                   "result_type",    JString result_type;
                   "pos",            Pos.json res.Find_refs.pos;
                   "internal_error", JBool false;
                 ]
      | _ -> JAssoc [ "internal_error", JBool false;
                    ] in
    Find_refs.find_method_at_cursor_target := None;
    output_json result
  with _ ->
    Find_refs.find_method_at_cursor_target := None;
    output_json (JAssoc [ "internal_error", JBool true;
                        ])

let hh_get_deps =
  let already_sent = ref Typing_deps.DSet.empty in
  fun () ->
    let result = ref [] in
    let deps = !(Typing_deps.deps) in
    Typing_deps.deps := Typing_deps.DSet.empty;
    Typing_deps.DSet.iter begin fun dep ->
      if Typing_deps.DSet.mem dep !already_sent
      then ()
      else begin
        already_sent := Typing_deps.DSet.add dep !already_sent;
        result :=
          (match dep with
          | Typing_deps.Dep.Class s
            when Typing_env.Classes.get s = None ->
              (JAssoc [ "name", JString s;
                        "type", JString "class";
                      ]) :: !result
          | Typing_deps.Dep.Fun s
            when Typing_env.Funs.get s = None ->
              (JAssoc [ "name", JString s;
                        "type", JString "fun";
                      ]) :: !result
          | _ -> !result
          )
      end
    end deps;
    output_json (JAssoc [ "deps",           JList !result;
                          "internal_error", JBool false;
                        ])

let infer_at_pos file line char =
  let clean() =
    Typing_defs.infer_type := None;
    Typing_defs.infer_target := None;
    Typing_defs.infer_pos := None;
  in
  try
    clean();
    Typing_defs.infer_target := Some (line, char);
    ignore (hh_check ~check_mode:false file);
    let ty = !Typing_defs.infer_type in
    let pos = !Typing_defs.infer_pos in
    clean();
    pos, ty
  with _ ->
    clean();
    None, None

let hh_find_lvar_refs file line char =
  let clean() =
    Find_refs.find_refs_target := None;
    Find_refs.find_refs_result := [];
  in
  try
    clean();
    Find_refs.find_refs_target := Some (line, char);
    ignore (hh_check ~check_mode:false file);
    let res_list = List.map Pos.json !Find_refs.find_refs_result in
    clean();
    output_json (JAssoc [ "positions",      JList res_list;
                          "internal_error", JBool false;
                        ])
  with _ ->
    clean();
    output_json (JAssoc [ "internal_error", JBool true;
                        ])

let hh_infer_type file line char =
  let _, ty = infer_at_pos file line char in
  let output = match ty with
  | Some ty -> JAssoc [ "type",           JString ty;
                        "internal_error", JBool false;
                      ]
  | None -> JAssoc [ "internal_error", JBool false;
                   ]
  in
  output_json output

let hh_infer_pos file line char =
  let pos, _ = infer_at_pos file line char in
  let output = match pos with
  | Some pos -> JAssoc [ "pos",            Pos.json pos;
                         "internal_error", JBool false;
                       ]
  | None -> JAssoc [ "internal_error", JBool false;
                   ]
  in
  output_json output

let hh_file_summary fn =
  try
    let content = Hashtbl.find files fn in
    let outline = FileOutline.outline content in
    let res_list = List.map begin fun (pos, name, type_) ->
      JAssoc [ "name", JString name;
               "type", JString type_;
               "pos",  Pos.json pos;
             ]
      end outline in
    output_json (JAssoc [ "summary",          JList res_list;
                          "internal_error",   JBool false;
                        ])
  with _ ->
    output_json (JAssoc [ "internal_error", JBool true;
                        ])

let hh_hack_coloring fn =
  Typing_defs.accumulate_types := true;
  ignore (hh_check ~check_mode:false fn);
  let result = !(Typing_defs.type_acc) in
  Typing_defs.accumulate_types := false;
  Typing_defs.type_acc := [];
  let result = ColorFile.go (Hashtbl.find files fn) result in
  let result = List.map (fun input ->
                        match input with
                        | (ColorFile.Unchecked_code, str) -> ("err", str)
                        | (ColorFile.Checked_code, str) -> ("checked", str)
                        | (ColorFile.Keyword, str) -> ("kwd", str)
                        | (ColorFile.Fun, str) -> ("fun", str)
                        | (ColorFile.Default, str) -> ("default", str)
                        ) result in
  let result = List.map (fun (checked, text) ->
                        JAssoc [ "checked", JString checked;
                                 "text",    JString text;
                               ]) result in
  output_json (JAssoc [ "coloring",       JList result;
                        "internal_error", JBool false;
                      ])

let hh_get_method_calls fn =
  Typing_defs.accumulate_method_calls := true;
  Typing_defs.accumulate_method_calls_result := [];
  ignore (hh_check ~check_mode:false fn);
  let results = !Typing_defs.accumulate_method_calls_result in
  let results = List.map begin fun (p, name) ->
    JAssoc [ "method_name", JString name;
             "pos",         Pos.json p;
           ]
    end results in
  Typing_defs.accumulate_method_calls := false;
  Typing_defs.accumulate_method_calls_result := [];
  output_json (JAssoc [ "method_calls",   JList results;
                        "internal_error", JBool false;
                      ])

let hh_arg_info fn line char =
  ArgumentInfoService.attach_hooks (line, char);
  ignore (hh_check ~check_mode:false fn);
  let result = ArgumentInfoService.get_result() in
  let result = match result with
    | Some result -> result
    | None -> (-1), []
  in
  ArgumentInfoService.detach_hooks();
  let json_res =
    ("internal_error", JBool false) :: ArgumentInfoService.to_json result
  in
  output_json (JAssoc json_res)

let hh_format contents start end_ =
  let result = Format_hack.region start end_ contents in
  let error, result, internal_error = match result with
    | Format_hack.Php_or_decl -> "Php_or_decl", "", false
    | Format_hack.Parsing_error _ -> "Parsing_error", "", false
    | Format_hack.Internal_error -> "", "", true
    | Format_hack.Success s -> "", s, false
  in
  output_json (JAssoc [ "error_message", JString error;
                        "result", JString result;
                        "internal_error",   JBool internal_error;
                      ])


let (hh_check: (string, string -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_check
let (hh_add_file: (string, string -> string -> unit) Js.meth_callback) = Js.wrap_callback hh_add_file
let (hh_auto_complete: (string, string -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_auto_complete
let (hh_get_deps: (string, unit -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_get_deps
let (hh_infer_type: (string, string -> int -> int -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_infer_type
let (hh_infer_pos: (string, string -> int -> int -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_infer_pos
let (hh_file_summary: (string, string -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_file_summary
let (hh_hack_coloring: (string, string -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_hack_coloring
let (hh_find_lvar_refs: (string, string -> int -> int -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_find_lvar_refs
let (hh_get_method_calls: (string, string -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_get_method_calls
let (hh_get_method_name: (string, string -> int -> int -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_get_method_at_position
let (hh_arg_info: (string, string -> int -> int -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_arg_info
let (hh_format: (string, string -> int -> int -> Js.js_string Js.t) Js.meth_callback) = Js.wrap_callback hh_format


let export_fun0 f fname =
  Js.Unsafe.set (Js.Unsafe.eval_string "hh_ide") fname [|Js.Unsafe.inject f|];
  let js_def = "self."^fname^" = function(x) { return hh_ide."^fname^"[1](); };" in
  Js.Unsafe.eval_string js_def

let export_fun1 f fname farg =
  Js.Unsafe.set (Js.Unsafe.eval_string "hh_ide") fname [|Js.Unsafe.inject f|];
  let js_def =
    "self."^fname^" = function(x) { return hh_ide."^fname^"[1]("^farg^"); };" in
  Js.Unsafe.eval_string js_def

let export_fun2 f fname farg1 farg2 =
  Js.Unsafe.set (Js.Unsafe.eval_string "hh_ide") fname [|Js.Unsafe.inject f|];
  let js_def =
    "self."^fname^" = function(x, y) { return hh_ide."^fname
    ^"[1]("^farg1^", "^farg2^"); };"
  in
  Js.Unsafe.eval_string js_def

let export_fun3 f fname farg1 farg2 farg3 =
  Js.Unsafe.set (Js.Unsafe.eval_string "hh_ide") fname [|Js.Unsafe.inject f|];
  let js_def =
    "self."^fname^" = function(x, y, z) { return hh_ide."^fname
    ^"[1]("^farg1^", "^farg2^", "^farg3^"); };"
  in
  Js.Unsafe.eval_string js_def

let () = Js.Unsafe.eval_string "self.hh_ide = { };"
let () = Js.Unsafe.set (Js.Unsafe.eval_string "hh_ide") "str" [|Js.Unsafe.inject (Js.wrap_callback Js.to_string)|]
let () = export_fun1 hh_check "hh_check_file" "hh_ide.str[1](x)"
let () = export_fun2 hh_add_file "hh_add_file" "hh_ide.str[1](x)" "hh_ide.str[1](y)"
let () = export_fun1 hh_auto_complete "hh_auto_complete" "hh_ide.str[1](x)"
let () = export_fun0 hh_get_deps "hh_get_deps"
let () = export_fun3 hh_infer_type "hh_infer_type" "hh_ide.str[1](x)" "y" "z"
let () = export_fun3 hh_infer_pos "hh_infer_pos" "hh_ide.str[1](x)" "y" "z"
let () = export_fun1 hh_file_summary "hh_file_summary" "hh_ide.str[1](x)"
let () = export_fun1 hh_hack_coloring "hh_hack_coloring" "hh_ide.str[1](x)"
let () = export_fun3 hh_find_lvar_refs "hh_find_lvar_refs" "hh_ide.str[1](x)" "y" "z"
let () = export_fun1 hh_get_method_calls "hh_get_method_calls" "hh_ide.str[1](x)"
let () = export_fun3 hh_get_method_name "hh_get_method_name" "hh_ide.str[1](x)" "y" "z"
let () = export_fun3 hh_arg_info "hh_arg_info" "hh_ide.str[1](x)" "y" "z"
let () = export_fun3 hh_format "hh_format" "hh_ide.str[1](x)" "y" "z"
