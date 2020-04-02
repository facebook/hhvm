(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Module checking that all the class members are properly initialized.
 * To be more precise, this checks that if the constructor does not throw,
 * it initializes all members. *)
open Hh_prelude
open Aast
open Nast
module DICheck = Decl_init_check
module DeferredMembers = Typing_deferred_members
module SN = Naming_special_names

let shallow_decl_enabled (ctx : Provider_context.t) : bool =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

module SSetWTop = struct
  type t =
    | Top
    | Set of SSet.t

  let union s1 s2 =
    match (s1, s2) with
    | (Top, _)
    | (_, Top) ->
      Top
    | (Set s1, Set s2) -> Set (SSet.union s1 s2)

  let inter s1 s2 =
    match (s1, s2) with
    | (Top, s)
    | (s, Top) ->
      s
    | (Set s1, Set s2) -> Set (SSet.inter s1 s2)

  let inter_list (sl : t list) = List.fold_left ~f:inter ~init:Top sl

  let add x s =
    match s with
    | Top -> Top
    | Set s -> Set (SSet.add x s)

  let mem x s =
    match s with
    | Top -> true
    | Set s -> SSet.mem x s

  let empty = Set SSet.empty
end

module S = SSetWTop

(* Exception raised when we hit a return statement and the initialization
 * is not over.
 * When that is the case, we bubble up back to the toplevel environment.
 * An example (right hand side is the set of things initialized):
 *
 *  $this->x = 0;  // { x }
 *  if(...) {
 *     $this->y = 1; // { x, y }
 *     if(...) {
 *        $this->z = 2; // { x, y, z }
 *        return; // raise InitReturn with set { x, y, z}
 *     } // exception caught, re-raise with { x, y }
 *  } // exception caught, re-reraise with { x }
 *
 *  What is effectively initialized: { x }
 *)
exception InitReturn of S.t

(* Module initializing the environment
   Originally, every class member has 2 possible states,
   Vok  ==> when it is declared as optional, it is the job of the
            typer to make sure it is always check for the null case
            not our problem here
   Vnull ==> The value is now null, it MUST be initialized,
             and cannot be used before it has been initialized.

   Concerning the methods, basically the information we are
   interested in is, which class members do they initialize?
   But we don't want to recompute it every time it is called.
   So we memoize the result: hence the type method status.
 *)
module Env = struct
  type method_status =
    (* We already computed this method *)
    | Done
    (* We have never computed this private method before *)
    | Todo of func_body

  type t = {
    methods: method_status ref SMap.t;
    props: SSet.t;
    tenv: Typing_env_types.env;
    class_init_props: SSet.t;
  }

  let rec make tenv c =
    let ctx = Typing_env.get_ctx tenv in
    let (_, _, methods) = split_methods c in
    let methods = List.fold_left ~f:method_ ~init:SMap.empty methods in
    let sc = Shallow_decl.class_ ctx c in
    ( if shallow_decl_enabled ctx then
      (* Run DeferredMembers.class_ for its error-emitting side effects.
         When shallow_class_decl is disabled, these are emitted by Decl. *)
      let (_ : SSet.t) = DeferredMembers.class_ tenv sc in
      () );
    let (add_initialized_props, add_trait_props, add_parent_props, add_parent) =
      if shallow_decl_enabled ctx then
        ( DeferredMembers.initialized_props,
          DeferredMembers.trait_props tenv,
          DeferredMembers.parent_props tenv,
          DeferredMembers.parent tenv )
      else
        let decl_env = tenv.Typing_env_types.decl_env in
        ( DICheck.initialized_props,
          DICheck.trait_props decl_env,
          DICheck.parent_props decl_env,
          DICheck.parent decl_env )
    in
    let class_init_props = add_initialized_props sc SSet.empty in
    let props =
      SSet.empty
      |> DeferredMembers.own_props sc
      (* If we define our own constructor, we need to pretend any traits we use
       * did *not* define a constructor, because they are not reachable through
       * parent::__construct or similar functions. *)
      |> add_trait_props sc
      |> add_parent_props sc
      |> add_parent sc
    in
    { methods; props; tenv; class_init_props }

  and method_ acc m =
    if not (Aast.equal_visibility m.m_visibility Private) then
      acc
    else
      let name = snd m.m_name in
      let acc = SMap.add name (ref (Todo m.m_body)) acc in
      acc

  let get_method env m = SMap.find_opt m env.methods
