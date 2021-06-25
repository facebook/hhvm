(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env
module SN = Naming_special_names

let check_types env ((p, _), te) =
  let rec check_types_helper = function
    | Lvar _ -> ()
    | Array_get (((_, ty1), te1), Some _) ->
      let rec iter ty1 =
        let (_, ety1) = Env.expand_type env ty1 in
        match get_node ety1 with
        | Tany _
        | Terr ->
          true
        | Tvarray _
        | Tdarray _
        | Tvarray_or_darray _
        | Tvec_or_dict _
        | Ttuple _
        | Tshape _ ->
          true
        | Tclass ((_, cn), _, _)
          when String.equal cn SN.Collections.cDict
               || String.equal cn SN.Collections.cKeyset
               || String.equal cn SN.Collections.cVec ->
          true
        | Tunion tyl -> List.for_all ~f:iter tyl
        | Tgeneric _
        | Tnewtype _
        | Tdependent _ ->
          let (_, tyl) =
            Env.get_concrete_supertypes ~abstract_enum:true env ety1
          in
          List.exists ~f:iter tyl
        | _ -> false
      in
      if iter ty1 then
        check_types_helper te1
      else
        let ty_str = Env.print_error_ty env ty1 in
        let msgl = Reason.to_string ("This is " ^ ty_str) (get_reason ty1) in
        Errors.inout_argument_bad_type p msgl
    (* Other invalid expressions are caught in Nast_check. *)
    | _ -> ()
  in
  check_types_helper te

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, Callconv (_, te)) -> check_types env te
      | _ -> ()
  end
