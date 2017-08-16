(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module type TypeParserType = sig
  type t
  val make : Full_fidelity_lexer.t -> Full_fidelity_syntax_error.t list
    -> Full_fidelity_parser_context.t -> t
  val lexer : t -> Full_fidelity_lexer.t
  val errors : t -> Full_fidelity_syntax_error.t list
  val parse_type_specifier : ?allow_var:bool -> t ->
    t * Full_fidelity_minimal_syntax.t
  val parse_return_type : t -> t * Full_fidelity_minimal_syntax.t
  val parse_possible_generic_specifier : t -> t * Full_fidelity_minimal_syntax.t
  val parse_type_constraint_opt : t -> t * Full_fidelity_minimal_syntax.t
  val parse_generic_type_parameter_list: t -> t * Full_fidelity_minimal_syntax.t
  val parse_generic_parameter_list_opt: t -> t * Full_fidelity_minimal_syntax.t
  val parse_generic_type_argument_list_opt: t ->
    t * Full_fidelity_minimal_syntax.t
end
