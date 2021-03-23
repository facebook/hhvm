(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Ad-hoc rules for typing some common idioms

    For printf-style functions: If the last argument (before varargs) of a
    function has the type FormatString<X>, we assume that the format
    string is interpreted by the formatter X, which should look like this:

    interface PrintfFormatter {
      function format_0x25() : string;
      function format_x(int) : string;
      function format_f(float) : string;
      function format_upcase_l() : PrintfFormatter;
    }

    Each method can return a string or another formatter (for
    multi-character sequences like %Ld); the parameters are copied into
    the function signature when instantiating it at the call site. *)

val retype_magic_func :
  Typing_env_types.env ->
  Typing_defs.locl_fun_type ->
  Nast.expr list ->
  Typing_env_types.env * Typing_defs.locl_fun_type
