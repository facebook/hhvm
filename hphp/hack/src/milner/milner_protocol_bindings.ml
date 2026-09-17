(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Milner_syntax

type t = {
  definitions: string list;
  operations: (expr -> expr) list;
}

let function_ parameters contexts return_hint expression =
  Lambda (parameters, contexts, return_hint, [Return (Some expression)])

let xhp ~name ~value_hint ~child =
  let value = fresh_local "value" in
  let payload_hint = "MilnerPayload<" ^ value_hint ^ ">" in
  let node_hint = ":" ^ name in
  let node child =
    Xhp (name, [("value", New (payload_hint, [Local value]))], [child])
  in
  let operation body argument =
    Call (function_ [parameter value_hint value] [] value_hint body, [argument])
  in
  let attribute = operation (Member (Member (node child, ":value"), "value")) in
  let children argument =
    let instance = fresh_local "node" in
    Call
      ( Lambda
          ( [parameter value_hint value],
            [],
            value_hint,
            [
              Bind (instance, node child);
              Eval
                (Call
                   ( Atom "invariant",
                     [
                       Binary
                         ( "===",
                           Member (Local instance, "children"),
                           Array ("vec", [child]) );
                       Atom "'XHP children preserve values'";
                     ] ));
              Return
                (Some (Member (Member (Local instance, ":value"), "value")));
            ] ),
        [argument] )
  in
  {
    definitions =
      [
        Format.sprintf
          "final class %s extends MilnerXhpBase { attribute %s value @required; }"
          node_hint
          payload_hint;
      ];
    operations = [attribute; children];
  }

let expression_tree ~value_hint value =
  let tree = fresh_local "tree" in
  let tree_hint = "MilnerTree<" ^ value_hint ^ ">" in
  let rec expression fuel =
    match
      if fuel = 0 then
        0
      else
        Random.int 3
    with
    | 0 ->
      Call (StaticMember ("MilnerDsl", "valueTree<" ^ value_hint ^ ">"), [value])
    | 1 -> Call (StaticMember ("MilnerDsl", "lift"), [expression (fuel - 1)])
    | _ ->
      Call
        ( function_
            [parameter tree_hint tree]
            ["defaults"]
            tree_hint
            (Quote ("MilnerDsl", Splice (Local tree))),
          [expression (fuel - 1)] )
  in
  Call (Member (expression (1 + Random.int 3), "visit"), [New ("MilnerDsl", [])])
