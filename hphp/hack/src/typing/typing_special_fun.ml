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

let update_param p ty = { p with fp_type = { p.fp_type with et_type = ty } }

(* Transform the special function `idx` according to the number
 * of arguments actually passed to the function
 *)
let transform_special_fun_ty fty id nargs =
  (* The idx function has two signatures, depending on number of arguments
   * actually passed:
   *
   * idx<Tk as arraykey, Tv>(?KeyedContainer<Tk, ?Tv> $collection, ?Tk $index): ?Tv
   * idx<Tk as arraykey, Tv>(?KeyedContainer<Tk, Tv> $collection, ?Tk $index, Tv $default): Tv
   *
   * In the hhi file, it has signature
   *
   * function idx<Tk as arraykey, Tv>
   *   (?KeyedContainer<Tk, Tv> $collection, ?Tk $index, $default = null)
   *
   * so this needs to be munged into the above.
   *)
  if String.equal (snd id) SN.FB.idx then
    let (param1, param2, param3) =
      match fty.ft_params with
      | [param1; param2; param3] -> (param1, param2, param3)
      | _ -> failwith "Expected 3 parameters for idx in hhi file"
    in
    let r3 = get_reason param1.fp_type.et_type in
    let rret = get_reason fty.ft_ret.et_type in
    let (params, ret) =
      match nargs with
      | 2 ->
        (* Transform ?KeyedContainer<Tk, Tv> to ?KeyedContainer<Tk, ?Tv> *)
        let fail () = failwith "Wrong type for idx in hhi file" in
        let ty1 =
          match deref param1.fp_type.et_type with
          | (r11, Toption t) ->
            begin
              match deref t with
              | (r12, Tapply (coll, [tk; tv])) ->
                let (r13, _) = deref tv in
                mk
                  ( r11,
                    Toption
                      (mk (r12, Tapply (coll, [tk; mk (r13, Toption tv)]))) )
              | _ -> fail ()
            end
          | _ -> fail ()
        in
        let param1 = update_param param1 ty1 in
        (* Return type should be ?Tv *)
        let ret = MakeType.nullable_decl rret (MakeType.generic rret "Tv") in
        ([param1; param2], ret)
      | 3 ->
        (* Third parameter should have type Tv *)
        let param3 = update_param param3 (MakeType.generic r3 "Tv") in
        (* Return type should be Tv *)
        let ret = MakeType.generic rret "Tv" in
        ([param1; param2; param3], ret)
      (* Shouldn't happen! *)
      | _ -> (fty.ft_params, fty.ft_ret.et_type)
    in
    { fty with ft_params = params; ft_ret = { fty.ft_ret with et_type = ret } }
  else
    fty
