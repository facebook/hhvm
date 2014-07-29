(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Converts a type hint into a type  *)
(*****************************************************************************)
open Utils
open Typing_defs
open Nast

module Env = Typing_env

let rec hint env (p, h) =
  let env, h = hint_ p env h in
  env, (Typing_reason.Rhint p, h)

and hint_ p env = function
  | Hany ->
      env, Tany
  | Hmixed ->
      env, Tmixed
  | Harray (h1, h2) ->
      if Env.is_strict env && h1 = None
      then Errors.generic_array_strict p;
      let env, h1 = opt hint env h1 in
      let env, h2 = opt hint env h2 in
      env, Tarray (true, h1, h2)
  | Hprim p -> env, Tprim p
  | Habstr (x, hopt) ->
      let env, ty_opt = opt hint env hopt in
      env, Tgeneric (x, ty_opt)
  | Hoption (_, Hprim Tvoid) ->
      Errors.nullable_void p;
      env, Tany
  | Hoption (_, Hmixed) ->
      Errors.option_mixed p;
      env, Tany
  | Hoption h ->
      let env, h = hint env h in
      env, Toption h
  | Hfun (hl, b, h) ->
      let env, paraml = lfold hint env hl in
      let paraml = List.map (fun x -> None, x) paraml in
      let env, ret = hint env h in
      let arity_min = List.length paraml in
      let arity = if b
        then Fellipsis arity_min
        else Fstandard (arity_min, arity_min)
      in
      env, Tfun {
        ft_pos = p;
        ft_unsafe = false;
        ft_abstract = false;
        ft_arity = arity;
        ft_tparams = [];
        ft_params = paraml;
        ft_ret = ret;
      }
  | Happly ((p, "\\Tuple"), _)
  | Happly ((p, "\\tuple"), _) ->
      Errors.tuple_syntax p;
      env, Tany
  | Happly (((p, c) as id), argl) ->
      Find_refs.process_class_ref p c None;
      let env = Env.add_wclass env c in
      let env, argl = lfold hint env argl in
      env, Tapply (id, argl)
  | Htuple hl ->
      let env, tyl = lfold hint env hl in
      env, Ttuple tyl
  | Hshape fdm ->
      let env, fdm = ShapeMap.map_env hint env fdm in
      env, Tshape fdm
