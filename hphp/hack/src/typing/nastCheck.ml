(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(* This module performs checks after the naming has been done.
   Basically any check that doesn't fall in the typing category. *)
(* Check of type application arity *)
(* Check no `new AbstractClass` (or trait, or interface) *)

(* NOTE: since the typing environment does not generally contain
   information about non-Hack code, some of these checks can
   only be done in strict (i.e. "everything is Hack") mode. Use
   `if Env.is_strict env.tenv` style checks accordingly.
*)


open Utils
open Nast
open Typing_defs
open Typing_deps
open Autocomplete

module Env = Typing_env

module Error = struct

  let type_arity name nargs =
    sl ["The type ";name;" expects ";nargs;" type parameter(s)"]

  let abstract_outside (p, _) =
    error p
    "This method is declared as abstract, in a class that isn't"

  let interface_with_body (p, _) =
    error p
    "A method cannot have a body in an interface"

  let abstract_with_body (p, _) =
    error p
    "This method is declared as abstract, but has a body"

  let not_abstract_without_body (p, _) =
    error p
    "This method is not declared as abstract, it must have a body"

  let return_in_gen p =
    error p
      ("Don't use return in a generator (a generator"^
       " is a function that uses yield)")

  let return_in_finally p =
    error p
      ("Don't use return in a finally block;"^
          " there's nothing to receive the return value")

  let yield_in_async_function p =
    error p
    "Don't use yield in an async function"

  let await_in_sync_function p =
    error p
    "await can only be used inside async functions"

  let magic (p, s) =
    error p
      ("Don't call "^s^" it's one of these magic things we want to avoid")

  let non_interface (p : Pos.t) (c2: string) (verb: string): 'a =
    error p ("Cannot " ^ verb ^ " " ^ c2 ^ " - it is not an interface")

  let toString_returns_string pos =
    error pos "__toString should return a string"

  let toString_visibility pos =
    error pos "__toString must have public visibility and cannot be static"

  let uses_non_trait (p: Pos.t) (n: string) (t: string) =
    error p (n ^ " is not a trait. It is " ^ t ^ ".")

  let requires_non_class (p: Pos.t) (n: string) (t: string) =
    error p (n ^ " is not a class. It is " ^ t ^ ".")

end


module CheckGenerator = struct

  let rec stmt = function
    | Return (p, _) ->
        Error.return_in_gen p
    | Throw (p, _) -> ()
    | Noop
    | Fallthrough
    | Break | Continue
    | Expr _
    | Static_var _ -> ()
    | If (_, b1, b2) ->
        block b1;
        block b2;
        ()
    | Do (b, _) ->
        block b;
        ()
    | While (_, b) ->
        block b;
        ()
    | For (_, _, _, b) ->
        block b;
        ()
    | Switch (_, cl) ->
        List.iter case cl;
        ()
    | Foreach (_, _, b) ->
        block b;
        ()
    | Try (b, cl, fb) ->
        block b;
        List.iter catch cl;
        block fb;
        ()

  and block stl =
    List.iter stmt stl

  and case = function
    | Default b -> block b
    | Case (_, b) ->
        block b;
        ()

  and catch (_, _, b) = block b

end

module CheckFunctionType = struct
  let rec stmt f_type = function
    | Return (_, None) -> ()
    | Return (_, Some e)
    | Throw (_, e)
    | Expr e ->
        expr f_type e;
        ()
    | Noop
    | Fallthrough
    | Break | Continue -> ()
    | Static_var _ -> ()
    | If (_, b1, b2) ->
        block f_type b1;
        block f_type b2;
        ()
    | Do (b, _) ->
        block f_type b;
        ()
    | While (_, b) ->
        block f_type b;
        ()
    | For (_, _, _, b) ->
        block f_type b;
        ()
    | Switch (_, cl) ->
        liter case f_type cl;
        ()
    | Foreach (_, _, b) ->
        block f_type b;
        ()
    | Try (b, cl, fb) ->
        block f_type b;
        liter catch f_type cl;
        block f_type fb;
        ()

  and block f_type stl =
    liter stmt f_type stl

  and case f_type = function
    | Default b -> block f_type b
    | Case (_, b) ->
        block f_type b;
        ()

  and catch f_type (_, _, b) = block f_type b

  and expr f_type (p, e) =
    expr_ p f_type e

  and expr2 f_type (e1, e2) =
  expr f_type e1;
  expr f_type e2;
  ()

  and expr_ p f_type exp = match f_type, exp with
    | _, Array _
    | _, Fun_id _
    | _, Method_id _
    | _, Smethod_id _
    | _, Method_caller _
    | _, This
    | _, Id _
    | _, Class_get _
    | _, Class_const _
    | _, Lvar _ -> ()
    | _, ValCollection (_, el) ->
      liter expr f_type el;
      ()
    | _, KeyValCollection (_, fdl) ->
        liter expr2 f_type fdl;
        ()
    | _, Clone e -> expr f_type e; ()
    | _, Obj_get (e, (_, Id s)) ->
        expr f_type e;
        ()
    | _, Obj_get (e1, e2) ->
        expr2 f_type (e1, e2);
        ()
    | _, Array_get (e, eopt) ->
        expr f_type e;
        maybe expr f_type eopt;
        ()
    | _, Call (_, e, el) ->
        expr f_type e;
        liter expr f_type el;
        ()
    | _, True | _, False | _, Int _
    | _, Float _ | _, Null | _, String _ -> ()
    | _, String2 (el, _) ->
        liter expr f_type el;
        ()
    | _, List el ->
      liter expr f_type el;
      ()
    | _, Pair (e1, e2) ->
      expr2 f_type (e1, e2);
      ()
    | _, Expr_list el ->
        liter expr f_type el;
        ()
    | _, Unop (_, e) -> expr f_type e
    | _, Binop (_, e1, e2) ->
        expr2 f_type (e1, e2);
        ()
    | _, Eif (e1, None, e3) ->
        expr2 f_type (e1, e3);
        ()
    | _, Eif (e1, Some e2, e3) ->
        liter expr f_type [e1; e2; e3];
      ()
    | _, New (_, el) ->
      liter expr f_type el;
        ()
    | _, InstanceOf (e, _) ->
        expr f_type e;
        ()
    | _, Cast (_, e) ->
        expr f_type e;
        ()
    | _, Efun (f, _) -> ()
    | Ast.FAsync, Yield_break
    | Ast.FAsync, Yield _ -> Error.yield_in_async_function p
    | Ast.FAsync, Special_func func ->
      (match func with
        | Gena e
        | Gen_array_rec e -> expr f_type e
        | Genva el
        | Gen_array_va_rec el -> liter expr f_type el);
      ()
    | Ast.FSync, Yield_break -> ()
    | Ast.FSync, Yield e -> expr f_type e; ()
    | Ast.FSync, Special_func func ->
      (match func with
        | Gena e
        | Gen_array_rec e -> expr f_type e
        | Genva el
        | Gen_array_va_rec el -> liter expr f_type el);
      ()
    | Ast.FSync, Await _ -> Error.await_in_sync_function p
    | Ast.FAsync, Await e -> expr f_type e; ()
    | _, Xml (_, attrl, el) ->
        List.iter (fun (_, e) -> expr f_type e) attrl;
        liter expr f_type el;
        ()
    | _, Assert (AE_assert e) ->
        expr f_type e;
        ()
    | _, Assert (AE_invariant_violation (e, el)) ->
        liter expr f_type (e :: el);
        ()
    | _, Assert (AE_invariant (e1, e2, el)) ->
        liter expr f_type (e1 :: e2 :: el);
        ()
    | _, Shape fdm ->
        SMap.iter (fun _ v -> expr f_type v) fdm;
        ()

end

type env = {
  t_is_gen: bool ref;
  t_is_finally: bool ref;
  class_name: string option;
  class_kind: Ast.class_kind option;
  tenv: Env.env;
}

let is_magic =
  let h = Hashtbl.create 23 in
  let a x = Hashtbl.add h x true in
  a "__set";
  a "__isset";
  a "__get";
  a "__unset";
  a "__call";
  a "__callStatic";
  fun (_, s) ->
    Hashtbl.mem h s

let rec fun_ tenv f =
  if f.f_mode = Ast.Mdecl || !auto_complete then () else begin
  let tenv = Typing_env.set_root tenv (Dep.Fun (snd f.f_name)) in
  let env = { t_is_gen = ref false; t_is_finally = ref false;
              class_name = None; class_kind = None;
              tenv = tenv } in
  func env f
  end

and func env f =
  let old_gen = !(env.t_is_gen) in
  let env = { env with tenv = Env.set_mode env.tenv f.f_mode } in
  maybe hint env f.f_ret;
  List.iter (fun_param env) f.f_params;
  block env f.f_body;
  CheckFunctionType.block f.f_type f.f_body;
  if !(env.t_is_gen) then CheckGenerator.block f.f_body;
  env.t_is_gen := old_gen

and hint env (p, h) =
  hint_ env p h

and hint_ env p = function
  | Hany  | Hmixed  | Habstr _ | Hprim _ ->
      ()
  | Harray (ty1, ty2) ->
      maybe hint env ty1;
      maybe hint env ty2
  | Htuple hl -> List.iter (hint env) hl
  | Hoption h ->
      hint env h; ()
  | Hfun (hl,_, h) ->
      List.iter (hint env) hl;
      hint env h;
      ()
  | Happly ((_, x), hl) when Typing_env.is_typedef env.tenv x ->
      let tdef = Typing_env.Typedefs.find_unsafe x in
      let params =
        match tdef with
        | Typing_env.Typedef.Error -> raise Ignore
        | Typing_env.Typedef.Ok (_, x, _, _) -> x
      in
      check_params env p x params hl
  | Happly ((_, x), hl) ->
      let _, class_ = Env.get_class env.tenv x in
      (match class_ with
      | None ->
          (match env.tenv.Typing_env.genv.Typing_env.mode with
          | Ast.Mstrict ->
              raise Ignore
          | Ast.Mpartial | Ast.Mdecl ->
              ()
          )
      | Some class_ ->
          check_params env p x class_.tc_tparams hl
      );
      ()
  | Hshape fdl ->
      SMap.iter (fun _ v -> hint env v) fdl

and check_params env p x params hl =
  let arity = List.length params in
  check_arity env p x arity (List.length hl);
  List.iter (hint env) hl;

and check_arity env p tname arity size =
  if size = arity then () else
  if size = 0 && not (Typing_env.is_strict env.tenv) then () else
  let nargs = soi arity in
  let msg   = Error.type_arity tname nargs in
  error p msg

and class_ tenv c =
  if c.c_mode = Ast.Mdecl || !auto_complete then () else begin
  let cname = Some (snd c.c_name) in
  let tenv = Typing_env.set_root tenv (Dep.Class (snd c.c_name)) in
  let env = { t_is_gen = ref false; t_is_finally = ref false;
              class_name = cname;
              class_kind = Some c.c_kind;
              tenv = tenv } in
  let env = { env with tenv = Env.set_mode env.tenv c.c_mode } in
  if c.c_kind = Ast.Cinterface then begin
    interface env c;
  end
  else begin
    maybe method_ (env, true) c.c_constructor;
  end;
  liter hint env c.c_extends;
  liter hint env c.c_implements;
  liter class_const env c.c_consts;
  liter class_var env c.c_static_vars;
  liter class_var env c.c_vars;
  liter method_ (env, true) c.c_static_methods;
  liter method_ (env, false) c.c_methods;
  liter check_is_interface (env, "implement") c.c_implements;
  liter check_is_interface (env, "require implementation of") c.c_req_implements;
  liter check_is_class env c.c_req_extends;
  liter check_is_trait env c.c_uses;
  end;
  ()

(** Make sure that the given hint points to an interface *)
and check_is_interface (env, error_verb) (x : hint) =
  match (snd x) with
    | Happly (id, _) ->
      let _, class_ = Env.get_class env.tenv (snd id) in
      (match class_ with
        | None ->
          (* in partial mode, we can fail to find the class if it's
             defined in PHP. *)
          (* in strict mode, we catch the unknown class error before
             even reaching here. *)
          ()
        | Some { tc_kind = Ast.Cinterface } -> ()
        | Some { tc_name } ->
          Error.non_interface (fst x) tc_name error_verb
      )
    | _ -> failwith "assertion failure: interface isn't a Happly"

(** Make sure that the given hint points to a non-final class *)
and check_is_class env (x : hint) =
  match (snd x) with
    | Happly (id, _) ->
      let _, class_ = Env.get_class env.tenv (snd id) in
      (match class_ with
        | None ->
          (* in partial mode, we can fail to find the class if it's
             defined in PHP. *)
          (* in strict mode, we catch the unknown class error before
             even reaching here. *)
          ()
        | Some { tc_kind = Ast.Cabstract } -> ()
        | Some { tc_kind = Ast.Cnormal } -> ()
        | Some { tc_kind; tc_name } ->
          Error.requires_non_class (fst x) tc_name (Ast.string_of_class_kind tc_kind)
      )
    | _ -> failwith "assertion failure: interface isn't a Happly"

(**
   * Make sure that all "use"s are with traits, and not
   * classes, interfaces, etc.
*)
and check_is_trait env (h : hint) =
  (* Second part of a hint contains Happly information *)
  (match (snd h) with
  (* An Happly contains identifying info (sid) and hint list (which we *)
  (* do not care about at this time *)
  | Happly (pos_and_name, _) ->
    (* Env.get_class will get the type info associated with the name *)
    let _, type_info = Env.get_class env.tenv (snd pos_and_name) in
    (match type_info with
      (* in partial mode, it's possible to not find the trait, because the *)
      (* trait may live in PHP land. In strict mode, we catch the unknown *)
      (* trait error before even reaching here. so it's ok to just return *)
      (* unit. *)
      | None -> ()
      (* tc_kind is part of the type_info. If we are a trait, all is good *)
      | Some { tc_kind = Ast.Ctrait } -> ()
      (* Anything other than a trait we are going to throw an error *)
      (* using the tc_kind and tc_name fields of our type_info *)
      | Some { tc_kind; tc_name } ->
        Error.uses_non_trait (fst h) tc_name (Ast.string_of_class_kind tc_kind)
    )
  | _ -> failwith "assertion failure: trait isn't an Happly"
  )

and interface env c =
  (* make sure that interfaces only have empty public methods *)
  liter begin fun env m ->
    if m.m_body <> []
    then error (fst m.m_name) "This method shouldn't have a body"
    else ();
    if m.m_visibility <> Public
    then error (fst m.m_name) "Access type for interface method must be public"
    else ()
  end env (c.c_static_methods @ c.c_methods);
  (* make sure that interfaces don't have any member variables *)
  match c.c_vars with
  | hd::_ ->
    let pos = fst (hd.cv_id) in
    error pos "Interfaces cannot have member variables";
  | _ -> ();
  (* make sure that interfaces don't have static variables *)
  match c.c_static_vars with
  | hd::_ ->
    let pos = fst (hd.cv_id) in
    error pos "Interfaces cannot have static variables";
  | _ -> ()

and class_const env (h, _, e) =
  maybe hint env h;
  expr env e

and class_var env cv =
  maybe hint env cv.cv_type;
  maybe expr env cv.cv_expr;
  ()

and check__toString m is_static =
  if snd m.m_name = "__toString"
  then begin
    if m.m_visibility <> Public || is_static
    then Error.toString_visibility (fst m.m_name);
    match m.m_ret with
      | Some (_, Hprim Tstring) -> ()
      | Some (p, _) -> Error.toString_returns_string p
      | None -> ()
  end

and method_ (env, is_static) m =
  check__toString m is_static;
  let old_gen = !(env.t_is_gen) in
  liter fun_param env m.m_params;
  block env m.m_body;
  maybe hint env m.m_ret;
  CheckFunctionType.block m.m_type m.m_body;
  if !(env.t_is_gen)
  then CheckGenerator.block m.m_body;
  if m.m_body <> [] && m.m_abstract
  then Error.abstract_with_body m.m_name;
  if m.m_body = [] && not m.m_abstract
  then Error.not_abstract_without_body m.m_name;
  (match env.class_name with
  | Some cname ->
      let p, mname = m.m_name in
      if String.lowercase cname = String.lowercase mname
          && env.class_kind <> Some Ast.Ctrait
      then
        error p ("This is a dangerous method name, "^
                 "if you want to define a constructor, use "^
                 "__construct")
      else ()
  | None -> assert false);
  env.t_is_gen := old_gen;
  ()

and fun_param env param =
  maybe hint env param.param_hint;
  maybe expr env param.param_expr;
  ()

and fun_param_opt env (h, _, e) =
  maybe hint env h;
  maybe expr env e;
  ()

and stmt env = function
  | Return (p, _) when !(env.t_is_finally) ->
    Error.return_in_finally p; ()
  | Return (_, None)
  | Noop
  | Fallthrough
  | Break | Continue -> ()
  | Return (_, Some e)
  | Expr e | Throw (_, e) ->
    expr env e
  | Static_var el ->
    liter expr env el
  | If (e, b1, b2) ->
    expr env e;
    block env b1;
    block env b2;
    ()
  | Do (b, e) ->
    block env b;
    expr env e;
    ()
  | While (e, b) ->
      expr env e;
      block env b;
      ()
  | For (e1, e2, e3, b) ->
      expr env e1;
      expr env e2;
      expr env e3;
      block env b;
      ()
  | Switch (e, cl) ->
      expr env e;
      liter case env cl;
      ()
  | Foreach (e1, ae, b) ->
      expr env e1;
      as_expr env ae;
      block env b;
      ()
  | Try (b, cl, fb) ->
      block env b;
      liter catch env cl;
      let is_fin_copy = !(env.t_is_finally) in
      env.t_is_finally := true;
      block env fb;
      env.t_is_finally := is_fin_copy;
      ()

and as_expr env = function
  | As_id e -> expr env e
  | As_kv (e1, e2) ->
      expr env e1;
      expr env e2;
      ()

and block env stl =
  liter stmt env stl

and expr env (_, e) =
  expr_ env e

and expr_ env = function
  | Array _
  | Fun_id _
  | Method_id _
  | Smethod_id _
  | Method_caller _
  | This
  | Id _
  | Class_get _
  | Class_const _
  | Lvar _ -> ()
  | ValCollection (_, el) ->
      liter expr env el;
      ()
  | KeyValCollection (_, fdl) ->
      liter field env fdl;
      ()
  | Clone e -> expr env e; ()
  | Obj_get (e, (_, Id s)) ->
      if is_magic s && Env.is_strict env.tenv
      then Error.magic s;
      expr env e;
      ()
  | Obj_get (e1, e2) ->
      expr env e1;
      expr env e2;
      ()
  | Array_get (e, eopt) ->
      expr env e;
      maybe expr env eopt;
      ()
  | Call (_, e, el) ->
      expr env e;
      liter expr env el;
      ()
  | True | False | Int _
  | Float _ | Null | String _ -> ()
  | String2 (el, _) ->
      liter expr env el;
      ()
  | Unop (_, e) -> expr env e
  | Yield_break -> ()
  | Special_func func ->
    (match func with
      | Gena e
      | Gen_array_rec e->
        expr env e
      | Genva el
      | Gen_array_va_rec el ->
        liter expr env el);
    ()
  | Yield e ->
      env.t_is_gen := true;
      expr env e;
      ()
  | Await e ->
      expr env e;
      ()
  | List el ->
      liter expr env el;
      ()
  | Pair (e1, e2) ->
    expr env e1;
    expr env e2;
    ()
  | Expr_list el ->
      liter expr env el;
      ()
  | Cast (h, e) ->
      hint env h;
      expr env e;
      ()
  | Assert (AE_invariant (e1, e2, el)) ->
      expr env e1;
      expr env e2;
      liter expr env el
  | Binop (_, e1, e2) ->
      expr env e1;
      expr env e2;
      ()
  | Eif (e1, None, e3) ->
      expr env e1;
      expr env e3;
      ()
  | Eif (e1, Some e2, e3) ->
      expr env e1;
      expr env e2;
      expr env e3;
      ()
  | Assert (AE_invariant_violation (e, el)) ->
      expr env e;
      liter expr env el
  | Assert (AE_assert e)
  | InstanceOf (e, _) ->
      expr env e;
      ()
  | New (_, el) ->
      liter expr env el
  | Efun (f, _) ->
    let is_gen_copy = !(env.t_is_gen) in
      env.t_is_gen := false;
      func env f;
      env.t_is_gen := is_gen_copy;
      ()
  | Xml (_, attrl, el) ->
      liter attribute env attrl;
      liter expr env el;
      ()
  | Shape fdm ->
      SMap.iter (fun _ v -> expr env v) fdm

and case env = function
  | Default b -> block env b
  | Case (e, b) ->
      expr env e;
      block env b;
      ()

and catch env (_, _, b) = block env b
and field env (e1, e2) =
  expr env e1;
  expr env e2;
  ()

and attribute env (_, e) =
  expr env e;
  ()

let typedef tenv (_, params, h) =
  let env = { t_is_gen = ref false;
              t_is_finally = ref false;
              class_name = None; class_kind = None;
              tenv = tenv } in
  hint env h;
  (match h with
  | p, Happly ((_, x), _) when Typing_env.is_typedef tenv x ->
      let tenv, ty = Typing_hint.hint tenv h in
      Typing_tdef.check_typedef SSet.empty tenv ty;
      ()
  | _ -> ())
