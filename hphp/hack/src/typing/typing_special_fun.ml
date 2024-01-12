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
module SN = Naming_special_names
module MakeType = Typing_make_type

let update_param : decl_fun_param -> decl_ty -> decl_fun_param =
 (fun param ty -> { param with fp_type = ty })

let wrap_with_like_type (pessimise : bool) (ty : decl_ty) =
  if pessimise then
    MakeType.like (Reason.Renforceable (get_pos ty)) ty
  else
    ty

(** Transform the special function `idx` according to the number
    of arguments actually passed to the function

    The idx function has two signatures, depending on number of arguments
    actually passed:

    idx<Tk as arraykey, Tv>(?KeyedContainer<Tk, Tv> $collection, ?Tk $index): ?Tv
    idx<Tk as arraykey, Tv>(?KeyedContainer<Tk, Tv> $collection, ?Tk $index, Tv $default): Tv

    In the hhi file, it has signature

    function idx<Tk as arraykey, Tv>
      (?KeyedContainer<Tk, Tv> $collection, ?Tk $index, $default = null)

    so this needs to be munged into the above. *)
let transform_idx_fun_ty :
    pessimise:bool -> decl_fun_type -> int -> decl_fun_type =
 fun ~pessimise fty nargs ->
  let (param1, param2, param3) =
    match fty.ft_params with
    | [param1; param2; param3] -> (param1, param2, param3)
    | _ -> failwith "Expected 3 parameters for idx in hhi file"
  in
  let rret = get_reason fty.ft_ret in
  let (params, ret) =
    match nargs with
    | 2 ->
      (* Return type should be ?Tv *)
      let ret = MakeType.nullable rret (MakeType.generic rret "Tv") in
      let ret = wrap_with_like_type pessimise ret in
      ([param1; param2], ret)
    | 3 ->
      (* Third parameter should have type Tv *)
      let param3 =
        let r3 = get_reason param1.fp_type in
        update_param param3 (MakeType.generic r3 "Tv")
      in
      (* Return type should be Tv *)
      let ret = MakeType.generic rret "Tv" in
      let ret = wrap_with_like_type pessimise ret in
      ([param1; param2; param3], ret)
    (* Shouldn't happen! *)
    | _ -> (fty.ft_params, fty.ft_ret)
  in
  { fty with ft_params = params; ft_ret = ret }

(** Transform the types of special functions whose type is not denotable in hack, e.g. idx *)
let transform_special_fun_ty :
    pessimise:bool -> decl_fun_type -> Aast.sid -> int -> decl_fun_type =
 fun ~pessimise fty id nargs ->
  if String.equal (snd id) SN.FB.idx || String.equal (snd id) SN.Readonly.idx
  then
    transform_idx_fun_ty ~pessimise fty nargs
  else
    fty
