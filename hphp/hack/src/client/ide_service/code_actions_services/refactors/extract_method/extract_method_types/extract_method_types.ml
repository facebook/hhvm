(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type iterator_kind =
  | Iterator
  | KeyedIterator

type var_tys = Code_action_types.Type_string.t String.Map.t

type selection_kind =
  | SelectionKindExpression of Code_action_types.Type_string.t
  | SelectionKindStatement

type candidate = {
  pos: Pos.t;  (** Position for selected statement(s) or expression *)
  placeholder_name: string;
  selection_kind: selection_kind;
  params: var_tys;
  return: var_tys;
  iterator_kind: iterator_kind option;
  is_async: bool;
  method_pos: Pos.t;
  method_is_static: bool;
}
