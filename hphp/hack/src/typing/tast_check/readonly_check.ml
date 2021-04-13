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
module MakeType = Typing_make_type
module Reason = Typing_reason

type rty =
  | Readonly
  | Mut [@deriving show]

type ctx = {
  lenv: rty SMap.t;
  (* whether the method/function returns readonly, and a Pos.t for error messages *)
  ret_ty: (rty * Pos_or_decl.t) option;
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

let merge_lenvs left right =
  let meet left right =
    match (left, right) with
    | (Mut, Mut) -> Mut
    | _ -> Readonly
  in
  (* We can't assume that the variable is not defined if it's not in one of the envs
     because the typechecker has smarter flow analysis than the readonly analysis (including
     terminality checks). Therefore, we should take the readonlyness from the branch that has
     a value, in case it's readonly.
   *)
  SMap.merge (fun _key -> Option.merge ~f:meet) left right

let pp_rty fmt rty = Format.fprintf fmt "%s" (rty_to_str rty)

(* Debugging tool for printing the local environment. Not actually called in code *)
let pp_lenv lenv = SMap.show pp_rty lenv

let lenv_from_params (params : Tast.fun_param list) : rty SMap.t =
  let result = SMap.empty in
  List.fold_left
    params
    ~f:(fun acc p ->
      SMap.add p.param_name (readonly_kind_to_rty p.param_readonly) acc)
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

let lenv_eq l1 l2 =
  SMap.equal
    (fun r1 r2 ->
      match (r1, r2) with
      | (Mut, Mut)
      | (Readonly, Readonly) ->
        true
      | _ -> false)
    l1
    l2

let check =
  object (self)
    inherit Tast_visitor.iter as super

    val mutable ctx : ctx = empty_ctx

    method run_with_lenv f lenv =
      ctx <- { ctx with lenv };
      let result = f () in
      let new_lenv = ctx.lenv in
      (result, new_lenv)

    method handle_single_block env b old_lenv =
      self#run_with_lenv (fun () -> self#on_block env b) old_lenv

    (* f_loop is a function that executes analysis on a single loop iteration
      Handle loop will iterate f_loop a maximum of X times, where X is the number of
      mutable variables in the lenv. Each loop, we check if the lenv has changed at
      all; if not, we can return early. Since each iteration of the loop must naturally
      make a mutable variable readonly, we only need to iterate a maximum X times to reach
      a fixed point.
      TODO: This may need to change once we have more than just two types, but with a constant
      number of types, it should generally be pretty scalable.
    *)
    method handle_loop f_loop =
      let rec iter_fixed_point lenv =
        (* Run the loop once and merge the lenv *)
        let (_, iter_lenv) = self#run_with_lenv f_loop lenv in
        let new_lenv = merge_lenvs lenv iter_lenv in
        (* If the merged lenv is equivalent to the old one we can stop  *)
        (* Note, this is O(m*f), where m is the number of vars and f is the cost of f_loop
          which can lead to exponential behavior in the case of a huge number of nested loops.
          TODO: break early if we're iterating too much here and throw an error.
        *)
        if lenv_eq new_lenv lenv then
          new_lenv
        else
          iter_fixed_point new_lenv
      in
      (* We need to run the f_loop first at least once,
      to find any mutable variables created within the loop *)
      let (_, loop_lenv) = self#run_with_lenv f_loop ctx.lenv in
      let lenv = merge_lenvs ctx.lenv loop_lenv in
      (* Then, iterate at most X more times to reach a fixed point *)
      let new_lenv = iter_fixed_point lenv in
      new_lenv

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
              [
                ( Tast.get_position rval |> Pos_or_decl.of_raw_pos,
                  "This expression is readonly" );
              ]
            ~reason_super:
              [
                ( Lazy.force elt.Typing_defs.ce_pos,
                  "But it's being assigned to a mutable property" );
              ]
        | _ -> ()
      in
      match lval with
      | (_, Array_get (array, _)) ->
        begin
          match (self#ty_expr env array, self#ty_expr env rval) with
          | (Readonly, _)
            when self#is_value_collection_ty env (Tast.get_type array) ->
            ()
          | (Mut, Readonly) ->
            Errors.readonly_mismatch
              "Invalid collection modification"
              (Tast.get_position lval)
              ~reason_sub:
                [
                  ( Tast.get_position rval |> Pos_or_decl.of_raw_pos,
                    "This expression is readonly" );
                ]
              ~reason_super:
                [
                  ( Tast.get_position array |> Pos_or_decl.of_raw_pos,
                    "But this value is mutable" );
                ]
          | (Readonly, _) -> Errors.readonly_modified (Tast.get_position array)
          | (Mut, Mut) -> ()
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
        let r = self#ty_expr env rval in
        let new_lenv = SMap.add (Local_id.to_string lid) r ctx.lenv in
        ctx <- { ctx with lenv = new_lenv }
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
        (caller_rty : rty)
        (args : Tast.expr list)
        (unpacked_arg : Tast.expr option) =
      let open Typing_defs in
      let (env, caller_ty) = Tast_env.expand_type env caller_ty in
      let check_readonly_closure caller_ty caller_rty =
        match (get_node caller_ty, caller_rty) with
        | (Tfun fty, Readonly) when not (get_ft_readonly_this fty) ->
          (* Get the position of why this function is its current type (usually a typehint) *)
          let reason = get_reason caller_ty in
          let f_pos = Reason.to_pos (get_reason caller_ty) in
          let suggestion =
            match reason with
            (* If we got this function from a typehint, we suggest marking the function (readonly function) *)
            | Typing_reason.Rhint _ ->
              let new_flags =
                Typing_defs_flags.(
                  set_bit ft_flags_readonly_this true fty.ft_flags)
              in
              let readonly_fty = Tfun { fty with ft_flags = new_flags } in
              let suggested_fty = mk (reason, readonly_fty) in
              let suggested_fty_str = Tast_env.print_ty env suggested_fty in
              "annotate this typehint as a " ^ suggested_fty_str
            (* Otherwise, it's likely from a Rwitness, but we suggest declaring it as readonly *)
            | _ -> "declaring this as a `readonly` function"
          in
          Errors.readonly_closure_call pos f_pos suggestion
        | _ -> ()
      in
      (* Check that function calls which return readonly are wrapped in readonly *)
      let check_readonly_return_call caller_ty is_readonly =
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
                ( Tast.get_position arg |> Pos_or_decl.of_raw_pos,
                  "This expression is " ^ rty_to_str arg_rty );
              ]
            ~reason_super:
              [
                ( param.fp_pos,
                  "It is incompatible with this parameter, which is "
                  ^ rty_to_str param_rty );
              ]
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
      check_readonly_closure caller_ty caller_rty;
      check_readonly_return_call caller_ty is_readonly;
      check_args caller_ty args unpacked_arg

    method is_value_collection_ty env ty =
      let mixed = MakeType.mixed Reason.none in
      let env = Tast_env.tast_env_as_typing_env env in
      let hackarray = MakeType.any_array Reason.none mixed mixed in
      (* Subtype against an empty open shape (shape(...)) *)
      let shape =
        MakeType.shape
          Reason.none
          Typing_defs.Open_shape
          Typing_defs.TShapeMap.empty
      in
      Typing_utils.is_sub_type env ty hackarray
      || Typing_utils.is_sub_type env ty shape

    method grab_class_elts_from_ty ~static ?(seen = SSet.empty) env ty prop_id =
      let open Typing_defs in
      (* Given a list of types, find recurse on the first type that
        has the property and return the result *)
      let find_first_in_list ~seen tyl =
        List.find_map
          ~f:(fun ty ->
            match self#grab_class_elts_from_ty ~static ~seen env ty prop_id with
            | [] -> None
            | tyl -> Some tyl)
          tyl
      in
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
        find_first_in_list ~seen tyl |> Option.value ~default:[]
      (* A union type is more interesting, where we must return all possible cases
      and be conservative in our use case. *)
      | Tunion tyl ->
        List.concat_map
          ~f:(fun ty ->
            self#grab_class_elts_from_ty ~static ~seen env ty prop_id)
          tyl
      (* Generic types can be treated similarly to an intersection type
        where we find the first prop that works from the upper bounds *)
      | Tgeneric (name, tyargs) ->
        if SSet.mem name seen then
          []
        else
          let new_seen = SSet.add name seen in
          let upper_bounds = Tast_env.get_upper_bounds env name tyargs in
          find_first_in_list ~seen:new_seen (Typing_set.elements upper_bounds)
          |> Option.value ~default:[]
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
          lenv = lenv_from_params m.m_params;
        }
      in
      ctx <- new_ctx;
      super#on_method_ env m

    method! on_fun_def env f =
      let ret_pos = Typing_defs.get_pos (fst f.f_ret) in
      let ret_ty = Some (readonly_kind_to_rty f.f_readonly_ret, ret_pos) in
      let new_ctx =
        { this_ty = None; ret_ty; lenv = lenv_from_params f.f_params }
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
      | Some (_, outer_ret) when Pos_or_decl.equal outer_ret ret_pos ->
        super#on_fun_ env f
      | _ ->
        (* Keep the old context for use later *)
        let old_ctx = ctx in
        (* First get the lenv from parameters, which override captured values *)
        let is_readonly_this = Option.is_some f.f_readonly_this in
        (* If the lambda is readonly, we need to treat the entire lenv as if it is readonly *)
        let old_lenv =
          if is_readonly_this then
            SMap.map (fun _ -> Readonly) ctx.lenv
          else
            ctx.lenv
        in
        let new_lenv = lenv_from_params f.f_params in
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
      let f () =
        (match as_e with
        | As_v lval
        | Await_as_v (_, lval) ->
          self#assign env lval e
        | As_kv (l1, l2)
        | Await_as_kv (_, l1, l2) ->
          self#assign env l1 e;
          self#assign env l2 e);
        super#on_Foreach env e as_e b
      in
      let lenv = self#handle_loop f in
      ctx <- { ctx with lenv }

    method! on_For env expr cond incr b =
      let f () = super#on_For env expr cond incr b in
      let lenv = self#handle_loop f in
      ctx <- { ctx with lenv }

    method! on_While env expr block =
      let f () = super#on_While env expr block in
      let lenv = self#handle_loop f in
      ctx <- { ctx with lenv }

    method! on_Try env try_ clist finally =
      (* Each of the catch blocks and the try blocks should be merged together,
        with each running without knowledge of each other. *)
      let old_lenv = ctx.lenv in
      let (_, try_lenv) = self#handle_single_block env try_ old_lenv in
      (* Starting with the try_lenv, run every catch lenv and merge them into a single lenv *)
      let fold_catch acc (_, _, catch_block) =
        let (_, catch_lenv) =
          self#handle_single_block env catch_block old_lenv
        in
        merge_lenvs acc catch_lenv
      in
      let new_lenv = List.fold clist ~f:fold_catch ~init:try_lenv in
      ctx <- { ctx with lenv = new_lenv };
      (* Finally, run the finally block given our new lenv *)
      self#on_block env finally

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
          (self#ty_expr env caller)
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
          (self#ty_expr env caller)
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
        self#call
          ~is_readonly:false
          env
          pos
          constructor_fty
          Mut
          args
          unpacked_arg
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

    method! on_If env condition b1 b2 =
      let _ = self#on_expr env condition in
      let old_lenv = ctx.lenv in
      let (_, left) = self#handle_single_block env b1 old_lenv in
      let (_, right) = self#handle_single_block env b2 old_lenv in
      let new_lenv = merge_lenvs left right in
      ctx <- { ctx with lenv = new_lenv };
      ()

    method! on_stmt_ env s =
      (match s with
      | Return (Some e) ->
        (match ctx.ret_ty with
        | Some (ret_ty, pos) when not (subtype_rty (self#ty_expr env e) ret_ty)
          ->
          Errors.readonly_mismatch
            "Invalid return"
            (Tast.get_position e)
            ~reason_sub:
              [
                ( Tast.get_position e |> Pos_or_decl.of_raw_pos,
                  "This expression is readonly" );
              ]
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
      (* Do blocks are executed at least once, so we can treat it as always occurring *)
      | While (_, _)
      | Using _
      | For (_, _, _, _)
      (* Inner assignments are handled by recursive step, same with flow typing *)
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
