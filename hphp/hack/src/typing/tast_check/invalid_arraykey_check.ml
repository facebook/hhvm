(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Nast
open Tast
open Typing_defs

module Env = Tast_env
module TCO = TypecheckerOptions
module Subtype = Typing_subtype

let should_enforce env =
  TCO.disallow_invalid_arraykey (Env.get_tcopt env)

let info_of_type (reason, typ): Pos.t * string =
  (Reason.to_pos reason, Typing_print.error typ)

let is_valid_arraykey env tcontainer tkey =
  let is_vector_container env e =
    List.exists
      [SN.Collections.cVector;
       SN.Collections.cImmVector;
       SN.Collections.cVec;
       SN.Collections.cConstVector]
      ~f:begin fun cls ->
        Env.can_subtype env e
          (Reason.Rnone,
           Tclass ((Pos.none, cls), Nonexact, [Reason.Rnone, Tany]))
      end
      in
  is_vector_container env tcontainer ||
  Env.can_subtype env tkey (Reason.Rnone, Tprim Tarraykey)

let handler = object
  inherit Tast_visitor.handler_base

  method! minimum_forward_compat_level = 2018_10_31

  method! at_expr env ((p, _), expr) =
    match expr with
    | Array_get (((_, tcontainer), _), Some ((_, tkey), _))
      when should_enforce env &&
        not (is_valid_arraykey env tcontainer tkey) ->
      Errors.invalid_arraykey p (info_of_type tcontainer) (info_of_type tkey)
    | _ -> ()
end
