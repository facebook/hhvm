(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open IdentifySymbolService
open Option.Monad_infix
open Typing_defs
module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Provider_context.PositionedSyntaxTree
module Cls = Decl_provider.Class
open Aast

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

let get_class_by_name ctx x =
  Naming_provider.get_type_path ctx x >>= fun fn ->
  Ide_parser_cache.with_ide_cache @@ fun () ->
  Ast_provider.find_class_in_file ctx fn x

let get_function_by_name ctx x =
  Naming_provider.get_fun_path ctx x >>= fun fn ->
  Ide_parser_cache.with_ide_cache @@ fun () ->
  Ast_provider.find_fun_in_file ctx fn x

let get_gconst_by_name ctx x =
  Naming_provider.get_const_path ctx x >>= fun fn ->
  Ide_parser_cache.with_ide_cache @@ fun () ->
  Ast_provider.find_gconst_in_file ctx fn x

(* Span information is stored only in parsing AST *)
let get_member_def (ctx : Provider_context.t) (x : class_element) =
  let (type_, member_origin, member_name) = x in
  get_class_by_name ctx member_origin >>= fun c ->
  let member_origin = Utils.strip_ns member_origin in
  match type_ with
  | Constructor
  | Method
  | Static_method ->
    List.find c.c_methods ~f:(fun m -> String.equal (snd m.m_name) member_name)
    >>= fun m -> Some (FileOutline.summarize_method member_origin m)
  | Property
  | Static_property ->
    let props =
      c.c_vars @ List.map c.c_xhp_attrs ~f:(fun (_, var, _, _) -> var)
    in
    let get_prop_name { cv_id; _ } = snd cv_id in
    List.find props ~f:(fun p -> String.equal (get_prop_name p) member_name)
    >>= fun p -> Some (FileOutline.summarize_property member_origin p)
  | Class_const ->
    let (consts, abs_consts) =
      List.partition_map c.c_consts ~f:(fun cc ->
          match cc.cc_kind with
          | CCConcrete _
          | CCAbstract (Some _) ->
            First cc
          | CCAbstract None -> Second cc)
    in
    let name_matches cc = String.equal (snd cc.cc_id) member_name in
    let res =
      Option.first_some
        (List.find consts ~f:name_matches)
        (List.find abs_consts ~f:name_matches)
    in
    Option.map ~f:(FileOutline.summarize_const member_origin) res
  | Typeconst ->
    let tconsts = c.c_typeconsts in
    List.find tconsts ~f:(fun t ->
        String.equal (snd t.c_tconst_name) member_name)
    >>= fun t -> Some (FileOutline.summarize_typeconst member_origin t)

let get_local_var_def ast name p =
  let (line, char, _) = Pos.info_pos p in
  let def = List.hd (ServerFindLocals.go_from_ast ~ast ~line ~char) in
  Option.map def ~f:(FileOutline.summarize_local name)

(* summarize a class, typedef or record *)
let summarize_class_typedef ctx x =
  Naming_provider.get_type_path_and_kind ctx x >>= fun (fn, ct) ->
  match ct with
  | Naming_types.TClass ->
    Ast_provider.find_class_in_file ctx fn x >>= fun c ->
    Some (FileOutline.summarize_class c ~no_children:true)
  | Naming_types.TTypedef ->
    Ast_provider.find_typedef_in_file ctx fn x >>= fun tdef ->
    Some (FileOutline.summarize_typedef tdef)
  | Naming_types.TRecordDef ->
    Ast_provider.find_record_def_in_file ctx fn x >>= fun rd ->
    Some (FileOutline.summarize_record_decl rd)

let go ctx ast result =
  match result.SymbolOccurrence.type_ with
  | SymbolOccurrence.Attribute
      (Some { SymbolOccurrence.class_name; method_name; is_static }) ->
    Decl_provider.get_class ctx class_name >>= fun cls ->
    let matching_method =
      Cls.all_ancestor_names cls
      |> List.filter_map ~f:(Decl_provider.get_class ctx)
      (* Find all inherited methods with the same name. *)
      |> List.filter_map ~f:(fun cls ->
             ( if is_static then
               Cls.get_smethod
             else
               Cls.get_method )
               cls
               method_name)
      (* It'd be nice to take the "earliest" method in the linearization,
      whatever that is. But alas order of all_ancestor_names isn't
      specified (in practice is alphabetical). So we'll just pick an
      arbitrary one. *)
      |> List.hd
    in
    (match matching_method with
    | Some meth -> get_member_def ctx (Method, meth.ce_origin, method_name)
    | None -> None)
  | SymbolOccurrence.Method (c_name, method_name) ->
    (* Classes on typing heap have all the methods from inheritance hierarchy
     * folded together, so we will correctly identify them even if method_name
     * is not defined directly in class c_name *)
    Decl_provider.get_class ctx c_name >>= fun class_ ->
    if String.equal method_name Naming_special_names.Members.__construct then
      match fst (Cls.construct class_) with
      | Some m -> get_member_def ctx (Constructor, m.ce_origin, method_name)
      | None ->
        get_class_by_name ctx c_name >>= fun c ->
        Some (FileOutline.summarize_class c ~no_children:true)
    else (
      match Cls.get_method class_ method_name with
      | Some m -> get_member_def ctx (Method, m.ce_origin, method_name)
      | None ->
        Cls.get_smethod class_ method_name >>= fun m ->
        get_member_def ctx (Static_method, m.ce_origin, method_name)
    )
  | SymbolOccurrence.Property (c_name, property_name)
  | SymbolOccurrence.XhpLiteralAttr (c_name, property_name) ->
    Decl_provider.get_class ctx c_name >>= fun class_ ->
    let property_name = clean_member_name property_name in
    begin
      match Cls.get_prop class_ property_name with
      | Some m -> get_member_def ctx (Property, m.ce_origin, property_name)
      | None ->
        Cls.get_sprop class_ ("$" ^ property_name) >>= fun m ->
        get_member_def ctx (Static_property, m.ce_origin, property_name)
    end
  | SymbolOccurrence.ClassConst (c_name, const_name) ->
    Decl_provider.get_class ctx c_name >>= fun class_ ->
    Cls.get_const class_ const_name >>= fun m ->
    get_member_def ctx (Class_const, m.cc_origin, const_name)
  | SymbolOccurrence.Function ->
    get_function_by_name ctx result.SymbolOccurrence.name >>= fun f ->
    Some (FileOutline.summarize_fun f)
  | SymbolOccurrence.GConst ->
    get_gconst_by_name ctx result.SymbolOccurrence.name >>= fun cst ->
    Some (FileOutline.summarize_gconst cst)
  | SymbolOccurrence.Class _ ->
    summarize_class_typedef ctx result.SymbolOccurrence.name
  | SymbolOccurrence.Record ->
    summarize_class_typedef ctx result.SymbolOccurrence.name
  | SymbolOccurrence.Typeconst (c_name, typeconst_name) ->
    Decl_provider.get_class ctx c_name >>= fun class_ ->
    Cls.get_typeconst class_ typeconst_name >>= fun m ->
    get_member_def ctx (Typeconst, m.ttc_origin, typeconst_name)
  | SymbolOccurrence.LocalVar ->
    begin
      match ast with
      | None -> None
      | Some ast ->
        get_local_var_def
          ast
          result.SymbolOccurrence.name
          result.SymbolOccurrence.pos
    end
  | SymbolOccurrence.Attribute _ -> None
  | SymbolOccurrence.EnumClassLabel (class_name, _member_name) ->
    summarize_class_typedef ctx class_name

let get_definition_cst_node_from_pos ctx entry kind pos =
  try
    let source_text = Ast_provider.compute_source_text ~entry in
    let tree =
      if Ide_parser_cache.is_enabled () then
        Ide_parser_cache.(with_ide_cache @@ fun () -> get_cst source_text)
      else
        Ast_provider.compute_cst ~ctx ~entry
    in
    let (line, start, _) = Pos.info_pos pos in
    let offset = SourceText.position_to_offset source_text (line, start) in
    let parents = Syntax.parentage (SyntaxTree.root tree) offset in
    List.find parents ~f:(fun syntax ->
        match (kind, Syntax.kind syntax) with
        | (SymbolDefinition.Function, SyntaxKind.FunctionDeclaration)
        | (SymbolDefinition.Class, SyntaxKind.ClassishDeclaration)
        | (SymbolDefinition.Method, SyntaxKind.MethodishDeclaration)
        | (SymbolDefinition.Property, SyntaxKind.PropertyDeclaration)
        | (SymbolDefinition.Property, SyntaxKind.XHPClassAttribute)
        | (SymbolDefinition.RecordDef, SyntaxKind.RecordDeclaration)
        | (SymbolDefinition.Const, SyntaxKind.ConstDeclaration)
        | (SymbolDefinition.Enum, SyntaxKind.EnumDeclaration)
        | (SymbolDefinition.Enum, SyntaxKind.EnumClassDeclaration)
        | (SymbolDefinition.Interface, SyntaxKind.ClassishDeclaration)
        | (SymbolDefinition.Trait, SyntaxKind.ClassishDeclaration)
        | (SymbolDefinition.LocalVar, SyntaxKind.VariableExpression)
        | (SymbolDefinition.Typeconst, SyntaxKind.TypeConstDeclaration)
        | (SymbolDefinition.Param, SyntaxKind.ParameterDeclaration)
        | (SymbolDefinition.Typedef, SyntaxKind.SimpleTypeSpecifier) ->
          true
        | _ -> false)
  with _ -> None

let get_definition_cst_node_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(kind : SymbolDefinition.kind)
    ~(pos : 'a Pos.pos) : Full_fidelity_positioned_syntax.t option =
  get_definition_cst_node_from_pos ctx entry kind pos
