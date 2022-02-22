(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * This checker raises an error when:
 * - a global variable is being written (e.g. $a = Foo::$bar; $a->prop = 1;)
 * - or a global variable is passed to (or returned from) a function call.
 *
 * Notice that the return value of a memoized function (if it is mutable)
 * is treated as a global variable as well.
 *
 * By default, this checker is turned off.
 * To turn on this checker:
 * - use the argument --enable-global-write-check
 *   to specify the prefixes of files to be checked (e.g. "/" for all files).
 * - use the argument --enable-global-write-check-function
 *   to specify a JSON file of functions names to be checked.
 *   Together with --config enable_type_check_filter_files=true, this option
 *   checks specified functions within listed files.
 *)

open Hh_prelude
open Aast
module MakeType = Typing_make_type
module Reason = Typing_reason

(* The context is a list of global variables. *)
type ctx = { global_vars: string list ref }

let current_ctx = { global_vars = ref [] }

let add_var_to_ctx ctx var =
  if not (List.mem !(ctx.global_vars) var ~equal:String.equal) then
    ctx.global_vars := var :: !(ctx.global_vars)

let add_vars_to_ctx ctx vars =
  List.iter vars ~f:(fun var -> add_var_to_ctx ctx var)

let remove_vars_to_ctx ctx vars =
  ctx.global_vars :=
    List.filter !(ctx.global_vars) ~f:(fun v ->
        not (List.mem vars v ~equal:String.equal))

(* Given a context of global variables and an expression,
  check if the expression is global or not. *)
let rec is_expr_global ctx (_, _, te) =
  match te with
  | Class_get (_, _, Is_prop) -> true
  | Lvar (_, id) ->
    List.mem !(ctx.global_vars) (Local_id.to_string id) ~equal:String.equal
  | Obj_get (e, _, _, Is_prop) -> is_expr_global ctx e
  | Darray (_, tpl) -> List.exists tpl ~f:(fun (_, e) -> is_expr_global ctx e)
  | Varray (_, el) -> List.exists el ~f:(fun e -> is_expr_global ctx e)
  | Shape tpl -> List.exists tpl ~f:(fun (_, e) -> is_expr_global ctx e)
  | ValCollection (_, _, el) ->
    List.exists el ~f:(fun e -> is_expr_global ctx e)
  | KeyValCollection (_, _, fl) ->
    List.exists fl ~f:(fun (_, e) -> is_expr_global ctx e)
  | Array_get (e, _) -> is_expr_global ctx e
  | Await e -> is_expr_global ctx e
  | ReadonlyExpr e -> is_expr_global ctx e
  | Tuple el -> List.exists el ~f:(fun e -> is_expr_global ctx e)
  | List el -> List.exists el ~f:(fun e -> is_expr_global ctx e)
  | Cast (_, e) -> is_expr_global ctx e
  | Eif (_, e1, e2) ->
    (match e1 with
    | Some e -> is_expr_global ctx e
    | None -> false)
    || is_expr_global ctx e2
  | As (e, _, _) -> is_expr_global ctx e
  | Upcast (e, _) -> is_expr_global ctx e
  | Pair (_, e1, e2) -> is_expr_global ctx e1 || is_expr_global ctx e2
  | Call (caller, _, _, _) ->
    let caller_ty = Tast.get_type caller in
    let open Typing_defs in
    (match get_node caller_ty with
    | Tfun fty when get_ft_is_memoized fty -> true
    | _ -> false)
  | _ -> false

(* Check if type is a collection. *)
let is_value_collection_ty env ty =
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

(* Check if the variable type does NOT has a reference to any object:
  if so, then it is OK to write to this variable.
  Copied from is_safe_mut_ty in readonly_check.ml.
  To do: check if any change is needed for the global write checker. *)
