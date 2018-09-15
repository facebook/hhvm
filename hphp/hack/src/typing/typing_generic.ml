(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module ExprDepTy = Typing_dependent_type
module Env = Typing_env
module ShapeMap = Nast.ShapeMap


(* Module checking if a type is generic, I like to use an exception for this sort
 * of things, the code is more readable (subjective :-), and the exception never
 * escapes anyway.
*)
module IsGeneric: sig

  (* Give back the position and name of a generic type parameter if found *)
  val ty: 'phase ty -> (Pos.t * string) option
end = struct

  exception Found of Reason.t * string

  let rec ty : type a. a ty -> _ = function (r, t) ->
    match t with
    | Tabstract ((AKdependent (_, _) | AKenum _), cstr) -> ty_opt cstr
    | Tabstract (AKgeneric x, cstr) when AbstractKind.is_generic_dep_ty x ->
      ty_opt cstr
    | Tabstract (AKgeneric x, _) -> raise (Found (r, x))
    | Tgeneric x -> raise (Found (r, x))
    | Tanon _ -> ()
    | Tthis -> ()
    | Tdynamic | Tany | Terr | Tmixed | Tnonnull | Tprim _ -> ()
    | Tarraykind akind ->
      begin match akind with
        | AKany -> ()
        | AKempty -> ()
        | AKvarray_or_darray tv
        | AKvarray tv
        | AKvec tv -> ty tv
        | AKdarray (tk, tv)
        | AKmap (tk, tv) -> ty tk; ty tv
        | AKshape fdm ->
            ShapeMap.iter (fun _ (tk, tv) -> ty tk; ty tv) fdm
        | AKtuple fields ->
            IMap.iter (fun _ tv -> ty tv) fields
      end
    | Tvar _ -> assert false (* Expansion got rid of Tvars ... *)
    | Toption x -> ty x
    | Tfun fty ->
        List.iter (List.map fty.ft_params (fun x -> x.fp_type)) ty;
        ty fty.ft_ret;
        (match fty.ft_arity with
          | Fvariadic (_min, { fp_type = var_ty; _ }) -> ty var_ty
          | _ -> ())
    | Tabstract (AKnewtype (_, tyl), x) ->
        List.iter tyl ty; ty_opt x
    | Ttuple tyl -> List.iter tyl ty
    | Tclass (_, tyl) -> List.iter tyl ty
    | Tunresolved tyl -> List.iter tyl ty
    | Tobject -> ()
    | Tapply (_, tyl) -> List.iter tyl ty
    | Taccess (t, _) -> ty t
    | Tarray (t1, t2) -> ty_opt t1; ty_opt t2
    | Tdarray (t1, t2) -> ty t1; ty t2
    | Tvarray t -> ty t
    | Tvarray_or_darray t -> ty t
    | Tshape (_, fdm) ->
        ShapeFieldMap.iter (fun _ v -> ty v) fdm

  and ty_opt : type a . a ty option -> _
    = function None -> () | Some x -> ty x

  let ty : type a . a ty -> _ =
    function x -> try ty x; None with Found (r, x) -> Some (Reason.to_pos r, x)

end

(* Function making sure that a type can be generalized, in our case it just
 * means the type should be monomorphic
*)
let no_generic p local_var_id env =
  let ty = Env.get_local env local_var_id in
  let ty = Typing_expand.fully_expand env ty in
  match IsGeneric.ty ty with
  | None -> env, false
  | Some (_, x) ->
      Errors.generic_static p x;
      env, true
