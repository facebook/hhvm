(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module type RewritableType = sig
  type t
  val children : t -> t list
  val from_children : Full_fidelity_syntax_kind.t -> t list -> t
  val kind: t -> Full_fidelity_syntax_kind.t
end

module WithSyntax(Syntax: RewritableType) = struct
  module Result = struct
    type 'a t = Remove | Keep | Replace of 'a
  end

  include Result

  (* The rewriter takes a function f that is
     parents      : [node]  ->
     current_node : node ->
     current_acc  : accumulator ->
     (accumulator, node result)

     The tree is walked in post order; leaves are rewritten first. After all
     the children of a node are rewritten, then f is called on the rewritten
     node.

     The function itself returns (acc, node)
     *)
  let parented_aggregating_rewrite_post f node init_acc =
    (* aux takes parents node accumulator and returns
      (acc, node result)  *)
    let rec aux parents node acc =
      (* Start by rewriting all the children.
         We begin by obtaining a node list, and then from it producing
         a new (accumulator, changed) value and a node opt list.
         We then transform the node opt list into a node list,
         which is the new list of children. *)
      let mapper (acc, changed) child =
        match aux (node :: parents) child acc with
        | (acc, Replace new_child) -> ((acc, true), Some new_child)
        | (acc, Keep) -> ((acc, changed), Some child)
        | (acc, Remove) -> ((acc, true), None)
      in
      let ((acc, child_changed), option_new_children) =
        Hh_core.List.map_env (acc, false) (Syntax.children node) ~f:mapper in
      let node =
        if child_changed then
          let new_children = Hh_core.List.filter_opt option_new_children in
          Syntax.from_children (Syntax.kind node) new_children
        else
          node
      in
      let (acc, result) = f parents node acc in
      let result = match (result, child_changed) with
        | (Keep, true) -> Replace node
        | _ -> result in
      (acc, result)
      in (* end of aux *)
    let (acc, result) = aux [] node init_acc in
    let result_node = match result with
      | Keep -> node
      | Replace new_node -> new_node
      | Remove -> failwith "rewriter removed the root node!" in
    (acc, result_node)

  (* The same as the above, except that f does not take parents. *)
  let aggregating_rewrite_post f node init_acc =
    let f parents node acc = f node acc in
    parented_aggregating_rewrite_post f node init_acc

  (* The same as the above, except that f does not take or return an
     accumulator. *)
  let parented_rewrite_post f node =
    let f parents node acc = ([], f parents node) in
    let (acc, result) = parented_aggregating_rewrite_post f node [] in
    result

  (* The same as the above, except that f does not take or return an
     accumulator, and f does not take parents *)
  let rewrite_post f node =
    let f parents node acc = ([], f node) in
    let (acc, result) = parented_aggregating_rewrite_post f node [] in
    result

  (* Here f is a function node -> node opt, with the semantics of
     "returning None means retain current node, returning Some node
     means replace it " *)
  let rewrite_post_opt f node =
    let f node =
      match f node with
    | None -> Keep
    | Some node -> Replace node in
    rewrite_post f node

  (* As above, but the rewrite happens to the node first and then
     recurses on the children. *)
  let parented_aggregating_rewrite_pre f node init_acc =
    let rec aux parents node acc =
      let mapper (acc, changed) child =
        match aux (node :: parents) child acc with
        | (acc, Replace new_child) -> ((acc, true), Some new_child)
        | (acc, Keep) -> ((acc, changed), Some child)
        | (acc, Remove) -> ((acc, true), None)
      in (* end of mapper *)
      let rewrite_children node_changed node acc =
        let children = Syntax.children node in
        let ((acc, child_changed), option_new_children) =
          Hh_core.List.map_env (acc, false) children ~f:mapper in
        let result =
          if child_changed then
            let new_children = Hh_core.List.filter_opt option_new_children in
            let node = Syntax.from_children (Syntax.kind node) new_children in
            Replace node
          else if node_changed then
            Replace node
          else
            Keep in
        (acc, result)
      in (* end of rewrite_children *)
      let (acc, result) = f parents node acc in
      match result with
      | Remove -> (acc, result)
      | Keep -> rewrite_children false node acc
      | Replace new_node -> rewrite_children true new_node acc
    in (* end of aux *)
    let (acc, result) = aux [] node init_acc in
    let result_node = match result with
      | Keep -> node
      | Replace new_node -> new_node
      | Remove -> failwith "rewriter removed the root node!" in
    (acc, result_node)

    (* The same as the above, except that f does not take parents. *)
    let aggregating_rewrite_pre f node init_acc =
      let f parents node acc = f node acc in
      parented_aggregating_rewrite_pre f node init_acc

    (* The same as the above, except that f does not take or return an
       accumulator. *)
    let parented_rewrite_pre f node =
      let f parents node acc = ([], f parents node) in
      let (acc, result) = parented_aggregating_rewrite_pre f node [] in
      result

    (* The same as the above, except that f does not take or return an
       accumulator, and f does not take parents *)
    let rewrite_pre f node =
      let f parents node acc = ([], f node) in
      let (acc, result) = parented_aggregating_rewrite_pre f node [] in
      result

  (* Here f is a function node -> node opt, with the semantics of
     "returning None means retain current node, returning Some node
     means replace it " *)
  let rewrite_pre_opt f node =
    let f node =
      match f node with
      | None -> Keep
      | Some node -> Replace node in
    rewrite_pre f node

end
