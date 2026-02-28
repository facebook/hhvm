(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*****************************************************************************)
(* Module types defined in the .mli                                          *)
(*****************************************************************************)

module type S = sig
  type t [@@deriving show, eq, ord]

  val of_string : string -> t

  val to_string_TRANSITIONAL : t -> string

  val of_string_TRANSITIONAL : string -> t
end

module type I = sig
  type t [@@deriving show, ord]

  type s

  val iequal : t -> t -> bool

  val canonical : t -> s

  val of_string : string -> t

  val to_string_TRANSITIONAL : t -> string

  val of_string_TRANSITIONAL : string -> t
end

module type Set = sig
  include Stdlib.Set.S

  val pp : Format.formatter -> t -> unit

  val show : t -> string

  val to_sset_TRANSITIONAL : t -> SSet.t

  val of_sset_TRANSITIONAL : SSet.t -> t
end

(*****************************************************************************)
(* All our opaque types are currently implemented just by strings.           *)
(* Here are the string-based implementations of each of them.                *)
(*****************************************************************************)

(** [S_Impl] implements an opaque type that supports eq+ord, by string *)
module S_Impl = struct
  type t = string [@@deriving ord, eq]

  (** This show method just prints the string; the default deriving-show doubles up backslashes which I don't prefer *)
  let show (t : t) : string = t

  let pp (fmt : Format.formatter) (t : t) : unit = String.pp fmt t

  let of_string (t : string) : t = t

  let to_string_TRANSITIONAL (t : t) : string = t

  let of_string_TRANSITIONAL (t : string) : t = t
end

(** [I_Impl] implements an opaque type that supports eq+ord+ieq, by string *)
module I_Impl = struct
  type t = string [@@deriving ord]

  type s = string

  (** This show method just prints the string; the default deriving-show doubles up backslashes which I don't prefer *)
  let show (t : t) : string = t

  let pp (fmt : Format.formatter) (t : t) : unit = String.pp fmt t

  let iequal (t1 : t) (t2 : t) : bool = String.equal t1 t2

  let canonical (t : t) : s = t

  let of_string (t : string) : t = Stdlib.String.lowercase_ascii t

  let to_string_TRANSITIONAL (t : t) : string = t

  let of_string_TRANSITIONAL (t : string) : t = t
end

(** [Set_Impl] implements an opaque set, via a SSet *)
module Set_Impl = struct
  include Stdlib.Set.Make (StringKey)

  let pp (fmt : Format.formatter) (t : t) =
    Format.fprintf fmt "@[<2>{";
    ignore
    @@ List.fold_left
         ~f:(fun sep s ->
           if sep then Format.fprintf fmt ";@ ";
           String.pp fmt s;
           true)
         ~init:false
         (elements t);
    Format.fprintf fmt "@,}@]"

  let show (x : t) : string = Format.asprintf "%a" pp x

  let to_sset_TRANSITIONAL (t : t) : SSet.t = t

  let of_sset_TRANSITIONAL (ss : SSet.t) : t = ss
end

(*****************************************************************************)
(* All our opaque types are currently implemented just by strings.           *)
(* Here's where we provide their implementations.                            *)
(*****************************************************************************)

module Fun = S_Impl
module IFun = I_Impl
module Type = S_Impl
module IType = I_Impl
module Const = S_Impl
module IConst = I_Impl
module FunSet = Set_Impl
module TypeSet = Set_Impl
module ConstSet = Set_Impl
