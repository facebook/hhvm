(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t
val empty : t
val expects : t -> Full_fidelity_token_kind.t -> bool
val expects_here : t -> Full_fidelity_token_kind.t -> bool
val expect : t -> Full_fidelity_token_kind.t list -> t
val expect_in_new_scope : t -> Full_fidelity_token_kind.t list -> t
val pop_scope : t -> Full_fidelity_token_kind.t list -> t

val carry_extra : t -> Full_fidelity_minimal_token.t option -> t
val carrying_extra : t -> bool
val flush_extra : t -> t * Full_fidelity_minimal_trivia.t list

val print_expected : t -> unit
