(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Syntax = Full_fidelity_editable_positioned_syntax

open Syntax

(**
 * Return a token similar to the given one, but with its text changed into
 * the given string.
 *)
val rename_token : string -> Syntax.t -> Syntax.t

(**
 * Return a function declaration heder similar to the given one, but with its
 * name changed into the given string.
 *)
val rename_function_declaration_header : string -> Syntax.t -> Syntax.t

(**
 * Return a function declaration similar to the given one, but with its name
 * changed into the given string.
 *)
val rename_function_declaration : string -> Syntax.t -> Syntax.t

(**
 * Return the name of the given function declaration syntax.
 *)
val extract_name_from_function_declaration : Syntax.t -> string
