(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Aast
module Env = Tast_env
module Cls = Decl_provider.Class
module SN = Naming_special_names

type rty =
  | Readonly
  | Mut [@deriving show]

type ctx = {
  lenv: rty SMap.t;
  (* whether the method/function returns readonly, and a Pos.t for error messages *)
  ret_ty: (rty * Pos.t) option;
  (* Whether $this is readonly and a Pos.t for error messages *)
  this_ty: (rty * Pos.t) option;
}

let empty_ctx = { lenv = SMap.empty; ret_ty = None; this_ty = None }

let readonly_kind_to_rty = function
  | Some Ast_defs.Readonly -> Readonly
  | _ -> Mut

let rty_to_str = function
  | Readonly -> "readonly"
  | Mut -> "mutable"

let has_const_attribute user_attributes =
  List.exists user_attributes ~f:(fun ua ->
      String.equal
        (snd ua.ua_name)
        Naming_special_names.UserAttributes.uaConstFun)

let pp_rty fmt rty = Format.fprintf fmt "%s" (rty_to_str rty)

(* Debugging tool for printing the local environment. Not actually called in code *)
let pp_lenv lenv = SMap.show pp_rty lenv

let lenv_from_params (params : Tast.fun_param list) user_attributes : rty SMap.t
    =
  let result = SMap.empty in
  let constfun = has_const_attribute user_attributes in
  List.fold_left
    params
    ~f:(fun acc p ->
      SMap.add
        p.param_name
        (readonly_kind_to_rty
           ( if constfun then
             Some Ast_defs.Readonly
           else
             p.param_readonly ))
        acc)
    ~init:result

let get_local lenv id =
  match SMap.find_opt id lenv with
  | Some r -> r
  | None -> Mut

(* Returns true if rty_sub is a subtype of rty_sup.
TODO: Later, we'll have to consider the regular type as well, for example
we could allow readonly int as equivalent to an int for devX purposes *)
let subtype_rty rty_sub rty_sup =
  match (rty_sub, rty_sup) with
  | (Readonly, Mut) -> false
  | _ -> true

let param_to_rty param =
  if Typing_defs.get_fp_readonly param then
    Readonly
  else
    Mut

let check =
  object (self)
    inherit Tast_visitor.iter as super

    val mutable ctx : ctx = empty_ctx

    method ty_expr env (e : Tast.expr) : rty =
      match snd e with
      | ReadonlyExpr _ -> Readonly
      | This ->
        (match ctx.this_ty with
        | Some (r, _) -> r
        | None -> Mut)
      | Lvar (_, lid) ->
        let varname = Local_id.to_string lid in
        get_local ctx.lenv varname
      (* If you have a bunch of property accesses in a row, i.e. $x->foo->bar->baz,
        ty_expr will take linear time, and the full check may take O(n^2) time
        if we recurse on expressions in the visitor. We expect this to generally
        be quite small, though. *)
      | Obj_get (e1, e2, _, _) ->
        (match self#ty_expr env e1 with
        | Readonly -> Readonly
        | Mut ->
          (* In the mut case, we need to check if the property is marked readonly *)
          let prop_elts = self#get_prop_elts env e1 e2 in
          let readonly_prop =
            List.find ~f:Typing_defs.get_ce_readonly_prop prop_elts
          in
          Option.value_map readonly_prop ~default:Mut ~f:(fun _ -> Readonly))
      | Await e -> self#ty_expr env e
      (* $array[$x] access *)
      | Array_get (e, Some _) -> self#ty_expr env e
      (* This is only valid as an lval *)
      | Array_get (_, None) -> Mut
      | Class_get (class_id, expr, _is_prop_call) ->
        (* If any of the static props could be readonly, treat the expression as readonly *)
        let class_elts = self#get_static_prop_elts env class_id expr in
        (* Note that the empty list case (when the prop doesn't exist) returns Mut *)
        if List.exists class_elts ~f:Typing_defs.get_ce_readonly_prop then
          Readonly
        else
          Mut
      | Smethod_id _
      | Method_caller _
      | Call _ ->
        Mut
      (* All calls return mut by default, unless they are wrapped in a readonly expression *)
      | Yield _ ->
        Mut (* TODO: yield is a statement, really, not an expression. *)
      | List _ ->
        Mut (* Only appears as an lvalue; relevant in assign but not here *)
      | Cast _ -> Mut
      | Unop _ ->
        Mut (* Unop only works on value types, so they can be mutable *)
      (* All binary operators are either assignments or primitive binops, which are all value types *)
      | Binop _ -> Mut
      (* I think the right side is always a function call so this could be Mut *)
      | Pipe (_, _left, right) -> self#ty_expr env right
      | KeyValCollection (_, _, fl) ->
        if
          List.exists fl ~f:(fun (_, value) ->
              match self#ty_expr env value with
              | Readonly -> true
              | _ -> false)
        then
          Readonly
        else
          Mut
      | ValCollection (_, _, el) ->
        if
          List.exists el ~f:(fun e ->
              match self#ty_expr env e with
              | Readonly -> true
              | _ -> false)
        then
          Readonly
        else
          Mut
      | Eif (_, Some e1, e2) ->
        (* Ternaries are readonly if either side is readonly *)
        (match (self#ty_expr env e1, self#ty_expr env e2) with
        | (Readonly, _)
        | (_, Readonly) ->
          Readonly
        | _ -> Mut)
      | Eif (_, None, e2) -> self#ty_expr env e2
      | As (expr, _, _) -> self#ty_expr env expr
      | Hole (expr, _, _, _) -> self#ty_expr env expr
      | Is _ -> Mut (* Booleans are value types *)
      | Pair (_, e1, e2) ->
        (match (self#ty_expr env e1, self#ty_expr env e2) with
        | (Readonly, _)
        | (_, Readonly) ->
          Readonly
        | _ -> Mut)
      | New _ -> Mut (* All constructors are mutable by default *)
      (* Things that don't appear in function bodies generally *)
      | Import _
      | Callconv _ ->
        Mut
      | Lplaceholder _ -> Mut
      (* Cloning something should always result in a mutable version of it *)
      | Clone _ -> Mut
      (* These are all value types without restrictions on mutability *)
      | ExpressionTree _
      | Xml _
      | Efun _
      | Any
      | Fun_id _
      | Method_id _
      | Lfun _
      | Record _
      | FunctionPointer _
      | Null
      | True
      | False
      | Omitted
      | Id _
      | Shape _
      | EnumAtom _
      | ET_Splice _
      | Darray _
      | Varray _
      | Int _
      | Dollardollar _
      | String _
      | String2 _
      | Collection (_, _, _)
      | Tuple _
      | Float _
      | PrefixedString _ ->
        Mut
      (* Disable formatting here so I can fit all of the above in one line *)
      | Class_const _ -> Mut

    method assign env lval rval =
      let check_prop_assignment prop_elts rval =
        let mutable_prop =
          List.find
            ~f:(fun r -> not (Typing_defs.get_ce_readonly_prop r))
            prop_elts
        in
        match (mutable_prop, self#ty_expr env rval) with
        | (Some elt, Readonly) when not (Typing_defs.get_ce_readonly_prop elt)
          ->
          Errors.readonly_mismatch
            "Invalid property assignment"
            (Tast.get_position lval)
            ~reason_sub:
              [(Tast.get_position rval, "This expression is readonly")]
            ~reason_super:
              [
                ( Lazy.force elt.Typing_defs.ce_pos,
                  "But it's being assigned to a mutable property" );
              ]
        | _ -> ()
      in
      match lval with
      | (_, Array_get (array, _)) ->
        (* TODO: appending to readonly value types is technically allowed *)
        begin
          match self#ty_expr env array with
          | Readonly -> Errors.readonly_modified (Tast.get_position array)
          | Mut -> ()
        end
      | (_, Class_get (id, expr, _)) ->
        let prop_elts = self#get_static_prop_elts env id expr in
        check_prop_assignment prop_elts rval
      | (_, Obj_get (obj, get, _, _)) ->
        begin
          match self#ty_expr env obj with
          | Readonly -> Errors.readonly_modified (Tast.get_position obj)
          | Mut -> ()
        end;
        let prop_elts = self#get_prop_elts env obj get in
        (* If there's a mutable prop, then there's a chance we're assigning to one *)
        check_prop_assignment prop_elts rval
      | (_, Lvar (_, lid)) ->
        let var_ro_opt = SMap.find_opt (Local_id.to_string lid) ctx.lenv in
        begin
          match (var_ro_opt, self#ty_expr env rval) with
          | (Some Readonly, Mut) ->
            Errors.var_readonly_mismatch
              (Tast.get_position lval)
              "readonly"
              (Tast.get_position rval)
              "mutable"
          | (Some Mut, Readonly) ->
            Errors.var_readonly_mismatch
              (Tast.get_position lval)
              "mutable"
              (Tast.get_position rval)
              "readonly"
          | (None, r) ->
            (* If it's a new assignment, add to the lenv *)
            let new_lenv = SMap.add (Local_id.to_string lid) r ctx.lenv in
            ctx <- { ctx with lenv = new_lenv }
          | (Some Mut, Mut) -> ()
          | (Some Readonly, Readonly) -> ()
        end
      | (_, List el) ->
        (* List expressions require all of their lvals assigned to the readonlyness of the rval *)
        List.iter el ~f:(fun list_lval -> self#assign env list_lval rval)
      (* TODO: make this exhaustive *)
      | _ -> ()

    (* Method call invocation *)
    method method_call env caller =
      let open Typing_defs in
      match caller with
      (* Method call checks *)
      | ((_, ty), Obj_get (e1, _, _, (* is_prop_call *) false)) ->
        let receiver_rty = self#ty_expr env e1 in
        (match (receiver_rty, get_node ty) with
        | (Readonly, Tfun fty) when not (get_ft_readonly_this fty) ->
          Errors.readonly_method_call (Tast.get_position e1) (get_pos ty)
        | _ -> ())
      | _ -> ()

    (* Checks related to calling a function or method
      is_readonly is true when the call is allowed to return readonly
      TODO: handle inout
    *)
    method call
        ~is_readonly
        (env : Tast_env.t)
        (pos : Pos.t)
        (caller_ty : Tast.ty)
        (args : Tast.expr list)
        (unpacked_arg : Tast.expr option) =
      let open Typing_defs in
      (* Check that function calls which return readonly are wrapped in readonly *)
      let check_readonly_call caller_ty is_readonly =
        match get_node caller_ty with
        | Tfun fty when get_ft_returns_readonly fty ->
          if not is_readonly then
            Errors.explicit_readonly_cast
              "function call"
              pos
              (Typing_defs.get_pos caller_ty)
        | _ -> ()
      in
      (* Checks a single arg against a parameter *)
      let check_arg param arg =
        let param_rty = param_to_rty param in
        let arg_rty = self#ty_expr env arg in
        if not (subtype_rty arg_rty param_rty) then
          Errors.readonly_mismatch
            "Invalid argument"
            (Tast.get_position arg)
            ~reason_sub:
              [
                ( Tast.get_position arg,
                  "This expression is " ^ rty_to_str arg_rty );
              ]
            ~reason_super:
              [
                ( param.fp_pos,
                  "It is incompatible with this parameter, which is "
                  ^ rty_to_str param_rty );
              ];
        (* Check fty const matching for an arg *)
        match
          (get_node param.fp_type.et_type, get_node (Tast.get_type arg))
        with
        | (Tfun _, Tfun fty)
        (* Passing a nonconst function to a const parameter *)
          when get_fp_const_function param
               && not (Typing_defs.get_ft_is_const fty) ->
          Errors.readonly_mismatch
            "Invalid argument"
            (Tast.get_position arg)
            ~reason_sub:
              [
                ( Tast.get_position arg,
                  "This function is not marked <<__ConstFun>>" );
              ]
            ~reason_super:
              [
                ( param.fp_pos,
                  "It is incompatible with this parameter, which is marked <<__ConstFun>>"
                );
              ]
        | _ -> ()
      in

      (* Check that readonly arguments match their parameters *)
      let check_args caller_ty args unpacked_arg =
        match get_node caller_ty with
        | Tfun fty ->
          let unpacked_rty = Option.to_list unpacked_arg in
          let args = args @ unpacked_rty in
          (* If the args are unequal length, we errored elsewhere so this does not care *)
          let _ = List.iter2 fty.ft_params args ~f:check_arg in
          ()
        | _ -> ()
      in
      check_readonly_call caller_ty is_readonly;
      check_args caller_ty args unpacked_arg

    method grab_class_elts_from_ty ~static env ty prop_id =
      let open Typing_defs in
      match get_node ty with
      | Tclass (id, _exact, _args) ->
        let provider_ctx = Tast_env.get_ctx env in
        let class_decl = Decl_provider.get_class provider_ctx (snd id) in
        (match class_decl with
        | Some class_decl ->
          let prop =
            if static then
              Cls.get_sprop class_decl (snd prop_id)
            else
              Cls.get_prop class_decl (snd prop_id)
          in
          Option.to_list prop
        | None -> [])
      (* Accessing a property off of an intersection type
        should involve exactly one kind of readonlyness, since for
        the intersection type to exist, the property must be related
        by some subtyping relationship anyways, and property readonlyness
        is invariant. Thus we just grab the first one from the list where the prop exists. *)
      | Tintersection [] -> []
      | Tintersection tyl ->
        (match
           List.find_map
             ~f:(fun ty ->
               match self#grab_class_elts_from_ty ~static env ty prop_id with
               | [] -> None
               | tyl -> Some tyl)
             tyl
         with
        | Some tyl -> tyl
        | None -> [])
      (* A union type is more interesting, where we must return all possible cases
      and be conservative in our use case. *)
      | Tunion tyl ->
        List.concat_map
          ~f:(fun ty -> self#grab_class_elts_from_ty ~static env ty prop_id)
          tyl
      (* Generic types can be treated similarly to an intersection type
        where we find the first prop that works from the upper bounds *)
      | Tgeneric (name, tyargs) ->
        let upper_bounds = Tast_env.get_upper_bounds env name tyargs in
        (match
           List.find_map
             ~f:(fun ty ->
               match self#grab_class_elts_from_ty ~static env ty prop_id with
               | [] -> None
               | tyl -> Some tyl)
             (Typing_set.elements upper_bounds)
         with
        | Some tyl -> tyl
        | None -> [])
      (* TODO: Handle more complex types *)
      | _ -> []

    (* Return a list of possible static prop elts given a class_get expression *)
    method get_static_prop_elts env class_id get =
      let (_, ty) = fst class_id in
      match get with
      | CGstring prop_id ->
        self#grab_class_elts_from_ty ~static:true env ty prop_id
      (* An expression is dynamic, so there's no way to tell the type generally *)
      | CGexpr _ -> []

    (* Return a list of possible prop elts given an obj get expression *)
    method get_prop_elts env obj get =
      let ty = Tast.get_type obj in
      match get with
      | (_, Id prop_id) ->
        self#grab_class_elts_from_ty ~static:false env ty prop_id
      (* TODO: Handle more complex  cases *)
      | _ -> []

    method obj_get env obj get =
      let prop_elts = self#get_prop_elts env obj get in
      (* If there's any property in the list of possible properties that could be readonly,
        it must be explicitly cast to readonly *)
      let readonly_prop =
        List.find ~f:Typing_defs.get_ce_readonly_prop prop_elts
      in
      match (readonly_prop, self#ty_expr env obj) with
      | (Some elt, Mut) when Typing_defs.get_ce_readonly_prop elt ->
        Errors.explicit_readonly_cast
          "property"
          (Tast.get_position get)
          (Lazy.force elt.Typing_defs.ce_pos)
      | _ -> ()

    (* TODO: support obj get on generics, aliases and expression dependent types *)
    method! on_method_ env m =
      let method_pos = fst m.m_name in
      let ret_pos = Typing_defs.get_pos (fst m.m_ret) in
      let this_ty =
        if m.m_readonly_this then
          Some (Readonly, method_pos)
        else
          Some (Mut, method_pos)
      in
      let new_ctx =
        {
          this_ty;
          ret_ty = Some (readonly_kind_to_rty m.m_readonly_ret, ret_pos);
          lenv = lenv_from_params m.m_params m.m_user_attributes;
        }
      in
      ctx <- new_ctx;
      super#on_method_ env m

    method! on_fun_def env f =
      let ret_pos = Typing_defs.get_pos (fst f.f_ret) in
      let ret_ty = Some (readonly_kind_to_rty f.f_readonly_ret, ret_pos) in
      let new_ctx =
        {
          this_ty = None;
          ret_ty;
          lenv = lenv_from_params f.f_params f.f_user_attributes;
        }
      in
      ctx <- new_ctx;
      super#on_fun_def env f

    (* Normal functions go through on_fun_def, but all functions including closures go through on_fun_*)
    method! on_fun_ env f =
      (* Copy the old ctx *)
      let ret_pos = Typing_defs.get_pos (fst f.f_ret) in
      match ctx.ret_ty with
      (* If the ret pos is the same between both functions,
          then this is just a fun_def, so ctx is correct already. Don't need to do anything *)
      | Some (_, outer_ret) when Pos.equal outer_ret ret_pos ->
        super#on_fun_ env f
      | _ ->
        (* Keep the old context for use later *)
        let old_ctx = ctx in
        (* First get the lenv from parameters, which override captured values *)
        let is_const = has_const_attribute f.f_user_attributes in
        (* If the lambda is const, we need to treat the entire lenv as if it is readonly, and all parameters as readonly *)
        let old_lenv =
          if is_const then
            SMap.map (fun _ -> Readonly) ctx.lenv
          else
            ctx.lenv
        in
        let new_lenv = lenv_from_params f.f_params f.f_user_attributes in
        let new_lenv = SMap.union new_lenv old_lenv in
        let new_ctx =
          {
            this_ty = None;
            ret_ty = Some (readonly_kind_to_rty f.f_readonly_ret, ret_pos);
            lenv = new_lenv;
          }
        in
        ctx <- new_ctx;
        let result = super#on_fun_ env f in
        (* Set the old context back *)
        ctx <- old_ctx;
        result

    method! on_Foreach env e as_e b =
      (* foreach ($vec as $x)
        The as expression always has the same readonlyness
        as the collection in question. If it is readonly,
        then the as expression's lvals are each assigned to readonly.
      *)
      (match as_e with
      | As_v lval
      | Await_as_v (_, lval) ->
        self#assign env lval e
      | As_kv (l1, l2)
      | Await_as_kv (_, l1, l2) ->
        self#assign env l1 e;
        self#assign env l2 e);
      super#on_Foreach env e as_e b

    method! on_expr env e =
      match e with
      (* Property assignment *)
      | ( _,
          Binop
            ( (Ast_defs.Eq _ as bop),
              ((_, Obj_get (obj, get, nullable, is_prop_call)) as lval),
              rval ) ) ->
        self#assign env lval rval;
        self#on_bop env bop;
        (* During a property assignment, skip the self#expr call to avoid erroring *)
        self#on_Obj_get env obj get nullable is_prop_call;
        self#on_expr env rval
      (* All other assignment *)
      | (_, Binop (Ast_defs.Eq _, lval, rval)) ->
        self#assign env lval rval;
        super#on_expr env e
      (* Readonly calls *)
      | (_, ReadonlyExpr (_, Call (caller, targs, args, unpacked_arg))) ->
        self#call
          ~is_readonly:true
          env
          (Tast.get_position caller)
          (Tast.get_type caller)
          args
          unpacked_arg;
        self#method_call env caller;
        (* Skip the recursive step into ReadonlyExpr to avoid erroring *)
        self#on_Call env caller targs args unpacked_arg
      (* Non readonly calls *)
      | (_, Call (caller, _, args, unpacked_arg)) ->
        self#call
          env
          ~is_readonly:false
          (Tast.get_position caller)
          (Tast.get_type caller)
          args
          unpacked_arg;
        self#method_call env caller;
        super#on_expr env e
      | (_, ReadonlyExpr (_, Obj_get (obj, get, nullable, is_prop_call))) ->
        (* Skip the recursive step into ReadonlyExpr to avoid erroring *)
        self#on_Obj_get env obj get nullable is_prop_call
      | (_, Obj_get (obj, get, _nullable, _is_prop_call)) ->
        self#obj_get env obj get;
        super#on_expr env e
      | (_, New (_, _, args, unpacked_arg, (pos, constructor_fty))) ->
        (* Constructors never return readonly, so that specific check is irrelevant *)
        self#call ~is_readonly:false env pos constructor_fty args unpacked_arg
      | (_, This)
      | (_, ValCollection (_, _, _))
      | (_, KeyValCollection (_, _, _))
      | (_, Lvar _)
      | (_, Clone _)
      | (_, Array_get (_, _))
      | (_, Class_get (_, _, _))
      | (_, Yield _)
      | (_, Await _)
      | (_, Tuple _)
      | (_, List _)
      | (_, Cast (_, _))
      | (_, Unop (_, _))
      | (_, Pipe (_, _, _))
      | (_, Eif (_, _, _))
      | (_, Is (_, _))
      | (_, As (_, _, _))
      | (_, Callconv (_, _))
      | (_, Import (_, _))
      | (_, Lplaceholder _)
      | (_, Pair (_, _, _))
      | (_, ReadonlyExpr _)
      | (_, Binop _)
      | (_, ExpressionTree _)
      | (_, Xml _)
      | (_, Efun _)
      | (_, Any)
      (* Neither this nor any of the *_id expressions call the function *)
      | (_, Method_caller (_, _))
      | (_, Smethod_id (_, _))
      | (_, Fun_id _)
      | (_, Method_id _)
      | (_, FunctionPointer _)
      | (_, Lfun _)
      | (_, Record _)
      | (_, Null)
      | (_, True)
      | (_, False)
      | (_, Omitted)
      | (_, Id _)
      | (_, Shape _)
      | (_, EnumAtom _)
      | (_, ET_Splice _)
      | (_, Darray _)
      | (_, Varray _)
      | (_, Int _)
      | (_, Dollardollar _)
      | (_, String _)
      | (_, String2 _)
      | (_, Collection (_, _, _))
      | (_, Class_const _)
      | (_, Float _)
      | (_, PrefixedString _)
      | (_, Hole _) ->
        super#on_expr env e

    method! on_stmt_ env s =
      (match s with
      | Return (Some e) ->
        (match ctx.ret_ty with
        | Some (ret_ty, pos) when not (subtype_rty (self#ty_expr env e) ret_ty)
          ->
          Errors.readonly_mismatch
            "Invalid return"
            (Tast.get_position e)
            ~reason_sub:[(Tast.get_position e, "This expression is readonly")]
            ~reason_super:[(pos, "But this function does not return readonly.")]
        (* If we don't have a ret ty we're not in a function, must have errored somewhere else *)
        | _ -> ())
      | Return None -> ()
      | Throw e ->
        (match self#ty_expr env e with
        | Readonly -> Errors.readonly_exception (Tast.get_position e)
        | Mut -> ())
      (* An awaitall contains assignment expressions that are recursed *)
      | Awaitall (_, _) -> ()
      | Break
      | Continue
      | Yield_break
      | Noop
      | Expr _
      | If (_, _, _)
      | Do (_, _)
      | While (_, _)
      | Using _
      | For (_, _, _, _) (* Inner assignments are handled by recursive step *)
      | Switch (_, _)
      | Try (_, _, _)
      | Block _
      | Markup _
      | AssertEnv (_, _)
      | Fallthrough
      (* Handled by on_Foreach *)
      | Foreach _ ->
        ());
      super#on_stmt_ env s
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_method_ env m =
      let tcopt = Tast_env.get_tcopt env in
      if TypecheckerOptions.readonly tcopt then
        check#on_method_ env m
      else
        ()

    method! at_fun_def env f =
      let tcopt = Tast_env.get_tcopt env in
      if TypecheckerOptions.readonly tcopt then
        check#on_fun_def env f
      else
        ()
  end
