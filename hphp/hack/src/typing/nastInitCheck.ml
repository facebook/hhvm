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
module Native = Typing_native

let shallow_decl_enabled (ctx : Provider_context.t) : bool =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

let use_direct_decl_parser (ctx : Provider_context.t) : bool =
  TypecheckerOptions.use_direct_decl_parser (Provider_context.get_tcopt ctx)

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

let parent_init_prop = "parent::" ^ SN.Members.__construct

let lookup_props env class_name props =
  SSet.fold
    begin
      fun name map ->
      let ty_opt =
        if String.equal name parent_init_prop then
          Some (Typing_make_type.nonnull Typing_reason.Rnone)
        else
          Typing_env.get_class env class_name
          |> Option.bind ~f:(fun cls ->
                 Typing_env.get_member false env cls name)
          |> Option.bind ~f:(fun ce -> Some (Lazy.force ce.Typing_defs.ce_type))
      in
      SMap.add name ty_opt map
    end
    props
    SMap.empty

(* If a type is missing, nullable, or dynamic, initialization is not required *)
let type_does_not_require_init env ty_opt =
  match ty_opt with
  | None -> true
  | Some ty ->
    let ((env, ty_err_opt), ty) =
      Typing_phase.localize_no_subst env ~ignore_errors:true ty
    in
    Option.iter ~f:Errors.add_typing_error ty_err_opt;
    let null = Typing_make_type.null Typing_reason.Rnone in
    Typing_subtype.is_sub_type env null ty
    ||
    let dynamic = Typing_make_type.dynamic Typing_reason.Rnone in
    Typing_subtype.is_sub_type env dynamic ty
    && Typing_subtype.is_sub_type env ty dynamic

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

let filter_props_by_type env cls props =
  lookup_props env cls props
  |> SMap.filter (fun _ ty -> not (type_does_not_require_init env ty))
  |> SMap.keys
  |> SSet.of_list

