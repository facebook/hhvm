(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module type SyntaxType = sig
  type t
  val children : t -> t list
end

module WithSyntax(Syntax: SyntaxType) = struct

  (* TODO: These could be made more efficient by iterating over the
           structure directly, rather than turning the children
           into a list first. *)

  let rec iter f node =
    f node;
    List.iter (iter f) (Syntax.children node)

  (* This depth-first traversal applies the function to the parent node,
     updating the accumulator, *before* recursing on the children
     left-to-right. *)
  let rec fold f acc node =
    let acc = (f acc node) in
    List.fold_left (fold f) acc (Syntax.children node)

  (* This depth-first traversal applies the function to the parent node,
     updating the accumulator, *after* recursing on the children
     left-to-right. *)
  let rec fold_post f acc node =
    let acc = List.fold_left (fold_post f) acc (Syntax.children node) in
    f acc node

  (* These are versions of the depth-first traversals above which have the
     context of the parents of the node being processed. *)
  let parented_fold_pre f acc node =
    let rec aux acc node parents =
      let rec do_children acc children =
        match children with
        | [] -> acc
        | h :: t ->
          let acc = aux acc h (node :: parents) in
          do_children acc t in
      let acc = f acc node parents in
      let acc = do_children acc (Syntax.children node) in
      acc in
    aux acc node []

  let parented_fold_post f acc node =
    let rec aux acc node parents =
      let rec do_children acc children =
        match children with
        | [] -> acc
        | h :: t ->
          let acc = aux acc h (node :: parents) in
          do_children acc t in
      let acc = do_children acc (Syntax.children node) in
      let acc = f acc node parents in
      acc in
    aux acc node []

end
