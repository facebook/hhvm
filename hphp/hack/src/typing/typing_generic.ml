(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Typing_env

(* Module checking if a type is generic, I like to use an exception for this sort
 * of things, the code is more readable (subjective :-), and the exception never
 * escapes anyway.
 *)
module IsGeneric : sig
  (* Give back the position and name of a generic type parameter if found *)
  val ty : decl_ty -> (Pos.t * string) option
end = struct
  exception Found of Pos.t * string

  let ty x =
    let rec ty t =
      match get_node t with
      | Tgeneric x -> raise (Found (get_pos t, x))
      | Tthis -> ()
      | Tmixed -> ()
      | Tdynamic
      | Tany _
      | Terr
      | Tnonnull
      | Tprim _ ->
        ()
      | Tvar _ -> ()
      | Toption x -> ty x
      | Tlike x -> ty x
      | Tfun fty ->
        List.iter (List.map fty.ft_params (fun x -> x.fp_type.et_type)) ty;
        ty fty.ft_ret.et_type;
        (match fty.ft_arity with
        | Fvariadic (_min, { fp_type = var_ty; _ }) -> ty var_ty.et_type
        | _ -> ())
      | Ttuple tyl
      | Tunion tyl
      | Tintersection tyl ->
        List.iter tyl ty
      | Tapply (_, tyl) -> List.iter tyl ty
      | Taccess (t, _) -> ty t
      | Tarray (t1, t2) ->
        Option.iter ~f:ty t1;
        Option.iter ~f:ty t2
      | Tdarray (t1, t2) ->
        ty t1;
        ty t2
      | Tvarray t -> ty t
      | Tvarray_or_darray (t1_opt, t2) ->
        Option.iter ~f:ty t1_opt;
        ty t2
      | Tshape (_, fdm) -> ShapeFieldMap.iter (fun _ v -> ty v) fdm
      | Tpu_access (base, _, _) -> ty base
    in
    try
      ty x;
      None
    with Found (p, x) -> Some (p, x)
end
