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

type ty_string = Ty of string

type var_tys = ty_string String.Map.t

type selection_kind =
  | SelectionKindExpression of ty_string
  | SelectionKindStatement

let ty_string_of_ty env locl_ty =
  if Typing_defs.is_denotable locl_ty then
    let env = Tast_env.tast_env_as_typing_env env in
    Ty (Typing_print.full env locl_ty)
  else
    Ty "_"

type candidate = {
  pos: Pos.t;
  placeholder_name: string;
  selection_kind: selection_kind;
  params: var_tys;
  return: var_tys;
  is_static: bool;
  iterator_kind: iterator_kind option;
  is_async: bool;
  method_pos: Pos.t;
}
