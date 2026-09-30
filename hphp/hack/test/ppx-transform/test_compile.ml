(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Compilation test: verifies that ppx_transform generates valid OCaml for
    various type patterns. If this file compiles, the test passes. *)

module Variant : sig
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform]
end = struct
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform]
end

module Gadt : sig
  type _ t =
    | T : bool t
    | F : bool t
    | Num : int -> int t
    | Plus : (int t * int t) -> int t
    | Leq : (int t * int t) -> bool t
    | Cond : (bool t * 'a t * 'a t) -> 'a t
  [@@deriving transform]
end = struct
  type _ t =
    | T : bool t
    | F : bool t
    | Num : int -> int t
    | Plus : (int t * int t) -> int t
    | Leq : (int t * int t) -> bool t
    | Cond : (bool t * 'a t * 'a t) -> 'a t
  [@@deriving transform]
end

module Nonregular : sig
  type 'a term =
    | Var of 'a
    | App of 'a term * 'a term
    | Abs of 'a bind term

  and 'a bind =
    | Zero
    | Succ of 'a
  [@@deriving transform]
end = struct
  type 'a term =
    | Var of 'a
    | App of 'a term * 'a term
    | Abs of 'a bind term

  and 'a bind =
    | Zero
    | Succ of 'a
  [@@deriving transform]
end

(** Non-regular type where a type parameter is instantiated with a concrete
    type rather than a different type variable. This exercises the
    is_nonregular_in_core_ty detection for non-variable substitutions. *)
module Nonregular_concrete : sig
  type 'a t =
    | Leaf
    | Node of int t
  [@@deriving transform]
end = struct
  type 'a t =
    | Leaf
    | Node of int t
  [@@deriving transform]
end

module Nonregular_mutual : sig
  type 'a one =
    | Nil
    | Two of 'a two

  and 'a two = MaybeOne of 'a option one [@@deriving transform]
end = struct
  type 'a one =
    | Nil
    | Two of 'a two

  and 'a two = MaybeOne of 'a option one [@@deriving transform]
end

(** A mutually recursive group mixing a GADT with a regular type. [plain] alone
    would need no quantifier, but sharing a [let rec ... and ...] group with
    [gadt] forces one: the rigid variable introduced by [gadt]'s match cannot
    unify with a monomorphic recursion variable. This compiles only if the
    quantifier is propagated across the whole group. *)
module Mixed_gadt_regular_mutual : sig
  type 'a gadt =
    | Lit : bool -> bool gadt
    | Wrap : 'a plain -> 'a gadt

  and 'a plain = Plain of 'a gadt option [@@deriving transform]
end = struct
  type 'a gadt =
    | Lit : bool -> bool gadt
    | Wrap : 'a plain -> 'a gadt

  and 'a plain = Plain of 'a gadt option [@@deriving transform]
end

module Builtins : sig
  type 'a t = {
    prim_ignored: char;
    ref: 'a t ref;
    opt: 'a t option;
    res: ('a t, 'a t) result;
    list: 'a t list;
    array: 'a t array;
    lazy_: 'a t lazy_t;
    nested: ('a t option list, 'a t array option) result list option lazy_t;
  }
  [@@deriving transform]
end = struct
  type 'a t = {
    prim_ignored: char;
    ref: 'a t ref;
    opt: 'a t option;
    res: ('a t, 'a t) result;
    list: 'a t list;
    array: 'a t array;
    lazy_: 'a t lazy_t;
    nested: ('a t option list, 'a t array option) result list option lazy_t;
  }
  [@@deriving transform]
end
