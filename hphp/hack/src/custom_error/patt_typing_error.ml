(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-66"]

type t =
  | Primary of primary
  | Apply of {
      patt_cb: callback;
      patt_err: t;
    }
  | Apply_reasons of {
      patt_rsns_cb: reasons_callback;
      patt_secondary: secondary;
    }
  | Or of {
      patt_fst: t;
      patt_snd: t;
    }
  | Invalid_typing of {
      errs: Validation_err.t list;
      patt: t;
    }

and primary =
  | Any_prim (* Existing wildcard *)
  (* Member not found pattern *)
  | Member_not_found of {
      patt_is_static: static_pattern option; (* Static vs instance pattern *)
      patt_kind: member_kind_pattern; (* Kind of member pattern *)
      patt_class_name: Patt_string.t; (* Class name pattern *)
      patt_member_name: Patt_string.t; (* Member name pattern *)
      patt_visibility: visibility_pattern option; (* Optional visibility *)
    }

and static_pattern =
  | Static_only (* Match only static members *)
  | Instance_only (* Match only instance members *)

and member_kind_pattern =
  | Any_member_kind (* Match any kind *)
  | Method_only (* Methods (static or instance) *)
  | Property_only (* Properties (static or instance) *)
  | Class_constant_only (* Class constants (only static) *)
  | Class_typeconst_only (* Type constants (only static) *)

and visibility_pattern =
  | Any_visibility
  | Public_only
  | Private_only
  | Protected_only
  | Internal_only
[@@deriving eq, show]

and secondary =
  | Of_error of t
  | Violated_constraint of {
      patt_cstr: Patt_string.t;
      patt_ty_sub: Patt_locl_ty.t;
      patt_ty_sup: Patt_locl_ty.t;
    }
  | Subtyping_error of {
      patt_ty_sub: Patt_locl_ty.t;
      patt_ty_sup: Patt_locl_ty.t;
    }
  | Any_snd

and callback = Any_callback

and reasons_callback = Any_reasons_callback [@@deriving eq, show]
