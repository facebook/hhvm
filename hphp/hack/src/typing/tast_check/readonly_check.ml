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
module Cls = Decl_provider.Class
module SN = Naming_special_names
module MakeType = Typing_make_type
module Reason = Typing_reason

(* Create a synthetic function type out of two fun_ty that has the correct readonlyness by being conservative *)
let union_fty_readonly
    (fty1 : Typing_defs.locl_ty Typing_defs.fun_type)
    (fty2 : Typing_defs.locl_ty Typing_defs.fun_type) :
    Typing_defs.locl_ty Typing_defs.fun_type =
  let open Typing_defs in
  let union_fp_readonly fp1 fp2 =
    match (get_fp_readonly fp1, get_fp_readonly fp2) with
    | (true, _) -> fp2 (* Must both be readonly to be readonly *)
    | (false, _) -> fp1
  in

  (* Not readonly *)

  (* Must both be readonly to be considered a readonly call *)
  let fty_readonly_this =
    get_ft_readonly_this fty1 && get_ft_readonly_this fty2
  in
  (* If either are readonly, consider readonly return *)
  let fty_returns_readonly =
    get_ft_returns_readonly fty1 || get_ft_returns_readonly fty2
  in
  let fty = set_ft_readonly_this fty1 fty_readonly_this in
  let fty = set_ft_returns_readonly fty fty_returns_readonly in
  let fps = List.map2 fty1.ft_params fty2.ft_params ~f:union_fp_readonly in
  (* If lenghts unequal we have other problems and errors, just return the first one *)
  let fps =
    match fps with
    | List.Or_unequal_lengths.Unequal_lengths -> fty1.ft_params
    | List.Or_unequal_lengths.Ok fp -> fp
  in
  { fty with Typing_defs.ft_params = fps }

