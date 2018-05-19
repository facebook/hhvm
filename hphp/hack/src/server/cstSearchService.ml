(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

module Syntax = Full_fidelity_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_positioned_syntax)

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
  (**
   * Match any node with the given kind, and whose children match the given
   * patterns.
   *)
  | NodePattern of {
      kind: node_kind;

      (**
       * Mapping from child name to pattern that the child must satisfy.
       * Children may be omitted from this map. No child type may be specified
       * more than once.
       *)
      children: (child_type * pattern) list;
    }

  (**
   * Return the given node in the result list, assuming that the pattern overall
   * matches. (This pattern by itself always matches; it's often used with
   * `AndPattern`).
   *)
  | MatchPattern of {
      match_name: match_name;
    }

  (**
   * Matches a given node if there is descendant node matching the given pattern
   * anywhere in the subtree underneath the parent node.
   *)
  | DescendantPattern of {
      pattern: pattern;
    }

  (**
   * Matches a given node if its raw text is exactly the specified string. The
   * "raw" text doesn't include trivia.
   *)
  | RawTextPattern of {
      raw_text: string;
    }

type matched_node = {
  match_name: match_name;
  kind: node_kind;
  start_offset: int;
  end_offset: int;
}

type result = {
  (**
   * The list of nodes for which a `MatchPattern` matched.
   *)
  matched_nodes: matched_node list;
}

type env = {
  syntax_tree: SyntaxTree.t;
}

let empty_result: result option = Some { matched_nodes = [] }

let merge_results (lhs: result option) (rhs: result option): result option =
  Option.merge lhs rhs ~f:(fun lhs rhs ->
    { matched_nodes = lhs.matched_nodes @ rhs.matched_nodes }
  )

let find_child_with_type
    (node: Syntax.t)
    (child_type: child_type)
    : Syntax.t option =
  let child_index =
    Syntax.children_names node
    |> List.findi ~f:(fun _i actual_child_type ->
        match child_type with
        | ChildType expected_child_type ->
          expected_child_type = actual_child_type
      )
    |> Option.map ~f:fst
  in
  match child_index with
  | None -> None
  | Some index -> List.nth (Syntax.children node) index

let rec search_node
    ~(env: env)
    ~(pattern: pattern)
    ~(node: Syntax.t)
    : env * result option =
  match pattern with
  | NodePattern { kind; children } ->
    let kind = match kind with NodeKind kind -> kind in
    if (node |> Syntax.kind |> SyntaxKind.to_string) <> kind
    then (env, None)
    else
    let (env, result) = List.fold_left_env env children
      ~init:empty_result
      ~f:(fun env acc_result (child_type, pattern) ->
        match acc_result with
        (* We failed to match a previous child pattern; short-circuit. *)
        | None -> (env, None)

        | Some _ as result ->
          let child_node = find_child_with_type node child_type in
          let child_node = Option.value_exn child_node in
          let (env, child_result) =
            search_node ~env ~pattern ~node:child_node in
          match child_result with
          | None -> (env, None)
          | Some _ as child_result ->
            let result = merge_results result child_result in
            (env, result)
      )
    in
    (env, result)

  | MatchPattern { match_name } ->
    let result = {
      matched_nodes = [
        {
          match_name;
          kind = NodeKind (SyntaxKind.to_string (Syntax.kind node));
          start_offset = Syntax.start_offset node;
          end_offset = Syntax.end_offset node;
        }
      ]
    } in
    (env, Some result)

  | DescendantPattern { pattern } ->
    search_descendants ~env ~pattern ~node

  | RawTextPattern { raw_text } ->
    if Syntax.text node = raw_text
    then (env, empty_result)
    else (env, None)

(* TODO: this will likely have to become more intelligent *)
and search_descendants
    ~(env: env)
    ~(pattern: pattern)
    ~(node: Syntax.t)
    : env * result option =
  List.fold_left_env
    env
    (Syntax.children node)
    ~init:None
    ~f:(fun env acc_result child ->
      let (env, child_result) = search_node ~env ~pattern ~node:child in
      let (env, descendants_result) =
        search_descendants ~env ~pattern ~node:child in
      let result = merge_results child_result descendants_result in
      (env, (merge_results result acc_result))
    )

