open Hh_core

module Syntax = Full_fidelity_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_positioned_syntax)

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
      kind: SyntaxKind.t;

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

type response = {
  (**
   * Map from file path to results for that file. Doesn't include files that
   * didn't have any results.
   *)
  results: (string * result) list;
}

and result = {
  (**
   * The list of nodes for which a `MatchPattern` matched.
   *)
  matched_nodes: matched_node list;
}

and matched_node = {
  match_name: match_name;
  node: Syntax.t;
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
    if (Syntax.kind node) <> kind
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
        { match_name; node }
      ]
    } in
    (env, Some result)

  | DescendantPattern { pattern } ->
    search_descendants ~env ~pattern ~node

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

let search ~(env: env) ~(pattern: pattern): result option =
  let (_env, result) =
    search_node ~env ~pattern ~node:(SyntaxTree.root env.syntax_tree) in
  result
