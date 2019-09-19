(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hh_bucket = Bucket
open Core_kernel
open Common
module Syntax = Full_fidelity_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax (Syntax)

(**
 * A `SyntaxKind.t`. We can generate a string from a kind, but not the other way
 * around, so we store the string value given to us in the input directly. Then,
 * when examining a node to see if it's a match, we'll convert the node's kind
 * to a string and simply use string comparison.
 *
 * (Note that multiple syntax kinds map to the same string anyways, so it would
 * be hard to reverse the mapping.)
 *)
type node_kind = NodeKind of string

(**
 * An identifier in the pattern used to identify the this node when returning
 * the results of a match to the caller. This identifier may not be unique in
 * the list of returned results.
 *)
type match_name = MatchName of string

(**
 * The name of a child of a node.
 *
 * For example, a binary expression has three possible child names:
 *
 *   * binary_left_operand
 *   * binary_operator
 *   * binary_right_operand
 *
 * See the FFP schema definition in `src/parser/schema/schema_definition.ml`.
 * Note that `BinaryExpression` has the prefix of `binary` and children like
 * `left_operand`. When combined, this forms the full name
 * `binary_left_operand`.
 *)
type child_type = ChildType of string

(**
 * A query that can be run on the syntax tree. It will return the matched nodes
 * of any matched `MatchPattern`s.
 *)
type pattern =
  (*
   * Match any node with the given kind, and whose children match the given
   * patterns.
   *)
  | NodePattern of {
      kind: node_kind;
      (*
       * Mapping from child name to pattern that the child must satisfy.
       * Children may be omitted from this map. No child type may be specified
       * more than once.
       *)
      children: (child_type * pattern) list;
    }
  (* Match any missing node. *)
  | MissingNodePattern
  (*
   * Return the given node in the result list, assuming that the pattern overall
   * matches. (This pattern by itself always matches; it's often used with
   * `AndPattern`).
   *)
  | MatchPattern of { match_name: match_name }
  (*
   * Matches a given node if there is descendant node matching the given pattern
   * anywhere in the subtree underneath the parent node.
   *)
  | DescendantPattern of { pattern: pattern }
  (*
   * Matches a list node (such as a statement list) only if all the children at
   * the given indexes match their respective patterns.
   *)
  | ListPattern of {
      children: (int * pattern) list;
      max_length: int option;
    }
  (*
   * Matches a given node if its raw text is exactly the specified string. The
   * "raw" text doesn't include trivia.
   *)
  | RawTextPattern of { raw_text: string }
  (*
   * Matches a given expression node if its type is a subtype of the given type.
   * TODO: decide if we want to handle exact type equality or supertypes.
   * TODO: decide what we want to name the pattern for decl types, if we create
   * one.
   *)
  | TypePattern of { subtype_of: Typing_defs.locl_ty }
  (*
   * Matches if all of the children patterns match.
   *)
  | AndPattern of { patterns: pattern list }
  (*
   * Matches if any of the children patterns match.
   *
   * Note that currently:
   *
   *   * This short-circuits, so the after any pattern succeeds, no other
   *   patterns will be evaluated. This means that if two patterns have
   *   `MatchPattern`s somewhere underneath them, then the `MatchPattern`s from
   *   at most one of these patterns will be included in the results.
   *   * The order in which the provided patterns are evaluated is not
   *   specified. (For example, in the future, we may want to order it so that
   *   patterns that rely only on syntactic information are executed before
   *   patterns that rely on type information.)
   *
   * Consequently, you shouldn't rely on `MatchPattern`s that are nested inside
   * the constituent patterns.
   *)
  | OrPattern of { patterns: pattern list }
  (*
   * Matches only if the given pattern does not match.
   *
   * Regardless of whether the child pattern matches or not, any matches that it
   * produced are thrown away. (In the case that it doesn't match, we don't keep
   * around "witness" matches explaining why this `NotPattern` didn't match.)
   *)
  | NotPattern of { pattern: pattern }

type matched_node = {
  match_name: match_name;
  kind: node_kind;
  start_offset: int;
  end_offset: int;
}

type result = {
  (*
   * The list of nodes for which a `MatchPattern` matched.
   *)
  matched_nodes: matched_node list;
}

type collected_type_map =
  Tast_type_collector.collected_type list Pos.AbsolutePosMap.t

(* The environment used in searching a single file. *)
type env = {
  tcopt: TypecheckerOptions.t;
  fileinfo: FileInfo.t;
  path: Relative_path.t;
  syntax_tree: SyntaxTree.t;
  collected_types: collected_type_map option;
}

let empty_result : result option = Some { matched_nodes = [] }

let merge_results (lhs : result option) (rhs : result option) : result option =
  Option.merge lhs rhs ~f:(fun lhs rhs ->
      { matched_nodes = lhs.matched_nodes @ rhs.matched_nodes })

let find_child_with_type (node : Syntax.t) (child_type : child_type) :
    Syntax.t option =
  let child_index =
    Syntax.children_names node
    |> List.findi ~f:(fun _i actual_child_type ->
           match child_type with
           | ChildType expected_child_type ->
             expected_child_type = actual_child_type)
    |> Option.map ~f:fst
  in
  match child_index with
  | None -> None
  | Some index -> List.nth (Syntax.children node) index

let collect_types (env : env) : env * collected_type_map =
  match env with
  | { collected_types = Some collected_types; _ } -> (env, collected_types)
  | { collected_types = None; tcopt; fileinfo; path; _ } ->
    let tast = ServerIdeUtils.check_fileinfo tcopt path fileinfo in
    let collected_types = Tast_type_collector.collect_types tast in
    let env = { env with collected_types = Some collected_types } in
    (env, collected_types)

let rec search_node ~(env : env) ~(pattern : pattern) ~(node : Syntax.t) :
    env * result option =
  match pattern with
  | NodePattern { kind; children } ->
    let kind =
      match kind with
      | NodeKind kind -> kind
    in
    if node |> Syntax.kind |> SyntaxKind.to_string <> kind then
      (env, None)
    else
      let patterns =
        List.map children ~f:(fun (child_type, pattern) ->
            let child_node = find_child_with_type node child_type in
            let child_node = Option.value_exn child_node in
            (child_node, pattern))
      in
      search_and ~env ~patterns
  | MissingNodePattern ->
    if Syntax.kind node = SyntaxKind.Missing then
      (env, empty_result)
    else
      (env, None)
  | MatchPattern { match_name } ->
    let result =
      {
        matched_nodes =
          [
            {
              match_name;
              kind = NodeKind (SyntaxKind.to_string (Syntax.kind node));
              start_offset = Syntax.start_offset node;
              end_offset = Syntax.end_offset node;
            };
          ];
      }
    in
    (env, Some result)
  | DescendantPattern { pattern } -> search_descendants ~env ~pattern ~node
  | ListPattern { children; max_length } ->
    let syntax_list =
      Syntax.(
        match node.syntax with
        | SyntaxList syntax_list ->
          begin
            match max_length with
            | None -> Some syntax_list
            | Some max_length ->
              if List.length syntax_list <= max_length then
                Some syntax_list
              else
                None
          end
        | _ -> None)
    in
    begin
      match syntax_list with
      | None -> (env, None)
      | Some syntax_list ->
        Option.Monad_infix.(
          let patterns =
            List.map children ~f:(fun (index, pattern) ->
                List.nth syntax_list index
                >>| (fun child_node -> (child_node, pattern)))
          in
          begin
            match Option.all patterns with
            | Some patterns -> search_and ~env ~patterns
            | None ->
              (* We tried to match a pattern for the child at at index N, but the syntax
        list didn't have an Nth element. *)
              (env, None)
          end)
    end
  | RawTextPattern { raw_text } ->
    if Syntax.text node = raw_text then
      (env, empty_result)
    else
      (env, None)
  | TypePattern { subtype_of } ->
    Line_break_map.reset_global_state ();
    let pos = Syntax.position env.path node in
    begin
      match pos with
      | None -> (env, None)
      | Some pos ->
        let (env, collected_types) = collect_types env in
        let pos = Pos.to_absolute pos in
        let tys = Tast_type_collector.get_from_pos_map pos collected_types in
        let is_subtype_of (tast_env, ty) =
          match ty with
          | Typing_defs.LoclTy ty ->
            Tast_env.can_subtype tast_env ty subtype_of
          | Typing_defs.DeclTy _ty -> false
        in
        (match tys with
        | Some tys when List.exists tys ~f:is_subtype_of -> (env, empty_result)
        | Some _
        | None ->
          (env, None))
    end
  | AndPattern { patterns } ->
    let patterns = List.map patterns ~f:(fun pattern -> (node, pattern)) in
    search_and ~env ~patterns
  | OrPattern { patterns } ->
    let patterns = List.map patterns ~f:(fun pattern -> (node, pattern)) in
    search_or ~env ~patterns
  | NotPattern { pattern } ->
    let (env, result) = search_node ~env ~node ~pattern in
    begin
      match result with
      | Some _ -> (env, None)
      | None -> (env, empty_result)
    end

(* TODO: this will likely have to become more intelligent *)
and search_descendants ~(env : env) ~(pattern : pattern) ~(node : Syntax.t) :
    env * result option =
  List.fold_left_env
    env
    (Syntax.children node)
    ~init:None
    ~f:(fun env acc_result child ->
      let (env, child_result) = search_node ~env ~pattern ~node:child in
      let (env, descendants_result) =
        search_descendants ~env ~pattern ~node:child
      in
      let result = merge_results child_result descendants_result in
      (env, merge_results result acc_result))

and search_and ~(env : env) ~(patterns : (Syntax.t * pattern) list) :
    env * result option =
  List.fold_left_env
    env
    patterns
    ~init:empty_result
    ~f:(fun env result (node, pattern) ->
      match result with
      | None ->
        (* Short-circuit. *)
        (env, None)
      | Some _ as result ->
        let (env, pattern_result) = search_node ~env ~pattern ~node in
        (match pattern_result with
        | None -> (env, None)
        | Some _ as pattern_result -> (env, merge_results result pattern_result)))

and search_or ~(env : env) ~(patterns : (Syntax.t * pattern) list) :
    env * result option =
  List.fold_left_env
    env
    patterns
    ~init:None
    ~f:(fun env result (node, pattern) ->
      match result with
      | Some _ as result ->
        (* Short-circuit. *)
        (env, result)
      | None -> search_node ~env ~pattern ~node)

let compile_pattern (json : Hh_json.json) : (pattern, string) Result.t =
  Result.(
    Result.Monad_infix.(
      let wrap_json_accessor f x =
        Result.map_error (f x) ~f:Hh_json.Access.access_failure_to_string
      in
      let get_string x = wrap_json_accessor (Hh_json.Access.get_string x) in
      let get_obj x = wrap_json_accessor (Hh_json.Access.get_obj x) in
      let get_array x = wrap_json_accessor (Hh_json.Access.get_array x) in
      let keytrace_to_string = Hh_json.Access.keytrace_to_string in
      let error_at_keytrace ~keytrace error_message =
        Error (error_message ^ keytrace_to_string keytrace)
      in
      let rec compile_pattern ~json ~keytrace : (pattern, string) Result.t =
        get_string "pattern_type" (json, keytrace)
        >>= fun (pattern_type, pattern_type_keytrace) ->
        match pattern_type with
        | "node_pattern" -> compile_node_pattern ~json ~keytrace
        | "missing_node_pattern" ->
          compile_missing_node_pattern ~json ~keytrace
        | "match_pattern" -> compile_match_pattern ~json ~keytrace
        | "descendant_pattern" -> compile_descendant_pattern ~json ~keytrace
        | "list_pattern" -> compile_list_pattern ~json ~keytrace
        | "raw_text_pattern" -> compile_raw_text_pattern ~json ~keytrace
        | "type_pattern" -> compile_type_pattern ~json ~keytrace
        | "and_pattern" -> compile_and_pattern ~json ~keytrace
        | "or_pattern" -> compile_or_pattern ~json ~keytrace
        | "not_pattern" -> compile_not_pattern ~json ~keytrace
        | pattern_type ->
          error_at_keytrace
            ~keytrace:pattern_type_keytrace
            (Printf.sprintf "Unknown pattern type '%s'" pattern_type)
      and compile_node_pattern ~json ~keytrace : (pattern, string) Result.t =
        get_string "kind" (json, keytrace)
        >>= fun (kind, kind_keytrace) ->
        Schema_definition.(
          let kind_info =
            List.find schema ~f:(fun schema_node ->
                schema_node.description = kind)
          in
          match kind_info with
          | None ->
            error_at_keytrace
              ~keytrace:kind_keytrace
              (Printf.sprintf "Kind '%s' doesn't exist" kind)
          | Some kind_info ->
            Ok kind_info
            >>= fun kind_info ->
            get_obj "children" (json, keytrace)
            >>= fun (children_json, children_keytrace) ->
            (* This has already been verified to be an object above. *)
            let children = Hh_json.get_object_exn children_json in
            let get_child_type
                (child_keytrace : Hh_json.Access.keytrace)
                (child_name : string) : (child_type, string) Result.t =
              (* We're given a field name like `binary_right_operand`, but the field
      names in the schema are things like `right_operand`, and you have to
      affix the prefix yourself. For consistency with other tooling, we want
      to use `binary_right_operand` instead of just `right_operand`. *)
              let get_prefixed_field_name field_name =
                kind_info.prefix ^ "_" ^ field_name
              in
              let field =
                List.find kind_info.fields ~f:(fun (field_name, _) ->
                    get_prefixed_field_name field_name = child_name)
              in
              match field with
              | None ->
                let valid_types =
                  List.map kind_info.fields ~f:(fun (field_name, _) ->
                      get_prefixed_field_name field_name)
                in
                error_at_keytrace
                  ~keytrace:child_keytrace
                  (Printf.sprintf
                     ( "Unknown child type '%s'; "
                     ^^ "valid child types for a node of kind '%s' are: %s" )
                     child_name
                     kind
                     (String.concat ~sep:", " valid_types))
              | Some _ -> Ok (ChildType child_name)
            in
            let children_patterns =
              List.mapi children ~f:(fun index (child_name, pattern_json) ->
                  let child_keytrace =
                    string_of_int index :: children_keytrace
                  in
                  get_child_type child_keytrace child_name
                  >>= fun child_name ->
                  compile_pattern ~json:pattern_json ~keytrace:child_keytrace
                  >>| (fun pattern -> (child_name, pattern)))
            in
            all children_patterns
            >>| (fun children -> NodePattern { kind = NodeKind kind; children }))
      and compile_missing_node_pattern ~json:_json ~keytrace:_keytrace =
        Ok MissingNodePattern
      and compile_match_pattern ~json ~keytrace =
        get_string "match_name" (json, keytrace)
        >>| fun (match_name, _match_name_keytrace) ->
        MatchPattern { match_name = MatchName match_name }
      and compile_descendant_pattern ~json ~keytrace =
        get_obj "pattern" (json, keytrace)
        >>= fun (pattern, pattern_keytrace) ->
        compile_pattern ~json:pattern ~keytrace:pattern_keytrace
        >>| (fun pattern -> DescendantPattern { pattern })
      and compile_list_pattern ~json ~keytrace =
        let max_length =
          Hh_json.get_field_opt
            (Hh_json.Access.get_number_int "max_length")
            json
        in
        get_obj "children" (json, keytrace)
        >>= fun (children_json, children_keytrace) ->
        (* This has already been verified to be an object above. *)
        let children = Hh_json.get_object_exn children_json in
        let children_patterns =
          List.map children ~f:(fun (index_str, pattern_json) ->
              let child_keytrace = index_str :: children_keytrace in
              begin
                match int_of_string_opt index_str with
                | Some index -> Ok index
                | None ->
                  error_at_keytrace
                    (Printf.sprintf "Invalid integer key: %s" index_str)
                    ~keytrace:child_keytrace
              end
              >>= fun index ->
              begin
                if index >= 0 then
                  Ok index
                else
                  error_at_keytrace
                    "Integer key must be non-negative"
                    ~keytrace:child_keytrace
              end
              >>= fun index ->
              compile_pattern ~json:pattern_json ~keytrace:child_keytrace
              >>| (fun pattern -> (index, pattern)))
        in
        Result.all children_patterns
        >>| (fun children -> ListPattern { children; max_length })
      and compile_raw_text_pattern ~json ~keytrace =
        get_string "raw_text" (json, keytrace)
        >>| (fun (raw_text, _raw_text_keytrace) -> RawTextPattern { raw_text })
      and compile_type_pattern ~json ~keytrace =
        get_obj "subtype_of" (json, keytrace)
        >>= fun (subtype_of_json, subtype_of_keytrace) ->
        let locl_ty =
          Typing_print.json_to_locl_ty
            ~keytrace:subtype_of_keytrace
            subtype_of_json
        in
        match locl_ty with
        | Ok locl_ty -> Ok (TypePattern { subtype_of = locl_ty })
        | Error (Typing_defs.Wrong_phase message)
        | Error (Typing_defs.Not_supported message)
        | Error (Typing_defs.Deserialization_error message) ->
          Error message
      and compile_child_patterns_helper ~json ~keytrace =
        get_array "patterns" (json, keytrace)
        >>= fun (pattern_list, pattern_list_keytrace) ->
        let compiled_patterns =
          List.mapi pattern_list (fun i json ->
              let keytrace = string_of_int i :: pattern_list_keytrace in
              compile_pattern ~json ~keytrace)
        in
        Result.all compiled_patterns
      and compile_and_pattern ~json ~keytrace =
        compile_child_patterns_helper ~json ~keytrace
        >>| (fun patterns -> AndPattern { patterns })
      and compile_or_pattern ~json ~keytrace =
        compile_child_patterns_helper ~json ~keytrace
        >>| (fun patterns -> OrPattern { patterns })
      and compile_not_pattern ~json ~keytrace =
        get_obj "pattern" (json, keytrace)
        >>= fun (json, keytrace) ->
        compile_pattern ~json ~keytrace
        >>| (fun pattern -> NotPattern { pattern })
      in
      compile_pattern ~json ~keytrace:[]))

let result_to_json ~(sort_results : bool) (result : result option) :
    Hh_json.json =
  Hh_json.(
    match result with
    | None -> JSON_Null
    | Some result ->
      let matched_nodes = result.matched_nodes in
      let matched_nodes =
        if sort_results then
          List.sort matched_nodes ~compare:Pervasives.compare
        else
          matched_nodes
      in
      let matched_nodes =
        List.map matched_nodes ~f:(fun matched_node ->
            let match_name =
              match matched_node.match_name with
              | MatchName match_name -> match_name
            in
            let kind =
              match matched_node.kind with
              | NodeKind kind -> kind
            in
            JSON_Object
              [
                ("match_name", JSON_String match_name);
                ("kind", JSON_String kind);
                ("start_offset", Hh_json.int_ matched_node.start_offset);
                ("end_offset", Hh_json.int_ matched_node.end_offset);
              ])
      in
      JSON_Object [("matched_nodes", JSON_Array matched_nodes)])

let search
    (tcopt : TypecheckerOptions.t)
    (path : Relative_path.t)
    (fileinfo : FileInfo.t)
    (pattern : pattern) : result option =
  let source_text = Full_fidelity_source_text.from_file path in
  let syntax_tree = SyntaxTree.make source_text in
  let env = { tcopt; fileinfo; path; syntax_tree; collected_types = None } in
  let (_env, result) =
    search_node ~env ~pattern ~node:(SyntaxTree.root env.syntax_tree)
  in
  result

let go
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    ~(sort_results : bool)
    ~(files_to_search : string list option)
    (input : Hh_json.json) : (Hh_json.json, string) Result.t =
  Result.Monad_infix.(
    compile_pattern input
    >>| fun pattern ->
    let num_files_searched = ref 0 in
    let last_printed_num_files_searched = ref 0 in
    let done_searching = ref false in
    let progress_fn ~total:_total ~start:_start ~(length : int) : unit =
      let is_bucket_empty = length = 0 in
      if not !done_searching then (
        num_files_searched := !num_files_searched + length;
        if
          !num_files_searched - !last_printed_num_files_searched >= 10000
          || is_bucket_empty
        then (
          ServerProgress.send_progress_to_monitor
            "CST search: searched %d files..."
            !num_files_searched;
          last_printed_num_files_searched := !num_files_searched
        )
      );
      if is_bucket_empty then done_searching := true
    in
    let next_files :
        (Relative_path.t * FileInfo.t * pattern) list Hh_bucket.next =
      let with_file_data path =
        let path = Relative_path.create_detect_prefix path in
        match Naming_table.get_file_info env.ServerEnv.naming_table path with
        | Some fileinfo -> Some (path, fileinfo, pattern)
        | None ->
          (* We may not have the file information for a file such as one that we
        ignore in `.hhconfig`. *)
          None
      in
      match files_to_search with
      | Some files_to_search ->
        let files_to_search =
          Sys_utils.parse_path_list files_to_search
          |> List.filter_map ~f:with_file_data
        in
        MultiWorker.next genv.ServerEnv.workers files_to_search ~progress_fn
      | None ->
        let indexer = genv.ServerEnv.indexer FindUtils.is_php in
        fun () ->
          let files = indexer () |> List.filter_map ~f:with_file_data in
          progress_fn ~total:0 ~start:0 ~length:(List.length files);
          Hh_bucket.of_list files
    in
    (* Extract the `tcopt` so that we don't close over the entire `env`. *)
    let tcopt = env.ServerEnv.tcopt in
    let job
        (acc : (Relative_path.t * result) list)
        (inputs : (Relative_path.t * FileInfo.t * pattern) list) :
        (Relative_path.t * result) list =
      List.fold inputs ~init:acc ~f:(fun acc (path, fileinfo, pattern) ->
          try
            match search tcopt path fileinfo pattern with
            | Some result -> (path, result) :: acc
            | None -> acc
          with e ->
            let stack = Printexc.get_backtrace () in
            let prefix =
              Printf.sprintf
                "Error while running CST search on path %s:\n"
                (Relative_path.to_absolute path)
            in
            Hh_logger.exc e ~prefix ~stack;
            raise e)
    in
    let results =
      MultiWorker.call
        genv.ServerEnv.workers
        ~job
        ~neutral:[]
        ~merge:List.rev_append
        ~next:next_files
    in
    let results =
      if sort_results then
        List.sort results ~compare:Pervasives.compare
      else
        results
    in
    Hh_json.JSON_Object
      (List.map results ~f:(fun (path, result) ->
           ( Relative_path.to_absolute path,
             result_to_json ~sort_results (Some result) ))))