let compile_pattern (json: Hh_json.json): (pattern, string) Core_result.t =
  let open Core_result in
  let open Core_result.Monad_infix in

  let wrap_json_accessor f =
    fun x -> Core_result.map_error (f x)
      ~f:Hh_json.Access.access_failure_to_string
  in

  let get_string x = wrap_json_accessor (Hh_json.Access.get_string x) in
  let get_obj x = wrap_json_accessor (Hh_json.Access.get_obj x) in
  let keytrace_to_string = Hh_json.Access.keytrace_to_string in
  let error_at_keytrace ~keytrace error_message =
    Error (error_message ^ (keytrace_to_string keytrace))
  in

  let rec compile_pattern ~json ~keytrace : (pattern, string) Core_result.t =
    get_string "pattern_type" (json, keytrace)

    >>= fun (pattern_type, pattern_type_keytrace) ->
    match pattern_type with
    | "node_pattern" ->
      compile_node_pattern ~json ~keytrace
    | "match_pattern" ->
      compile_match_pattern ~json ~keytrace
    | "descendant_pattern" ->
      compile_descendant_pattern ~json ~keytrace
    | "raw_text_pattern" ->
      compile_raw_text_pattern ~json ~keytrace
    | pattern_type ->
      error_at_keytrace ~keytrace:pattern_type_keytrace
        (Printf.sprintf "Unknown pattern type '%s'" pattern_type)

  and compile_node_pattern ~json ~keytrace : (pattern, string) Core_result.t =
    get_string "kind" (json, keytrace)

    >>= fun (kind, kind_keytrace) ->
    let open Schema_definition in
    let kind_info = List.find schema ~f:(fun schema_node ->
      schema_node.description = kind
    ) in
    match kind_info with
    | None ->
      error_at_keytrace ~keytrace:kind_keytrace
        (Printf.sprintf "Kind '%s' doesn't exist" kind)
    | Some kind_info -> Ok kind_info

    >>= fun kind_info ->
    get_obj "children" (json, keytrace)

    >>= fun (children_json, children_keytrace) ->
    (* This has already been verified to be an object above. *)
    let children = Hh_json.get_object_exn children_json in

    let get_child_type
        (child_name: string)
        : (child_type, string) Core_result.t =
      (* We're given a field name like `binary_right_operand`, but the field
      names in the schema are things like `right_operand`, and you have to
      affix the prefix yourself. For consistency with other tooling, we want
      to use `binary_right_operand` instead of just `right_operand`. *)
      let get_prefixed_field_name field_name =
        kind_info.prefix ^ "_" ^ field_name
      in
      let field = List.find kind_info.fields ~f:(fun (field_name, _) ->
        (get_prefixed_field_name field_name) = child_name)
      in
      match field with
      | None ->
        let valid_types = List.map kind_info.fields ~f:(fun (field_name, _) ->
          get_prefixed_field_name field_name
        ) in
        error_at_keytrace ~keytrace:children_keytrace
          (Printf.sprintf
            ("Unknown child type '%s'; "^^
            "valid child types for a node of kind '%s' are: %s")
            child_name
            kind
            (String.concat ", " valid_types))
      | Some _ -> Ok (ChildType child_name)
    in
    let children_patterns =
      List.map children ~f:(fun (child_name, pattern_json) ->
        get_child_type child_name >>= fun child_name ->
        compile_pattern ~json:pattern_json ~keytrace:children_keytrace
        >>| fun pattern ->
        (child_name, pattern)
      )
    in
    all children_patterns >>| fun children ->
    NodePattern {
      kind = NodeKind kind;
      children;
    }

  and compile_match_pattern ~json ~keytrace =
    get_string "match_name" (json, keytrace)
    >>| fun (match_name, _match_name_keytrace) ->
    MatchPattern {
      match_name = MatchName match_name;
    }

  and compile_descendant_pattern ~json ~keytrace =
    get_obj "pattern" (json, keytrace) >>= fun (pattern, pattern_keytrace) ->
    compile_pattern ~json:pattern ~keytrace:pattern_keytrace >>| fun pattern ->
    DescendantPattern {
      pattern;
    }

  and compile_raw_text_pattern ~json ~keytrace =
    get_string "raw_text" (json, keytrace)
    >>| fun (raw_text, _raw_text_keytrace) ->
    RawTextPattern {
      raw_text;
    }

  in
  compile_pattern ~json ~keytrace:[]

(* TODO(T28496995): This only converts a single result to JSON. We also need to
convert an entire response -- a mapping from file path to result -- to JSON. *)
let result_to_json (result: result option): Hh_json.json =
  let open Hh_json in
  match result with
  | None -> JSON_Null
  | Some result ->
    let matched_nodes = List.map result.matched_nodes ~f:(fun matched_node ->
      let match_name =
        match matched_node.match_name
        with MatchName match_name -> match_name
      in
      let kind =
        match matched_node.kind
        with NodeKind kind -> kind
      in
      JSON_Object [
        "match_name", JSON_String match_name;
        "kind", JSON_String kind;
        "start_offset", Hh_json.int_ matched_node.start_offset;
        "end_offset", Hh_json.int_ matched_node.end_offset;
      ])
    in
    JSON_Object [
      "matched_nodes", JSON_Array matched_nodes;
    ]

let search
    ~(syntax_tree: SyntaxTree.t)
    (pattern: pattern)
    : result option =
  let env = { syntax_tree } in
  let (_env, result) =
    search_node ~env ~pattern ~node:(SyntaxTree.root env.syntax_tree) in
  result

let job
    (acc: (Relative_path.t * result) list)
    (inputs: (Relative_path.t * pattern) list)
    : (Relative_path.t * result) list =
  List.fold inputs
    ~init:acc
    ~f:(fun acc (path, pattern) ->
      try
        let source_text = Full_fidelity_source_text.from_file path in
        let syntax_tree = SyntaxTree.make source_text in
        match search ~syntax_tree pattern with
        | Some result -> (path, result) :: acc
        | None -> acc
      with e ->
        let prefix = Printf.sprintf
          "Error while running CST search on path %s:\n"
          (Relative_path.to_absolute path)
        in
        Hh_logger.exc e ~prefix;
        raise e
    )

let go
    (genv: ServerEnv.genv)
    (input: Hh_json.json)
    : (Hh_json.json, string) Core_result.t
  =
  let open Core_result.Monad_infix in
  compile_pattern input >>| fun pattern ->

  let num_files_searched = ref 0 in
  let indexer = genv.ServerEnv.indexer FindUtils.is_php in
  let next_files () =
    let files = indexer ()
      |> List.map ~f:(fun path ->
        let path = Relative_path.create Relative_path.Root path in
        (path, pattern)
      )
    in
    Hh_logger.log "CST search: searched %d files..." !num_files_searched;
    num_files_searched := !num_files_searched + (List.length files);
    Bucket.of_list files
  in
  let results = MultiWorker.call
    genv.ServerEnv.workers
    ~job
    ~neutral:[]
    ~merge:List.rev_append
    ~next:next_files
  in

  Hh_json.JSON_Object (List.map results ~f:(fun (path, result) ->
    (Relative_path.to_absolute path, result_to_json (Some result))
  ))
