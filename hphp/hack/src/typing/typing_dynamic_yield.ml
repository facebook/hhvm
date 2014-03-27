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
(* Module used to type DynamicYield
 * Each class that uses the DynamicYield trait or which extends a class that
 * uses the DynamicYield trait implicitly defines a few methods. If it
 * explicitly defines a yieldFoo method, then it implicitly also defines genFoo,
 * prepareFoo, and getFoo (unless any of those methods are explicitly defined).
 * It does this with __call().
 *)
(*****************************************************************************)
open Nast
open Utils
open Typing_defs

module Reason = Typing_reason
module Type   = Typing_ops
module Env    = Typing_env


(* Classes that use the DynamicYield trait and implement yieldFoo also provide
 * prepareFoo, genFoo(), and getFoo()
 *)
let rec decl env methods =
  SMap.fold begin fun name ce (env, acc) ->
    match parse_yield_name name with
      | Some base ->
        let ce_r, ft = match ce.ce_type with
          | r, Tfun ft -> r, ft
          | _ ->  assert false in
        let r = fst ft.ft_ret in
        let p = Reason.to_pos r in
        (* Fail silently, since we're in an early stage. A later check will assert
         * that yieldFoo() has the right type *)
        let env, base_ty =
          try check_yield_types env ft.ft_pos ft.ft_ret
          with _ -> (env, (Reason.Rwitness ft.ft_pos, Tany)) in
        (* Define genFoo(), which is Awaitable<T> if yieldFoo() is Awaitable<T>
         * If yieldFoo() is Tany, then genFoo() is Awaitable<Tany> *)
        let gen_name = "gen"^base in
        let gen_r = Reason.Rdynamic_yield (p, ft.ft_pos, gen_name, name) in
        let gen_ty = ce_r, Tfun {ft with
          ft_ret =  gen_r, Tapply ((p, "Awaitable"), [base_ty])
        } in
        let acc = add gen_name {ce with ce_type = gen_ty} acc in

        (* Define prepareFoo(), which is always Awaitable<void> *)
        let prepare_name = "prepare"^base in
        let prepare_r = Reason.Rdynamic_yield (p, ft.ft_pos, prepare_name, name) in
        let prepare_ty = ce_r, Tfun {ft with
          ft_ret =  prepare_r, Tapply ((p, "Awaitable"), [r, Tprim Nast.Tvoid])
        } in
        let acc = add prepare_name {ce with ce_type = prepare_ty} acc in

        (* Define getFoo(), which is T if yieldFoo() is Awaitable<T>. As an
         * annoying special-case, unfinalize this, since the runtime allows you
         * to "override" a yieldFoo() with a getFoo(), and people unfortunately
         * do this quite a bit. *)
        let get_name = "get"^base in
        let base_p = Reason.to_pos (fst base_ty) in
        let get_r = Reason.Rdynamic_yield (base_p, ft.ft_pos, get_name, name) in
        let get_ty = ce_r, Tfun {ft with ft_ret = get_r, snd base_ty} in
        let acc = add get_name {ce with ce_type = get_ty ; ce_final = false} acc in
        env, acc
      | None ->
        (match parse_get_name name with
          | None -> env, acc
          | Some base ->
            let yield_name = "yield"^base in
            if SMap.mem yield_name acc then
              (* if there's a yield and a get defined, the gen from the yield wins *)
              env, acc
            else
              let gen_name = "gen"^base in
              (* Define genFoo(), which is Awaitable<T> if getFoo() is T *
               * If getFoo() is Tany, then genFoo() is Awaitable<Tany> *)
              let ce_r, ft = match ce.ce_type with
                | r, Tfun ft -> r, ft
                | _ ->  assert false in
              let p = Reason.to_pos (fst ft.ft_ret) in
              let gen_r = Reason.Rdynamic_yield (p, ft.ft_pos, gen_name, name) in
              let gen_ty = ce_r, Tfun {ft with
                ft_ret = gen_r, Tapply ((p, "Awaitable"), [ft.ft_ret])
              } in
              let acc = add gen_name {ce with ce_type = gen_ty} acc in
              env, acc
        )
  end methods (env, methods)

and check_yield_types env p hret =
  let type_var = Env.fresh_type() in
  let r = Reason.Rwitness p in
  let expected_type = r, Tapply ((p, "Awaitable"), [type_var]) in
  let env = Type.sub_type p (Reason.URdynamic_yield) env expected_type hret in
  (* Fully expand to make doubly sure we don't leak any type variables *)
  env, Typing_expand.fully_expand env type_var

and contains_dynamic_yield = SSet.mem "DynamicYield"
and contains_dynamic_yield_interface = SSet.mem "IUseDynamicYield"
and implements_dynamic_yield_interface ancestors = SMap.mem "IUseDynamicYield" ancestors
and is_dynamic_yield name = (name = "DynamicYield")

and parse_yield_name name =
  if Str.string_match (Str.regexp "^yield\\(.*\\)") name 0
  then Some (Str.matched_group 1 name)
  else None

and parse_get_name name =
  if Str.string_match (Str.regexp "^get\\(.*\\)") name 0
  then Some (Str.matched_group 1 name)
  else None

and add name ce acc =
  match SMap.get name acc with
    (* Only add to the map if the element isn't there, or if its type is
     * abstract. Normally we don't want to overwrite existing elements to allow
     * having yieldFoo and getFoo both declared in a class with potentially
     * incompatible types (yes this is very confusing but www does it to some
     * extent). However, if the existing method is abstract, we do want abstract
     * implementations from abstract classes or traits to be allowed to be
     * overwritten (i.e., implemented) by subclasses or including classes. *)
    | None
    | Some { ce_type = (_, Tfun { ft_abstract = true; _ }) } ->
      SMap.add name ce acc
    | _ -> acc

let clean_dynamic_yield env methods =
  env, SMap.filter begin fun name _ -> name <> "__call" end methods

let method_def env name hret =
  let env, class_ = Env.get_class env (Env.get_self_id env) in
  match class_, parse_yield_name (snd name) with
    | None, _
    | _, None -> env
    | Some c, Some base_name ->
      if contains_dynamic_yield c.tc_extends
      then fst (check_yield_types env (fst name) hret)
      else env

let check_yield_visibility env c =
  let uses_dy_directly = List.exists begin fun trait ->
    match trait with
      | (_, Happly ((pos, name), _)) -> begin
          (* Either you directly use DynamicYield, or something you directly
           * use itself uses DynamicYield. *)
          is_dynamic_yield name || match snd (Env.get_class_dep env name) with
            | Some parent_type -> contains_dynamic_yield parent_type.tc_extends
            | None -> false
          end
      | _ -> assert false
  end c.c_uses in
  if not uses_dy_directly then List.iter begin fun m ->
    match (parse_yield_name (snd m.m_name), m.m_visibility) with
      | (Some _, Private) ->
          error_l [
            fst m.m_name,
            "DynamicYield cannot see private methods in subclasses"
         ]
      | _ -> ()
  end c.c_methods
