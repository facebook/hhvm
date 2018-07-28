(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type type_specifier =
  | TSsimple of string
  | TSoption of type_specifier

type query_type =
  | QTtype of type_specifier
  | QTwildcard

type signature_query = {
  function_params : query_type list;
  function_output : query_type;
}

val parse_query : string -> signature_query option
