(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types

module Tag : sig
  include module type of Typing_tag_defs

  type ctx = Typing_env_types.env

  val generics_for_class_and_tyl : env -> string -> locl_ty list -> generic list

  val generics_for_class_and_tag_generic_l :
    env -> string -> Typing_defs_core.type_tag_generic list -> generic list

  val describe : env -> t -> string

  val relation : t -> ctx:ctx -> t -> SetRelation.t
end

(** Tracks the reason why a particular tag is assumed to be a part of the
    data type of a type. Tracking is optional, depending on if an origin
    type is provided or not.
*)
module DataTypeReason : sig
  type subreason =
    | NoSubreason
    | Shapes
    | Tuples
    | Nullable
    | Nums
    | Arraykeys
    | SpecialInterface of string
    | Cyclic

  type trail_kind =
    | VariantOfCaseType of string
    | GenericUpperbound of string
    | UpperboundOfNewType of string
    | UpperboundOfEnum of string
    | SealedInterface of string
    | Requirement of string
    | ExpansionOfTypeConstant of {
        root_ty: locl_ty;
        name: string;
      }

  type trail = {
    instances: (trail_kind * Reason.t) list;
    expansions: Type_expansions.t;
  }

  type t = subreason * trail

  val case_type : trail:trail -> Reason.t -> string -> (trail, trail) result

  val generic : trail:trail -> Reason.t -> string -> (trail, trail) result

  val newtype : trail:trail -> Reason.t -> string -> (trail, trail) result

  val enum : trail:trail -> Reason.t -> string -> (trail, trail) result

  val type_constant :
    trail:trail -> Reason.t -> locl_ty -> string -> (trail, trail) result

  val sealed_interface :
    trail:trail -> Reason.t -> string -> (trail, trail) result

  val requirement : trail:trail -> Reason.t -> string -> (trail, trail) result

  val make : subreason -> trail -> t

  val make_trail : trail
end

module TagWithReason : sig
  type ctx = Tag.ctx

  type t = {
    subreason: DataTypeReason.subreason;
    trail: DataTypeReason.trail;
    tag: Tag.t;
  }

  val relation : t -> ctx:ctx -> t -> SetRelation.t

  val make : DataTypeReason.t -> Tag.t -> t

  val to_message :
    env -> f:(string -> string) -> decl_ty * t -> (Pos_or_decl.t * string) list
end

module type SET = sig
  type t

  val empty : t

  val singleton : reason:DataTypeReason.t -> Typing_tag_defs.t -> t

  val of_list : reason:DataTypeReason.t -> Typing_tag_defs.t list -> t

  val union : t -> t -> t

  val inter : t -> t -> t

  val diff : t -> t -> t

  val are_disjoint : Typing_env_types.env -> t -> t -> bool
end

module ApproxTagSet : sig
  include SET

  type disjoint =
    | Sat
    | Unsat of {
        left: TagWithReason.t;
        relation: SetRelation.t;
        right: TagWithReason.t;
      }

  val disjoint : TagWithReason.ctx -> t -> t -> disjoint
end

module Make (S : SET) : sig
  type t = S.t

  val fromHint :
    safe_for_are_disjoint:bool -> env -> Aast.hint -> env * (decl_ty * t)

  val fromTy : safe_for_are_disjoint:bool -> env -> locl_ty -> env * t

  val fun_to_datatypes : trail:DataTypeReason.trail -> t

  val nonnull_to_datatypes : trail:DataTypeReason.trail -> t

  val tuple_to_datatypes : trail:DataTypeReason.trail -> t

  val shape_to_datatypes : trail:DataTypeReason.trail -> t

  val label_to_datatypes : trail:DataTypeReason.trail -> t

  val prim_to_datatypes : trail:DataTypeReason.trail -> Ast_defs.tprim -> t

  val mixed : reason:DataTypeReason.subreason * DataTypeReason.trail -> S.t

  module Class : sig
    val to_datatypes :
      safe_for_are_disjoint:bool ->
      trail:DataTypeReason.trail ->
      env ->
      string ->
      Tag.generic list ->
      env * t
  end
end

module DataType : sig
  module Set = ApproxTagSet

  type t = ApproxTagSet.t

  val fromHint :
    safe_for_are_disjoint:bool -> env -> Aast.hint -> env * (decl_ty * t)

  val fromTy : safe_for_are_disjoint:bool -> env -> locl_ty -> env * t

  val fun_to_datatypes : trail:DataTypeReason.trail -> t

  val nonnull_to_datatypes : trail:DataTypeReason.trail -> t

  val tuple_to_datatypes : trail:DataTypeReason.trail -> t

  val shape_to_datatypes : trail:DataTypeReason.trail -> t

  val label_to_datatypes : trail:DataTypeReason.trail -> t

  val prim_to_datatypes : trail:DataTypeReason.trail -> Ast_defs.tprim -> t

  val mixed : reason:DataTypeReason.subreason * DataTypeReason.trail -> Set.t

  module Class : sig
    val to_datatypes :
      safe_for_are_disjoint:bool ->
      trail:DataTypeReason.trail ->
      env ->
      string ->
      Tag.generic list ->
      env * t
  end
end