let parent_props_set_during_cstr env (c : Shallow_decl_defs.shallow_class) :
    SSet.t =
  match DeferredMembers.parents env c with
  | pc :: _ ->
    (* All properties that don't have an explicit initialiser. *)
    let (_, parent_props) = DeferredMembers.get_deferred_init_props env pc in
    (* Filter properties whose type means they don't need initialising. *)
    filter_props_by_type env (snd pc.Shallow_decl_defs.sc_name) parent_props
  | [] -> SSet.empty

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
    props: Typing_defs.decl_ty option SMap.t;
    tenv: Typing_env_types.env;
    parent_cstr_props: SSet.t;
    init_not_required_props: SSet.t;
  }

  let rec make tenv c =
    let ctx = Typing_env.get_ctx tenv in
    let (_, _, methods) = split_methods c.c_methods in
    let methods = List.fold_left ~f:method_ ~init:SMap.empty methods in
    let sc =
      lazy
        (assert (shallow_decl_enabled ctx);
         if use_direct_decl_parser ctx then
           Option.value_exn (Shallow_classes_provider.get ctx (snd c.c_name))
         else
           Shallow_decl.class_DEPRECATED ctx c)
    in

    (* In Zoncolan, we don't support eviction. We don't support lazy reparsing of
       shallow decls. If we try and the shallow decl is not available, we'll crash
       Zoncolan. Therefore, we shouldn't fallback. *)
    let ctx = Typing_env.get_ctx tenv in
    let supports_eviction =
      Provider_backend.supports_eviction (Provider_context.get_backend ctx)
    in
    let fallback =
      if not supports_eviction then
        Decl_env.no_fallback
      else
        fun _env x ->
      Option.map
        ~f:fst
        (Decl_folded_class.class_decl_if_missing ~sh:SharedMem.Uses ctx x)
    in
    let get_class_add_dep env x =
      Decl_env.get_class_and_add_dep
        ~cache:SMap.empty
        ~shmem_fallback:true
        ~fallback
        env
        x
    in

    (* Error when an abstract class has private properties but lacks a constructor *)
    let has_own_cstr =
      let (c_constructor, _, _) = split_methods c.c_methods in
      match c_constructor with
      | Some s -> not s.m_abstract
      | None -> false
    in
    let (private_props, _) =
      if shallow_decl_enabled ctx then
        DeferredMembers.class_ tenv (Lazy.force sc)
      else
        (DICheck.private_deferred_init_props ~has_own_cstr c, SSet.empty)
    in
    let private_props = lookup_props tenv (snd c.c_name) private_props in
    (if Ast_defs.is_c_abstract c.c_kind && not has_own_cstr then
      let uninit =
        SMap.filter
          (fun _ ty_opt -> not (type_does_not_require_init tenv ty_opt))
          private_props
      in
      if not @@ SMap.is_empty uninit then
        let prop_names = SMap.bindings uninit |> List.map ~f:fst
        and (pos, class_name) = c.c_name in
        Errors.add_nast_check_error
        @@ Nast_check_error.Constructor_required { pos; class_name; prop_names });

    let ( add_init_not_required_props,
          add_trait_props,
          add_parent_props,
          add_parent,
          parent_cstr_props ) =
      if shallow_decl_enabled ctx then
        let sc = Lazy.force sc in
        ( DeferredMembers.init_not_required_props sc,
          DeferredMembers.trait_props tenv sc,
          DeferredMembers.parent_props tenv sc,
          DeferredMembers.parent tenv sc,
          parent_props_set_during_cstr tenv sc )
      else
        let decl_env = tenv.Typing_env_types.decl_env in
        ( DICheck.init_not_required_props c,
          DICheck.trait_props ~get_class_add_dep decl_env c,
          DICheck.parent_props ~get_class_add_dep decl_env c,
          DICheck.parent ~get_class_add_dep decl_env c,
          DICheck.parent_initialized_members ~get_class_add_dep decl_env c
          |> filter_props_by_type tenv (snd c.c_name) )
    in
    let init_not_required_props = add_init_not_required_props SSet.empty in
    let props =
      SSet.empty
      |> DICheck.own_props c
      (* If we define our own constructor, we need to pretend any traits we use
       * did *not* define a constructor, because they are not reachable through
       * parent::__construct or similar functions. *)
      |> add_trait_props
      |> add_parent_props
      |> add_parent
      |> lookup_props tenv (snd c.c_name)
      |> SMap.filter (fun _ ty_opt ->
             not (type_does_not_require_init tenv ty_opt))
    in
    { methods; props; parent_cstr_props; tenv; init_not_required_props }

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

let is_lateinit cv =
  Naming_attributes.mem SN.UserAttributes.uaLateInit cv.cv_user_attributes

let class_prop_pos class_name prop_name ctx : Pos_or_decl.t =
  match Decl_provider.get_class ctx class_name with
  | None -> Pos_or_decl.none
  | Some decl ->
    (match Decl_provider.Class.get_prop decl prop_name with
    | None -> Pos_or_decl.none
    | Some elt ->
      let member_origin = elt.Typing_defs.ce_origin in
      if shallow_decl_enabled ctx then
        match Shallow_classes_provider.get ctx member_origin with
        | None -> Pos_or_decl.none
        | Some sc ->
          let prop =
            List.find_exn sc.Shallow_decl_defs.sc_props ~f:(fun prop ->
                String.equal (snd prop.Shallow_decl_defs.sp_name) prop_name)
          in
          fst prop.Shallow_decl_defs.sp_name
      else
        let get_class_by_name ctx x =
          let open Option.Monad_infix in
          Naming_provider.get_type_path ctx x >>= fun fn ->
          Ast_provider.find_class_in_file ctx fn x
        in
        (match get_class_by_name ctx member_origin with
        | None -> Pos_or_decl.none
        | Some cls ->
          (match
             List.find cls.Aast.c_vars ~f:(fun cv ->
                 String.equal (snd cv.Aast.cv_id) prop_name)
           with
          | None ->
            (* We found the class prop's origin via Typing_defs.ce_origin, so we
               *should* find the prop in the class. This is an invariant violation.
            *)
            HackEventLogger.invariant_violation_bug
              ~desc:
                ("nastInitCheck can't find expected class prop. class_name = "
                ^ class_name
                ^ "; member_origin = "
                ^ member_origin
                ^ "; prop_name = "
                ^ prop_name)
              ~path:Relative_path.default
              ~pos:""
              (Telemetry.create ());
            Errors.add_typing_error
              Typing_error.(
                primary
                @@ Primary.Internal_error
                     {
                       pos = Pos.none;
                       msg =
                         "Invariant violation:  please report this bug via the VSCode bug button. Expected to find prop_name "
                         ^ prop_name
                         ^ " in class "
                         ^ member_origin;
                     });
            Pos_or_decl.none
          | Some cv -> Pos_or_decl.of_raw_pos @@ fst cv.Aast.cv_id)))

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
let rec constructor env cstr =
  match cstr with
  | None -> S.empty
  | Some cstr ->
    let check_param_initializer e = ignore (expr env S.empty e) in
    List.iter cstr.m_params ~f:(fun p ->
        Option.iter p.param_expr ~f:check_param_initializer);
    let b = cstr.m_body in
    toplevel env S.empty b.fb_ast

and assign _env acc x = S.add x acc

and assign_expr env acc e1 =
  match e1 with
  | (_, _, Obj_get ((_, _, This), (_, _, Id (_, y)), _, Is_prop)) ->
    assign env acc y
  | (_, _, List el) -> List.fold_left ~f:(assign_expr env) ~init:acc el
  | _ -> acc

and argument_list env acc el =
  List.fold_left ~f:(fun acc_ (_, e) -> expr env acc_ e) ~init:acc el

and stmt env acc st =
  let expr = expr env in
  let block = block env in
  let catch = catch env in
  let case = case env in
  let default_case = default_case env in
  match snd st with
  | Expr (* only in top level!*)
      (_, _, Call ((_, _, Class_const ((_, _, CIparent), (_, m))), _, el, _uel))
    when String.equal m SN.Members.__construct ->
    let acc = argument_list env acc el in
    assign env acc DeferredMembers.parent_init_prop
  | Expr e ->
    if Typing_func_terminality.expression_exits env.tenv e then
      S.Top
    else
      expr acc e
  | Break -> acc
  | Continue -> acc
  | Throw _ -> S.Top
  | Return None ->
    if are_all_init env acc then
      acc
    else
      raise (InitReturn acc)
  | Yield_break -> S.Top
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
    let acc = List.fold_left (snd us.us_exprs) ~f:expr ~init:acc in
    block acc us.us_block
  | For (e1, _, _, _) -> exprl env acc e1
  | Switch (e, cl, dfl) ->
    let acc = expr acc e in
    (* Filter out cases that fallthrough *)
    (* NOTE: 'default' never fallthough *)
    let cl_body = List.filter cl ~f:case_has_body in
    let cl = List.map cl_body ~f:(case acc) in
    let cdfl = dfl |> Option.map ~f:(default_case acc) in
    let c = S.inter_list cl in
    let c = Option.fold ~init:c ~f:S.inter cdfl in
    S.union acc c
  | Foreach (e, _, _) ->
    let acc = expr acc e in
    acc
  | Try (b, cl, fb) ->
    let c = block acc b in
    let f = block acc fb in
    let cl = List.map cl ~f:(catch acc) in
    let c = S.inter_list (c :: cl) in
    (* the finally block executes even if *none* of try and catch do *)
    let acc = S.union acc f in
    S.union acc c
  | Fallthrough -> S.empty
  | Noop -> acc
  | Block b -> block acc b
  | Markup _ -> acc
  | AssertEnv _ -> acc

and toplevel env acc l =
  try List.fold_left ~f:(stmt env) ~init:acc l with
  | InitReturn acc -> acc

and block env acc l =
  let acc_before_block = acc in
  try List.fold_left ~f:(stmt env) ~init:acc l with
  | InitReturn _ ->
    (* The block has a return statement, forget what was initialized in it *)
    raise (InitReturn acc_before_block)

and are_all_init env set =
  SMap.fold (fun cv _ acc -> acc && S.mem cv set) env.props true

and check_all_init pos env acc =
  SMap.iter
    begin
      fun prop_name _ ->
      if not (S.mem prop_name acc) then
        Errors.add_nast_check_error
        @@ Nast_check_error.Call_before_init { pos; prop_name }
    end
    env.props

and exprl env acc l = List.fold_left ~f:(expr env) ~init:acc l

and expr env acc (_, p, e) = expr_ env acc p e

and expr_ env acc p e =
  let expr = expr env in
  let exprl = exprl env in
  let field = field env in
  let afield = afield env in
  let fun_paraml = fun_paraml env in
  match e with
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
  | EnumClassLabel _
  | Id _ ->
    acc
  | Lvar _
  | Lplaceholder _
  | Dollardollar _ ->
    acc
  | Obj_get ((_, _, This), (_, _, Id ((_, vx) as v)), _, Is_prop) ->
    if SMap.mem vx env.props && not (S.mem vx acc) then (
      let (pos, member_name) = v in
      Errors.add_typing_error
        Typing_error.(primary @@ Primary.Read_before_write { pos; member_name });
      acc
    ) else if
        SSet.mem vx env.parent_cstr_props
        && (not (SSet.mem vx env.init_not_required_props))
        && (not (S.mem vx acc))
        && not (S.mem parent_init_prop acc)
      then (
      (* We're reading a property that's initialised in the parent
         constructor, but we haven't called the parent constructor
         yet. *)
      let (pos, member_name) = v in
      Errors.add_typing_error
        Typing_error.(primary @@ Primary.Read_before_write { pos; member_name });
      acc
    ) else
      acc
  | Clone e -> expr acc e
  | Obj_get (e1, e2, _, _) ->
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
  | Call
      ( (_, p, Obj_get ((_, _, This), (_, _, Id (_, f)), _, Is_method)),
        _,
        el,
        unpacked_element ) ->
    let method_ = Env.get_method env f in
    (match method_ with
    | None ->
      check_all_init p env acc;
      acc
    | Some method_ ->
      (match !method_ with
      | Done -> acc
      | Todo b ->
        (* First time we encounter this private method. Let's check its
         * arguments first, and then recurse into the method body.
         *)
        let acc = argument_list env acc el in
        let acc =
          Option.value_map ~f:(expr acc) ~default:acc unpacked_element
        in
        method_ := Done;
        toplevel env acc b.fb_ast))
  | Call (e, _, el, unpacked_element) ->
    let el =
      match e with
      | (_, _, Id (_, fun_name)) when is_whitelisted fun_name ->
        List.filter el ~f:(function
            | (_, (_, _, This)) -> false
            | _ -> true)
      | _ -> el
    in
    let acc = argument_list env acc el in
    let acc = Option.value_map ~f:(expr acc) ~default:acc unpacked_element in
    expr acc e
  | True
  | False
  | Int _
  | Float _
  | Null
  | String _
  | String2 _
  | PrefixedString _ ->
    acc
  | Yield e -> afield acc e
  | Await e -> expr acc e
  | Tuple el -> List.fold_left ~f:expr ~init:acc el
  | List _ ->
    (* List is always an lvalue *)
    acc
  | New (_, _, el, unpacked_element, _) ->
    let acc = exprl acc el in
    let acc = Option.value_map ~default:acc ~f:(expr acc) unpacked_element in
    acc
  | Pair (_, e1, e2) ->
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
  | Upcast (e, _) -> expr acc e
  | Efun (f, _)
  | Lfun (f, _) ->
    let acc = fun_paraml acc f.f_params in
    (* We don't need to analyze the body of closures *)
    acc
  | Xml (_, l, el) ->
    let l = List.map l ~f:get_xhp_attr_expr in
    let acc = exprl acc l in
    exprl acc el
  | Shape fdm ->
    List.fold_left
      ~f:
        begin
          fun acc (_, v) ->
          expr acc v
        end
      ~init:acc
      fdm
  | ExpressionTree _ -> acc
  | Omitted -> acc
  | Import _ -> acc
  | Collection _ -> acc
  | FunctionPointer _ -> acc
  | ET_Splice e -> expr acc e
  | ReadonlyExpr e -> expr acc e
  | Hole (e, _, _, _) -> expr acc e

and case env acc ((_, b) : (_, _) Aast.case) = block env acc b

and case_has_body ((_, b) : (_, _) Aast.case) = not (List.is_empty b)

and default_case env acc ((_, b) : (_, _) Aast.default_case) = block env acc b

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

let class_ tenv c =
  if not (FileInfo.is_hhi c.c_mode) then
    List.iter c.c_vars ~f:(fun cv ->
        match cv.cv_expr with
        | Some _ when is_lateinit cv ->
          Errors.add_typing_error
            Typing_error.(
              primary @@ Primary.Lateinit_with_default (fst cv.cv_id))
        | None when cv.cv_is_static ->
          let ty_opt =
            Option.map
              ~f:(Decl_hint.hint tenv.Typing_env_types.decl_env)
              (hint_of_type_hint cv.cv_type)
          in
          if
            is_lateinit cv
            || cv.cv_abstract
            || type_does_not_require_init tenv ty_opt
          then
            ()
          else
            Errors.add_typing_error
              Typing_error.(
                wellformedness
                @@ Primary.Wellformedness.Missing_assign (fst cv.cv_id))
        | _ -> ());
  let (c_constructor, _, _) = split_methods c.c_methods in
  match c_constructor with
  | _ when Ast_defs.is_c_interface c.c_kind -> ()
  | Some _ when FileInfo.is_hhi c.c_mode -> ()
  | Some m when Native.is_native_meth ~env:tenv m ->
    (* If we're checking a `__Native` constructor then all bets are off: there's
     * no way to verify that properties are initialized correctly, including if
     * we've called parent::__construct.
     *)
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
      let uninit_props =
        SMap.filter (fun k _ -> not (SSet.mem k inits)) env.props
      in
      if not (SMap.is_empty uninit_props) then
        if SMap.mem DeferredMembers.parent_init_prop uninit_props then
          Errors.add_nast_check_error @@ Nast_check_error.No_construct_parent p
        else
          let class_uninit_props =
            SMap.filter
              (fun prop _ -> not (SSet.mem prop env.init_not_required_props))
              uninit_props
          in
          if not (SMap.is_empty class_uninit_props) then
            Errors.add_nast_check_error
            @@ Nast_check_error.Not_initialized
                 {
                   pos = p;
                   class_name = snd c.c_name;
                   props =
                     SMap.bindings class_uninit_props
                     |> List.map ~f:(fun (name, _) ->
                            let pos =
                              class_prop_pos
                                (snd c.c_name)
                                name
                                (Typing_env.get_ctx tenv)
                            in
                            (pos, name));
                 }
    in
    let check_throws_or_init_all inits =
      match inits with
      | S.Top ->
        (* Constructor always throw, so checking that all properties are
         * initialized is irrelevant. *)
        ()
      | S.Set inits -> check_inits inits
    in
    if Ast_defs.is_c_trait c.c_kind || Ast_defs.is_c_abstract c.c_kind then
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
