(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* TODO: investigate this warning 40 and how to fix it correctly *)
[@@@warning "-40"]

open Core

type t =
  | Apply of {
      patt_name: Patt_name.t;
      patt_params: params;
    }
  | Prim of prim
  | Shape of shape_fields
  | Option of t
  | Tuple of t list
  | Dynamic
  | Nonnull
  | Any
  | Or of {
      patt_fst: t;
      patt_snd: t;
    }
  | As of {
      lbl: Patt_var.t;
      patt: t;
    }
  | Invalid of Validation_err.t list * t

and params =
  | Nil
  | Wildcard
  | Cons of {
      patt_hd: t;
      patt_tl: params;
    }
  | Exists of t

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

and shape_fields =
  | Fld of {
      patt_fld: shape_field;
      patt_rest: shape_fields;
    }
  | Open
  | Closed

and shape_field = {
  lbl: shape_label;
  optional: bool;
  patt: t;
}

and shape_label =
  | StrLbl of string
  | IntLbl of string
  | CConstLbl of {
      cls_nm: string;
      cnst_nm: string;
    }
[@@deriving compare, eq, sexp, show]
