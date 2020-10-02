(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Used to type Spliced Expressions within Expression Trees
 *   Assumes that Expression Trees cannot be nested.
 *     i.e. cannot have ET(Splice(ET(...)))
 * Returns
 * - A list of spliced expression and ids in execution order
 * - The original tree with Spliced Expression replaced with Ids
 *)
val extract_spliced_expressions :
  Nast.expr -> (int * Nast.expr) list * Nast.expr