end

open Env

(*****************************************************************************)
(* List of functions that can use '$this' before the initialization is
 * over.
 *)
(*****************************************************************************)

let is_whitelisted = function
  | x when String.equal x SN.StdlibFunctions.get_class -> true
  | _ -> false

let rec class_ tenv c =
  if FileInfo.(equal_mode c.c_mode Mdecl) then
    ()
  else
    let (c_constructor, _, _) = split_methods c in
    match c_constructor with
    | _ when Ast_defs.(equal_class_kind c.c_kind Cinterface) -> ()
    | Some { m_body = { fb_annotation = Nast.NamedWithUnsafeBlocks; _ }; _ } ->
      ()
    | _ ->
      let p =
        match c_constructor with
        | Some m -> fst m.m_name
        | None -> fst c.c_name
      in
      let env = Env.make tenv c in
      let inits = constructor env c_constructor in
      let check_inits inits =
        let uninit_props = SSet.diff env.props inits in
        if not (SSet.is_empty uninit_props) then
          if SSet.mem DeferredMembers.parent_init_prop uninit_props then
            Errors.no_construct_parent p
          else
            let class_uninit_props =
              SSet.filter
                (fun v -> not (SSet.mem v env.class_init_props))
                uninit_props
            in
            if not (SSet.is_empty class_uninit_props) then
              Errors.not_initialized
                (p, snd c.c_name)
                (SSet.elements class_uninit_props)
      in
      let check_throws_or_init_all inits =
        match inits with
        | S.Top ->
          (* Constructor always throw, so checking that all properties are
           * initialized is irrelevant. *)
          ()
        | S.Set inits -> check_inits inits
      in
      if
        Ast_defs.(equal_class_kind c.c_kind Ctrait)
        || Ast_defs.(equal_class_kind c.c_kind Cabstract)
      then
        let has_constructor =
          match c_constructor with
          | None -> false
          | Some m when m.m_abstract -> false
          | Some _ -> true
        in
        if has_constructor then
          check_throws_or_init_all inits
        else
          ()
      else
        check_throws_or_init_all inits

(**
 * Returns the set of properties initialized by the constructor.
 * More exactly, returns a SSetWTop.t, i.e. either a set, or Top, which is
 * the top element of the set of sets of properties, i.e. a set containing
 * all the possible properties.
 * Top is returned for a block of statements if
 * this block always throws. It is an abstract construct used to deal
 * gracefully with control flow.
 *
 * For example, if we have an `if` statement like:
 *
 * ```
 * if (...) {
 *   // This branch initialize a set of properties S1
 *   ...
 * } else {
 *   // This branch initialize another set of properties S2
 *   ...
 * }
 * ```
 *
 * then the set `S` of properties initialized by this `if` statement is the
 * intersection `S1 /\ S2`.
 * If one of the branches throws, say the first branch, then the set `S`
 * of properties initialized by the `if` statement is equal to the set of
 * properties initialized by the branch that does not throw, i.e. `S = S2`.
 * This amounts to saying that `S1` is some top element of the set of sets
 * of variables, which we call `Top`, which has the property that for all
 * set S of properties, S is included in `Top`, such that `S = S1 /\ S2`
 * still holds.
 *)
and constructor env cstr =
  match cstr with
  | None -> S.empty
  | Some cstr ->
    let check_param_initializer e = ignore (expr env S.empty e) in
    List.iter cstr.m_params (fun p ->
        Option.iter p.param_expr check_param_initializer);
    let b = Nast.assert_named_body cstr.m_body in
    toplevel env S.empty b.fb_ast

and assign _env acc x = S.add x acc

and assign_expr env acc e1 =
  match e1 with
  | (_, Obj_get ((_, This), (_, Id (_, y)), _)) -> assign env acc y
  | (_, List el) -> List.fold_left ~f:(assign_expr env) ~init:acc el
  | _ -> acc

