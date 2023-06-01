(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Defines pattern matches over a subset of Hack types *)
type t =
  | Apply of {
      patt_name: Patt_name.t;
      patt_params: params;
    }
      (** Matches type-constructor like types e.g. class, vec_or_dict and newtypes *)
  | Prim of prim  (** Matches Hack primitives  *)
  | Shape of shape_fields  (** Matches Hack shapes *)
  | Option of t  (** Matches Hack optional types *)
  | Tuple of t list  (** Matches Hack tuples *)
  | Dynamic  (** Matches dynamic *)
  | Nonnull  (** Matches non-null *)
  | Any  (** Matches any Hack type *)
  | Or of {
      patt_fst: t;
      patt_snd: t;
    }
      (** Matches either the first pattern or, if that does not match, the second  *)
  | As of {
      lbl: Patt_var.t;
      patt: t;
    }
      (** Match the provided pattern and bind the result to the supplied variable name *)

(** Defines pattern matches over list of Hack types appearing as type parameters *)
and params =
  | Nil  (** Matches the empty parameter list *)
  | Wildcard  (** Matches any paramter list *)
  | Cons of {
      patt_hd: t;
      patt_tl: params;
    }
      (** Mathes a parameter list where the first element matches [patt_hd] and
      the remaining parameters match [patt_tl]  *)
  | Exists of t
      (** Matches a parameter list where at least one Hint matches the supplied
      pattern  *)

(** Match Hack primitive types  *)
and prim =
  | Null
  | Void
  | Int
  | Bool
  | Float
  | String
  | Resource
  | Num
  | Arraykey
  | Noreturn

(** Matches the fields of a Hack shape  *)
and shape_fields =
  | Fld of {
      patt_fld: shape_field;
      patt_rest: shape_fields;
    }
      (** Matches a shape which contains the a field matching the given [shape_field]
      and matches the remaining [shape_fields] *)
  | Open
      (** Matches any shape which contains fields which have not already been matched  *)
  | Closed
      (** Matches a shape in which all other fields have already been matched  *)

(** Matches an individual Hack shape field *)
and shape_field = {
  lbl: shape_label;
  optional: bool;
  patt: t;
}

(** Matches a Hack shape label *)
and shape_label =
  | StrLbl of string
  | IntLbl of string
  | CConstLbl of {
      cls_nm: string;
      cnst_nm: string;
    }
[@@deriving show, yojson]
