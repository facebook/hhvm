(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
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
  | Invalid of {
      errs: Validation_err.t list;
      patt: t;
    }

and primary = Any_prim

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

and reasons_callback = Any_reasons_callback [@@deriving show, yojson]

include Can_validate.S with type t := t
