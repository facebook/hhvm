(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** DEPRECATED: When TypecheckerOptions.use_direct_decl_parser is enabled, parse
    the source text using the direct decl parser (or ast_and_decl_parser, if the
    AST is also needed) instead of transforming the AST to a decl. *)
val class_DEPRECATED :
  Provider_context.t -> Nast.class_ -> Shallow_decl_defs.shallow_class
