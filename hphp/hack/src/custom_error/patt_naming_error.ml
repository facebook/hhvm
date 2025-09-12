(* Copyright (c) Meta Platforms, Inc. and affiliates.
   *
   * This source code is licensed under the MIT license found in the
   * LICENSE file in the "hack" directory of this source tree.
   *
*)
[@@@warning "-66"]

type patt_name_context =
  | Any_name_context
  | One_of_name_context of Name_context.t list
[@@deriving eq, show]

type t =
  | Unbound_name of {
      patt_name_context: patt_name_context;
      patt_name: Patt_name.t;
    }
  | Invalid_naming of {
      errs: Validation_err.t list;
      patt: t;
    }
[@@deriving eq, show]
