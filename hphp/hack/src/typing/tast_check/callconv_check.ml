(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Tast
open Typing_defs

module Env = Tast_env

let check_types env ((p, _), te) =
  let rec check_types_helper = function
    | Lvar _ -> ()
    | Array_get (((_, ty1), te1), Some _) ->
      let rec iter ty1 =
        let _, ety1 = Env.expand_type env ty1 in
        match ety1 with
        | _, Tany -> true
        | _, (Tarraykind _ | Ttuple _ | Tshape _) -> true
        | _, Tclass ((_, cn), _, _)
          when cn = SN.Collections.cDict
            || cn = SN.Collections.cKeyset
            || cn = SN.Collections.cVec -> true
        | _, Tunresolved tyl -> List.for_all ~f:iter tyl
        | _, Tabstract _ ->
          let _, tyl = Env.get_concrete_supertypes env ety1 in
          List.exists ~f:iter tyl
        | _ -> false in
      if iter ty1
      then check_types_helper te1
      else
        let ty_str = Env.print_error_ty env ty1 in
        let msgl = Reason.to_string ("This is " ^ ty_str) (fst ty1) in
        Errors.inout_argument_bad_type p msgl
    (* Other invalid expressions are caught in NastCheck. *)
    | _ -> () in
  check_types_helper te

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env = function
    | _, Callconv (_, te) -> check_types env te
    | _ -> ()
end
