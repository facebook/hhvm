(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Utils
open Typing_defs

module Reason = Typing_reason
module Env    = Typing_env
module Inst   = Typing_instantiate
module TUtils = Typing_utils

(*****************************************************************************)
(* Expanding type definition *)
(*****************************************************************************)

let rec expand_typedef seen env r x argl =
  let pos = Reason.to_pos r in
  let env, tdef = Typing_env.get_typedef env x in
  let tdef = match tdef with None -> assert false | Some x -> x in
  let tdef =
    match tdef with
    | Env.Typedef.Error -> raise Ignore
    | Env.Typedef.Ok x -> x
  in
  let visibility, tparaml, tcstr, expanded_ty = tdef in
  let should_expand =
    match visibility with
    | Env.Typedef.Private fn ->
        fn = env.Env.genv.Env.file
    | Env.Typedef.Public -> true
  in
  if List.length tparaml <> List.length argl
  then begin
    let n = List.length tparaml in
    let n = string_of_int n in
    error pos ("The type "^x^" expects "^n^" parameters");
  end;
  let subst = ref SMap.empty in
  List.iter2 begin fun ((_, param), _) ty ->
    subst := SMap.add param ty !subst
  end tparaml argl;
  let env, expanded_ty =
    if should_expand
    then begin
      Inst.instantiate !subst env expanded_ty
    end
    else begin
      let env, tcstr =
        match tcstr with
        | None -> env, None
        | Some tcstr ->
            let env, tcstr = Inst.instantiate !subst env tcstr in
            env, Some tcstr
      in
      env, (r, Tabstract ((pos, x), argl, tcstr))
    end
  in
  check_typedef seen env expanded_ty;
  env, (r, snd expanded_ty)

and check_typedef seen env (r, t) =
  match t with
  | Tany -> ()
  | Tmixed -> ()
  | Tarray (_, ty1, ty2) ->
      check_typedef_opt seen env ty1;
      check_typedef_opt seen env ty2;
      ()
  | Tgeneric (_, ty) ->
      check_typedef_opt seen env ty
  | Toption ty -> check_typedef seen env ty
  | Tprim _ -> ()
  | Tvar _ ->
      (* This can happen after intantiantion of the typedef.
       * Having a cyclic typedef defined this way is fine, because of the
       * type variable, it will be handled gracefully.
       * Besides, it's not that the typedef depends on itself, it's that
       * it depends on a parameter that could use itself, which is different.
       * (cf tdef_tvar.php unit test for a use case).
       *)
      ()
  | Tfun fty ->
      check_fun_typedef seen env fty
  | Tapply ((p, x), argl) when Typing_env.is_typedef env x ->
      if SSet.mem x seen
      then error p "Cyclic typedef"
      else
        let seen = SSet.add x seen in
        let env, ty = expand_typedef seen env r x argl in
        check_typedef seen env ty
  | Tabstract (_, tyl, cstr) ->
      check_typedef_list seen env tyl;
      check_typedef_opt seen env cstr
  | Tapply (_, tyl)
  | Ttuple tyl ->
      check_typedef_list seen env tyl
  | Tanon _ -> assert false
  | Tunresolved _ -> assert false
  | Tobject -> ()
  | Tshape tym ->
      SMap.iter (fun _ v -> check_typedef seen env v) tym

and check_typedef_list seen env x =
  List.iter (check_typedef seen env) x

and check_fun_typedef seen env ft =
  check_typedef_tparam_list seen env ft.ft_tparams;
  check_typedef_fun_param_list seen env ft.ft_params;
  check_typedef seen env ft.ft_ret;
  ()

and check_typedef_fun_param_list seen env x =
  List.iter (check_typedef_fun_param seen env) x

and check_typedef_fun_param seen env (_, ty) =
  check_typedef seen env ty

and check_typedef_tparam_list seen env x =
  List.iter (check_typedef_tparam seen env) x

and check_typedef_tparam seen env (_, x) =
  check_typedef_opt seen env x

and check_typedef_opt seen env = function
  | None -> ()
  | Some x -> check_typedef seen env x

(*****************************************************************************)
(*****************************************************************************)

let () = TUtils.expand_typedef_ref := expand_typedef
