(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(** Takes a docblock with asterisks and leading/ending slashes removed.
    Returns the parameters mentioned in the docblock (with @param) and their
    descriptions with newlines removed.
    Parameters can be mentioned with the leading '$' or not: they will be
    indexed in the map with the '$' regardless.
 *)
val get_param_docs : docblock:string -> string String.Map.t
