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
open IdentifySymbolService
open Option.Monad_infix
open Typing_defs

(* Element type, class name, element name. Class name refers to "origin" class,
 * we expect to find said element in AST/NAST of this class *)
type class_element = class_element_ * string * string
and class_element_ =
| Constructor
| Method
| Static_method
| Property
| Static_property
| Class_const
| Typeconst

let get_class_by_name x =
  Naming_heap.TypeIdHeap.get x >>= fun (p, _) ->
  Parser_heap.find_class_in_file (Pos.filename p) x

let get_function_by_name x =
  Naming_heap.FunPosHeap.get x >>= fun p ->
  Parser_heap.find_fun_in_file (Pos.filename p) x

let get_gconst_by_name x =
  Naming_heap.ConstPosHeap.get x >>= fun p ->
  Parser_heap.find_const_in_file (Pos.filename p) x

(* Span information is stored only in parsing AST *)
let get_member_def (x : class_element) =
  let type_, member_origin, member_name = x in
  get_class_by_name member_origin >>= fun c ->
  let member_origin = Utils.strip_ns member_origin in
  match type_ with
  | Constructor
  | Method
  | Static_method ->
    let methods = List.filter_map c.Ast.c_body begin function
      | Ast.Method m -> Some m
      | _ -> None
    end in
    List.find methods (fun m -> (snd m.Ast.m_name) = member_name) >>= fun m ->
    Some (FileOutline.summarize_method member_origin m)
  | Property
  | Static_property ->
    let props = List.concat_map c.Ast.c_body begin function
      | Ast.ClassVars (kinds, _, vars) ->
        List.map vars (fun var -> (kinds, var))
      | Ast.XhpAttr (_, var, _, _) -> [([], var)]
      | _ -> []
    end in
    let get_prop_name (_, (_, x), _) = x in
    List.find props (fun p -> get_prop_name (snd p) = member_name) >>=
      fun (kinds, p) ->
    Some (FileOutline.summarize_property member_origin kinds p)
  | Class_const ->
    let consts = List.concat_map c.Ast.c_body begin function
      | Ast.Const (_, consts) ->
        List.map consts begin fun (((_, name), _) as const) ->
          (const, name)
        end
      | _ -> []
    end in
    let res = List.find consts (fun c -> snd c = member_name) >>= fun c ->
      Some (FileOutline.summarize_const member_origin (fst c))
    in
    if Option.is_some res then res else
    let abs_consts = List.concat_map c.Ast.c_body begin function
      | Ast.AbsConst (_, id) -> [id]
      | _ -> []
    end in
    List.find abs_consts (fun c -> snd c = member_name) >>= fun c ->
      Some (FileOutline.summarize_abs_const member_origin c)
  | Typeconst ->
    let tconsts = List.filter_map c.Ast.c_body begin function
      | Ast.TypeConst t -> Some t
      | _ -> None
    end in
    List.find tconsts (fun m -> (snd m.Ast.tconst_name) = member_name)
      >>= fun t ->
    Some (FileOutline.summarize_typeconst member_origin t)

let get_local_var_def ast name p =
  let line, char, _ = Pos.info_pos p in
  let def = List.hd (ServerFindLocals.go_from_ast ast line char) in
  Option.map def ~f:(FileOutline.summarize_local name)

let go tcopt ast result =
  match result.SymbolOccurrence.type_ with
    | SymbolOccurrence.Method (c_name, method_name) ->
      (* Classes on typing heap have all the methods from inheritance hierarchy
       * folded together, so we will correctly identify them even if method_name
       * is not defined directly in class c_name *)
      Typing_lazy_heap.get_class tcopt c_name >>= fun class_ ->
      if method_name = Naming_special_names.Members.__construct then begin
        match fst class_.tc_construct with
          | Some m ->
            get_member_def (Constructor, m.ce_origin, method_name)
          | None ->
            get_class_by_name c_name >>= fun c ->
            Some (FileOutline.summarize_class c ~no_children:true)
      end else begin
        match SMap.get method_name class_.tc_methods with
        | Some m -> get_member_def (Method, m.ce_origin, method_name)
        | None ->
          SMap.get method_name class_.tc_smethods >>= fun m ->
          get_member_def (Static_method, m.ce_origin, method_name)
      end
    | SymbolOccurrence.Property (c_name, property_name) ->
      Typing_lazy_heap.get_class tcopt c_name >>= fun class_ ->
      begin match SMap.get property_name class_.tc_props with
      | Some m -> get_member_def (Property, m.ce_origin, property_name)
      | None ->
        SMap.get property_name class_.tc_sprops >>= fun m ->
        get_member_def
          (Static_property, m.ce_origin, clean_member_name property_name)
      end
    | SymbolOccurrence.ClassConst (c_name, const_name) ->
      Typing_lazy_heap.get_class tcopt c_name >>= fun class_ ->
      SMap.get const_name class_.tc_consts >>= fun m ->
      get_member_def (Class_const, m.cc_origin, const_name)
    | SymbolOccurrence.Function ->
      get_function_by_name result.SymbolOccurrence.name >>= fun f ->
      Some (FileOutline.summarize_fun f)
    | SymbolOccurrence.GConst ->
      get_gconst_by_name result.SymbolOccurrence.name >>= fun cst ->
      Some (FileOutline.summarize_gconst cst)
    | SymbolOccurrence.Class ->
      get_class_by_name result.SymbolOccurrence.name >>= fun c ->
      Some (FileOutline.summarize_class c ~no_children:true)
    | SymbolOccurrence.Typeconst (c_name, typeconst_name) ->
      Typing_lazy_heap.get_class tcopt c_name >>= fun class_ ->
      SMap.get typeconst_name class_.tc_typeconsts >>= fun m ->
      get_member_def (Typeconst, m.ttc_origin, typeconst_name)
    | SymbolOccurrence.LocalVar ->
      get_local_var_def
        ast result.SymbolOccurrence.name result.SymbolOccurrence.pos

let build_symbol_occurence kind name =
  {
    SymbolOccurrence.name = "\\" ^ name;
    type_ = kind;
    pos = Pos.none;
  }

let from_symbol_id tcopt id =
  match Str.split (Str.regexp_string "::") id with
  | [kind; name] when kind = SymbolDefinition.function_kind_name ->
      go tcopt [] (build_symbol_occurence SymbolOccurrence.Function name)
  | [kind; name] when kind = SymbolDefinition.type_id_kind_name ->
      go tcopt [] (build_symbol_occurence SymbolOccurrence.Class name)
  | [kind; class_name; method_name ]
        when kind = SymbolDefinition.method_kind_name ->
      go tcopt [] (build_symbol_occurence
        (SymbolOccurrence.Method ("\\" ^ class_name, method_name)) "")
  | [kind; class_name; property_name ]
        when kind = SymbolDefinition.property_kind_name ->
      go tcopt [] (build_symbol_occurence
        (SymbolOccurrence.Property ("\\" ^ class_name, property_name)) "")
  | [kind; class_name; const_name ]
        when kind = SymbolDefinition.class_const_kind_name ->
      let try_const () = go tcopt [] (build_symbol_occurence
        (SymbolOccurrence.ClassConst ("\\" ^ class_name, const_name)) "")
      in
      let try_typeconst () = go tcopt [] (build_symbol_occurence
        (SymbolOccurrence.Typeconst ("\\" ^ class_name, const_name)) "")
      in
      begin match try_const () with
      | Some result -> Some result
      | None -> try_typeconst ()
      end
  | _ -> None
