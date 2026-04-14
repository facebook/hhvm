(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SN = Naming_special_names

(** A type is "effectively enforced" when the runtime enforces the container
    type and all erased generic parameters are top types that cannot be
    violated. We check each type argument positionally:
    - Key positions accept [arraykey] (the key top type — keys are bounded
      at [arraykey] and can never be [mixed])
    - Value positions accept [mixed] (the global top type)

    For example, [dict<arraykey, mixed>] is effectively enforced.
    [dict<string, mixed>] is NOT — [string] is a strict subtype of [arraykey]
    that the runtime does not verify per-element. *)
let is_effectively_enforced_container (decl_ty : Typing_defs.decl_ty) : bool =
  let rec is_mixed ty =
    match Typing_defs.get_node ty with
    | Typing_defs.Tmixed -> true
    (* Look through supportdyn<mixed> which wraps mixed under SDT *)
    | Typing_defs.Tapply ((_, name), [inner])
      when String.equal name SN.Classes.cSupportDyn ->
      is_mixed inner
    | _ -> false
  in
  let is_arraykey ty =
    match Typing_defs.get_node ty with
    | Typing_defs.Tprim Aast.Tarraykey -> true
    | _ -> false
  in
  let check_args name tyl =
    match (name, tyl) with
    | (n, [kty; vty])
      when String.equal n SN.Collections.cDict
           || String.equal n SN.Collections.cKeyedContainer
           || String.equal n SN.Collections.cMap ->
      is_arraykey kty && is_mixed vty
    | (n, [ty])
      when String.equal n SN.Collections.cVec
           || String.equal n SN.Collections.cContainer
           || String.equal n SN.Collections.cVector ->
      is_mixed ty
    | (n, [ty])
      when String.equal n SN.Collections.cKeyset
           || String.equal n SN.Collections.cSet ->
      is_arraykey ty
    | _ -> false
  in
  match Typing_defs.get_node decl_ty with
  | Typing_defs.Tapply ((_, name), tyl) -> check_args name tyl
  | Typing_defs.Toption inner ->
    (match Typing_defs.get_node inner with
    | Typing_defs.Tapply ((_, name), tyl) -> check_args name tyl
    | _ -> false)
  | _ -> false

(** Extract the decl_fun_type from a decl_ty, if it's a function type. *)
let get_decl_ft (ty : Typing_defs.decl_ty) :
    Typing_defs.decl_ty Typing_defs.fun_type option =
  match Typing_defs.get_node ty with
  | Typing_defs.Tfun ft -> Some ft
  | _ -> None

(** Extract a class name from a localized type, handling simple class types,
    dependent types (e.g. from static::), and Tgeneric "this". *)
let rec get_class_name_from_ty env (ty : Typing_defs.locl_ty) : string option =
  match Typing_defs.get_node ty with
  | Typing_defs.Tclass ((_, name), _, _) -> Some name
  | Typing_defs.Tdependent (_, bound_ty) -> get_class_name_from_ty env bound_ty
  | Typing_defs.Tgeneric name when String.equal name "this" ->
    Tast_env.get_self_id env
  | _ -> None

(** Extract a class name from an inferred receiver type only when we can
    trust the inference. The receiver is trusted when:
    - The type is exact (e.g. [new C()] produces an exact [C]), or
    - The TAST was checked only once (no dynamic assumptions) and the
      type is not a like type (like types indicate the value may be dynamic).
    This allows us to resolve instance method calls and property assignments
    on non-[$this] receivers when the inferred type is reliable. *)
let get_trusted_class_name ~checked_once (ty : Typing_defs.locl_ty) :
    string option =
  match Typing_defs.get_node ty with
  | Typing_defs.Tclass ((_, name), Typing_defs.Exact, _) -> Some name
  | Typing_defs.Tclass ((_, name), _, _) when checked_once -> Some name
  | _ -> None

(** Look up a method's unpessimised fun_type from the shallow class where
    it is declared, avoiding the like-type wrapping that class folding
    applies under SDT. Uses the folded class to resolve [ce_origin] for
    inherited methods, then reads the shallow class of the origin. *)
let resolve_shallow_method_type env ctx class_name method_name ~is_static :
    Typing_defs.decl_ty Typing_defs.fun_type option =
  let cls = Tast_env.get_class env class_name |> Decl_entry.to_option in
  Option.bind cls ~f:(fun c ->
      let ce =
        if is_static then
          Tast_env.get_static_member true env c method_name
        else
          Tast_env.get_method env c method_name
      in
      Option.bind ce ~f:(fun ce ->
          let origin = ce.Typing_defs.ce_origin in
          let shallow = Decl_provider_internals.get_shallow_class ctx origin in
          Option.bind shallow ~f:(fun sc ->
              let methods =
                if is_static then
                  sc.Shallow_decl_defs.sc_static_methods
                else
                  sc.Shallow_decl_defs.sc_methods
              in
              let sm =
                List.find methods ~f:(fun m ->
                    String.equal (snd m.Shallow_decl_defs.sm_name) method_name)
              in
              Option.bind sm ~f:(fun m ->
                  get_decl_ft m.Shallow_decl_defs.sm_type))))

(** Look up a constructor's unpessimised fun_type from the shallow class
    where it is declared. Handles inherited constructors via [ce_origin]. *)
let resolve_shallow_constructor_type env ctx class_name :
    Typing_defs.decl_ty Typing_defs.fun_type option =
  let cls = Tast_env.get_class env class_name |> Decl_entry.to_option in
  Option.bind cls ~f:(fun c ->
      let (ce_opt, _) = Folded_class.construct c in
      Option.bind ce_opt ~f:(fun ce ->
          let origin = ce.Typing_defs.ce_origin in
          let shallow = Decl_provider_internals.get_shallow_class ctx origin in
          Option.bind shallow ~f:(fun sc ->
              Option.bind sc.Shallow_decl_defs.sc_constructor ~f:(fun m ->
                  get_decl_ft m.Shallow_decl_defs.sm_type))))

(** Look up a property's unpessimised decl_ty from the shallow class where
    it is declared. Handles inherited properties via [ce_origin]. *)
let resolve_shallow_prop_type env ctx class_name prop_name :
    Typing_defs.decl_ty option =
  let cls = Tast_env.get_class env class_name |> Decl_entry.to_option in
  Option.bind cls ~f:(fun c ->
      let ce = Tast_env.get_prop env c prop_name in
      Option.bind ce ~f:(fun ce ->
          let origin = ce.Typing_defs.ce_origin in
          let shallow = Decl_provider_internals.get_shallow_class ctx origin in
          Option.bind shallow ~f:(fun sc ->
              let sp =
                List.find sc.Shallow_decl_defs.sc_props ~f:(fun p ->
                    String.equal (snd p.Shallow_decl_defs.sp_name) prop_name)
              in
              Option.map sp ~f:(fun p -> p.Shallow_decl_defs.sp_type))))

(** Extract the class name from a class_id_ when syntactically determined.
    Returns None for CIstatic/CIexpr which depend on runtime dispatch or
    type inference. *)
let get_class_name_from_cid env = function
  | Aast.CI (_, name) -> Some name
  | Aast.CIself -> Tast_env.get_self_id env
  | _ -> None

(** Given a TAST function expression, look up the called function/method's
    declaration and return its decl-level fun_type.
    Uses unpessimised declarations so that SDT like-type wrapping
    does not obscure the programmer-written type.

    For instance method calls, the receiver is trusted when:
    - It is [$this] (syntactically determined), or
    - The inferred type is exact, or
    - The TAST was checked once and the type is not like-typed.
    Otherwise the receiver type comes from inference which may be
    inaccurate, and we skip the position. *)
let resolve_decl_fun_type ~checked_once env func_expr_ :
    Typing_defs.decl_ty Typing_defs.fun_type option =
  let ctx = Tast_env.get_ctx env in
  match func_expr_ with
  | Aast.Id (_, name) ->
    (* Regular function call: foo(...) *)
    let fe =
      Decl_provider_internals.get_fun_without_pessimise ctx name
      |> Decl_entry.to_option
    in
    Option.bind fe ~f:(fun fe -> get_decl_ft fe.Typing_defs.fe_type)
  | Aast.Obj_get ((_, _, Aast.This), (_, _, Aast.Id (_, method_name)), _, _) ->
    (* $this->method(...) — enclosing class is syntactically determined *)
    Option.bind (Tast_env.get_self_id env) ~f:(fun name ->
        resolve_shallow_method_type env ctx name method_name ~is_static:false)
  | Aast.Obj_get ((recv_ty, _, _), (_, _, Aast.Id (_, method_name)), _, _) ->
    (* $obj->method(...) — trust the receiver if exact or checked once *)
    Option.bind (get_trusted_class_name ~checked_once recv_ty) ~f:(fun name ->
        resolve_shallow_method_type env ctx name method_name ~is_static:false)
  | Aast.Class_const ((recv_ty, _, _), (_, method_name)) ->
    (* Static method call: Class::method(), self::method(), static::method(), parent::method() *)
    Option.bind (get_class_name_from_ty env recv_ty) ~f:(fun name ->
        resolve_shallow_method_type env ctx name method_name ~is_static:true)
  | _ -> None

let enforcement_at_pos
    ~(checked_once : bool)
    (ctx : Provider_context.t)
    (tast : Tast.program)
    (cursor : File_content.Position.t) : EnforcementAtPosService.result =
  let visitor =
    object (self)
      inherit [_] Tast_visitor.reduce as super

      inherit [_] Visitors_runtime.monoid

      method private zero = None

      method private plus lhs rhs =
        match (lhs, rhs) with
        | (Some (lpos, _), Some (rpos, _)) ->
          if Pos.length lpos <= Pos.length rpos then
            lhs
          else
            rhs
        | (Some _, None) -> lhs
        | (None, Some _) -> rhs
        | (None, None) -> None

      method private cursor_inside pos =
        let (line, char) = File_content.Position.line_column_one_based cursor in
        Pos.inside_one_based pos line char

      method private decl_ty_to_json env decl_ty =
        let (_, locl_ty) =
          Tast_env.localize_no_subst env ~ignore_errors:true decl_ty
        in
        Tast_env.ty_to_json env locl_ty

      method private make_enforced_type env decl_ty =
        let open EnforcementAtPosService in
        let ty_str = Tast_env.print_decl_ty env decl_ty in
        let ty_json = self#decl_ty_to_json env decl_ty in
        { ty_str; ty_json }

      method private compute_enforcement_result env pos decl_ty =
        let Equal = Tast_env.eq_typing_env in
        let this_class = Tast_env.get_self_class env |> Decl_entry.to_option in
        let enforcement =
          Typing_enforceability.get_enforcement ~this_class env decl_ty
        in
        let open EnforcementAtPosService in
        match enforcement with
        | Typing_defs_core.Enforced ->
          Some (pos, Enforced [self#make_enforced_type env decl_ty])
        | Typing_defs_core.Unenforced ->
          if is_effectively_enforced_container decl_ty then
            Some (pos, Enforced [self#make_enforced_type env decl_ty])
          else (
            match Typing_defs.get_node decl_ty with
            | Typing_defs.Tgeneric name ->
              let bounds = self#find_enforceable_bounds env ~this_class name in
              if List.is_empty bounds then
                Some (pos, Unenforced)
              else
                Some (pos, Enforced bounds)
            | _ -> Some (pos, Unenforced)
          )

      (* --- Type annotations (hints) ---
         Only match the outermost hint to avoid reporting enforcement
         for nested type arguments (e.g. the [int] inside [Ref<int>]
         is NOT at an enforced boundary). *)
      val mutable inside_hint = false

      method! on_hint env ((pos, _) as hint) =
        if inside_hint then
          (* Already inside a hint — this is a nested type argument,
             skip enforcement check but still recurse for sub-hints. *)
          super#on_hint env hint
        else begin
          inside_hint <- true;
          let acc = super#on_hint env hint in
          inside_hint <- false;
          if self#cursor_inside pos then
            let decl_ty = Tast_env.hint_to_ty env hint in
            let res = self#compute_enforcement_result env pos decl_ty in
            self#plus res acc
          else
            acc
        end

      (* --- Call arguments --- *)
      method! on_Call env call_expr =
        let acc = super#on_Call env call_expr in
        let { Aast.func = (_, _, func_expr_); args; _ } = call_expr in
        let decl_ft_opt = resolve_decl_fun_type ~checked_once env func_expr_ in
        match decl_ft_opt with
        | None -> acc
        | Some ft ->
          let res = self#match_arg_to_param env ft.Typing_defs.ft_params args in
          self#plus res acc

      method private match_arg_to_param env params args =
        match (params, args) with
        | (param :: params_rest, arg :: args_rest) ->
          let (_, arg_pos, _) = Aast_utils.arg_to_expr arg in
          if self#cursor_inside arg_pos then
            self#compute_enforcement_result
              env
              arg_pos
              param.Typing_defs.fp_type
          else
            self#match_arg_to_param env params_rest args_rest
        | _ -> None

      (* --- Return statements and type parameter tracking --- *)
      val mutable current_is_async : bool = false

      val mutable current_return_hint : Aast.hint option = None

      (** Constraint hints from tparams currently in scope. *)
      val mutable current_tparam_constraints
          : (string * (Ast_defs.constraint_kind * Aast.hint) list) list =
        []

      (** Look up a generic's upper-bound constraints and return all
          enforceable ones. For example, [T as I as J as arraykey] would
          return [I; J; arraykey] since all are enforced at runtime. *)
      method private find_enforceable_bounds env ~this_class name =
        let Equal = Tast_env.eq_typing_env in
        let constraints =
          List.find_map current_tparam_constraints ~f:(fun (tp_name, cstrs) ->
              if String.equal tp_name name then
                Some cstrs
              else
                None)
        in
        match constraints with
        | None -> []
        | Some cstrs ->
          List.filter_map cstrs ~f:(fun (kind, hint) ->
              match kind with
              | Ast_defs.Constraint_as ->
                let bound_ty = Tast_env.hint_to_ty env hint in
                (* Skip mixed — it's the implicit bound on every generic
                   and doesn't add enforcement information. *)
                let rec is_mixed_bound ty =
                  match Typing_defs.get_node ty with
                  | Typing_defs.Tmixed -> true
                  | Typing_defs.Tapply ((_, name), [inner])
                    when String.equal name SN.Classes.cSupportDyn ->
                    is_mixed_bound inner
                  | _ -> false
                in
                if is_mixed_bound bound_ty then
                  None
                else
                  let bound_enforcement =
                    Typing_enforceability.get_enforcement
                      ~this_class
                      env
                      bound_ty
                  in
                  (match bound_enforcement with
                  | Typing_defs_core.Enforced ->
                    Some (self#make_enforced_type env bound_ty)
                  | Typing_defs_core.Unenforced ->
                    (* Also check for effectively enforced containers
                       e.g. T as vec<mixed> — the bound is effectively
                       enforced even though get_enforcement says not. *)
                    if is_effectively_enforced_container bound_ty then
                      Some (self#make_enforced_type env bound_ty)
                    else
                      None)
              | _ -> None)

      method private save_tparams (tparams : Tast.tparam list) =
        List.map tparams ~f:(fun tp ->
            (snd tp.Aast.tp_name, tp.Aast.tp_constraints))

      method! on_fun_def env fd =
        let saved = current_tparam_constraints in
        current_tparam_constraints <-
          self#save_tparams fd.Aast.fd_tparams @ current_tparam_constraints;
        let acc = super#on_fun_def env fd in
        current_tparam_constraints <- saved;
        acc

      method! on_class_ env c =
        let saved = current_tparam_constraints in
        current_tparam_constraints <-
          self#save_tparams c.Aast.c_tparams @ current_tparam_constraints;
        let acc = super#on_class_ env c in
        current_tparam_constraints <- saved;
        acc

      method! on_fun_ env f =
        let saved_hint = current_return_hint in
        let saved_async = current_is_async in
        current_return_hint <- Aast.hint_of_type_hint f.Aast.f_ret;
        current_is_async <-
          Ast_defs.equal_fun_kind f.Aast.f_fun_kind Ast_defs.FAsync;
        let acc = super#on_fun_ env f in
        current_return_hint <- saved_hint;
        current_is_async <- saved_async;
        acc

      method! on_method_ env m =
        let saved_hint = current_return_hint in
        let saved_cstrs = current_tparam_constraints in
        let saved_async = current_is_async in
        current_return_hint <- Aast.hint_of_type_hint m.Aast.m_ret;
        current_is_async <-
          Ast_defs.equal_fun_kind m.Aast.m_fun_kind Ast_defs.FAsync;
        current_tparam_constraints <-
          self#save_tparams m.Aast.m_tparams @ current_tparam_constraints;
        let acc = super#on_method_ env m in
        current_return_hint <- saved_hint;
        current_tparam_constraints <- saved_cstrs;
        current_is_async <- saved_async;
        acc

      (** For async functions, unwrap [Awaitable<T>] to get the inner type [T]
          since the runtime enforces [T], not [Awaitable<T>]. For non-async
          functions returning [Awaitable<T>], the return type IS [Awaitable<T>]. *)
      method private unwrap_awaitable_hint hint =
        if current_is_async then
          match snd hint with
          | Aast.Happly ((_, name), [inner_hint])
            when String.equal name SN.Classes.cAwaitable ->
            inner_hint
          | _ -> hint
        else
          hint

      method! on_stmt_ env stmt_ =
        let acc = super#on_stmt_ env stmt_ in
        match stmt_ with
        | Aast.Return (Some (_, pos, _)) when self#cursor_inside pos ->
          (match current_return_hint with
          | Some hint ->
            let hint = self#unwrap_awaitable_hint hint in
            let decl_ty = Tast_env.hint_to_ty env hint in
            let res = self#compute_enforcement_result env pos decl_ty in
            self#plus res acc
          | None -> acc)
        | _ -> acc

      (* --- Constructor arguments, property assignments, array indexing --- *)
      method! on_expr env ((_, _, expr_) as expr) =
        let acc = super#on_expr env expr in
        match expr_ with
        (* --- Constructor arguments: new ClassName(args) ---
           The class name is syntactically determined for CI and CIself.
           CIstatic/CIexpr are excluded as they depend on runtime dispatch. *)
        | Aast.New ((_, _, cid), _, args, _, _) ->
          let ctx = Tast_env.get_ctx env in
          let ft_opt =
            Option.bind (get_class_name_from_cid env cid) ~f:(fun name ->
                resolve_shallow_constructor_type env ctx name)
          in
          (match ft_opt with
          | None -> acc
          | Some ft ->
            let res =
              self#match_arg_to_param env ft.Typing_defs.ft_params args
            in
            self#plus res acc)
        (* --- Property assignment RHS ---
           $this->prop: enclosing class is syntactically determined.
           $obj->prop: receiver is trusted if the inferred type is exact
           or the TAST was checked once and the type is not like-typed. *)
        | Aast.Assign
            ( ( _,
                _,
                Aast.Obj_get
                  ( (recv_ty, _, recv_expr_),
                    (_, _, Aast.Id (_, prop_name)),
                    _,
                    Aast.Is_prop ) ),
              _,
              (_, rhs_pos, _) )
          when self#cursor_inside rhs_pos ->
          let class_name =
            match recv_expr_ with
            | Aast.This -> Tast_env.get_self_id env
            | _ -> get_trusted_class_name ~checked_once recv_ty
          in
          let ctx = Tast_env.get_ctx env in
          let res =
            Option.bind class_name ~f:(fun name ->
                Option.bind
                  (resolve_shallow_prop_type env ctx name prop_name)
                  ~f:(fun decl_ty ->
                    self#compute_enforcement_result env rhs_pos decl_ty))
          in
          self#plus res acc
        (* --- Array index expressions --- *)
        | Aast.Array_get (_, Some (_, idx_pos, _))
          when self#cursor_inside idx_pos ->
          let arraykey_ty = Typing_make_type.arraykey Typing_reason.none in
          let res = self#compute_enforcement_result env idx_pos arraykey_ty in
          self#plus res acc
        | _ -> acc
    end
  in
  visitor#go ctx tast |> Option.map ~f:snd

let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    (cursor : File_content.Position.t) : EnforcementAtPosService.result =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let checked_once =
    Option.is_none tast.Tast_with_dynamic.under_dynamic_assumptions
  in
  enforcement_at_pos
    ~checked_once
    ctx
    tast.Tast_with_dynamic.under_normal_assumptions
    cursor

let result_to_json_string
    (result : EnforcementAtPosService.result)
    ((fn, pos) : string * File_content.Position.t) : string =
  let open EnforcementAtPosService in
  let (line, char) = File_content.Position.line_column_one_based pos in
  let obj =
    `Assoc
      ([
         ( "position",
           `Assoc
             [
               ("file", `String fn);
               ("line", `Int line);
               ("character", `Int char);
             ] );
       ]
      @
      match result with
      | Some (Enforced types) ->
        let json_types = List.map types ~f:(fun t -> t.ty_json) in
        [("enforcement", `String "Enforced"); ("types", `List json_types)]
      | Some Unenforced -> [("enforcement", `String "Unenforced")]
      | None -> [("enforcement", `Null)])
  in
  Hh_json_helpers.Out.to_string obj
