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
  expressions: (string * expr) list;
}

let function_ parameters contexts return_hint expression =
  Lambda (parameters, contexts, return_hint, [Return (Some expression)])

let xhp ~name ~value_hint ~child =
  let value = fresh_local "value" in
  let payload = fresh_local "payload" in
  let node = fresh_local "node" in
  let child_parameter = fresh_local "child" in
  let payload_hint = "MilnerPayload<" ^ value_hint ^ ">" in
  let node_hint = ":" ^ name in
  {
    definitions =
      [
        Format.sprintf
          "final class %s extends MilnerXhpBase { attribute %s value @required; }"
          node_hint
          payload_hint;
      ];
    expressions =
      [
        ( "xhp_box",
          function_
            [parameter value_hint value]
            []
            payload_hint
            (New (payload_hint, [Local value])) );
        ( "xhp_make",
          function_
            [parameter payload_hint payload; parameter "string" child_parameter]
            []
            node_hint
            (Xhp (name, [("value", Local payload)], [Local child_parameter])) );
        ( "xhp_attribute",
          function_
            [parameter node_hint node]
            []
            payload_hint
            (Member (Local node, ":value")) );
        ( "xhp_children",
          function_
            [parameter node_hint node]
            []
            "varray<mixed>"
            (Member (Local node, "children")) );
        ("xhp_child", child);
      ];
  }

let expression_tree ~value_hint =
  let value = fresh_local "value" in
  let tree = fresh_local "tree" in
  let tree_hint = "MilnerTree<" ^ value_hint ^ ">" in
  {
    definitions = [];
    expressions =
      [
        ( "tree_value",
          function_
            [parameter value_hint value]
            []
            tree_hint
            (Call
               ( StaticMember ("MilnerDsl", "valueTree<" ^ value_hint ^ ">"),
                 [Local value] )) );
        ( "tree_splice",
          function_
            [parameter tree_hint tree]
            ["defaults"]
            tree_hint
            (Quote ("MilnerDsl", Splice (Local tree))) );
        ( "tree_lift",
          function_
            [parameter tree_hint tree]
            []
            tree_hint
            (Call (StaticMember ("MilnerDsl", "lift"), [Local tree])) );
        ( "tree_visit",
          function_
            [parameter tree_hint tree]
            ["defaults"]
            "mixed"
            (Call (Member (Local tree, "visit"), [New ("MilnerDsl", [])])) );
      ];
  }