let rec has_no_object_ref_ty env (seen : SSet.t) ty =
  let open Typing_defs_core in
  let (env, ty) = Tast_env.expand_type env ty in
  match get_node ty with
  (* Allow all primitive types *)
  | Tprim _ -> true
  (* Open shapes can technically have objects in them, but as long as the current fields don't have objects in them
     we will allow you to call the function. Note that the function fails at runtime if any shape fields are objects. *)
  | Tshape (_, fields) ->
    TShapeMap.for_all
      (fun _k v -> has_no_object_ref_ty env seen v.sft_ty)
      fields
  (* If it's a Tclass it's an array type by is_value_collection *)
  | Tintersection tyl ->
    List.exists tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  (* Only error if there isn't a type that it could be that's primitive *)
  | Tunion tyl -> List.exists tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  | Ttuple tyl -> List.for_all tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  | Tdependent (_, upper) ->
    (* check upper bounds *)
    has_no_object_ref_ty env seen upper
  | Tclass ((_, id), _, tyl)
    when is_value_collection_ty env ty || String.equal id "\\HH\\Awaitable" ->
    List.for_all tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  | Tgeneric (name, tyargs) ->
    (* Avoid circular generics with a set *)
    if SSet.mem name seen then
      false
    else
      let new_seen = SSet.add name seen in
      let upper_bounds = Tast_env.get_upper_bounds env name tyargs in
      Typing_set.exists
        (fun l -> has_no_object_ref_ty env new_seen l)
        upper_bounds
  | _ ->
    (* Otherwise, check if there's any primitive type it could be *)
    let env = Tast_env.tast_env_as_typing_env env in
    let primitive_types =
      [
        MakeType.bool Reason.none;
        MakeType.int Reason.none;
        MakeType.arraykey Reason.none;
        MakeType.string Reason.none;
        MakeType.float Reason.none;
        MakeType.num Reason.none;
        (* Keysets only contain arraykeys so if they're readonly its safe to remove *)
        MakeType.keyset Reason.none (MakeType.arraykey Reason.none);
        (* We don't put null here because we want to exclude ?Foo.
           as_mut(null) itself is allowed by the Tprim above*)
      ]
    in
    (* Make sure that a primitive *could* be this type by intersecting all primitives and subtyping. *)
    let union = MakeType.union Reason.none primitive_types in
    not (Typing_subtype.is_type_disjoint env ty union)

let is_expr_global_and_mutable env ctx (tp, p, te) =
  is_expr_global ctx (tp, p, te) && not (has_no_object_ref_ty env SSet.empty tp)

(* Given an expression that appears on LHS of an assignment,
  this method gets the list of variables whose value may be assigned. *)
let rec get_vars_in_expr vars (_, _, te) =
  match te with
  | Lvar (_, id) -> vars := [Local_id.to_string id] @ !vars
  | Obj_get (e, _, _, Is_prop) -> get_vars_in_expr vars e
  | Array_get (e, _) -> get_vars_in_expr vars e
  | ReadonlyExpr e -> get_vars_in_expr vars e
  | List el -> List.iter el ~f:(get_vars_in_expr vars)
  | _ -> ()

(* Suppose te is on LHS of an assignment, check if we can write to global variables
  by accessing either directly static variables or an object's properties. *)
let rec has_global_write_access (_, _, te) =
  match te with
  | Class_get (_, _, Is_prop)
  | Obj_get (_, _, _, Is_prop) ->
    true
  | List el -> List.exists el ~f:has_global_write_access
  | Lvar _
  | ReadonlyExpr _
  | Array_get _
  | _ ->
    false

let visitor =
  object (self)
    inherit [_] Tast_visitor.iter_with_state as super

    method! on_method_ (env, ctx) m =
      ctx.global_vars := [];
      super#on_method_ (env, ctx) m

    method! on_fun_def (env, ctx) f =
      ctx.global_vars := [];
      super#on_fun_def (env, ctx) f

    method! on_fun_ (env, ctx) f =
      let ctx_cpy = !(ctx.global_vars) in
      super#on_fun_ (env, ctx) f;
      ctx.global_vars := ctx_cpy

    method! on_stmt_ (env, ctx) s =
      match s with
      | If (_, b1, b2) ->
        (* Union the contexts from two branches *)
        let ctx1 = { global_vars = ref !(ctx.global_vars) } in
        super#on_block (env, ctx1) b1;
        super#on_block (env, ctx) b2;
        add_vars_to_ctx ctx !(ctx1.global_vars)
      | Return r ->
        (match r with
        | Some ((_, p, _) as e) ->
          if is_expr_global_and_mutable env ctx e then
            Errors.global_var_in_fun_call_error p
        | None ->
          ();
          super#on_stmt_ (env, ctx) s)
      | _ -> super#on_stmt_ (env, ctx) s

    method! on_expr (env, ctx) ((_, p, e) as te) =
      (match e with
      | Binop (Ast_defs.Eq _, le, re) ->
        let () = self#on_expr (env, ctx) re in
        let is_le_global = is_expr_global ctx le in
        let is_re_global_and_mutable = is_expr_global_and_mutable env ctx re in
        let vars_in_le = ref [] in
        let () = get_vars_in_expr vars_in_le le in
        (match has_global_write_access le with
        | true -> if is_le_global then Errors.global_var_write_error p
        | false ->
          if is_le_global && not is_re_global_and_mutable then
            remove_vars_to_ctx ctx !vars_in_le);
        if is_re_global_and_mutable then add_vars_to_ctx ctx !vars_in_le
      | Unop (op, e) ->
        (match op with
        | Ast_defs.Uincr
        | Ast_defs.Udecr
        | Ast_defs.Upincr
        | Ast_defs.Updecr ->
          if has_global_write_access e && is_expr_global ctx e then
            Errors.global_var_write_error p
        | _ -> ())
      | Call (_, _, tpl, _) ->
        (* Check if a global variable is used as the parameter. *)
        List.iter tpl ~f:(fun (pk, ((_, pos, _) as expr)) ->
            match pk with
            | Ast_defs.Pinout _ ->
              if is_expr_global ctx expr then
                Errors.global_var_in_fun_call_error pos
            | Ast_defs.Pnormal ->
              if is_expr_global_and_mutable env ctx expr then
                Errors.global_var_in_fun_call_error pos)
      | New (_, _, el, _, _) ->
        List.iter el ~f:(fun ((_, pos, _) as expr) ->
            if is_expr_global_and_mutable env ctx expr then
              Errors.global_var_in_fun_call_error pos)
      | _ -> ());
      super#on_expr (env, ctx) te
  end

let global_write_check_enabled_on_file tcopt file =
  let enabled_paths = TypecheckerOptions.global_write_check_enabled tcopt in
  let path = "/" ^ Relative_path.suffix file in
  List.exists enabled_paths ~f:(fun prefix ->
      String_utils.string_starts_with path prefix)

let global_write_check_enabled_on_function tcopt function_name =
  let enabled_functions =
    TypecheckerOptions.global_write_check_functions_enabled tcopt
  in
  (* Function name starts with '\' or ';' which is not included in JSON *)
  let function_name =
    String.sub function_name ~pos:1 ~len:(String.length function_name - 1)
  in
  SSet.mem function_name enabled_functions

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_method_ env m =
      let class_name = Tast_env.get_self_id env in
      let (_, method_name) = m.m_name in
      let full_name =
        match class_name with
        | Some s -> s ^ "::" ^ method_name
        | _ -> method_name
      in
      if
        global_write_check_enabled_on_file
          (Tast_env.get_tcopt env)
          (Tast_env.get_file env)
        || global_write_check_enabled_on_function
             (Tast_env.get_tcopt env)
             full_name
      then
        visitor#on_method_ (env, current_ctx) m

    method! at_fun_def env f =
      let (_, function_name) = f.fd_fun.f_name in
      if
        global_write_check_enabled_on_file
          (Tast_env.get_tcopt env)
          (Tast_env.get_file env)
        || global_write_check_enabled_on_function
             (Tast_env.get_tcopt env)
             function_name
      then
        visitor#on_fun_def (env, current_ctx) f
  end