and stmt env acc st =
  let expr = expr env in
  let block = block env in
  let catch = catch env in
  let case = case env in
  match snd st with
  | Expr
      (_, Call (Cnormal, (_, Class_const ((_, CIparent), (_, m))), _, el, _uel))
    when String.equal m SN.Members.__construct ->
    let acc = List.fold_left ~f:expr ~init:acc el in
    assign env acc DeferredMembers.parent_init_prop
  | Expr e ->
    if Typing_func_terminality.expression_exits env.tenv e then
      S.Top
    else
      expr acc e
  | GotoLabel _
  | Goto _
  | Break ->
    acc
  | Continue -> acc
  | Throw _ -> S.Top
  | Return None ->
    if are_all_init env acc then
      acc
    else
      raise (InitReturn acc)
  | Return (Some x) ->
    let acc = expr acc x in
    if are_all_init env acc then
      acc
    else
      raise (InitReturn acc)
  | Awaitall (el, b) ->
    let acc = List.fold_left el ~init:acc ~f:(fun acc (_, e2) -> expr acc e2) in
    let acc = block acc b in
    acc
  | If (e1, b1, b2) ->
    let acc = expr acc e1 in
    let b1 = block acc b1 in
    let b2 = block acc b2 in
    S.union acc (S.inter b1 b2)
  | Do (b, e) ->
    let acc = block acc b in
    expr acc e
  | While (e, _) -> expr acc e
  | Using us ->
    let acc = expr acc us.us_expr in
    block acc us.us_block
  | For (e1, _, _, _) -> expr acc e1
  | Switch (e, cl) ->
    let acc = expr acc e in
    (* Filter out cases that fallthrough *)
    let cl_body = List.filter cl case_has_body in
    let cl = List.map cl_body (case acc) in
    let c = S.inter_list cl in
    S.union acc c
  | Foreach (e, _, _) ->
    let acc = expr acc e in
    acc
  | Try (b, cl, fb) ->
    let c = block acc b in
    let f = block acc fb in
    let cl = List.map cl (catch acc) in
    let c = S.inter_list (c :: cl) in
    (* the finally block executes even if *none* of try and catch do *)
    let acc = S.union acc f in
    S.union acc c
  | Fallthrough -> S.empty
  | Noop -> acc
  | Block b -> block acc b
  | Markup _ -> acc

and toplevel env acc l =
  (try List.fold_left ~f:(stmt env) ~init:acc l with InitReturn acc -> acc)

and block env acc l =
  let acc_before_block = acc in
  try List.fold_left ~f:(stmt env) ~init:acc l
  with InitReturn _ ->
    (* The block has a return statement, forget what was initialized in it *)
    raise (InitReturn acc_before_block)

and are_all_init env set =
  SSet.fold (fun cv acc -> acc && S.mem cv set) env.props true

and check_all_init p env acc =
  SSet.iter
    begin
      fun cv ->
      if not (S.mem cv acc) then Errors.call_before_init p cv
    end
    env.props

and exprl env acc l = List.fold_left ~f:(expr env) ~init:acc l

and expr env acc (p, e) = expr_ env acc p e

