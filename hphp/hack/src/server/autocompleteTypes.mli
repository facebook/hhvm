(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* Details about functions to be added in json output *)
type func_param_result = {
    param_name     : string;
    param_ty       : string;
    param_variadic : bool;
  }

type func_details_result = {
    params    : func_param_result list;
    return_ty : string;
    min_arity : int;
  }

type autocomplete_kind =
  | Abstract_class_kind
  | Class_kind
  | Class_constant_kind
  | Constructor_kind
  | Enum_kind
  | Function_kind
  | Interface_kind
  | Keyword_kind
  | Method_kind
  | Namespace_kind
  | Property_kind
  | Trait_kind
  | Variable_kind

(* Results ready to be displayed to the user *)
type complete_autocomplete_result = {
    res_pos      : Pos.absolute;
    res_ty       : string;
    res_name     : string;
    res_kind     : autocomplete_kind;
    func_details : func_details_result option;
  }

(* Results that still need a typing environment to convert ty information
   into strings *)
type partial_autocomplete_result = {
    ty   : Typing_defs.phase_ty;
    name : string;
    kind_: autocomplete_kind;
  }

type autocomplete_result =
  | Partial of partial_autocomplete_result
  | Complete of complete_autocomplete_result

(* The type returned to the client *)
type ide_result = {
  completions : complete_autocomplete_result list;
  char_at_pos : char;
  is_complete : bool;
}

type result = complete_autocomplete_result list
