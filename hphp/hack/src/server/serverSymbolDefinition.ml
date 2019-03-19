(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open IdentifySymbolService
open Option.Monad_infix
open Typing_defs

module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax(Full_fidelity_positioned_syntax)
module Cls = Typing_classes_heap

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
  Naming_table.Types.get_pos x >>= fun (pos, _) ->
  let fn = FileInfo.get_pos_filename pos in
  Ide_parser_cache.with_ide_cache @@ fun () ->
    Parser_heap.find_class_in_file fn x

let get_function_by_name x =
  Naming_table.Funs.get_pos x >>= fun pos ->
  let fn = FileInfo.get_pos_filename pos in
  Ide_parser_cache.with_ide_cache @@ fun () ->
    Parser_heap.find_fun_in_file fn x

let get_gconst_by_name x =
  Naming_table.Consts.get_pos x >>= fun pos ->
  let fn = FileInfo.get_pos_filename pos in
  Ide_parser_cache.with_ide_cache @@ fun () ->
    Parser_heap.find_const_in_file fn x

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
      | Ast.ClassVars { Ast.cv_kinds = kinds; Ast.cv_names = vars; _ } ->
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

(* summarize a class or typedef carried with SymbolOccurrence.Class *)
let summarize_class_typedef x =
  Naming_table.Types.get_pos x >>= fun (pos, ct) ->
    let fn = FileInfo.get_pos_filename pos in
    match ct with
      | Naming_table.TClass -> (Parser_heap.find_class_in_file fn x >>=
                fun c -> Some (FileOutline.summarize_class c ~no_children:true))
      | Naming_table.TTypedef -> (Parser_heap.find_typedef_in_file fn x >>=
                fun tdef -> Some (FileOutline.summarize_typedef tdef))

let go ast result =
  match result.SymbolOccurrence.type_ with
    | SymbolOccurrence.Method (c_name, method_name) ->
      (* Classes on typing heap have all the methods from inheritance hierarchy
       * folded together, so we will correctly identify them even if method_name
       * is not defined directly in class c_name *)
      Typing_lazy_heap.get_class c_name >>= fun class_ ->
      if method_name = Naming_special_names.Members.__construct then begin
        match fst (Cls.construct class_) with
          | Some m ->
            get_member_def (Constructor, m.ce_origin, method_name)
          | None ->
            get_class_by_name c_name >>= fun c ->
            Some (FileOutline.summarize_class c ~no_children:true)
      end else begin
        match Cls.get_method class_ method_name with
        | Some m -> get_member_def (Method, m.ce_origin, method_name)
        | None ->
          Cls.get_smethod class_ method_name >>= fun m ->
          get_member_def (Static_method, m.ce_origin, method_name)
      end
    | SymbolOccurrence.Property (c_name, property_name) ->
      Typing_lazy_heap.get_class c_name >>= fun class_ ->
      let property_name = clean_member_name property_name in
      begin match Cls.get_prop class_ property_name with
      | Some m -> get_member_def (Property, m.ce_origin, property_name)
      | None ->
        Cls.get_sprop class_ ("$" ^ property_name) >>= fun m ->
        get_member_def
          (Static_property, m.ce_origin, property_name)
      end
    | SymbolOccurrence.ClassConst (c_name, const_name) ->
      Typing_lazy_heap.get_class c_name >>= fun class_ ->
      Cls.get_const class_ const_name >>= fun m ->
      get_member_def (Class_const, m.cc_origin, const_name)
    | SymbolOccurrence.Function ->
      get_function_by_name result.SymbolOccurrence.name >>= fun f ->
      Some (FileOutline.summarize_fun f)
    | SymbolOccurrence.GConst ->
      get_gconst_by_name result.SymbolOccurrence.name >>= fun cst ->
      Some (FileOutline.summarize_gconst cst)
    | SymbolOccurrence.Class ->
      summarize_class_typedef result.SymbolOccurrence.name
    | SymbolOccurrence.Typeconst (c_name, typeconst_name) ->
      Typing_lazy_heap.get_class c_name >>= fun class_ ->
      Cls.get_typeconst class_ typeconst_name >>= fun m ->
      get_member_def (Typeconst, m.ttc_origin, typeconst_name)
    | SymbolOccurrence.LocalVar ->
      get_local_var_def
        ast result.SymbolOccurrence.name result.SymbolOccurrence.pos

let get_definition_cst_node_from_pos kind source_text pos =
  let tree = if Ide_parser_cache.is_enabled () then
    Ide_parser_cache.(with_ide_cache @@ fun () -> get_cst source_text)
  else begin
    let env = Full_fidelity_parser_env.default in
    SyntaxTree.make ~env source_text
  end in
  let (line, start, _) = Pos.info_pos pos in
  let offset = SourceText.position_to_offset source_text (line, start) in
  let parents = Syntax.parentage (SyntaxTree.root tree) offset in
  List.find parents ~f:begin fun syntax ->
    match kind, Syntax.kind syntax with
    | SymbolDefinition.Function, SyntaxKind.FunctionDeclaration
    | SymbolDefinition.Class, SyntaxKind.ClassishDeclaration
    | SymbolDefinition.Method, SyntaxKind.MethodishDeclaration
    | SymbolDefinition.Property, SyntaxKind.PropertyDeclaration
    | SymbolDefinition.Const, SyntaxKind.ConstDeclaration
    | SymbolDefinition.Enum, SyntaxKind.EnumDeclaration
    | SymbolDefinition.Interface, SyntaxKind.ClassishDeclaration
    | SymbolDefinition.Trait, SyntaxKind.ClassishDeclaration
    | SymbolDefinition.LocalVar, SyntaxKind.VariableExpression
    | SymbolDefinition.Typeconst, SyntaxKind.TypeConstDeclaration
    | SymbolDefinition.Param, SyntaxKind.ParameterDeclaration
    | SymbolDefinition.Typedef, SyntaxKind.SimpleTypeSpecifier -> true
    | _ -> false
  end

let get_definition_cst_node fallback_fn definition =
  let open SymbolDefinition in
  let source_text = if Pos.filename definition.pos = ServerIdeUtils.path
    then
      (* When the definition is in an IDE buffer with local changes, the filename
         in the definition will be empty. *)
      ServerCommandTypesUtils.source_tree_of_file_input fallback_fn
    else SourceText.from_file (Pos.filename definition.pos)
  in
  get_definition_cst_node_from_pos definition.kind source_text definition.pos