and expr_ env acc p e =
  let expr = expr env in
  let exprl = exprl env in
  let field = field env in
  let afield = afield env in
  let fun_paraml = fun_paraml env in
  match e with
  | Any -> acc
  | Array fdl -> List.fold_left ~f:afield ~init:acc fdl
  | Darray (_, fdl) -> List.fold_left ~f:field ~init:acc fdl
  | Varray (_, fdl) -> List.fold_left ~f:expr ~init:acc fdl
  | ValCollection (_, _, el) -> exprl acc el
  | KeyValCollection (_, _, fdl) -> List.fold_left ~f:field ~init:acc fdl
  | This ->
    check_all_init p env acc;
    acc
  | Fun_id _
  | Method_id _
  | Smethod_id _
  | Method_caller _
  | Typename _
  | PU_atom _
  | Id _ ->
    acc
  | Lvar _
  | Lplaceholder _
  | Dollardollar _ ->
    acc
  | Obj_get ((_, This), (_, Id ((_, vx) as v)), _) ->
    if SSet.mem vx env.props && not (S.mem vx acc) then (
      Errors.read_before_write v;
      acc
    ) else
      acc
  | Clone e -> expr acc e
  | Obj_get (e1, e2, _) ->
    let acc = expr acc e1 in
    expr acc e2
  | Array_get (e, eo) ->
    let acc = expr acc e in
    (match eo with
    | None -> acc
    | Some e -> expr acc e)
  | Class_const _
  | Class_get _ ->
    acc
  | Call (Cnormal, (p, Obj_get ((_, This), (_, Id (_, f)), _)), _, _, _) ->
    let method_ = Env.get_method env f in
    (match method_ with
    | None ->
      check_all_init p env acc;
      acc
    | Some method_ ->
      (match !method_ with
      | Done -> acc
      | Todo b ->
        method_ := Done;
        let fb = Nast.assert_named_body b in
        toplevel env acc fb.fb_ast))
  | Call (_, e, _, el, unpacked_element) ->
    let el =
      match e with
      | (_, Id (_, fun_name)) when is_whitelisted fun_name ->
        List.filter el (function
            | (_, This) -> false
            | _ -> true)
      | _ -> el
    in
    let acc = List.fold_left ~f:expr ~init:acc el in
    let acc = Option.value_map ~f:(expr acc) ~default:acc unpacked_element in
    expr acc e
  | FunctionPointer (e, _targs) -> expr acc e
  | True
  | False
  | Int _
  | Float _
  | Null
  | String _
  | String2 _
  | PrefixedString _ ->
    acc
  | Assert (AE_assert e) -> expr acc e
  | Yield e -> afield acc e
  | Yield_from e -> expr acc e
  | Yield_break -> acc
  | Await e -> expr acc e
  | Suspend e -> expr acc e
  | List _ ->
    (* List is always an lvalue *)
    acc
  | Expr_list el -> exprl acc el
  | New (_, _, el, unpacked_element, _) ->
    let acc = exprl acc el in
    let acc = Option.value_map ~default:acc ~f:(expr acc) unpacked_element in
    acc
  | Record (_, _, fdl) -> List.fold_left ~f:field ~init:acc fdl
  | Pair (e1, e2) ->
    let acc = expr acc e1 in
    expr acc e2
  | Cast (_, e)
  | Unop (_, e) ->
    expr acc e
  | Binop (Ast_defs.Eq None, e1, e2) ->
    let acc = expr acc e2 in
    assign_expr env acc e1
  | Binop (Ast_defs.Ampamp, e, _)
  | Binop (Ast_defs.Barbar, e, _) ->
    expr acc e
  | Binop (_, e1, e2) ->
    let acc = expr acc e1 in
    expr acc e2
  | Pipe (_, e1, e2) ->
    let acc = expr acc e1 in
    expr acc e2
  | Eif (e1, None, e3) ->
    let acc = expr acc e1 in
    expr acc e3
  | Eif (e1, Some e2, e3) ->
    let acc = expr acc e1 in
    let acc = expr acc e2 in
    expr acc e3
  | Is (e, _) -> expr acc e
  | As (e, _, _) -> expr acc e
  | Efun (f, _)
  | Lfun (f, _) ->
    let acc = fun_paraml acc f.f_params in
    (* We don't need to analyze the body of closures *)
    acc
  | Xml (_, l, el) ->
    let l = List.map l get_xhp_attr_expr in
    let acc = exprl acc l in
    exprl acc el
  | Callconv (_, e) -> expr acc e
  | Shape fdm ->
    List.fold_left
      ~f:
        begin
          fun acc (_, v) ->
          expr acc v
        end
      ~init:acc
      fdm
  | Omitted -> acc
  | Import _ -> acc
  | Collection _ -> acc
  | BracedExpr _ -> acc
  | ParenthesizedExpr _ -> acc
  | PU_identifier _ -> acc

and case env acc = function
  | Default (_, b)
  | Case (_, b) ->
    block env acc b

and case_has_body = function
  | Default _ -> true
  | Case (_, []) -> false
  | Case _ -> true

and catch env acc (_, _, b) = block env acc b

and field env acc (e1, e2) =
  let acc = expr env acc e1 in
  let acc = expr env acc e2 in
  acc

and afield env acc = function
  | AFvalue e -> expr env acc e
  | AFkvalue (e1, e2) ->
    let acc = expr env acc e1 in
    let acc = expr env acc e2 in
    acc

and fun_param env acc param =
  match param.param_expr with
  | None -> acc
  | Some x -> expr env acc x

and fun_paraml env acc l = List.fold_left ~f:(fun_param env) ~init:acc l
