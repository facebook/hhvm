(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Result_set : Set.S with type elt = Relative_path.t SymbolOccurrence.t

val clean_member_name : string -> string
val all_symbols : Tast.program -> Result_set.elt list
val go : Tast.program -> int -> int -> Result_set.elt list
