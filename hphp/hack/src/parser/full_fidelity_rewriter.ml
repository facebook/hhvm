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

  (* The rewriter takes a function f that is [node] -> node -> (node, bool),
     where the bool indicates whether something changed. The rewriter
     itself returns (node, bool).  The list of nodes passed in to f
     is the parent chain of the node. First the children are rewritten,
     then f is called on the rewritten node. *)

  let parented_rewrite_post f node =
    let rec aux parents node =
      let mapper changed child = match aux (node :: parents) child with
        | Some (child, changed') -> changed || changed', Some child
        | None -> true, None
      in
      let (changed, option_new_children) =
        Core.List.map_env false (Syntax.children node) ~f:mapper in
      let new_children = Core.List.filter_opt option_new_children in
      let node = if changed then
          Syntax.from_children (Syntax.kind node) new_children
        else
          node
      in
      Option.map (f parents node) ~f:(fun (n, c) -> n, c || changed) in
    Option.value ~default:(node, false) (aux [] node)

  (* As above, but the rewrite happens to the node first and then
     recurses on the children. *)

  let parented_rewrite_pre f node =
    let rec aux parents node =
      let mapper changed child = match aux (node :: parents) child with
        | Some (child, changed') -> changed || changed', Some child
        | None -> true, None
      in
      Option.map (f parents node) ~f:begin fun (new_node, node_changed) ->
        let cs = Syntax.children new_node in
        let (child_changed, option_new_children) =
          Core.List.map_env false cs ~f:mapper in
        let new_children = Core.List.filter_opt option_new_children in
        let with_new_children =
          if child_changed then
            Syntax.from_children (Syntax.kind new_node) new_children
          else
            new_node in
        (with_new_children, node_changed || child_changed)
      end in
    Option.value ~default:(node, false) (aux [] node)

  (* These are simpler versions of the rewriter, for cases where the caller
     does not need the parent context.
     The rewriter takes a function f that is node -> (node, bool), where
     the bool indicates whether something changed.  *)

  let rewrite_post f node =
    let g parents node = f node in
    parented_rewrite_post g node

  let rewrite_pre f node =
    let g parents node = f node in
    parented_rewrite_pre g node

end