let rec get_fty ty =
  let open Typing_defs in
  match get_node ty with
  | Tnewtype (name, _, ty2) when String.equal name SN.Classes.cSupportDyn ->
    get_fty ty2
  | Tfun fty -> Some fty
  | Tunion tyl ->
    (* Filter out dynamic types *)
    let ftys = List.filter_map tyl ~f:get_fty in
    (* Because Typing_union already aggressively simplifies function unions to a single function type,
       there should be a single function type here 99% of the time. *)
    (match ftys with
    (* Not calling a function, ignore *)
    | [] -> None
    (* In the rare case where union did not simplify, use readonly union to merge all fty's together with a union into a single function type, and return it. *)
    | fty1 :: rest ->
      let result = List.fold ~init:fty1 ~f:union_fty_readonly rest in
      Some result)
  | _ -> None

type rty =
  | Readonly
  | Mut [@deriving show]

(* Returns true if rty_sub is a subtype of rty_sup.
   TODO: Later, we'll have to consider the regular type as well, for example
   we could allow readonly int as equivalent to an int for devX purposes.
   This would require TIC to handle correctly, though. *)
let subtype_rty rty_sub rty_sup =
  match (rty_sub, rty_sup) with
  | (Readonly, Mut) -> false
  | _ -> true

let param_to_rty param =
  if Typing_defs.get_fp_readonly param then
    Readonly
  else
    Mut

let rec grab_class_elts_from_ty ~static ?(seen = SSet.empty) env ty prop_id =
  let open Typing_defs in
  (* Given a list of types, find recurse on the first type that
     has the property and return the result *)
  let find_first_in_list ~seen tyl =
    List.find_map
      ~f:(fun ty ->
        match grab_class_elts_from_ty ~static ~seen env ty prop_id with
        | [] -> None
        | tyl -> Some tyl)
      tyl
  in
  match get_node ty with
  | Tclass (id, _exact, _args) ->
    let provider_ctx = Tast_env.get_ctx env in
    let class_decl = Decl_provider.get_class provider_ctx (snd id) in
    (match class_decl with
    | Decl_entry.Found class_decl ->
      let prop =
        if static then
          Cls.get_sprop class_decl (snd prop_id)
        else
          Cls.get_prop class_decl (snd prop_id)
      in
      Option.to_list prop
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      [])
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
      ~f:(fun ty -> grab_class_elts_from_ty ~static ~seen env ty prop_id)
      tyl
  (* Generic types can be treated similarly to an intersection type
     where we find the first prop that works from the upper bounds *)
  | Tgeneric (name, tyargs) ->
    (* Avoid circular generics with a set *)
    if SSet.mem name seen then
      []
    else
      let new_seen = SSet.add name seen in
      let upper_bounds = Tast_env.get_upper_bounds env name tyargs in
      find_first_in_list ~seen:new_seen (Typing_set.elements upper_bounds)
      |> Option.value ~default:[]
  | Tdependent (_, ty) ->
    (* Dependent types have an upper bound that's a class or generic *)
    grab_class_elts_from_ty ~static ~seen env ty prop_id
  | Toption ty ->
    (* If it's nullable, take the *)
    grab_class_elts_from_ty ~static ~seen env ty prop_id
  | _ -> []

(* Return a list of possible static prop elts given a class_get expression *)
let get_static_prop_elts env class_id get =
  let (ty, _, _) = class_id in
  match get with
  | CGstring prop_id -> grab_class_elts_from_ty ~static:true env ty prop_id
  (* An expression is dynamic, so there's no way to tell the type generally *)
  | CGexpr _ -> []

(* Return a list of possible prop elts given an obj get expression *)
let get_prop_elts env obj get =
  let (env, ty) = Tast_env.expand_type env (Tast.get_type obj) in
  match get with
  | (_, _, Id prop_id) -> grab_class_elts_from_ty ~static:false env ty prop_id
  (* TODO: Handle more complex  cases *)
  | _ -> []

let rec ty_expr env ((_, _, expr_) : Tast.expr) : rty =
  match expr_ with
  | ReadonlyExpr _ -> Readonly
  (* Obj_get, array_get, and class_get are here for better error messages when used as lval *)
  | Obj_get (e1, e2, _, Is_prop) ->
    (match ty_expr env e1 with
    | Readonly -> Readonly
    | Mut ->
      (* In the mut case, we need to check if the property is marked readonly *)
      let prop_elts = get_prop_elts env e1 e2 in
      let readonly_prop =
        List.find ~f:Typing_defs.get_ce_readonly_prop prop_elts
      in
      Option.value_map readonly_prop ~default:Mut ~f:(fun _ -> Readonly))
  | Class_get (class_id, expr, Is_prop) ->
    (* If any of the static props could be readonly, treat the expression as readonly *)
    let class_elts = get_static_prop_elts env class_id expr in
    (* Note that the empty list case (when the prop doesn't exist) returns Mut *)
    if List.exists class_elts ~f:Typing_defs.get_ce_readonly_prop then
      Readonly
    else
      Mut
  | Array_get (array, _) -> ty_expr env array
  | _ -> Mut

let is_value_collection_ty env ty =
  let mixed = MakeType.mixed Reason.none in
  let env = Tast_env.tast_env_as_typing_env env in
  let ty = Typing_utils.strip_dynamic env ty in
  let hackarray = MakeType.any_array Reason.none mixed mixed in
  (* Subtype against an empty open shape (shape(...)) *)
  let shape = MakeType.open_shape Reason.none Typing_defs.TShapeMap.empty in
  Typing_utils.is_sub_type env ty hackarray
  || Typing_utils.is_sub_type env ty shape

(* Check if type is safe to convert from readonly to mut
    TODO(readonly): Update to include more complex types. *)
let rec is_safe_mut_ty env (seen : SSet.t) ty =
  let open Typing_defs_core in
  let (env, ty) = Tast_env.expand_type env ty in
  match get_node ty with
  (* Allow all primitive types *)
  | Tprim _ -> true
  (* Open shapes can technically have objects in them, but as long as the current fields don't have objects in them
     we will allow you to call the function. Note that the function fails at runtime if any shape fields are objects. *)
  | Tshape { s_fields = fields; _ } ->
    TShapeMap.for_all (fun _k v -> is_safe_mut_ty env seen v.sft_ty) fields
  (* If it's a Tclass it's an array type by is_value_collection *)
  | Tintersection tyl -> List.exists tyl ~f:(fun l -> is_safe_mut_ty env seen l)
  (* Only error if there isn't a type that it could be that's primitive *)
  | Tunion tyl -> List.exists tyl ~f:(fun l -> is_safe_mut_ty env seen l)
  | Ttuple tyl -> List.for_all tyl ~f:(fun l -> is_safe_mut_ty env seen l)
  | Tdependent (_, upper) ->
    (* check upper bounds *)
    is_safe_mut_ty env seen upper
  | Tclass (_, _, tyl) when is_value_collection_ty env ty ->
    List.for_all tyl ~f:(fun l -> is_safe_mut_ty env seen l)
  | Tgeneric (name, tyargs) ->
    (* Avoid circular generics with a set *)
    if SSet.mem name seen then
      false
    else
      let new_seen = SSet.add name seen in
      let upper_bounds = Tast_env.get_upper_bounds env name tyargs in
      Typing_set.exists (fun l -> is_safe_mut_ty env new_seen l) upper_bounds
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

(* Check that function calls which return readonly are wrapped in readonly *)
let rec check_readonly_return_call env pos caller_ty is_readonly =
  if is_readonly then
    ()
  else
    let open Typing_defs in
    match get_node caller_ty with
    | Tunion tyl ->
      List.iter tyl ~f:(fun ty ->
          check_readonly_return_call env pos ty is_readonly)
    | _ ->
      (match get_fty caller_ty with
      | Some fty when get_ft_returns_readonly fty ->
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            readonly
            @@ Primary.Readonly.Explicit_readonly_cast
                 {
                   pos;
                   kind = `fn_call;
                   decl_pos = Typing_defs.get_pos caller_ty;
                 })
      | _ -> ())

let check_readonly_property env obj get obj_ro =
  let open Typing_defs in
  let prop_elts = get_prop_elts env obj get in
  (* If there's any property in the list of possible properties that could be readonly,
      it must be explicitly cast to readonly *)
  let readonly_prop = List.find ~f:get_ce_readonly_prop prop_elts in
  match (readonly_prop, obj_ro) with
  | (Some elt, Mut) ->
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(
        readonly
        @@ Primary.Readonly.Explicit_readonly_cast
             {
               pos = Tast.get_position get;
               kind = `property;
               decl_pos = Lazy.force elt.ce_pos;
             })
  | _ -> ()

let check_static_readonly_property pos env (class_ : Tast.class_id) get obj_ro =
  let prop_elts = get_static_prop_elts env class_ get in
  (* If there's any property in the list of possible properties that could be readonly,
      it must be explicitly cast to readonly *)
  let readonly_prop = List.find ~f:Typing_defs.get_ce_readonly_prop prop_elts in
  match (readonly_prop, obj_ro) with
  | (Some elt, Mut) when Typing_defs.get_ce_readonly_prop elt ->
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(
        readonly
        @@ Primary.Readonly.Explicit_readonly_cast
             {
               pos;
               kind = `static_property;
               decl_pos = Lazy.force elt.Typing_defs.ce_pos;
             })
  | _ -> ()

let is_method_caller (caller : Tast.expr) =
  match caller with
  | (_, _, ReadonlyExpr (_, _, Obj_get (_, _, _, Is_method)))
  | (_, _, Obj_get (_, _, _, Is_method)) ->
    true
  | _ -> false

let is_special_builtin = function
  (* none of these functions require readonly checks, and can take in readonly values safely *)
  | "HH\\dict"
  | "HH\\varray"
  | "HH\\darray"
  | "HH\\vec"
  | "HH\\keyset"
  | "hphp_array_idx" ->
    true
  | _ -> false

let rec assign env lval rval =
  (* Check that we're assigning a readonly value to a readonly property *)
  let check_ro_prop_assignment prop_elts =
    let mutable_prop =
      List.find ~f:(fun r -> not (Typing_defs.get_ce_readonly_prop r)) prop_elts
    in
    match mutable_prop with
    | Some elt when not (Typing_defs.get_ce_readonly_prop elt) ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          readonly
          @@ Primary.Readonly.Readonly_mismatch
               {
                 pos = Tast.get_position lval;
                 what = `prop_assign;
                 pos_sub = Tast.get_position rval |> Pos_or_decl.of_raw_pos;
                 pos_super = Lazy.force elt.Typing_defs.ce_pos;
               })
    | _ -> ()
  in
  match lval with
  (* List assignment *)
  | (_, _, List exprs) -> List.iter exprs ~f:(fun lval -> assign env lval rval)
  | (_, _, Array_get (array, _)) -> begin
    match (ty_expr env array, ty_expr env rval) with
    | (Readonly, _) when is_value_collection_ty env (Tast.get_type array) ->
      (* In the case of (expr)[0] = rvalue, where expr is a value collection like vec,
         we need to check assignment recursively because ($x->prop)[0] is only valid if $x is mutable and prop is readonly. *)
      (match array with
      | (_, _, Array_get _)
      | (_, _, Obj_get _) ->
        assign env array rval
      | _ -> ())
    | (Mut, Readonly) ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          readonly
          @@ Primary.Readonly.Readonly_mismatch
               {
                 pos = Tast.get_position lval;
                 what = `collection_mod;
                 pos_sub = Tast.get_position rval |> Pos_or_decl.of_raw_pos;
                 pos_super = Tast.get_position array |> Pos_or_decl.of_raw_pos;
               })
    | (Readonly, _) ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          readonly
          @@ Primary.Readonly.Readonly_modified
               { pos = Tast.get_position array; reason_opt = None })
    | (Mut, Mut) -> ()
  end
  | (_, _, Class_get (id, expr, Is_prop)) ->
    (match ty_expr env rval with
    | Readonly ->
      let prop_elts = get_static_prop_elts env id expr in
      check_ro_prop_assignment prop_elts
    | _ -> ())
  | (_, _, Obj_get (obj, get, _, Is_prop)) ->
    (* Here to check for nested property accesses that are accessing readonly values *)
    begin
      match ty_expr env obj with
      | Readonly ->
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            readonly
            @@ Primary.Readonly.Readonly_modified
                 { pos = Tast.get_position obj; reason_opt = None })
      | Mut -> ()
    end;
    (match ty_expr env rval with
    | Readonly ->
      let prop_elts = get_prop_elts env obj get in
      (* If there's a mutable prop, then there's a chance we're assigning to one *)
      check_ro_prop_assignment prop_elts
    | _ -> ())
  (* TODO: make this exhaustive *)
  | _ -> ()

(* Method call invocation *)
let method_call env caller =
  let open Typing_defs in
  match caller with
  (* Readonly call checks *)
  | (ty, _, ReadonlyExpr (_, _, Obj_get (e1, _, _, Is_method))) ->
    (match get_fty ty with
    | Some fty when not (get_ft_readonly_this fty) ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          readonly
          @@ Primary.Readonly.Readonly_method_call
               { pos = Tast.get_position e1; decl_pos = get_pos ty })
    | _ -> ())
  | _ -> ()

let check_special_function env caller args =
  match (caller, args) with
  | ((_, _, Id (pos, x)), [(_, arg)])
    when String.equal (Utils.strip_ns x) (Utils.strip_ns SN.Readonly.as_mut) ->
    let arg_ty = Tast.get_type arg in
    if not (is_safe_mut_ty env SSet.empty arg_ty) then
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(readonly @@ Primary.Readonly.Readonly_invalid_as_mut pos)
    else
      ()
  | _ -> ()

(* Checks related to calling a function or method
   is_readonly is true when the call is allowed to return readonly
*)
let call
    ~is_readonly
    ~method_call
    (env : Tast_env.t)
    (pos : Pos.t)
    (caller_ty : Tast.ty)
    (caller_rty : rty)
    (args : (Ast_defs.param_kind * Tast.expr) list)
    (unpacked_arg : Tast.expr option) =
  let open Typing_defs in
  let (env, caller_ty) = Tast_env.expand_type env caller_ty in
  let check_readonly_closure caller_ty caller_rty =
    match (get_fty caller_ty, caller_rty) with
    | (Some fty, Readonly)
      when (not (get_ft_readonly_this fty)) && not method_call ->
      (* Get the position of why this function is its current type (usually a typehint) *)
      let reason = get_reason caller_ty in
      let f_pos = Reason.to_pos (get_reason caller_ty) in
      let suggestion =
        match reason with
        (* If we got this function from a typehint, we suggest marking the function (readonly function) *)
        | Typing_reason.Rhint _ ->
          let fty = Typing_defs_core.set_ft_readonly_this fty true in
          let suggested_fty = mk (reason, Tfun fty) in
          let suggested_fty_str = Tast_env.print_ty env suggested_fty in
          "annotate this typehint as a " ^ suggested_fty_str
        (* Otherwise, it's likely from a Rwitness, but we suggest declaring it as readonly *)
        | _ -> "declaring this as a `readonly` function"
      in
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          readonly
          @@ Primary.Readonly.Readonly_closure_call
               { pos; decl_pos = f_pos; suggestion })
    | _ -> ()
  in
  (* Checks a single arg against a parameter *)
  let check_arg env param (_, arg) =
    let param_rty = param_to_rty param in
    let arg_rty = ty_expr env arg in
    if not (subtype_rty arg_rty param_rty) then
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          readonly
          @@ Primary.Readonly.Readonly_mismatch
               {
                 pos = Tast.get_position arg;
                 what =
                   (match arg_rty with
                   | Readonly -> `arg_readonly
                   | Mut -> `arg_mut);
                 pos_sub = Tast.get_position arg |> Pos_or_decl.of_raw_pos;
                 pos_super = param.fp_pos;
               })
  in

  (* Check that readonly arguments match their parameters *)
  let check_args env caller_ty args unpacked_arg =
    match get_fty caller_ty with
    | Some fty ->
      let rec check args params =
        match (args, params) with
        (* Remaining args should be checked against variadic *)
        | (x1 :: args1, [x2]) when get_ft_variadic fty ->
          check_arg env x2 x1;
          check args1 [x2]
        | (x1 :: args1, x2 :: params2) ->
          check_arg env x2 x1;
          check args1 params2
        | ([], _) ->
          (* If args are empty, it's either a type error already or a default arg that's not filled in
             either way, no need to check readonlyness *)
          ()
        | (_x1 :: _args1, []) ->
          (* Too many args and no variadic: there was a type error somewhere*)
          ()
      in
      let unpacked_rty =
        unpacked_arg
        |> Option.map ~f:(fun e -> (Ast_defs.Pnormal, e))
        |> Option.to_list
      in
      let args = args @ unpacked_rty in
      check args fty.ft_params
    | None -> ()
  in
  check_readonly_closure caller_ty caller_rty;
  check_readonly_return_call env pos caller_ty is_readonly;
  check_args env caller_ty args unpacked_arg

let caller_is_special_builtin caller =
  match caller with
  | (_, _, Id (_, name)) when is_special_builtin (Utils.strip_ns name) -> true
  | _ -> false

let check =
  object (self)
    inherit Tast_visitor.iter as super

    method! on_expr env e =
      match e with
      | (_, _, Binop { bop = Ast_defs.Eq _; lhs; rhs }) ->
        assign env lhs rhs;
        self#on_expr env rhs
      | ( _,
          _,
          ReadonlyExpr
            (_, _, Call ({ func; args; unpacked_arg; _ } as call_expr)) ) ->
        let default () =
          (* Skip the recursive step into ReadonlyExpr to avoid erroring *)
          self#on_Call env call_expr
        in
        if caller_is_special_builtin func then
          default ()
        else
          call
            ~is_readonly:true
            ~method_call:(is_method_caller func)
            env
            (Tast.get_position func)
            (Tast.get_type func)
            (ty_expr env func)
            args
            unpacked_arg;
        check_special_function env func args;
        method_call env func;
        default ()
      (* Non readonly calls *)
      | (_, _, Call { func; args; unpacked_arg; _ }) ->
        if caller_is_special_builtin func then
          super#on_expr env e
        else
          call
            env
            ~is_readonly:false
            ~method_call:(is_method_caller func)
            (Tast.get_position func)
            (Tast.get_type func)
            (ty_expr env func)
            args
            unpacked_arg;
        check_special_function env func args;
        method_call env func;
        super#on_expr env e
      | (_, _, ReadonlyExpr (_, _, Obj_get (obj, get, nullable, is_prop_call)))
        ->
        (* Skip the recursive step into ReadonlyExpr to avoid erroring *)
        self#on_Obj_get env obj get nullable is_prop_call
      | (_, _, ReadonlyExpr (_, _, Class_get (class_, get, x))) ->
        (* Skip the recursive step into ReadonlyExpr to avoid erroring *)
        self#on_Class_get env class_ get x
      | (_, _, Obj_get (obj, get, _nullable, Is_prop)) ->
        check_readonly_property env obj get Mut;
        super#on_expr env e
      | (_, pos, Class_get (class_, get, Is_prop)) ->
        check_static_readonly_property pos env class_ get Mut;
        super#on_expr env e
      | (_, pos, New (_, _, args, unpacked_arg, constructor_fty)) ->
        (* Constructors never return readonly, so that specific check is irrelevant *)
        call
          ~is_readonly:false
          ~method_call:false
          env
          pos
          constructor_fty
          Mut
          (List.map ~f:(fun e -> (Ast_defs.Pnormal, e)) args)
          unpacked_arg;
        super#on_expr env e
      | (_, _, Obj_get _)
      | (_, _, Class_get _)
      | (_, _, This)
      | (_, _, ValCollection (_, _, _))
      | (_, _, KeyValCollection (_, _, _))
      | (_, _, Lvar _)
      | (_, _, Clone _)
      | (_, _, Array_get (_, _))
      | (_, _, Yield _)
      | (_, _, Await _)
      | (_, _, Tuple _)
      | (_, _, List _)
      | (_, _, Cast (_, _))
      | (_, _, Unop (_, _))
      | (_, _, Pipe (_, _, _))
      | (_, _, Eif (_, _, _))
      | (_, _, Is (_, _))
      | (_, _, As _)
      | (_, _, Upcast (_, _))
      | (_, _, Import (_, _))
      | (_, _, Lplaceholder _)
      | (_, _, Pair (_, _, _))
      | (_, _, ReadonlyExpr _)
      | (_, _, Binop _)
      | (_, _, ExpressionTree _)
      | (_, _, Xml _)
      | (_, _, Efun _)
      (* Neither this nor any of the *_id expressions call the function *)
      | (_, _, Method_caller (_, _))
      | (_, _, FunctionPointer _)
      | (_, _, Lfun _)
      | (_, _, Null)
      | (_, _, True)
      | (_, _, False)
      | (_, _, Omitted)
      | (_, _, Id _)
      | (_, _, Shape _)
      | (_, _, EnumClassLabel _)
      | (_, _, ET_Splice _)
      | (_, _, Darray _)
      | (_, _, Varray _)
      | (_, _, Int _)
      | (_, _, Dollardollar _)
      | (_, _, String _)
      | (_, _, String2 _)
      | (_, _, Collection (_, _, _))
      | (_, _, Class_const _)
      | (_, _, Float _)
      | (_, _, PrefixedString _)
      | (_, _, Hole _)
      | (_, _, Nameof _)
      | (_, _, Package _) ->
        super#on_expr env e
      (* Stop at invalid marker *)
      | (_, _, Invalid _) -> ()
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    (* Ref updated before every function def *)
    val fun_has_readonly = ref false

    method! at_method_ env m =
      let env = Tast_env.restore_method_env env m in
      if Tast_env.fun_has_readonly env then (
        fun_has_readonly := true;
        check#on_method_ env m
      ) else (
        fun_has_readonly := false;
        ()
      )

    method! at_fun_def env f =
      let env = Tast_env.restore_fun_env env f.fd_fun in
      if Tast_env.fun_has_readonly env then (
        fun_has_readonly := true;
        check#on_fun_def env f
      ) else (
        fun_has_readonly := false;
        ()
      )

    (*
        The following error checks are ones that need to run even if
        readonly analysis is not enabled by the file attribute.
      *)
    method! at_Call env { func; _ } =
      (* this check is already handled by the readonly analysis,
         which handles cases when there's a readonly keyword *)
      if !fun_has_readonly then
        ()
      else
        let caller_pos = Tast.get_position func in
        let caller_ty = Tast.get_type func in
        let (_, caller_ty) = Tast_env.expand_type env caller_ty in
        check_readonly_return_call env caller_pos caller_ty false

    method! at_expr env e =
      (* this check is already handled by the readonly analysis,
         which handles cases when there's a readonly keyword *)
      let check =
        if !fun_has_readonly then
          fun _e ->
        ()
        else
          fun e ->
        let val_kind = Tast_env.get_val_kind env in
        match (e, val_kind) with
        | ((_, _, Binop { bop = Ast_defs.Eq _; lhs; rhs }), _) ->
          (* Check property assignments to make sure they're safe *)
          assign env lhs rhs
        (* Assume obj is mutable here since you can't have a readonly thing
           without readonly keyword/analysis *)
        (* Only check this for rvalues, not lvalues *)
        | ((_, _, Obj_get (obj, get, _, Is_prop)), Typing_defs.Other) ->
          check_readonly_property env obj get Mut
        | ((_, pos, Class_get (class_id, get, Is_prop)), Typing_defs.Other) ->
          check_static_readonly_property pos env class_id get Mut
        | _ -> ()
      in
      match e with
      | (_, _, Aast.Invalid _) -> ()
      | _ -> check e
  end
