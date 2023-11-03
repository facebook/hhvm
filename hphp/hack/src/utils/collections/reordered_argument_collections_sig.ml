(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Avoids warning 66 about unused open Ppx_yojson_conv_lib.Yojson_conv.Primitives *)
let _ = yojson_of_unit

module type Map_S = sig
  include WrappedMap.S

  val add : 'a t -> key:key -> data:'a -> 'a t

  val filter : 'a t -> f:(key -> 'a -> bool) -> 'a t

  val filter_map : 'a t -> f:(key -> 'a -> 'b option) -> 'b t

  val fold : 'a t -> init:'b -> f:(key -> 'a -> 'b -> 'b) -> 'b

  val find_opt : 'a t -> key -> 'a option

  val find : 'a t -> key -> 'a

  val iter : 'a t -> f:(key -> 'a -> unit) -> unit

  val map : 'a t -> f:('a -> 'b) -> 'b t

  val mapi : 'a t -> f:(key -> 'a -> 'b) -> 'b t

  val mem : 'a t -> key -> bool

  val remove : 'a t -> key -> 'a t

  val exists : 'a t -> f:(key -> 'a -> bool) -> bool

  val merge :
    'a t -> 'b t -> f:(key -> 'a option -> 'b option -> 'c option) -> 'c t

  val partition : 'a t -> f:(key -> 'a -> bool) -> 'a t * 'a t
end

module type Set_S = sig
  include Set.S

  val add : t -> elt -> t

  val filter : t -> f:(elt -> bool) -> t

  val fold : t -> init:'a -> f:(elt -> 'a -> 'a) -> 'a

  val iter : t -> f:(elt -> unit) -> unit

  val mem : t -> elt -> bool

  val remove : t -> elt -> t

  val exists : t -> f:(elt -> bool) -> bool

  val of_list : elt list -> t

  val make_pp :
    (Format.formatter -> elt -> unit) -> Format.formatter -> t -> unit

  (** Find an element that satisfies some boolean predicate.
      No other guarantees are given (decidability, ordering, ...).  *)
  val find_one_opt : t -> f:(elt -> bool) -> elt option

  val find_first : (elt -> bool) -> t -> elt
    [@@alert
      deprecated
        "`find_first f` requires f to be monotone which is rarely the case, please consider using first_one_opt instead"]

  val find_first_opt : (elt -> bool) -> t -> elt option
    [@@alert
      deprecated
        "`find_first_opt f` requires f to be monotone which is rarely the case, please consider using first_one_opt instead"]
end

module type SSet_S = sig
  include Set_S

  val pp : Format.formatter -> t -> unit

  val show : t -> string
end

module type SMap_S = sig
  include Map_S

  val pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit

  val show : (Format.formatter -> 'a -> unit) -> 'a t -> string
end
