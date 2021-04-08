(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open File_content
open Hh_json
open Provider_context
open Tast
open Typing_defs
module Nast = Aast
module Tast = Aast
module Cls = Decl_provider.Class
module Syntax = Full_fidelity_positioned_syntax
module TokenKind = Full_fidelity_token_kind
open Syntax

(*
 * Helpers for extracting previous tokens before the cursor
 *)
let long_token = "__VERY_LONG_TOKEN__"

let matches_auto_complete_suffix x =
  let regex_str =
    Printf.sprintf ".*%s.*" AutocompleteTypes.autocomplete_token
  in
  String.length x >= AutocompleteTypes.autocomplete_token_length
  && Str.string_match (Str.regexp regex_str) x 0

module Extract : sig
  val extract_token_literal : Token.t -> string option
end = struct
  let should_extract token =
    let open TokenKind in
    match Token.kind token with
    | Name -> true
    | SingleQuotedStringLiteral -> true
    | DoubleQuotedStringLiteral -> true
    | StringLiteralBody -> true
    | HeredocStringLiteral -> true
    | NowdocStringLiteral -> true
    | XHPCategoryName -> true
    | XHPElementName -> true
    | XHPClassName -> true
    | XHPStringLiteral -> true
    | XHPBody -> true
    | _ -> false

  let extract_token_literal token =
    match Token.kind token with
    | TokenKind.XHPComment -> None
    | _ ->
      if should_extract token then
        let text = Token.text token in
        let text =
          if String.length text > 100 then
            long_token
          else
            text
        in
        let whitespace = Str.regexp "[\r\n\t ]" in
        Some (Str.global_replace whitespace "_" text)
      else
        Some (Token.text token)
end

module TraverseSyntaxTree : sig
  val traverse : string option -> Syntax.t -> string option
end = struct
  let rec traverse_tokens (acc : bool * string list) (node : Syntax.t) :
      bool * string list =
    let (contains_cursor, tokens) = acc in
    let children_nodes = children node in
    let (current, traverse_after) =
      match syntax node with
      | Token t -> (Extract.extract_token_literal t, children_nodes)
      | _ -> (None, children_nodes)
    in
    let (contains_cursor, tokens) =
      if not contains_cursor then
        match current with
        | Some token ->
          if matches_auto_complete_suffix token then
            (true, tokens)
          else
            (contains_cursor, token :: tokens)
        | None -> acc
      else
        acc
    in
    List.fold_left
      ~f:traverse_tokens
      ~init:(contains_cursor, tokens)
      traverse_after

  let rec traverse (acc : string option) (node : Syntax.t) : string option =
    let children_nodes = children node in
    let (contains_cursor, tokens) =
      match syntax node with
      | FunctionDeclaration _ -> traverse_tokens (false, []) node
      | MethodishDeclaration _ -> traverse_tokens (false, []) node
      | ClassishDeclaration _ -> traverse_tokens (false, []) node
      | _ -> (false, [])
    in
    let acc =
      match (contains_cursor, tokens) with
      | (true, _ :: _) -> Some (tokens |> List.rev |> String.concat ~sep:" ")
      | _ -> acc
    in
    List.fold_left ~f:traverse ~init:acc children_nodes
end

let extract ~(cst : Provider_context.PositionedSyntaxTree.t) : string option =
  TraverseSyntaxTree.traverse
    None
    (Provider_context.PositionedSyntaxTree.root cst)

(*
 * Helpers for extracting local variable candidates
 *)
let auto_complete_suffix_finder =
  object
    inherit [_] Tast.reduce

    method zero = false

    method plus = ( || )

    method! on_Lvar () (_, id) =
      matches_auto_complete_suffix (Local_id.get_name id)
  end

let method_contains_cursor = auto_complete_suffix_finder#on_method_ ()

let fun_contains_cursor = auto_complete_suffix_finder#on_fun_ ()

class local_types =
  object (self)
    inherit Tast_visitor.iter as super

    val mutable results = Local_id.Map.empty

    val mutable after_cursor = false

    method get_types ctx tast =
      self#go ctx tast;
      results

    method add id ty =
      (* If we already have a type for this identifier, don't overwrite it with
       results from after the cursor position. *)
      if not (Local_id.Map.mem id results && after_cursor) then
        results <- Local_id.Map.add id ty results

    method! on_fun_ env f = if fun_contains_cursor f then super#on_fun_ env f

    method! on_method_ env m =
      if method_contains_cursor m then (
        if not m.Tast.m_static then
          self#add Typing_defs.this (Tast_env.get_self_ty_exn env);
        super#on_method_ env m
      )

    method! on_expr env e =
      let ((_, ty), e_) = e in
      match e_ with
      | Tast.Lvar (_, id) ->
        if matches_auto_complete_suffix (Local_id.get_name id) then
          after_cursor <- true
        else
          self#add id ty
      | Tast.Binop (Ast_defs.Eq _, e1, e2) ->
        (* Process the rvalue before the lvalue, since the lvalue is annotated
         with its type after the assignment. *)
        self#on_expr env e2;
        self#on_expr env e1
      | _ -> super#on_expr env e

    method! on_fun_param _ fp =
      let id = Local_id.make_unscoped fp.Tast.param_name in
      let (_, ty) = fp.Tast.param_annotation in
      self#add id ty
  end

(*
 * Visit cst for extracting target tokens
 *)
type autocomplete_data_entry = {
  filename: string;
  completion_type: string;
  tokens: string;
  target: string;
  candidates: string;
}

class visitor ~ctx ~entry ~filename ~source_text =
  object (self)
    inherit Tast_visitor.iter as super

    val ctx : Provider_context.t = ctx

    val entry : Provider_context.entry = entry

    val filename : string = filename

    val source_text : string = source_text

    (* Acclass_get completion data *)
    val mutable entries : autocomplete_data_entry list = []

    val positions : SSet.t ref = ref SSet.empty

    (* Candidate list for each Acclass_get autocompletion *)
    val candidates : string list ref = ref []

    method get_entries ctx tast =
      self#go ctx tast;
      entries

    (* Helper for extracting previous tokens *)
    method extract_context (p : Pos.t) : string option =
      let (line, column) = Pos.line_column p in
      let column = column + 1 in
      let pos = { line; column } in
      let edits =
        [
          {
            range = Some { st = pos; ed = pos };
            text = AutocompleteTypes.autocomplete_token;
          };
        ]
      in
      let auto332_content = File_content.edit_file_unsafe source_text edits in
      let (ctx, entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:entry.Provider_context.path
          ~contents:auto332_content
      in
      let cst = Ast_provider.compute_cst ~ctx ~entry in
      extract ~cst

    (* Helper functions to extract candidate list
     * These are borrowed from autcompleteService.ml
     *)
    method get_class_elt_types env class_ cid elts =
      let is_visible (_, elt) =
        Tast_env.is_visible env (elt.ce_visibility, get_ce_lsb elt) cid class_
      in
      elts
      |> List.filter ~f:is_visible
      |> List.map ~f:(fun (id, { ce_type = (lazy ty); _ }) -> (id, ty))

    method autocomplete_member ~is_static env class_ cid =
      let match_both_static_and_instance =
        match cid with
        | Some Nast.CIparent -> true
        | _ -> false
      in
      let add _ (name, _) = candidates := name :: !candidates in
      if is_static || match_both_static_and_instance then (
        List.iter
          (self#get_class_elt_types env class_ cid (Cls.smethods class_))
          ~f:(add SearchUtils.SI_ClassMethod);
        List.iter
          (self#get_class_elt_types env class_ cid (Cls.sprops class_))
          ~f:(add SearchUtils.SI_Property);
        List.iter (Cls.consts class_) ~f:(fun (name, cc) ->
            add SearchUtils.SI_ClassConstant (name, cc.cc_type))
      );
      if (not is_static) || match_both_static_and_instance then (
        List.iter
          (self#get_class_elt_types env class_ cid (Cls.methods class_))
          ~f:(add SearchUtils.SI_ClassMethod);
        List.iter
          (self#get_class_elt_types env class_ cid (Cls.props class_))
          ~f:(add SearchUtils.SI_Property)
      )

    method autocomplete_typed_member ~is_static env class_ty cid =
      candidates := [];
      Tast_env.get_class_ids env class_ty
      |> List.iter ~f:(fun cname ->
             Decl_provider.get_class (Tast_env.get_ctx env) cname
             |> Option.iter ~f:(fun class_ ->
                    let cid = Option.map cid to_nast_class_id_ in
                    self#autocomplete_member ~is_static env class_ cid))

    method autocomplete_static_member env ((_, ty), cid) =
      self#autocomplete_typed_member ~is_static:true env ty (Some cid)

    method compute_complete_local (p : Pos.t) (target : string) =
      (* extract lvar candidates *)
      candidates := [];
      let (line, column) = Pos.line_column p in
      let column = column + 1 in
      let pos = { line; column } in
      let edits =
        [
          {
            range = Some { st = pos; ed = pos };
            text = "$" ^ AutocompleteTypes.autocomplete_token;
          };
        ]
      in
      let auto332_content = File_content.edit_file_unsafe source_text edits in
      let filepath = entry.path in
      let (ctx, entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:filepath
          ~contents:auto332_content
      in
      let new_tast =
        (Tast_provider.compute_tast_quarantined ~ctx ~entry)
          .Tast_provider.Compute_tast.tast
      in
      let add name = candidates := name :: !candidates in
      (new local_types)#get_types ctx new_tast
      |> Local_id.Map.iter (fun x _ -> add (Local_id.get_name x));

      (* extract context tokens *)
      let cst = Ast_provider.compute_cst ~ctx ~entry in
      let context = extract ~cst in

      (* add lvar entry *)
      let candidates = !candidates |> List.rev |> String.concat ~sep:" " in
      let actype_str = SearchUtils.show_autocomplete_type SearchUtils.Acprop in
      let (line, column) = Pos.line_column p in
      let column = column + 1 in
      let pos_str = Printf.sprintf "%d-%d" line column in
      ( if not (SSet.exists (fun key -> String.equal key pos_str) !positions)
      then
        match context with
        | Some tokens ->
          let entry =
            {
              filename;
              completion_type = actype_str;
              tokens;
              target;
              candidates;
            }
          in
          entries <- entry :: entries
        | None -> () );
      positions := SSet.add pos_str !positions

    (* Add the completion token and wrap up the context and candidate list *)
    method add_entry
        (completion_type : SearchUtils.autocomplete_type)
        (p : Pos.t)
        (target : string) : unit =
      let candidates = !candidates |> List.rev |> String.concat ~sep:" " in
      let context = self#extract_context p in
      let actype_str = SearchUtils.show_autocomplete_type completion_type in
      let (line, column) = Pos.line_column p in
      let column = column + 1 in
      let pos_str = Printf.sprintf "%d-%d" line column in
      ( if not (SSet.exists (fun key -> String.equal key pos_str) !positions)
      then
        match context with
        | Some tokens ->
          let entry =
            {
              filename;
              completion_type = actype_str;
              tokens;
              target;
              candidates;
            }
          in
          entries <- entry :: entries
        | None -> () );
      positions := SSet.add pos_str !positions

    method! on_Lvar env lid =
      (* add local variable as Acprop token *)
      let text = Local_id.get_name (snd lid) in
      self#compute_complete_local (fst lid) text;
      super#on_Lvar env lid

    method! on_Class_get env cid mid =
      let res = super#on_Class_get env cid mid in
      (match mid with
      | Tast.CGstring p ->
        self#autocomplete_static_member env cid;
        self#add_entry SearchUtils.Acclass_get (fst p) (snd p)
      | Tast.CGexpr _ -> ());
      res

    method! on_Class_const env cid mid =
      self#autocomplete_static_member env cid;
      self#add_entry SearchUtils.Acclass_get (fst mid) (snd mid);
      super#on_Class_const env cid mid

    method! on_Obj_get env obj mid ognf =
      let res = super#on_Obj_get env obj mid ognf in
      (match mid with
      | (_, Tast.Id mid) ->
        self#autocomplete_typed_member ~is_static:false env (get_type obj) None;
        self#add_entry SearchUtils.Acclass_get (fst mid) (snd mid)
      | _ -> ());
      res
  end

let go_ctx ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    string =
  (* TODO(ljw): surely this doesn't need quarantine? *)
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  (* Visit the TAST and extract autocompleteion items for Acclass_get, Acprop *)
  let filename = Relative_path.suffix entry.path in
  let source_text = Provider_context.read_file_contents_exn entry in
  let data =
    (new visitor ~ctx ~entry ~filename ~source_text)#get_entries ctx tast
  in
  let completions =
    List.map data ~f:(fun entry ->
        JSON_Object
          [
            ("filename", JSON_String entry.filename);
            ("completion_type", JSON_String entry.completion_type);
            ("tokens", JSON_String entry.tokens);
            ("target", JSON_String entry.target);
            ("candidates", JSON_String entry.candidates);
          ])
  in
  let output = JSON_Object [("completions", JSON_Array completions)] in
  Hh_json.json_to_string output
