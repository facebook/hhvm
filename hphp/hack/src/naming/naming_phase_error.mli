(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val empty : t

val suppress_like_type_errors : t -> t

val emit : t -> unit

module Free_monoid : sig
  type 'a t

  val zero : 'a t

  val plus : 'a t -> 'a t -> 'a t

  class virtual ['a] monoid :
    object
      method private plus : 'a t -> 'a t -> 'a t

      method private zero : 'a t
    end
end

type err

val naming : Naming_error.t -> err Free_monoid.t

val nast_check : Nast_check_error.t -> err Free_monoid.t

val typing : Typing_error.Primary.t -> err Free_monoid.t

val like_type : Pos.t -> err Free_monoid.t

val unexpected_hint : Pos.t -> err Free_monoid.t

val malformed_access : Pos.t -> err Free_monoid.t

val supportdyn : Pos.t -> err Free_monoid.t

class monoid : [err] Free_monoid.monoid

val from_monoid : ?init:t -> err Free_monoid.t -> t

val invalid_expr_ : Pos.t -> (unit, unit) Aast.expr_
