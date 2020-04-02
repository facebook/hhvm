(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module "naming" a program.
 *
 * The naming phase consists in several things
 * 1- get all the global names
 * 2- transform all the local names into a unique identifier
 *)

open Core_kernel
open Common
open Utils
open String_utils
module N = Aast
module SN = Naming_special_names
module NS = Namespaces
module Partial = Partial_provider
module GEnv = Naming_global.GEnv

(*****************************************************************************)
(* The types *)
(*****************************************************************************)

(* We want to keep the positions of names that have been
 * replaced by identifiers.
 *)
type positioned_ident = Pos.t * Local_id.t

(* <T as A>, A is a type constraint *)
type type_constraint =
  Aast.reify_kind * (Ast_defs.constraint_kind * Aast.hint) list

type is_final = bool

type genv = {
  (* strict? decl? partial? *)
  in_mode: FileInfo.mode;
  (* various options that control the strictness of the typechecker *)
  ctx: Provider_context.t;
  (* are we in a __PPL attributed class *)
  in_ppl: bool;
  (* In function foo<T1, ..., Tn> or class<T1, ..., Tn>, the field
   * type_params knows T1 .. Tn. It is able to find out about the
   * constraint on these parameters. *)
  type_params: type_constraint SMap.t;
  (* The current class, None if we are in a function *)
  current_cls: (Ast_defs.id * Ast_defs.class_kind * is_final) option;
  (* Normally we don't need to add dependencies at this stage, but there
   * are edge cases when we do. *)
  droot: Typing_deps.Dep.dependent Typing_deps.Dep.variant;
  (* Namespace environment, e.g., what namespace we're in and what use
   * declarations are in play. *)
  namespace: Namespace_env.env;
}

(* Handler called when we see an unbound name. *)
type unbound_handler = Pos.t * string -> positioned_ident

(* The primitives to manipulate the naming environment *)
module Env : sig
  type lenv

  val empty_local : unbound_handler option -> lenv

  val make_class_env :
    Provider_context.t -> type_constraint SMap.t -> Nast.class_ -> genv * lenv

  val make_typedef_env :
    Provider_context.t -> type_constraint SMap.t -> Nast.typedef -> genv * lenv

  val make_top_level_env : Provider_context.t -> genv * lenv

  val make_fun_decl_genv :
    Provider_context.t -> type_constraint SMap.t -> Nast.fun_ -> genv

  val make_file_attributes_env :
    Provider_context.t -> FileInfo.mode -> Aast.nsenv -> genv * lenv

  val make_const_env : Provider_context.t -> Nast.gconst -> genv * lenv

  val in_ppl : genv * lenv -> bool

  val set_ppl : genv * lenv -> bool -> genv * lenv

  val add_lvar : genv * lenv -> Ast_defs.id -> positioned_ident -> unit

  val add_param : genv * lenv -> Nast.fun_param -> genv * lenv

  val new_lvar : genv * lenv -> Ast_defs.id -> positioned_ident

  val lvar : genv * lenv -> Ast_defs.id -> positioned_ident

  val global_const : genv * lenv -> Ast_defs.id -> Ast_defs.id

  val type_name :
    ?context:Errors.name_context ->
    genv * lenv ->
    Ast_defs.id ->
    allow_typedef:bool ->
    allow_generics:bool ->
    Ast_defs.id

  val fun_id : genv * lenv -> Ast_defs.id -> Ast_defs.id

  val goto_label : genv * lenv -> string -> Pos.t option

  val new_goto_label : genv * lenv -> Aast.pstring -> unit

  val new_goto_target : genv * lenv -> Aast.pstring -> unit

  val check_goto_references : genv * lenv -> unit

  val scope : genv * lenv -> (genv * lenv -> 'a) -> 'a

  val remove_locals : genv * lenv -> Ast_defs.id list -> unit
end = struct
  type map = positioned_ident SMap.t

  (* The local environment *)
  type lenv = {
    (* The set of locals *)
    locals: map ref;
    (* Handler called when we see an unbound name.
     * This is used to compute an approximation of the list of captured
     * variables for closures: when we see an undefined variable, we add it
     * to the list of captured variables.
     *
     * See expr_lambda for details.
     *)
    unbound_handler: unbound_handler option;
    (*
     * A map from goto label strings to named labels.
     *)
    goto_labels: Pos.t SMap.t ref;
    (*
     * A map from goto label used in a goto statement to the position of that
     * goto label usage.
     *)
    goto_targets: Pos.t SMap.t ref;
  }

  let empty_local unbound_handler =
    {
      locals = ref SMap.empty;
      unbound_handler;
      goto_labels = ref SMap.empty;
      goto_targets = ref SMap.empty;
    }

  let make_class_genv ctx tparams mode (cid, ckind) namespace is_ppl final =
    {
      in_mode = mode;
      ctx;
      in_ppl = is_ppl;
      type_params = tparams;
      current_cls = Some (cid, ckind, final);
      droot = Typing_deps.Dep.Class (snd cid);
      namespace;
    }

  let unbound_name_error genv pos name kind =
    (* Naming pretends to be local and not dependent on other files, so it
     * doesn't bother with adding dependencies (even though it does look up
     * things in global state). This is mostly brushed aside because "they
     * will be added during typing". Unfortunately, there are multiple scenarios
     * when typechecker will name an expression, but gives up on typechecking
     * it. We are then left with a unrecorded dependency. This should be fixed
     * on some more basic level, but so far the only incorrectness that anyone
     * has observed due to this is that we fail to remove "unbound name" errors
     * sometimes. I add this dependency here for now to fix the annoyance it
     * causes developers. *)
    begin
      match kind with
      | Errors.FunctionNamespace -> Typing_deps.Dep.Fun name
      | Errors.TypeNamespace -> Typing_deps.Dep.Class name
      | Errors.ConstantNamespace -> Typing_deps.Dep.GConst name
      | Errors.TraitContext -> Typing_deps.Dep.Class name
      | Errors.RecordContext -> Typing_deps.Dep.RecordDef name
      | Errors.ClassContext -> Typing_deps.Dep.Class name
    end
    |> Typing_deps.add_idep genv.droot;
    Errors.unbound_name pos name kind

  let make_class_env ctx tparams c =
    let is_ppl =
      List.exists c.Aast.c_user_attributes (fun { Aast.ua_name; _ } ->
          snd ua_name = SN.UserAttributes.uaProbabilisticModel)
    in
    let genv =
      make_class_genv
        ctx
        tparams
        c.Aast.c_mode
        (c.Aast.c_name, c.Aast.c_kind)
        c.Aast.c_namespace
        is_ppl
        c.Aast.c_final
    in
    let lenv = empty_local None in
    (genv, lenv)

  let make_typedef_genv ctx cstrs tdef_name tdef_namespace =
    {
      in_mode = FileInfo.Mstrict;
      ctx;
      in_ppl = false;
      type_params = cstrs;
      current_cls = None;
      droot = Typing_deps.Dep.Class tdef_name;
      namespace = tdef_namespace;
    }

  let make_typedef_env ctx cstrs tdef =
    let genv =
      make_typedef_genv ctx cstrs (snd tdef.Aast.t_name) tdef.Aast.t_namespace
    in
    let lenv = empty_local None in
    (genv, lenv)

  let make_fun_genv ctx params f_mode f_name f_namespace =
    {
      in_mode = f_mode;
      ctx;
      in_ppl = false;
      type_params = params;
      current_cls = None;
      droot = Typing_deps.Dep.Fun f_name;
      namespace = f_namespace;
    }

  let make_fun_decl_genv ctx params f =
    make_fun_genv
      ctx
      params
      f.Aast.f_mode
      (snd f.Aast.f_name)
      f.Aast.f_namespace

  let make_const_genv ctx cst =
    {
      in_mode = cst.Aast.cst_mode;
      ctx;
      in_ppl = false;
      type_params = SMap.empty;
      current_cls = None;
      droot = Typing_deps.Dep.GConst (snd cst.Aast.cst_name);
      namespace = cst.Aast.cst_namespace;
    }

  let make_top_level_genv ctx =
    {
      in_mode = FileInfo.Mpartial;
      ctx;
      in_ppl = false;
      type_params = SMap.empty;
      current_cls = None;
      droot = Typing_deps.Dep.Fun "";
      namespace = Namespace_env.empty_with_default;
    }

  let make_top_level_env ctx =
    let genv = make_top_level_genv ctx in
    let lenv = empty_local None in
    let env = (genv, lenv) in
    env

  let make_file_attributes_genv ctx mode namespace =
    {
      in_mode = mode;
      ctx;
      in_ppl = false;
      type_params = SMap.empty;
      current_cls = None;
      droot = Typing_deps.Dep.Fun "";
      namespace;
    }

  let make_file_attributes_env ctx mode namespace =
    let genv = make_file_attributes_genv ctx mode namespace in
    let lenv = empty_local None in
    let env = (genv, lenv) in
    env

  let make_const_env ctx cst =
    let genv = make_const_genv ctx cst in
    let lenv = empty_local None in
    let env = (genv, lenv) in
    env

  let in_ppl (genv, _lenv) = genv.in_ppl

  let set_ppl (genv, lenv) in_ppl =
    let genv = { genv with in_ppl } in
    (genv, lenv)

  let if_unbound_then_dep_edge_and_report
      genv (is_defined : Provider_context.t -> string -> bool) (p, x) =
    if not (is_defined genv.ctx x) then
      match genv.in_mode with
      | FileInfo.Mstrict
      | FileInfo.Mpartial
      | FileInfo.Mdecl ->
        unbound_name_error genv p x Errors.ConstantNamespace
      | FileInfo.Mphp -> ()

  let handle_unbound_name genv get_full_pos get_canon (p, name) kind =
    match get_canon genv.ctx name with
    | Some canonical ->
      canonical
      |> get_full_pos genv.ctx
      |> Option.iter ~f:(fun p_canon ->
             Errors.did_you_mean_naming p name p_canon canonical);

      (* Recovering from the capitalization error means
       * returning the name in its canonical form *)
      (p, canonical)
    | None ->
      (match genv.in_mode with
      | FileInfo.Mpartial
      | FileInfo.Mdecl
        when name = SN.Classes.cUnknown ->
        ()
      | FileInfo.Mphp -> ()
      | FileInfo.Mstrict -> unbound_name_error genv p name kind
      | FileInfo.Mpartial
      | FileInfo.Mdecl ->
        unbound_name_error genv p name kind);
      (p, name)

  let canonicalize genv is_defined get_full_pos get_canon (p, name) kind =
    (* Get the canonical name to check if the name exists in the heap *)
    if is_defined name then
      (p, name)
    else
      handle_unbound_name genv get_full_pos get_canon (p, name) kind

  (* Adds a local variable, without any check *)
  let add_lvar (_, lenv) (_, name) (p, x) =
    lenv.locals := SMap.add name (p, x) !(lenv.locals);
    ()

  let add_param env param =
    let p_name = param.N.param_name in
    let id = Local_id.make_unscoped p_name in
    let p_pos = param.N.param_pos in
    let () = add_lvar env (p_pos, p_name) (p_pos, id) in
    env

  (* Defines a new local variable.
     Side effects:
     1) if the local is not in the local environment then it is added.
     Return value: the given position and deduced/created identifier. *)
  let new_lvar (_, lenv) (p, x) =
    let lcl = SMap.find_opt x !(lenv.locals) in
    let ident =
      match lcl with
      | Some lcl -> snd lcl
      | None ->
        let ident = Local_id.make_unscoped x in
        lenv.locals := SMap.add x (p, ident) !(lenv.locals);
        ident
    in
    (p, ident)

  let handle_undefined_variable (_genv, env) (p, x) =
    match env.unbound_handler with
    | None -> (p, Local_id.make_unscoped x)
    | Some f -> f (p, x)

  (* Function used to name a local variable *)
  let lvar (genv, env) (p, x) =
    let (p, ident) =
      if
        (SN.Superglobals.globals = x || SN.Superglobals.is_superglobal x)
        && genv.in_mode = FileInfo.Mpartial
      then
        (p, Local_id.make_unscoped x)
      else
        let lcl = SMap.find_opt x !(env.locals) in
        match lcl with
        | Some lcl -> (p, snd lcl)
        | None -> handle_undefined_variable (genv, env) (p, x)
    in
    (p, ident)

  let global_const (genv, _env) x =
    match Provider_backend.get () with
    | Provider_backend.Shared_memory
    | Provider_backend.Local_memory _ ->
      if_unbound_then_dep_edge_and_report genv Naming_provider.const_exists x;
      x
    | Provider_backend.Decl_service _ ->
      (* TODO: we need to refactor this so naming phase doesn't report *)
      (* unbound name errors; that should come later, during typchecking, *)
      (* which is when we look up the decl service. The decl service *)
      (* is the only one who'll tell us whether a name is unbound. *)
      x

  let type_name
      ?(context = Errors.TypeNamespace)
      (genv, _)
      ((p, name) as x)
      ~allow_typedef
      ~allow_generics =
    match SMap.find_opt name genv.type_params with
    | Some (reified, _) ->
      if not allow_generics then Errors.generics_not_allowed p;
      begin
        match reified with
        | N.Erased -> Errors.generic_at_runtime p "Erased"
        | N.SoftReified -> Errors.generic_at_runtime p "Soft reified"
        | N.Reified -> ()
      end;
      x
    | None ->
      (match Naming_provider.get_type_pos_and_kind genv.ctx name with
      | Some (_def_pos, Naming_types.TClass) ->
        (* Don't let people use strictly internal classes
         * (except when they are being declared in .hhi files) *)
        if
          name = SN.Classes.cHH_BuiltinEnum
          && not
               (string_ends_with (Relative_path.suffix (Pos.filename p)) ".hhi")
        then
          Errors.using_internal_class p (strip_ns name);
        x
      | Some (def_pos, Naming_types.TTypedef) when not allow_typedef ->
        let (full_pos, _) = GEnv.get_full_pos genv.ctx (def_pos, name) in
        Errors.unexpected_typedef p full_pos;
        x
      | Some (_def_pos, Naming_types.TTypedef) -> x
      | Some (_def_pos, Naming_types.TRecordDef) -> x
      | None ->
        handle_unbound_name genv GEnv.type_pos GEnv.type_canon_name x context)

  let fun_id (genv, _) x =
    canonicalize
      genv
      (Naming_provider.fun_exists genv.ctx)
      GEnv.fun_pos
      GEnv.fun_canon_name
      x
      Errors.FunctionNamespace

  (**
   * Returns the position of the goto label declaration, if it exists.
   *)
  let goto_label (_, { goto_labels; _ }) label =
    SMap.find_opt label !goto_labels

  (**
   * Adds a goto label and the position of its declaration to the known labels.
   *)
  let new_goto_label (_, { goto_labels; _ }) (pos, label) =
    goto_labels := SMap.add label pos !goto_labels

  (**
   * Adds a goto target and its reference position to the known targets.
   *)
  let new_goto_target (_, { goto_targets; _ }) (pos, label) =
    goto_targets := SMap.add label pos !goto_targets

  (**
   * Ensures that goto statements do not reference goto labels that are not
   * known within the current lenv.
   *)
  let check_goto_references (_, { goto_labels; goto_targets; _ }) =
    let check_label referenced_label referenced_label_pos =
      if not (SMap.mem referenced_label !goto_labels) then
        Errors.goto_label_undefined referenced_label_pos referenced_label
    in
    SMap.iter check_label !goto_targets

  (* Scope, keep the locals, go and name the body, and leave the
   * local environment intact
   *)
  let scope env f =
    let (_genv, lenv) = env in
    let lenv_copy = !(lenv.locals) in
    let res = f env in
    lenv.locals := lenv_copy;
    res

  let remove_locals env vars =
    let (_genv, lenv) = env in
    lenv.locals :=
      List.fold_left
        vars
        ~f:(fun l id -> SMap.remove (snd id) l)
        ~init:!(lenv.locals)
end

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let elaborate_namespaces =
  new Naming_elaborate_namespaces_endo.generic_elaborator

let check_repetition s param =
  let name = param.Aast.param_name in
  if SSet.mem name s then Errors.already_bound param.Aast.param_pos name;
  if name <> SN.SpecialIdents.placeholder then
    SSet.add name s
  else
    s

let convert_shape_name env = function
  | Ast_defs.SFlit_int (pos, s) -> Ast_defs.SFlit_int (pos, s)
  | Ast_defs.SFlit_str (pos, s) -> Ast_defs.SFlit_str (pos, s)
  | Ast_defs.SFclass_const (x, (pos, y)) ->
    let class_name =
      if snd x = SN.Classes.cSelf then (
        match (fst env).current_cls with
        | Some (cid, _, _) -> cid
        | None ->
          Errors.self_outside_class pos;
          (pos, SN.Classes.cUnknown)
      ) else
        Env.type_name
          env
          x
          ~allow_typedef:false
          ~allow_generics:false
          ~context:Errors.ClassContext
    in
    Ast_defs.SFclass_const (class_name, (pos, y))

let arg_unpack_unexpected = function
  | None -> ()
  | Some (pos, _) ->
    Errors.naming_too_few_arguments pos;
    ()

(************************************************************************)
(* Naming of type hints *)
(************************************************************************)

(**
 * The existing hint function goes from Ast_defs.hint -> Nast.hint
 * This hint function goes from Aast.hint -> Nast.hint
 * Used with with Ast_to_nast to go from Ast_defs.hint -> Nast.hint
 *)
let rec hint
    ?(context = Errors.TypeNamespace)
    ?(forbid_this = false)
    ?(allow_retonly = false)
    ?(allow_typedef = true)
    ?(allow_wildcard = false)
    ?(allow_like = false)
    ?(in_where_clause = false)
    ?(tp_depth = 0)
    env
    (hh : Aast.hint) =
  let (mut, (p, h)) = unwrap_mutability hh in
  if Option.is_some mut then Errors.misplaced_mutability_hint p;
  ( p,
    hint_
      ~context
      ~forbid_this
      ~allow_retonly
      ~allow_typedef
      ~allow_wildcard
      ~allow_like
      ~in_where_clause
      ~tp_depth
      env
      (p, h) )

and unwrap_mutability p =
  match p with
  | (_, Aast.Happly ((_, hn), [t])) when hn = SN.Rx.hMutable ->
    (Some N.PMutable, t)
  | (_, Aast.Happly ((_, hn), [t])) when hn = SN.Rx.hMaybeMutable ->
    (Some N.PMaybeMutable, t)
  | (_, Aast.Happly ((_, hn), [t])) when hn = SN.Rx.hOwnedMutable ->
    (Some N.POwnedMutable, t)
  | t -> (None, t)

and hfun env reactivity is_coroutine hl kl variadic_hint h =
  let variadic_hint = Option.map variadic_hint (hint env) in
  let (muts, hl) =
    List.map
      ~f:(fun h ->
        let (mut, h1) = unwrap_mutability h in
        if Option.is_some mut && reactivity = N.FNonreactive then
          Errors.mutability_hint_in_non_rx_function (fst h);
        (mut, hint env h1))
      hl
    |> List.unzip
  in
  let (ret_mut, rh) = unwrap_mutability h in
  let ret_mut =
    match ret_mut with
    | None -> false
    | Some N.POwnedMutable -> true
    | Some _ ->
      Errors.invalid_mutability_in_return_type_hint (fst h);
      true
  in
  N.Hfun
    N.
      {
        hf_reactive_kind = reactivity;
        hf_is_coroutine = is_coroutine;
        hf_param_tys = hl;
        hf_param_kinds = kl;
        hf_param_mutability = muts;
        hf_variadic_ty = variadic_hint;
        hf_return_ty = hint ~allow_retonly:true env rh;
        hf_is_mutable_return = ret_mut;
      }

and hint_
    ~context
    ~forbid_this
    ~allow_retonly
    ~allow_typedef
    ~allow_wildcard
    ~allow_like
    ~in_where_clause
    ?(tp_depth = 0)
    env
    (p, x) =
  let tcopt = Provider_context.get_tcopt (fst env).ctx in
  let like_type_hints_enabled = TypecheckerOptions.like_type_hints tcopt in
  let union_intersection_type_hints_enabled =
    TypecheckerOptions.union_intersection_type_hints tcopt
  in
  let hint = hint ~forbid_this ~allow_typedef ~allow_wildcard ~allow_like in
  match x with
  | Aast.Hunion hl ->
    if not union_intersection_type_hints_enabled then
      Errors.experimental_feature p "union type hints";
    N.Hunion (List.map hl ~f:(hint ~allow_retonly env))
  | Aast.Hintersection hl ->
    if not union_intersection_type_hints_enabled then
      Errors.experimental_feature p "intersection type hints";
    N.Hintersection (List.map hl ~f:(hint ~allow_retonly env))
  | Aast.Htuple hl ->
    N.Htuple (List.map hl ~f:(hint ~allow_retonly ~tp_depth:(tp_depth + 1) env))
  | Aast.Hoption h ->
    (* void/noreturn are permitted for Typing.option_return_only_typehint *)
    N.Hoption (hint ~allow_retonly env h)
  | Aast.Hlike h ->
    if not (allow_like || like_type_hints_enabled) then
      Errors.experimental_feature p "like-types";
    N.Hlike (hint ~allow_retonly env h)
  | Aast.Hsoft h ->
    let h = hint ~allow_retonly env h in
    snd h
  | Aast.Hfun
      Aast.
        {
          hf_reactive_kind = reactivity;
          hf_is_coroutine = coroutine;
          hf_param_tys = hl;
          hf_param_kinds = kl;
          hf_param_mutability = _;
          hf_variadic_ty = variadic_hint;
          hf_return_ty = h;
          hf_is_mutable_return = _;
        } ->
    hfun env reactivity coroutine hl kl variadic_hint h
  (* Special case for Rx<function> *)
  | Aast.Happly
      ( (_, hname),
        [
          ( _,
            Aast.Hfun
              Aast.
                {
                  hf_reactive_kind = _;
                  hf_is_coroutine;
                  hf_param_tys = hl;
                  hf_param_kinds = kl;
                  hf_param_mutability = _;
                  hf_variadic_ty = variadic_hint;
                  hf_return_ty = h;
                  hf_is_mutable_return = _;
                } );
        ] )
    when hname = SN.Rx.hRx ->
    hfun env N.FReactive hf_is_coroutine hl kl variadic_hint h
  (* Special case for RxShallow<function> *)
  | Aast.Happly
      ( (_, hname),
        [
          ( _,
            Aast.Hfun
              Aast.
                {
                  hf_reactive_kind = _;
                  hf_is_coroutine;
                  hf_param_tys = hl;
                  hf_param_kinds = kl;
                  hf_param_mutability = _;
                  hf_variadic_ty = variadic_hint;
                  hf_return_ty = h;
                  hf_is_mutable_return = _;
                } );
        ] )
    when hname = SN.Rx.hRxShallow ->
    hfun env N.FShallow hf_is_coroutine hl kl variadic_hint h
  (* Special case for RxLocal<function> *)
  | Aast.Happly
      ( (_, hname),
        [
          ( _,
            Aast.Hfun
              Aast.
                {
                  hf_reactive_kind = _;
                  hf_is_coroutine;
                  hf_param_tys = hl;
                  hf_param_kinds = kl;
                  hf_param_mutability = _;
                  hf_variadic_ty = variadic_hint;
                  hf_return_ty = h;
                  hf_is_mutable_return = _;
                } );
        ] )
    when hname = SN.Rx.hRxLocal ->
    hfun env N.FLocal hf_is_coroutine hl kl variadic_hint h
  | Aast.Happly (((p, _x) as id), hl) ->
    let hint_id =
      hint_id
        ~context
        ~forbid_this
        ~allow_retonly
        ~allow_typedef
        ~allow_wildcard
        ~tp_depth
        env
        id
        hl
    in
    (match hint_id with
    | N.Hprim _
    | N.Hmixed
    | N.Hnonnull
    | N.Hdynamic
    | N.Hnothing ->
      if hl <> [] then Errors.unexpected_type_arguments p
    | _ -> ());
    hint_id
  | Aast.Haccess ((pos, root_id), ids) ->
    let root_ty =
      match root_id with
      | Aast.Happly ((pos, x), _) when x = SN.Classes.cSelf ->
        begin
          match (fst env).current_cls with
          | None ->
            Errors.self_outside_class pos;
            N.Herr
          | Some (cid, _, _) -> N.Happly (cid, [])
        end
      | Aast.Happly ((pos, x), _)
        when x = SN.Classes.cStatic || x = SN.Classes.cParent ->
        Errors.invalid_type_access_root (pos, x);
        N.Herr
      | Aast.Happly (root, _) ->
        let h =
          hint_id
            ~context
            ~forbid_this
            ~allow_retonly
            ~allow_typedef
            ~allow_wildcard:false
            ~tp_depth
            env
            root
            []
        in
        begin
          match h with
          | N.Hthis
          | N.Happly _ ->
            h
          | N.Habstr _ when in_where_clause -> h
          | _ ->
            Errors.invalid_type_access_root root;
            N.Herr
        end
      | _ ->
        Errors.internal_error
          pos
          "Malformed hint: expected Haccess (Happly ...) from ast_to_nast";
        N.Herr
    in
    N.Haccess ((pos, root_ty), ids)
  | Aast.Hshape { Aast.nsi_allows_unknown_fields; nsi_field_map } ->
    let nsi_field_map =
      List.map
        ~f:(fun { Aast.sfi_optional; sfi_hint; sfi_name } ->
          let new_key = convert_shape_name env sfi_name in
          let new_field =
            {
              N.sfi_optional;
              sfi_hint =
                hint ~allow_retonly ~tp_depth:(tp_depth + 1) env sfi_hint;
              sfi_name = new_key;
            }
          in
          new_field)
        nsi_field_map
    in
    N.Hshape { N.nsi_allows_unknown_fields; nsi_field_map }
  | Aast.Herr
  | Aast.Hany
  | Aast.Hmixed
  | Aast.Hnonnull
  | Aast.Habstr _
  | Aast.Harray _
  | Aast.Hdarray _
  | Aast.Hvarray _
  | Aast.Hvarray_or_darray _
  | Aast.Hprim _
  | Aast.Hthis
  | Aast.Hdynamic
  | Aast.Hnothing ->
    Errors.internal_error Pos.none "Unexpected hint not present on legacy AST";
    N.Herr
  | Aast.Hpu_access (h, id, pu_loc) ->
    let (genv, _) = env in
    let pu_loc =
      match SMap.find_opt (snd id) genv.type_params with
      | Some _ -> Aast.TypeParameter
      | _ -> pu_loc
    in
    N.Hpu_access (hint ~allow_retonly env h, id, pu_loc)

and hint_id
    ?(context = Errors.TypeNamespace)
    ~forbid_this
    ~allow_retonly
    ~allow_typedef
    ~allow_wildcard
    ~tp_depth
    env
    ((p, x) as id)
    hl =
  let params = (fst env).type_params in
  (* some common Xhp screw ups *)
  if x = "Xhp" || x = ":Xhp" || x = "XHP" then Errors.disallowed_xhp_type p x;
  match try_castable_hint ~forbid_this ~allow_wildcard ~tp_depth env p x hl with
  | Some h -> h
  | None ->
    begin
      match x with
      | x when x = SN.Typehints.wildcard ->
        if allow_wildcard && tp_depth >= 1 (* prevents 3 as _ *) then
          if hl <> [] then (
            Errors.tparam_with_tparam p x;
            N.Herr
          ) else
            N.Happly (id, [])
        else (
          Errors.wildcard_disallowed p;
          N.Herr
        )
      | x
        when x = "\\" ^ SN.Typehints.void
             || x = "\\" ^ SN.Typehints.null
             || x = "\\" ^ SN.Typehints.noreturn
             || x = "\\" ^ SN.Typehints.int
             || x = "\\" ^ SN.Typehints.bool
             || x = "\\" ^ SN.Typehints.float
             || x = "\\" ^ SN.Typehints.num
             || x = "\\" ^ SN.Typehints.string
             || x = "\\" ^ SN.Typehints.resource
             || x = "\\" ^ SN.Typehints.mixed
             || x = "\\" ^ SN.Typehints.nonnull
             || x = "\\" ^ SN.Typehints.array
             || x = "\\" ^ SN.Typehints.arraykey ->
        Errors.primitive_toplevel p;
        N.Herr
      | x when x = "\\" ^ SN.Typehints.nothing ->
        Errors.primitive_toplevel p;
        N.Herr
      | x when x = SN.Typehints.void && allow_retonly -> N.Hprim N.Tvoid
      | x when x = SN.Typehints.void ->
        Errors.return_only_typehint p `void;
        N.Herr
      | x when x = SN.Typehints.noreturn && allow_retonly -> N.Hprim N.Tnoreturn
      | x when x = SN.Typehints.noreturn ->
        Errors.return_only_typehint p `noreturn;
        N.Herr
      | x when x = SN.Typehints.null -> N.Hprim N.Tnull
      | x when x = SN.Typehints.num -> N.Hprim N.Tnum
      | x when x = SN.Typehints.resource -> N.Hprim N.Tresource
      | x when x = SN.Typehints.arraykey -> N.Hprim N.Tarraykey
      | x when x = SN.Typehints.mixed -> N.Hmixed
      | x when x = SN.Typehints.nonnull -> N.Hnonnull
      | x when x = SN.Typehints.dynamic -> N.Hdynamic
      | x when x = SN.Typehints.nothing -> N.Hnothing
      | x when x = SN.Typehints.this && not forbid_this ->
        if not (phys_equal hl []) then Errors.this_no_argument p;
        N.Hthis
      | x when x = SN.Typehints.this ->
        Errors.this_type_forbidden p;
        N.Herr
      (* TODO: Duplicate of a Typing[4101] error if namespaced correctly
       * T56198838 *)
      | x
        when (x = SN.Classes.cClassname || x = "classname")
             && List.length hl <> 1 ->
        Errors.classname_param p;
        N.Hprim N.Tstring
      | _ when String.lowercase x = SN.Typehints.this ->
        Errors.lowercase_this p x;
        N.Herr
      | _ when SMap.mem x params ->
        if hl <> [] then Errors.tparam_with_tparam p x;
        N.Habstr x
      | _ ->
        let name =
          Env.type_name ~context env id ~allow_typedef ~allow_generics:false
        in
        (* Note that we are intentionally setting allow_typedef to `true` here.
         * In general, generics arguments can be typedefs -- there is no
         * runtime restriction. *)
        N.Happly
          ( name,
            hintl
              ~allow_wildcard
              ~forbid_this
              ~allow_typedef:true
              ~allow_retonly:true
              ~tp_depth:(tp_depth + 1)
              env
              hl )
    end

(* Hints that are valid both as casts and type annotations.  Neither
 * casts nor annotations are a strict subset of the other: For
 * instance, 'object' is not a valid annotation.  Thus callers will
 * have to handle the remaining cases. *)
and try_castable_hint
    ?(forbid_this = false) ?(allow_wildcard = false) ~tp_depth env p x hl =
  let hint =
    hint
      ~forbid_this
      ~tp_depth:(tp_depth + 1)
      ~allow_wildcard
      ~allow_retonly:false
  in
  let canon = String.lowercase x in
  let opt_hint =
    match canon with
    | nm when nm = SN.Typehints.int -> Some (N.Hprim N.Tint)
    | nm when nm = SN.Typehints.bool -> Some (N.Hprim N.Tbool)
    | nm when nm = SN.Typehints.float -> Some (N.Hprim N.Tfloat)
    | nm when nm = SN.Typehints.string -> Some (N.Hprim N.Tstring)
    | nm when nm = SN.Typehints.array ->
      let tcopt = Provider_context.get_tcopt (fst env).ctx in
      let array_typehints_disallowed =
        TypecheckerOptions.disallow_array_typehint tcopt
      in
      if array_typehints_disallowed then Errors.array_typehints_disallowed p;
      Some
        (match hl with
        | [] -> N.Harray (None, None)
        | [val_] -> N.Harray (Some (hint env val_), None)
        | [key_; val_] -> N.Harray (Some (hint env key_), Some (hint env val_))
        | _ ->
          Errors.too_many_type_arguments p;
          N.Herr)
    | nm when nm = SN.Typehints.darray ->
      Some
        (match hl with
        | [] ->
          if Partial.should_check_error (fst env).in_mode 2071 then
            Errors.too_few_type_arguments p;
          N.Hdarray ((p, N.Hany), (p, N.Hany))
        | [_] ->
          Errors.too_few_type_arguments p;
          N.Hany
        | [key_; val_] -> N.Hdarray (hint env key_, hint env val_)
        | _ ->
          Errors.too_many_type_arguments p;
          N.Hany)
    | nm when nm = SN.Typehints.varray ->
      Some
        (match hl with
        | [] ->
          if Partial.should_check_error (fst env).in_mode 2071 then
            Errors.too_few_type_arguments p;
          N.Hvarray (p, N.Hany)
        | [val_] -> N.Hvarray (hint env val_)
        | _ ->
          Errors.too_many_type_arguments p;
          N.Hany)
    | nm when nm = SN.Typehints.varray_or_darray ->
      Some
        (match hl with
        | [] ->
          if Partial.should_check_error (fst env).in_mode 2071 then
            Errors.too_few_type_arguments p;

          (* Warning: These Hanys are here because they produce subtle
              errors because of interaction with tco_experimental_isarray
              if you change them to Herr *)
          N.Hvarray_or_darray (None, (p, N.Hany))
        | [val_] -> N.Hvarray_or_darray (None, hint env val_)
        | [key; val_] -> N.Hvarray_or_darray (Some (hint env key), hint env val_)
        | _ ->
          Errors.too_many_type_arguments p;
          N.Hany)
    | _ -> None
  in
  let () =
    match opt_hint with
    | Some _ when canon <> x -> Errors.primitive_invalid_alias p x canon
    | _ -> ()
  in
  opt_hint

and hintl
    ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard ~tp_depth env l =
  List.map
    ~f:
      (hint
         ~forbid_this
         ~allow_retonly
         ~allow_typedef
         ~allow_wildcard
         ~tp_depth
         env)
    l

let constraint_ ?(forbid_this = false) env (ck, h) =
  (ck, hint ~forbid_this env h)

let targ env (p, t) =
  ( p,
    hint
      ~allow_wildcard:true
      ~forbid_this:false
      ~allow_typedef:true
      ~allow_retonly:true
      ~tp_depth:1
      env
      t )

let targl env _ tal = List.map tal ~f:(targ env)

(**************************************************************************)
(* All the methods and static methods of an interface are "implicitly"
 * declared as abstract
 *)
(**************************************************************************)

let add_abstract m = { m with N.m_abstract = true }

let add_abstractl methods = List.map methods add_abstract

let interface c constructor methods smethods =
  if c.Aast.c_kind <> Ast_defs.Cinterface then
    (constructor, methods, smethods)
  else
    let constructor = Option.map constructor add_abstract in
    let methods = add_abstractl methods in
    let smethods = add_abstractl smethods in
    (constructor, methods, smethods)

let ensure_name_not_dynamic env e =
  match e with
  | (_, (Aast.Id _ | Aast.Lvar _)) -> ()
  | (p, _) ->
    if Partial.should_check_error (fst env).in_mode 2078 then
      Errors.dynamic_class_name_in_strict_mode p

(* Naming of a class *)
let rec class_ ctx c =
  let constraints = make_constraints c.Aast.c_tparams.Aast.c_tparam_list in
  let env = Env.make_class_env ctx constraints c in
  let c =
    elaborate_namespaces#on_class_
      (Naming_elaborate_namespaces_endo.make_env (fst env).namespace)
      c
  in
  let where_constraints =
    type_where_constraints env c.Aast.c_where_constraints
  in
  let name =
    Env.type_name
      ~context:Errors.ClassContext
      env
      c.Aast.c_name
      ~allow_typedef:false
      ~allow_generics:false
  in
  let (constructor, smethods, methods) = Aast.split_methods c in
  let smethods = List.map ~f:(method_ (fst env)) smethods in
  let (sprops, props) = Aast.split_vars c in
  let sprops = List.map ~f:(class_prop_static env) sprops in
  let attrs = user_attributes env c.Aast.c_user_attributes in
  let const = Naming_attributes.find SN.UserAttributes.uaConst attrs in
  let props = List.map ~f:(class_prop_non_static ~const env) props in
  let xhp_attrs = List.map ~f:(xhp_attribute_decl env) c.Aast.c_xhp_attrs in
  (* These would be out of order with the old attributes, but that shouldn't matter? *)
  let props = props @ xhp_attrs in
  let parents =
    List.map
      c.Aast.c_extends
      (hint
         ~allow_retonly:false
         ~allow_typedef:false
         ~context:Errors.ClassContext
         env)
  in
  let parents =
    match c.Aast.c_kind with
    (* Make enums implicitly extend the BuiltinEnum class in order to provide
     * utility methods. *)
    | Ast_defs.Cenum ->
      let pos = fst name in
      let enum_type = (pos, N.Happly (name, [])) in
      let parent =
        ( pos,
          N.Happly
            ((pos, Naming_special_names.Classes.cHH_BuiltinEnum), [enum_type])
        )
      in
      parent :: parents
    | _ -> parents
  in
  let methods = List.map ~f:(method_ (fst env)) methods in
  let uses =
    List.map
      ~f:(hint ~allow_typedef:false ~context:Errors.TraitContext env)
      c.Aast.c_uses
  in
  let pu_enums = List.map ~f:(class_pu_enum env) c.Aast.c_pu_enums in
  let redeclarations =
    List.map ~f:(method_redeclaration env) c.Aast.c_method_redeclarations
  in
  let xhp_attr_uses =
    List.map ~f:(hint ~allow_typedef:false env) c.Aast.c_xhp_attr_uses
  in
  let (c_req_extends, c_req_implements) = Aast.split_reqs c in
  if c_req_implements <> [] && c.Aast.c_kind <> Ast_defs.Ctrait then
    Errors.invalid_req_implements (fst (List.hd_exn c_req_implements));
  let req_implements =
    List.map
      ~f:(hint ~allow_typedef:false ~context:Errors.ClassContext env)
      c_req_implements
  in
  let req_implements = List.map ~f:(fun h -> (h, false)) req_implements in
  if
    c_req_extends <> []
    && c.Aast.c_kind <> Ast_defs.Ctrait
    && c.Aast.c_kind <> Ast_defs.Cinterface
  then
    Errors.invalid_req_extends (fst (List.hd_exn c_req_extends));
  let req_extends =
    List.map
      ~f:(hint ~allow_typedef:false ~context:Errors.ClassContext env)
      c_req_extends
  in
  let req_extends = List.map ~f:(fun h -> (h, true)) req_extends in
  (* Setting a class type parameters constraint to the 'this' type is weird
   * so lets forbid it for now.
   *)
  let tparam_l =
    type_paraml ~forbid_this:true env c.Aast.c_tparams.Aast.c_tparam_list
  in
  let consts = List.map ~f:(class_const env) c.Aast.c_consts in
  let typeconsts = List.map ~f:(typeconst env) c.Aast.c_typeconsts in
  let implements =
    List.map
      ~f:
        (hint
           ~allow_retonly:false
           ~context:Errors.ClassContext
           ~allow_typedef:false
           env)
      c.Aast.c_implements
  in
  let constructor = Option.map constructor (method_ (fst env)) in
  let (constructor, methods, smethods) =
    interface c constructor methods smethods
  in
  let enum = Option.map c.Aast.c_enum (enum_ env) in
  let file_attributes =
    file_attributes ctx c.Aast.c_mode c.Aast.c_file_attributes
  in
  let c_tparams =
    { N.c_tparam_list = tparam_l; N.c_tparam_constraints = constraints }
  in
  let methods =
    match constructor with
    | None -> smethods @ methods
    | Some c -> (c :: smethods) @ methods
  in
  {
    N.c_annotation = ();
    N.c_span = c.Aast.c_span;
    N.c_mode = c.Aast.c_mode;
    N.c_final = c.Aast.c_final;
    N.c_is_xhp = c.Aast.c_is_xhp;
    N.c_has_xhp_keyword = c.Aast.c_has_xhp_keyword;
    N.c_kind = c.Aast.c_kind;
    N.c_name = name;
    N.c_tparams;
    N.c_extends = parents;
    N.c_uses = uses;
    (* c_use_as_alias and c_insteadof_alias are PHP features not supported
     * in Hack but are required since we have runtime support for it
     *)
    N.c_use_as_alias = [];
    N.c_insteadof_alias = [];
    N.c_method_redeclarations = redeclarations;
    N.c_xhp_attr_uses = xhp_attr_uses;
    N.c_xhp_category = c.Aast.c_xhp_category;
    N.c_reqs = req_extends @ req_implements;
    N.c_implements = implements;
    N.c_where_constraints = where_constraints;
    N.c_consts = consts;
    N.c_typeconsts = typeconsts;
    N.c_vars = sprops @ props;
    N.c_methods = methods;
    N.c_user_attributes = attrs;
    N.c_file_attributes = file_attributes;
    N.c_namespace = c.Aast.c_namespace;
    N.c_enum = enum;
    N.c_doc_comment = c.Aast.c_doc_comment;
    N.c_pu_enums = pu_enums;
    N.c_xhp_children = c.Aast.c_xhp_children;
    (* Naming and typechecking shouldn't use these fields *)
    N.c_attributes = [];
    N.c_xhp_attrs = [];
    N.c_emit_id = c.Aast.c_emit_id;
  }

and user_attributes env attrl =
  let seen = Caml.Hashtbl.create 0 in
  let validate_seen ua_name =
    let (pos, name) = ua_name in
    let existing_attr_pos =
      (try Some (Caml.Hashtbl.find seen name) with Caml.Not_found -> None)
    in
    match existing_attr_pos with
    | Some p ->
      Errors.duplicate_user_attribute ua_name p;
      false
    | None ->
      Caml.Hashtbl.add seen name pos;
      true
  in
  let on_attr acc { Aast.ua_name; ua_params } =
    let name = snd ua_name in
    let ua_name =
      if SN.UserAttributes.is_reserved name then
        ua_name
      else
        Env.type_name
          ~context:Errors.ClassContext
          env
          ua_name
          ~allow_typedef:false
          ~allow_generics:false
    in
    if not (validate_seen ua_name) then
      acc
    else
      let attr =
        { N.ua_name; N.ua_params = List.map ~f:(expr env) ua_params }
      in
      attr :: acc
  in
  List.fold_left ~init:[] ~f:on_attr attrl

and file_attributes ctx mode fal = List.map ~f:(file_attribute ctx mode) fal

and file_attribute ctx mode fa =
  let env = Env.make_file_attributes_env ctx mode fa.Aast.fa_namespace in
  let ua = user_attributes env fa.Aast.fa_user_attributes in
  N.{ fa_user_attributes = ua; fa_namespace = fa.Aast.fa_namespace }

(* h cv is_required maybe_enum *)
and xhp_attribute_decl env (h, cv, tag, maybe_enum) =
  let (p, id) = cv.Aast.cv_id in
  let default = cv.Aast.cv_expr in
  let is_required = Option.is_some tag in
  if is_required && Option.is_some default then
    Errors.xhp_required_with_default p id;
  let hint_ =
    match maybe_enum with
    | Some (pos, _optional, items) ->
      let is_int item =
        match item with
        | (_, Aast.Int _) -> true
        | _ -> false
      in
      let contains_int = List.exists ~f:is_int items in
      let is_string item =
        match item with
        | (_, Aast.String _)
        | (_, Aast.String2 _) ->
          true
        | _ -> false
      in
      let contains_str = List.exists ~f:is_string items in
      if contains_int && not contains_str then
        Some (pos, Aast.Happly ((pos, "int"), []))
      else if (not contains_int) && contains_str then
        Some (pos, Aast.Happly ((pos, "string"), []))
      else
        Some (pos, Aast.Happly ((pos, "mixed"), []))
    | _ -> Aast.hint_of_type_hint h
  in
  let hint_ =
    match hint_ with
    | Some (p, Aast.Hoption _) ->
      if is_required then Errors.xhp_optional_required_attr p id;
      hint_
    | Some (_, Aast.Happly ((_, "mixed"), [])) -> hint_
    | Some (p, h) ->
      let has_default =
        match default with
        | None
        | Some (_, Aast.Null) ->
          false
        | _ -> true
      in
      if is_required || has_default then
        hint_
      else
        Some (p, Aast.Hoption (p, h))
    | None -> None
  in
  let hint_ = ((), hint_) in
  let hint_ = Aast.type_hint_option_map hint_ ~f:(hint env) in
  let (expr, _) = class_prop_expr_is_xhp env cv in
  let xhp_attr_info = Some { N.xai_tag = tag } in
  {
    N.cv_final = cv.Aast.cv_final;
    N.cv_xhp_attr = xhp_attr_info;
    N.cv_abstract = cv.Aast.cv_abstract;
    N.cv_visibility = cv.Aast.cv_visibility;
    N.cv_type = hint_;
    N.cv_id = cv.Aast.cv_id;
    N.cv_expr = expr;
    N.cv_user_attributes = [];
    N.cv_is_promoted_variadic = cv.Aast.cv_is_promoted_variadic;
    N.cv_doc_comment = cv.Aast.cv_doc_comment (* Can make None to save space *);
    N.cv_is_static = cv.Aast.cv_is_static;
    N.cv_span = cv.Aast.cv_span;
  }

and enum_ env e =
  {
    N.e_base = hint env e.Aast.e_base;
    N.e_constraint = Option.map e.Aast.e_constraint (hint env);
  }

and type_paraml ?(forbid_this = false) env tparams =
  List.map tparams ~f:(type_param ~forbid_this env)

and type_param ~forbid_this ((genv, _) as env) t =
  begin
    if
    TypecheckerOptions.experimental_feature_enabled
      (Provider_context.get_tcopt genv.ctx)
      TypecheckerOptions.experimental_type_param_shadowing
  then
      (* Treat type params as inline class declarations that don't go into the naming heap *)
      let (pos, name) =
        NS.elaborate_id genv.namespace NS.ElaborateClass t.Aast.tp_name
      in
      match Naming_provider.get_type_pos genv.ctx name with
      | Some def_pos ->
        let (def_pos, _) = GEnv.get_full_pos genv.ctx (def_pos, name) in
        Errors.error_name_already_bound name name pos def_pos
      | None ->
        (match GEnv.type_canon_name genv.ctx name with
        | Some canonical ->
          let def_pos =
            Option.value ~default:Pos.none (GEnv.type_pos genv.ctx canonical)
          in
          Errors.error_name_already_bound name canonical pos def_pos
        | None -> ())
  end;
  {
    N.tp_variance = t.Aast.tp_variance;
    tp_name = t.Aast.tp_name;
    tp_constraints =
      List.map t.Aast.tp_constraints (constraint_ ~forbid_this env);
    tp_reified = t.Aast.tp_reified;
    tp_user_attributes = user_attributes env t.Aast.tp_user_attributes;
  }

and type_where_constraints env locl_cstrl =
  List.map
    ~f:(fun (h1, ck, h2) ->
      let ty1 = hint ~in_where_clause:true env h1 in
      let ty2 = hint ~in_where_clause:true env h2 in
      (ty1, ck, ty2))
    locl_cstrl

and class_prop_expr_is_xhp env cv =
  let expr = Option.map cv.Aast.cv_expr (expr env) in
  let expr =
    if (fst env).in_mode = FileInfo.Mdecl && expr = None then
      Some (fst cv.Aast.cv_id, N.Any)
    else
      expr
  in
  let is_xhp =
    try String.sub (snd cv.Aast.cv_id) 0 1 = ":"
    with Invalid_argument _ -> false
  in
  (expr, is_xhp)

and make_xhp_attr = function
  | true -> Some { N.xai_tag = None }
  | false -> None

and class_prop_static env cv =
  let attrs = user_attributes env cv.Aast.cv_user_attributes in
  let lsb = Naming_attributes.mem SN.UserAttributes.uaLSB attrs in
  let forbid_this = not lsb in
  let h =
    Aast.type_hint_option_map ~f:(hint ~forbid_this env) cv.Aast.cv_type
  in
  let (expr, is_xhp) = class_prop_expr_is_xhp env cv in
  {
    N.cv_final = cv.Aast.cv_final;
    N.cv_xhp_attr = make_xhp_attr is_xhp;
    N.cv_abstract = cv.Aast.cv_abstract;
    N.cv_visibility = cv.Aast.cv_visibility;
    N.cv_type = h;
    N.cv_id = cv.Aast.cv_id;
    N.cv_expr = expr;
    N.cv_user_attributes = attrs;
    N.cv_is_promoted_variadic = cv.Aast.cv_is_promoted_variadic;
    N.cv_doc_comment = cv.Aast.cv_doc_comment (* Can make None to save space *);
    N.cv_is_static = cv.Aast.cv_is_static;
    N.cv_span = cv.Aast.cv_span;
  }

and class_prop_non_static env ?(const = None) cv =
  let h = Aast.type_hint_option_map ~f:(hint env) cv.Aast.cv_type in
  let attrs = user_attributes env cv.Aast.cv_user_attributes in
  (* if class is __Const, make all member fields __Const *)
  let attrs =
    match const with
    | Some c ->
      if not (Naming_attributes.mem SN.UserAttributes.uaConst attrs) then
        c :: attrs
      else
        attrs
    | None -> attrs
  in
  let (expr, is_xhp) = class_prop_expr_is_xhp env cv in
  {
    N.cv_final = cv.Aast.cv_final;
    N.cv_xhp_attr = make_xhp_attr is_xhp;
    N.cv_visibility = cv.Aast.cv_visibility;
    N.cv_type = h;
    N.cv_abstract = cv.Aast.cv_abstract;
    N.cv_id = cv.Aast.cv_id;
    N.cv_expr = expr;
    N.cv_user_attributes = attrs;
    N.cv_is_promoted_variadic = cv.Aast.cv_is_promoted_variadic;
    N.cv_doc_comment = cv.Aast.cv_doc_comment (* Can make None to save space *);
    N.cv_is_static = cv.Aast.cv_is_static;
    N.cv_span = cv.Aast.cv_span;
  }

and check_constant_expr env (pos, e) =
  match e with
  | Aast.Id _
  | Aast.Null
  | Aast.True
  | Aast.False
  | Aast.Int _
  | Aast.Float _
  | Aast.String _ ->
    true
  | Aast.Class_const ((_, Aast.CIexpr (_, cls)), _)
    when match cls with
         | Aast.Id (_, "static") -> false
         | _ -> true ->
    true
  | Aast.Unop
      ((Ast_defs.Uplus | Ast_defs.Uminus | Ast_defs.Utild | Ast_defs.Unot), e)
    ->
    check_constant_expr env e
  | Aast.Binop (op, e1, e2) ->
    (* Only assignment is invalid *)
    begin
      match op with
      | Ast_defs.Eq _ ->
        Errors.illegal_constant pos;
        false
      | _ -> check_constant_expr env e1 && check_constant_expr env e2
    end
  | Aast.Eif (e1, e2, e3) ->
    check_constant_expr env e1
    && Option.for_all e2 (check_constant_expr env)
    && check_constant_expr env e3
  | Aast.Array l -> List.for_all l ~f:(check_afield_constant_expr env)
  | Aast.Darray (_, l) ->
    List.for_all l ~f:(fun (e1, e2) ->
        check_constant_expr env e1 && check_constant_expr env e2)
  | Aast.Varray (_, l) -> List.for_all l ~f:(check_constant_expr env)
  | Aast.Shape fdl ->
    (* Only check the values because shape field names are always legal *)
    List.for_all fdl ~f:(fun (_, e) -> check_constant_expr env e)
  | Aast.Call (_, (_, Aast.Id (_, cn)), _, el, unpacked_element)
    when cn = SN.AutoimportedFunctions.fun_
         || cn = SN.AutoimportedFunctions.class_meth
         || cn = SN.StdlibFunctions.mark_legacy_hack_array
         || cn = SN.StdlibFunctions.array_mark_legacy
         (* Tuples are not really function calls, they are just parsed that way*)
         || cn = SN.SpecialFunctions.tuple ->
    arg_unpack_unexpected unpacked_element;
    List.for_all el ~f:(check_constant_expr env)
  | Aast.FunctionPointer (((_, Aast.Id _) | (_, Aast.Class_const _)), _) -> true
  | Aast.Collection (id, _, l) ->
    let (p, cn) = NS.elaborate_id (fst env).namespace NS.ElaborateClass id in
    (* Only vec/keyset/dict are allowed because they are value types *)
    if
      cn = SN.Collections.cVec
      || cn = SN.Collections.cKeyset
      || cn = SN.Collections.cDict
    then
      List.for_all l ~f:(check_afield_constant_expr env)
    else (
      Errors.illegal_constant p;
      false
    )
  | Aast.As (e, (_, Aast.Hlike _), _) -> check_constant_expr env e
  | Aast.As (e, (_, Aast.Happly (id, [_])), _) ->
    let (p, cn) = NS.elaborate_id (fst env).namespace NS.ElaborateClass id in
    if cn = SN.FB.cIncorrectType then
      check_constant_expr env e
    else (
      Errors.illegal_constant p;
      false
    )
  | _ ->
    Errors.illegal_constant pos;
    false

and check_afield_constant_expr env afield =
  match afield with
  | Aast.AFvalue e -> check_constant_expr env e
  | Aast.AFkvalue (e1, e2) ->
    check_constant_expr env e1 && check_constant_expr env e2

and constant_expr env e =
  let valid_constant_expression = check_constant_expr env e in
  if valid_constant_expression then
    expr env e
  else
    (fst e, N.Any)

and class_const env cc =
  let h = Option.map cc.Aast.cc_type (hint env) in
  let e = Option.map cc.Aast.cc_expr (constant_expr env) in
  {
    N.cc_type = h;
    N.cc_id = cc.Aast.cc_id;
    N.cc_expr = e;
    N.cc_doc_comment = cc.Aast.cc_doc_comment;
  }

and typeconst env t =
  let abstract =
    match t.Aast.c_tconst_abstract with
    | Aast.TCAbstract (Some default) ->
      Aast.TCAbstract (Some (hint env default))
    | _ -> t.Aast.c_tconst_abstract
  in
  let constr = Option.map t.Aast.c_tconst_constraint (hint env) in
  let type_ = Option.map t.Aast.c_tconst_type (hint env) in
  let attrs = user_attributes env t.Aast.c_tconst_user_attributes in
  N.
    {
      c_tconst_abstract = abstract;
      c_tconst_name = t.Aast.c_tconst_name;
      c_tconst_constraint = constr;
      c_tconst_type = type_;
      c_tconst_user_attributes = attrs;
      c_tconst_span = t.Aast.c_tconst_span;
      c_tconst_doc_comment = t.Aast.c_tconst_doc_comment;
    }

and method_ genv m =
  let genv = extend_params genv m.Aast.m_tparams in
  let env = (genv, Env.empty_local None) in
  (* Cannot use 'this' if it is a public instance method *)
  let (variadicity, paraml) = fun_paraml env m.Aast.m_params in
  let tparam_l = type_paraml env m.Aast.m_tparams in
  let where_constraints =
    type_where_constraints env m.Aast.m_where_constraints
  in
  let ret =
    Aast.type_hint_option_map ~f:(hint ~allow_retonly:true env) m.Aast.m_ret
  in
  let body =
    match genv.in_mode with
    | FileInfo.Mdecl
    | FileInfo.Mphp ->
      { N.fb_ast = []; fb_annotation = Nast.NamedWithUnsafeBlocks }
    | FileInfo.Mstrict
    | FileInfo.Mpartial ->
      if Nast.is_body_named m.Aast.m_body then (
        let env = List.fold_left ~f:Env.add_param m.N.m_params ~init:env in
        let env =
          match m.N.m_variadic with
          | N.FVellipsis _
          | N.FVnonVariadic ->
            env
          | N.FVvariadicArg param -> Env.add_param env param
        in
        let fub_ast = block env m.N.m_body.N.fb_ast in
        let annotation = Nast.Named in
        Env.check_goto_references env;
        { N.fb_ast = fub_ast; fb_annotation = annotation }
      ) else
        failwith "ast_to_nast error unnamedbody in method_"
  in
  let attrs = user_attributes env m.Aast.m_user_attributes in
  {
    N.m_annotation = ();
    N.m_span = m.Aast.m_span;
    N.m_final = m.Aast.m_final;
    N.m_visibility = m.Aast.m_visibility;
    N.m_abstract = m.Aast.m_abstract;
    N.m_static = m.Aast.m_static;
    N.m_name = m.Aast.m_name;
    N.m_tparams = tparam_l;
    N.m_where_constraints = where_constraints;
    N.m_params = paraml;
    N.m_body = body;
    N.m_fun_kind = m.Aast.m_fun_kind;
    N.m_ret = ret;
    N.m_variadic = variadicity;
    N.m_user_attributes = attrs;
    N.m_external = m.Aast.m_external;
    N.m_doc_comment = m.Aast.m_doc_comment;
  }

and method_redeclaration env mt =
  if
    not
      (TypecheckerOptions.experimental_feature_enabled
         (Provider_context.get_tcopt (fst env).ctx)
         TypecheckerOptions.experimental_trait_method_redeclarations)
  then
    Errors.experimental_feature
      (fst mt.Aast.mt_name)
      "trait method redeclarations";
  let genv = extend_params (fst env) mt.Aast.mt_tparams in
  let env = (genv, Env.empty_local None) in
  let (variadicity, paraml) = fun_paraml env mt.Aast.mt_params in
  let tparam_l = type_paraml env mt.Aast.mt_tparams in
  let where_constraints =
    type_where_constraints env mt.Aast.mt_where_constraints
  in
  let ret =
    Aast.type_hint_option_map ~f:(hint ~allow_retonly:true env) mt.Aast.mt_ret
  in
  {
    N.mt_final = mt.Aast.mt_final;
    N.mt_visibility = mt.Aast.mt_visibility;
    N.mt_abstract = mt.Aast.mt_abstract;
    N.mt_static = mt.Aast.mt_static;
    N.mt_name = mt.Aast.mt_name;
    N.mt_tparams = tparam_l;
    N.mt_where_constraints = where_constraints;
    N.mt_params = paraml;
    N.mt_fun_kind = mt.Aast.mt_fun_kind;
    N.mt_ret = ret;
    N.mt_variadic = variadicity;
    N.mt_trait = hint ~allow_typedef:false env mt.Aast.mt_trait;
    N.mt_method = mt.Aast.mt_method;
    N.mt_user_attributes = [];
  }

and fun_paraml env paraml =
  let _ = List.fold_left ~f:check_repetition ~init:SSet.empty paraml in
  let (variadicity, paraml) = determine_variadicity env paraml in
  (variadicity, List.map ~f:(fun_param env) paraml)

(* Variadic params are removed from the list *)
and determine_variadicity env paraml =
  match paraml with
  | [] -> (N.FVnonVariadic, [])
  | [x] ->
    begin
      match (x.Aast.param_is_variadic, x.Aast.param_name) with
      | (false, _) -> (N.FVnonVariadic, paraml)
      | (true, "...") -> (N.FVellipsis x.Aast.param_pos, [])
      | (true, _) -> (N.FVvariadicArg (fun_param env x), [])
    end
  | x :: rl ->
    let (variadicity, rl) = determine_variadicity env rl in
    (variadicity, x :: rl)

and fun_param env (param : Nast.fun_param) =
  let p = param.Aast.param_pos in
  let name = param.Aast.param_name in
  let ident = Local_id.make_unscoped name in
  Env.add_lvar env (p, name) (p, ident);
  let tyhi =
    Aast.type_hint_option_map param.Aast.param_type_hint ~f:(hint env)
  in
  let eopt = Option.map param.Aast.param_expr (expr env) in
  {
    N.param_annotation = p;
    param_type_hint = tyhi;
    param_is_variadic = param.Aast.param_is_variadic;
    param_pos = p;
    param_name = name;
    param_expr = eopt;
    param_callconv = param.Aast.param_callconv;
    param_user_attributes = user_attributes env param.Aast.param_user_attributes;
    param_visibility = param.Aast.param_visibility;
  }

and make_constraints paraml =
  List.fold_right
    ~init:SMap.empty
    ~f:(fun { Aast.tp_name = (_, x); tp_constraints; tp_reified; _ } acc ->
      SMap.add x (tp_reified, tp_constraints) acc)
    paraml

and extend_params genv paraml =
  let params =
    List.fold_right
      paraml
      ~init:genv.type_params
      ~f:(fun {
                Aast.tp_name = (_, x);
                tp_constraints = cstr_list;
                tp_reified = r;
                _;
              }
              acc
              -> SMap.add x (r, cstr_list) acc)
  in
  { genv with type_params = params }

and fun_ ctx f =
  let tparams = make_constraints f.Aast.f_tparams in
  let genv = Env.make_fun_decl_genv ctx tparams f in
  let lenv = Env.empty_local None in
  let env = (genv, lenv) in
  let f =
    elaborate_namespaces#on_fun_def
      (Naming_elaborate_namespaces_endo.make_env (fst env).namespace)
      f
  in
  let name = Env.fun_id env f.Aast.f_name in
  let where_constraints =
    type_where_constraints env f.Aast.f_where_constraints
  in
  let h =
    Aast.type_hint_option_map ~f:(hint ~allow_retonly:true env) f.Aast.f_ret
  in
  let (variadicity, paraml) = fun_paraml env f.Aast.f_params in
  let f_tparams = type_paraml env f.Aast.f_tparams in
  let f_kind = f.Aast.f_fun_kind in
  let body =
    match genv.in_mode with
    | FileInfo.Mdecl
    | FileInfo.Mphp ->
      { N.fb_ast = []; fb_annotation = Nast.NamedWithUnsafeBlocks }
    | FileInfo.Mstrict
    | FileInfo.Mpartial ->
      if Nast.is_body_named f.Aast.f_body then
        let env = List.fold_left ~f:Env.add_param paraml ~init:env in
        let env =
          match variadicity with
          | N.FVellipsis _
          | N.FVnonVariadic ->
            env
          | N.FVvariadicArg param -> Env.add_param env param
        in
        let fb_ast = block env f.Aast.f_body.Aast.fb_ast in
        let annotation = Nast.Named in
        let _ = Env.check_goto_references env in
        { N.fb_ast; fb_annotation = annotation }
      else
        failwith "ast_to_nast error unnamedbody in fun_"
  in
  let named_fun =
    {
      N.f_annotation = ();
      f_span = f.Aast.f_span;
      f_mode = f.Aast.f_mode;
      f_ret = h;
      f_name = name;
      f_tparams;
      f_where_constraints = where_constraints;
      f_params = paraml;
      f_body = body;
      f_fun_kind = f_kind;
      f_variadic = variadicity;
      f_user_attributes = user_attributes env f.Aast.f_user_attributes;
      (* Fix file attributes if they are important *)
      f_file_attributes = [];
      f_external = f.Aast.f_external;
      f_namespace = f.Aast.f_namespace;
      f_doc_comment = f.Aast.f_doc_comment;
      f_static = f.Aast.f_static;
    }
  in
  named_fun

and get_using_vars (_, e) =
  match e with
  | Aast.Expr_list using_clauses -> List.concat_map using_clauses get_using_vars
  (* Simple assignment to local of form `$lvar = e` *)
  | Aast.Binop (Ast_defs.Eq None, (_, Aast.Lvar (p, lid)), _) ->
    [(p, Local_id.get_name lid)]
  (* Arbitrary expression. This will be assigned to a temporary *)
  | _ -> []

and stmt env (pos, st) =
  let stmt =
    match st with
    | Aast.Block _ -> failwith "stmt block error"
    | Aast.Fallthrough -> N.Fallthrough
    | Aast.Noop -> N.Noop
    | Aast.Markup _ -> N.Noop
    | Aast.Break -> Aast.Break
    | Aast.Continue -> Aast.Continue
    | Aast.Throw e -> N.Throw (expr env e)
    | Aast.Return e -> N.Return (Option.map e (expr env))
    | Aast.GotoLabel label -> name_goto_label env label
    | Aast.Goto label -> name_goto env label
    | Aast.Awaitall (el, b) -> awaitall_stmt env el b
    | Aast.If (e, b1, b2) -> if_stmt env e b1 b2
    | Aast.Do (b, e) -> do_stmt env b e
    | Aast.While (e, b) -> N.While (expr env e, block env b)
    | Aast.Using s ->
      using_stmt env s.Aast.us_has_await s.Aast.us_expr s.Aast.us_block
    | Aast.For (st1, e, st2, b) -> for_stmt env st1 e st2 b
    | Aast.Switch (e, cl) -> switch_stmt env e cl
    | Aast.Foreach (e, ae, b) -> foreach_stmt env e ae b
    | Aast.Try (b, cl, fb) -> try_stmt env b cl fb
    | Aast.Expr
        (cp, Aast.Call (_, (p, Aast.Id (fp, fn)), hl, el, unpacked_element))
      when fn = SN.AutoimportedFunctions.invariant ->
      (* invariant is subject to a source-code transform in the HHVM
       * runtime: the arguments to invariant are lazily evaluated only in
       * the case in which the invariant condition does not hold. So:
       *
       *   invariant_violation(<condition>, <format>, <format_args...>)
       *
       * ... is rewritten as:
       *
       *   if (!<condition>) {
       *     invariant_violation(<format>, <format_args...>);
       *   }
       *)
      begin
        match el with
        | []
        | [_] ->
          Errors.naming_too_few_arguments p;
          N.Expr (cp, N.Any)
        | (cond_p, cond) :: el ->
          let violation =
            ( cp,
              Aast.Call
                ( Aast.Cnormal,
                  (p, Aast.Id (fp, SN.AutoimportedFunctions.invariant_violation)),
                  hl,
                  el,
                  unpacked_element ) )
          in
          if cond <> Aast.False then
            let (b1, b2) =
              ([(cp, Aast.Expr violation)], [(Pos.none, Aast.Noop)])
            in
            let cond = (cond_p, Aast.Unop (Ast_defs.Unot, (cond_p, cond))) in
            if_stmt env cond b1 b2
          else
            (* a false <condition> means unconditional invariant_violation *)
            N.Expr (expr env violation)
      end
    | Aast.Expr e -> N.Expr (expr env e)
  in
  (pos, stmt)

and if_stmt env e b1 b2 =
  let e = expr env e in
  Env.scope env (fun env ->
      let b1 = branch env b1 in
      let b2 = branch env b2 in
      N.If (e, b1, b2))

and do_stmt env b e =
  let b = block ~new_scope:false env b in
  let e = expr env e in
  N.Do (b, e)

(* Scoping is essentially that of do: block is always executed *)
and using_stmt env has_await e b =
  let vars = get_using_vars e in
  let e = expr env e in
  let b = block ~new_scope:false env b in
  Env.remove_locals env vars;
  N.Using
    N.
      {
        us_is_block_scoped = false;
        (* This isn't used for naming so provide a default *)
        us_has_await = has_await;
        us_expr = e;
        us_block = b;
      }

and for_stmt env e1 e2 e3 b =
  (* The initialization and condition expression should be in the outer scope,
   * as they are always executed. *)
  let e1 = expr env e1 in
  let e2 = expr env e2 in
  Env.scope env (fun env ->
      (* The third expression (iteration step) should have the same scope as the
       * block, as it is not always executed. *)
      let b = block ~new_scope:false env b in
      let e3 = expr env e3 in
      N.For (e1, e2, e3, b))

and switch_stmt env e cl =
  let e = expr env e in
  Env.scope env (fun env ->
      let cl = casel env cl in
      N.Switch (e, cl))

and foreach_stmt env e ae b =
  let e = expr env e in
  Env.scope env (fun env ->
      let ae = as_expr env ae in
      let b = block env b in
      N.Foreach (e, ae, b))

and get_lvalues (acc : Pos.t SMap.t) (p, e) : Pos.t SMap.t =
  match e with
  | Aast.List lv -> List.fold_left ~init:acc ~f:get_lvalues lv
  | Aast.Lvar (_, lid) -> SMap.add (Local_id.to_string lid) p acc
  | _ -> acc

and as_expr env ae =
  let handle_v ev =
    match ev with
    | (p, Aast.Id _) ->
      Errors.expected_variable p;
      (p, N.Lvar (Env.new_lvar env (p, "__internal_placeholder")))
    | ev ->
      let vars = get_lvalues SMap.empty ev in
      SMap.iter (fun x p -> ignore (Env.new_lvar env (p, x))) vars;
      expr env ev
  in
  let handle_k ek =
    match ek with
    | (_, Aast.Lvar (p, lid)) ->
      let x = (p, Local_id.get_name lid) in
      (p, N.Lvar (Env.new_lvar env x))
    | (p, _) ->
      Errors.expected_variable p;
      (p, N.Lvar (Env.new_lvar env (p, "__internal_placeholder")))
  in
  match ae with
  | Aast.As_v ev ->
    let ev = handle_v ev in
    N.As_v ev
  | Aast.As_kv (k, ev) ->
    let k = handle_k k in
    let ev = handle_v ev in
    N.As_kv (k, ev)
  | N.Await_as_v (p, ev) ->
    let ev = handle_v ev in
    N.Await_as_v (p, ev)
  | N.Await_as_kv (p, k, ev) ->
    let k = handle_k k in
    let ev = handle_v ev in
    N.Await_as_kv (p, k, ev)

and try_stmt env b cl fb =
  Env.scope env (fun env ->
      let fb = branch env fb in
      let b = branch env b in
      let cl = catchl env cl in
      N.Try (b, cl, fb))

and stmt_list stl env =
  match stl with
  | [] -> []
  | (_, Aast.Block b) :: rest ->
    let b = stmt_list b env in
    let rest = stmt_list rest env in
    b @ rest
  | x :: rest ->
    let x = stmt env x in
    let rest = stmt_list rest env in
    x :: rest

and block ?(new_scope = true) env stl =
  if new_scope then
    Env.scope env (stmt_list stl)
  else
    stmt_list stl env

and branch env stmt_l = Env.scope env (stmt_list stmt_l)

(**
 * Names a goto label.
 *
 * The goto label is added to the local labels if it is not already there.
 * Otherwise, an error is produced.
 *
 *)
and name_goto_label env ((label_pos, label_name) as label) =
  (match Env.goto_label env label_name with
  | Some original_declaration_pos ->
    Errors.goto_label_already_defined
      label_name
      label_pos
      original_declaration_pos
  | None -> Env.new_goto_label env label);
  N.GotoLabel label

(**
 * Names a goto target.
 *
 * The goto statement's target label is added to the local goto targets.
 *
 *)
and name_goto env label =
  Env.new_goto_target env label;
  N.Goto label

and awaitall_stmt env el b =
  let el =
    List.map
      ~f:(fun (e1, e2) ->
        let e2 = expr env e2 in
        let e1 =
          match e1 with
          | Some lid ->
            let e = (Pos.none, Aast.Lvar lid) in
            let vars = get_lvalues SMap.empty e in
            SMap.iter (fun x p -> ignore (Env.new_lvar env (p, x))) vars;
            e1
          | None -> None
        in
        (e1, e2))
      el
  in
  let s = block env b in
  N.Awaitall (el, s)

and expr_obj_get_name env expr_ =
  match expr_ with
  | (p, Aast.Id x) -> (p, N.Id x)
  | (p, e) -> expr env (p, e)

and exprl env l = List.map ~f:(expr env) l

and oexpr env e = Option.map e (expr env)

and expr env (p, e) = (p, expr_ env p e)

and expr_ env p (e : Nast.expr_) =
  match e with
  | Aast.ParenthesizedExpr e -> N.ParenthesizedExpr (expr env e)
  | Aast.Array l ->
    let tcopt = Provider_context.get_tcopt (fst env).ctx in
    if TypecheckerOptions.disallow_array_literal tcopt then
      Errors.array_literals_disallowed p;
    N.Array (List.map l (afield env))
  | Aast.Varray (ta, l) ->
    N.Varray (Option.map ~f:(targ env) ta, List.map l (expr env))
  | Aast.Darray (tap, l) ->
    let nargs =
      Option.map ~f:(fun (t1, t2) -> (targ env t1, targ env t2)) tap
    in
    N.Darray (nargs, List.map l (fun (e1, e2) -> (expr env e1, expr env e2)))
  | Aast.Collection (id, tal, l) ->
    let (p, cn) = NS.elaborate_id (fst env).namespace NS.ElaborateClass id in
    begin
      match cn with
      | x when Nast.is_vc_kind x ->
        let ta =
          match tal with
          | Some (Aast.CollectionTV tv) -> Some (targ env tv)
          | Some (Aast.CollectionTKV _) ->
            Errors.naming_too_many_arguments p;
            None
          | None -> None
        in
        N.ValCollection
          (Nast.get_vc_kind cn, ta, List.map l (afield_value env cn))
      | x when Nast.is_kvc_kind x ->
        let ta =
          match tal with
          | Some (Aast.CollectionTV _) ->
            Errors.naming_too_few_arguments p;
            None
          | Some (Aast.CollectionTKV (tk, tv)) -> Some (targ env tk, targ env tv)
          | None -> None
        in
        N.KeyValCollection
          (Nast.get_kvc_kind cn, ta, List.map l (afield_kvalue env cn))
      | x when x = SN.Collections.cPair ->
        begin
          match l with
          | [] ->
            Errors.naming_too_few_arguments p;
            N.Any
          | [e1; e2] ->
            let pn = SN.Collections.cPair in
            N.Pair (afield_value env pn e1, afield_value env pn e2)
          | _ ->
            Errors.naming_too_many_arguments p;
            N.Any
        end
      | _ ->
        Errors.expected_collection p cn;
        N.Any
    end
  | Aast.Clone e -> N.Clone (expr env e)
  | Aast.Null -> N.Null
  | Aast.True -> N.True
  | Aast.False -> N.False
  | Aast.Int s -> N.Int s
  | Aast.Float s -> N.Float s
  | Aast.String s -> N.String s
  | Aast.String2 idl -> N.String2 (string2 env idl)
  | Aast.PrefixedString (n, e) -> N.PrefixedString (n, expr env e)
  | Aast.Id x -> N.Id (Env.global_const env x)
  | Aast.Lvar (_, x) when Local_id.to_string x = SN.SpecialIdents.this -> N.This
  | Aast.Lvar (p, x) when Local_id.to_string x = SN.SpecialIdents.dollardollar
    ->
    N.Dollardollar (p, Local_id.make_unscoped SN.SpecialIdents.dollardollar)
  | Aast.Lvar (p, x) when Local_id.to_string x = SN.SpecialIdents.placeholder ->
    N.Lplaceholder p
  | Aast.Lvar x ->
    let x = (fst x, Local_id.to_string @@ snd x) in
    N.Lvar (Env.lvar env x)
  | Aast.PU_atom x -> N.PU_atom x
  | Aast.Obj_get (e1, e2, nullsafe) ->
    (* If we encounter Obj_get(_,_,true) by itself, then it means "?->"
       is being used for instance property access; see the case below for
       handling nullsafe instance method calls to see how this works *)
    N.Obj_get (expr env e1, expr_obj_get_name env e2, nullsafe)
  | Aast.Array_get ((p, Aast.Lvar x), None) ->
    let x = (fst x, Local_id.to_string @@ snd x) in
    let id = (p, N.Lvar (Env.lvar env x)) in
    N.Array_get (id, None)
  | Aast.Array_get (e1, e2) -> N.Array_get (expr env e1, oexpr env e2)
  | Aast.Class_get ((_, Aast.CIexpr (_, Aast.Id x1)), Aast.CGstring x2) ->
    N.Class_get (make_class_id env x1, N.CGstring x2)
  | Aast.Class_get ((_, Aast.CIexpr (_, Aast.Lvar (p, lid))), Aast.CGstring x2)
    ->
    let x1 = (p, Local_id.to_string lid) in
    N.Class_get (make_class_id env x1, N.CGstring x2)
  | Aast.Class_get ((_, Aast.CIexpr x1), Aast.CGstring _) ->
    ensure_name_not_dynamic env x1;
    N.Any
  | Aast.Class_get ((_, Aast.CIexpr x1), Aast.CGexpr x2) ->
    ensure_name_not_dynamic env x1;
    ensure_name_not_dynamic env x2;
    N.Any
  | Aast.Class_get _ -> failwith "Error in Ast_to_nast module for Class_get"
  | Aast.Class_const ((_, Aast.CIexpr (_, Aast.Id x1)), x2) ->
    let (genv, _) = env in
    let (_, name) = NS.elaborate_id genv.namespace NS.ElaborateClass x1 in
    begin
      match Naming_provider.get_type_kind genv.ctx name with
      | Some Naming_types.TTypedef when snd x2 = "class" ->
        N.Typename
          (Env.type_name
             ~context:Errors.ClassContext
             env
             x1
             ~allow_typedef:true
             ~allow_generics:false)
      | _ -> N.Class_const (make_class_id env x1, x2)
    end
  | Aast.Class_const ((_, Aast.CIexpr (_, Aast.Lvar (p, lid))), x2) ->
    let x1 = (p, Local_id.to_string lid) in
    let (genv, _) = env in
    let (_, name) = NS.elaborate_id genv.namespace NS.ElaborateClass x1 in
    begin
      match Naming_provider.get_type_kind genv.ctx name with
      | Some Naming_types.TTypedef when snd x2 = "class" ->
        N.Typename
          (Env.type_name
             ~context:Errors.ClassContext
             env
             x1
             ~allow_typedef:true
             ~allow_generics:false)
      | _ -> N.Class_const (make_class_id env x1, x2)
    end
  | Aast.Class_const _ -> (* TODO: report error in strict mode *) N.Any
  | Aast.PU_identifier ((_, c), s1, s2) ->
    begin
      match c with
      | Aast.CIexpr (_, Aast.Id x1) ->
        N.PU_identifier (make_class_id env x1, s1, s2)
      | _ -> failwith "TODO(T35357243): Error during parsing of PU_identifier"
    end
  | Aast.Call (_, (_, Aast.Id (p, pseudo_func)), tal, el, unpacked_element)
    when pseudo_func = SN.SpecialFunctions.echo ->
    arg_unpack_unexpected unpacked_element;
    N.Call
      ( N.Cnormal,
        (p, N.Id (p, pseudo_func)),
        targl env p tal,
        exprl env el,
        None )
  | Aast.Call (_, (p, Aast.Id (_, cn)), tal, el, unpacked_element)
    when cn = SN.StdlibFunctions.call_user_func ->
    arg_unpack_unexpected unpacked_element;
    begin
      match el with
      | [] ->
        Errors.naming_too_few_arguments p;
        N.Any
      | f :: el ->
        N.Call (N.Cuser_func, expr env f, targl env p tal, exprl env el, None)
    end
  | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, unpacked_element)
    when cn = SN.AutoimportedFunctions.fun_ ->
    arg_unpack_unexpected unpacked_element;
    let (genv, _) = env in
    begin
      match el with
      | [] ->
        Errors.naming_too_few_arguments p;
        N.Any
      | [(_, Aast.String s)] when genv.in_ppl && SN.PPLFunctions.is_reserved s
        ->
        Errors.ppl_meth_pointer p ("fun(" ^ s ^ ")");
        N.Any
      | [(p, Aast.String x)] -> N.Fun_id (p, x)
      | [(p, _)] ->
        Errors.illegal_fun p;
        N.Any
      | _ ->
        Errors.naming_too_many_arguments p;
        N.Any
    end
  | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, unpacked_element)
    when cn = SN.AutoimportedFunctions.inst_meth ->
    arg_unpack_unexpected unpacked_element;
    begin
      match el with
      | []
      | [_] ->
        Errors.naming_too_few_arguments p;
        N.Any
      | [instance; (p, Aast.String meth)] ->
        N.Method_id (expr env instance, (p, meth))
      | [(p, _); _] ->
        Errors.illegal_inst_meth p;
        N.Any
      | _ ->
        Errors.naming_too_many_arguments p;
        N.Any
    end
  | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, unpacked_element)
    when cn = SN.AutoimportedFunctions.meth_caller ->
    arg_unpack_unexpected unpacked_element;
    begin
      match el with
      | []
      | [_] ->
        Errors.naming_too_few_arguments p;
        N.Any
      | [e1; e2] ->
        begin
          match (expr env e1, expr env e2) with
          | ((pc, N.String cl), (pm, N.String meth)) ->
            N.Method_caller
              ( Env.type_name
                  ~context:Errors.ClassContext
                  env
                  (pc, cl)
                  ~allow_typedef:false
                  ~allow_generics:false,
                (pm, meth) )
          | ((_, N.Class_const ((_, N.CI cl), (_, mem))), (pm, N.String meth))
            when mem = SN.Members.mClass ->
            N.Method_caller
              ( Env.type_name
                  ~context:Errors.ClassContext
                  env
                  cl
                  ~allow_typedef:false
                  ~allow_generics:false,
                (pm, meth) )
          | ((p, _), _) ->
            Errors.illegal_meth_caller p;
            N.Any
        end
      | _ ->
        Errors.naming_too_many_arguments p;
        N.Any
    end
  | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, unpacked_element)
    when cn = SN.AutoimportedFunctions.class_meth ->
    arg_unpack_unexpected unpacked_element;
    begin
      match el with
      | []
      | [_] ->
        Errors.naming_too_few_arguments p;
        N.Any
      | [e1; e2] ->
        begin
          match (expr env e1, expr env e2) with
          | ((pc, N.String cl), (pm, N.String meth)) ->
            N.Smethod_id
              ( Env.type_name
                  ~context:Errors.ClassContext
                  env
                  (pc, cl)
                  ~allow_typedef:false
                  ~allow_generics:false,
                (pm, meth) )
          | ((_, N.Id (_, const)), (pm, N.String meth))
            when const = SN.PseudoConsts.g__CLASS__ ->
            (* All of these that use current_cls aren't quite correct
             * inside a trait, as the class should be the using class.
             * It's sufficient for typechecking purposes (we require
             * subclass to be compatible with the trait member/method
             * declarations).
             *)
            (match (fst env).current_cls with
            | Some (cid, _, _) -> N.Smethod_id (cid, (pm, meth))
            | None ->
              Errors.illegal_class_meth p;
              N.Any)
          | ((_, N.Class_const ((_, N.CI cl), (_, mem))), (pm, N.String meth))
            when mem = SN.Members.mClass ->
            N.Smethod_id
              ( Env.type_name
                  ~context:Errors.ClassContext
                  env
                  cl
                  ~allow_typedef:false
                  ~allow_generics:false,
                (pm, meth) )
          | ((p, N.Class_const ((_, N.CIself), (_, mem))), (pm, N.String meth))
            when mem = SN.Members.mClass ->
            (match (fst env).current_cls with
            | Some (cid, _, true) -> N.Smethod_id (cid, (pm, meth))
            | Some (cid, _, false) ->
              Errors.class_meth_non_final_self p (snd cid);
              N.Any
            | None ->
              Errors.illegal_class_meth p;
              N.Any)
          | ((p, N.Class_const ((_, N.CIstatic), (_, mem))), (pm, N.String meth))
            when mem = SN.Members.mClass ->
            (match (fst env).current_cls with
            | Some (cid, _, _) -> N.Smethod_id (cid, (pm, meth))
            | None ->
              Errors.illegal_class_meth p;
              N.Any)
          | ((p, _), _) ->
            Errors.illegal_class_meth p;
            N.Any
        end
      | _ ->
        Errors.naming_too_many_arguments p;
        N.Any
    end
  | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, unpacked_element)
    when cn = SN.SpecialFunctions.assert_ ->
    arg_unpack_unexpected unpacked_element;
    if List.length el <> 1 then Errors.assert_arity p;
    N.Assert
      (N.AE_assert
         (Option.value_map (List.hd el) ~default:(p, N.Any) ~f:(expr env)))
  | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, unpacked_element)
    when cn = SN.SpecialFunctions.tuple ->
    arg_unpack_unexpected unpacked_element;
    (match el with
    | [] ->
      Errors.naming_too_few_arguments p;
      N.Any
    | el -> N.List (exprl env el))
  (* sample, factor, observe, condition *)
  | Aast.Call (_, (p1, Aast.Id (p2, cn)), tal, el, unpacked_element)
    when Env.in_ppl env && SN.PPLFunctions.is_reserved cn ->
    let n_expr = N.Id (p2, cn) in
    N.Call
      ( N.Cnormal,
        (p1, n_expr),
        targl env p tal,
        exprl env el,
        oexpr env unpacked_element )
  | Aast.Call (_, (p, Aast.Id f), tal, el, unpacked_element) ->
    let qualified = Env.fun_id env f in
    N.Call
      ( N.Cnormal,
        (p, N.Id qualified),
        targl env p tal,
        exprl env el,
        oexpr env unpacked_element )
  (* match *)
  (* Handle nullsafe instance method calls here. Because Obj_get is used
     for both instance property access and instance method calls, we need
     to match the entire "Call(Obj_get(..), ..)" pattern here so that we
     only match instance method calls *)
  | Aast.Call
      ( _,
        (p, Aast.Obj_get (e1, e2, Aast.OG_nullsafe)),
        tal,
        el,
        unpacked_element ) ->
    N.Call
      ( N.Cnormal,
        (p, N.Obj_get (expr env e1, expr_obj_get_name env e2, N.OG_nullsafe)),
        targl env p tal,
        exprl env el,
        oexpr env unpacked_element )
  (* Handle all kinds of calls that weren't handled by any of the cases above *)
  | Aast.Call (_, e, tal, el, unpacked_element) ->
    N.Call
      ( N.Cnormal,
        expr env e,
        targl env p tal,
        exprl env el,
        oexpr env unpacked_element )
  | Aast.FunctionPointer ((p, Aast.Id fid), targs) ->
    let e = N.Id (Env.fun_id env fid) in
    N.FunctionPointer ((p, e), targl env p targs)
  | Aast.FunctionPointer (e, targs) ->
    N.FunctionPointer (expr env e, targl env p targs)
  | Aast.Yield_break -> N.Yield_break
  | Aast.Yield e -> N.Yield (afield env e)
  | Aast.Await e -> N.Await (expr env e)
  | Aast.Suspend e -> N.Suspend (expr env e)
  | Aast.List el -> N.List (exprl env el)
  | Aast.Expr_list el -> N.Expr_list (exprl env el)
  | Aast.Cast (ty, e2) ->
    let ((p, x), hl) =
      match ty with
      | (_, Aast.Happly (id, hl)) -> (id, hl)
      | _ -> assert false
    in
    let ty =
      match try_castable_hint ~tp_depth:1 env p x hl with
      | Some ty -> (p, ty)
      | None ->
        let h = hint ~allow_typedef:false env ty in
        Errors.object_cast p;
        h
    in
    N.Cast (ty, expr env e2)
  | Aast.Unop (uop, e) -> N.Unop (uop, expr env e)
  | Aast.Binop ((Ast_defs.Eq None as op), lv, e2) ->
    let e2 = expr env e2 in
    let vars = get_lvalues SMap.empty lv in
    SMap.iter (fun x p -> ignore (Env.new_lvar env (p, x))) vars;
    N.Binop (op, expr env lv, e2)
  | Aast.Binop ((Ast_defs.Eq _ as bop), e1, e2) ->
    N.Binop (bop, expr env e1, expr env e2)
  | Aast.Binop (bop, e1, e2) -> N.Binop (bop, expr env e1, expr env e2)
  | Aast.Pipe (dollardollar, e1, e2) ->
    N.Pipe
      ( (fst dollardollar, Local_id.make_unscoped SN.SpecialIdents.dollardollar),
        expr env e1,
        expr env e2 )
  | Aast.Eif (e1, e2opt, e3) ->
    (* The order matters here, of course -- e1 can define vars that need to
     * be available in e2 and e3. *)
    let e1 = expr env e1 in
    let (e2opt, e3) =
      Env.scope env (fun env ->
          let e2opt = Env.scope env (fun env -> oexpr env e2opt) in
          let e3 = Env.scope env (fun env -> expr env e3) in
          (e2opt, e3))
    in
    N.Eif (e1, e2opt, e3)
  | Aast.Is (e, h) ->
    N.Is (expr env e, hint ~allow_wildcard:true ~allow_like:true env h)
  | Aast.As (e, h, b) ->
    N.As (expr env e, hint ~allow_wildcard:true ~allow_like:true env h, b)
  | Aast.New ((_, Aast.CIexpr (p, Aast.Id x)), tal, el, unpacked_element, _) ->
    N.New
      ( make_class_id env x,
        targl env p tal,
        exprl env el,
        oexpr env unpacked_element,
        p )
  | Aast.New
      ((_, Aast.CIexpr (_, Aast.Lvar (pos, x))), tal, el, unpacked_element, p)
    ->
    N.New
      ( make_class_id env (pos, Local_id.to_string x),
        targl env p tal,
        exprl env el,
        oexpr env unpacked_element,
        p )
  | Aast.New ((_, Aast.CIexpr (p, _e)), tal, el, unpacked_element, _) ->
    if Partial.should_check_error (fst env).in_mode 2060 then
      Errors.dynamic_new_in_strict_mode p;
    N.New
      ( make_class_id env (p, SN.Classes.cUnknown),
        targl env p tal,
        exprl env el,
        oexpr env unpacked_element,
        p )
  | Aast.New _ -> failwith "ast_to_nast aast.new"
  | Aast.Record (id, is_array, l) ->
    let id =
      Env.type_name
        ~context:Errors.RecordContext
        env
        id
        ~allow_typedef:false
        ~allow_generics:false
    in
    let l = List.map l (fun (e1, e2) -> (expr env e1, expr env e2)) in
    N.Record (id, is_array, l)
  | Aast.Efun (f, idl) ->
    let idl =
      List.fold_right idl ~init:[] ~f:(fun ((p, x) as id) acc ->
          if Local_id.to_string x = SN.SpecialIdents.this then (
            Errors.this_as_lexical_variable p;
            acc
          ) else
            id :: acc)
    in
    let idl = List.map ~f:(fun (p, lid) -> (p, Local_id.to_string lid)) idl in
    let idl' = List.map idl (Env.lvar env) in
    let env = (fst env, Env.empty_local None) in
    List.iter2_exn idl idl' (Env.add_lvar env);
    let f = expr_lambda env f in
    N.Efun (f, idl')
  | Aast.Lfun (_, _ :: _) -> assert false
  | Aast.Lfun (f, []) ->
    (* We have to build the capture list while we're finding names in
       the closure body---accumulate it in to_capture. *)
    let to_capture = ref [] in
    let handle_unbound (p, x) =
      let cap = Env.lvar env (p, x) in
      to_capture := cap :: !to_capture;
      cap
    in
    let lenv = Env.empty_local @@ Some handle_unbound in
    let env = (fst env, lenv) in
    let f = expr_lambda env f in
    N.Lfun (f, !to_capture)
  | Aast.Xml (x, al, el) ->
    N.Xml
      ( Env.type_name
          ~context:Errors.ClassContext
          env
          x
          ~allow_typedef:false
          ~allow_generics:false,
        attrl env al,
        exprl env el )
  | Aast.Shape fdl ->
    let shp =
      List.map fdl ~f:(fun (pname, value) ->
          (convert_shape_name env pname, expr env value))
    in
    N.Shape shp
  | Aast.BracedExpr _ -> N.Any
  | Aast.Yield_from e -> N.Yield_from (expr env e)
  | Aast.Import _ -> N.Any
  | Aast.Omitted -> N.Omitted
  | Aast.Callconv (kind, e) -> N.Callconv (kind, expr env e)
  (* The below were not found on the AST.ml so they are not implemented here *)
  | Aast.ValCollection _
  | Aast.KeyValCollection _
  | Aast.This
  | Aast.Dollardollar _
  | Aast.Lplaceholder _
  | Aast.Fun_id _
  | Aast.Method_id _
  | Aast.Method_caller _
  | Aast.Smethod_id _
  | Aast.Pair _
  | Aast.Assert _
  | Aast.Typename _
  | Aast.Any ->
    Errors.internal_error
      p
      "Malformed expr: Expr not found on legacy AST: T39599317";
    Aast.Any

and expr_lambda env f =
  let env = Env.set_ppl env false in
  let h =
    Aast.type_hint_option_map ~f:(hint ~allow_retonly:true env) f.Aast.f_ret
  in
  let (variadicity, paraml) = fun_paraml env f.Aast.f_params in
  (* The bodies of lambdas go through naming in the containing local
   * environment *)
  let body_nast = f_body env f.Aast.f_body in
  let annotation = Nast.Named in
  (* These could all be probably be replaced with a {... where ...} *)
  let body = { N.fb_ast = body_nast; fb_annotation = annotation } in
  {
    N.f_annotation = ();
    f_span = f.Aast.f_span;
    f_mode = (fst env).in_mode;
    f_ret = h;
    f_name = f.Aast.f_name;
    f_params = paraml;
    f_tparams = [];
    f_where_constraints = [];
    f_body = body;
    f_fun_kind = f.Aast.f_fun_kind;
    f_variadic = variadicity;
    f_file_attributes = [];
    f_user_attributes = user_attributes env f.Aast.f_user_attributes;
    f_external = f.Aast.f_external;
    f_namespace = f.Aast.f_namespace;
    f_doc_comment = f.Aast.f_doc_comment;
    f_static = f.Aast.f_static;
  }

and f_body env f_body =
  if Nast.is_body_named f_body then
    block env f_body.Aast.fb_ast
  else
    failwith "Malformed f_body: unexpected UnnamedBody from ast_to_nast"

and make_class_id env ((p, x) as cid) =
  ( p,
    match x with
    | x when x = SN.Classes.cParent ->
      if (fst env).current_cls = None then
        let () = Errors.parent_outside_class p in
        N.CI (p, SN.Classes.cUnknown)
      else
        N.CIparent
    | x when x = SN.Classes.cSelf ->
      if (fst env).current_cls = None then
        let () = Errors.self_outside_class p in
        N.CI (p, SN.Classes.cUnknown)
      else
        N.CIself
    | x when x = SN.Classes.cStatic ->
      if (fst env).current_cls = None then
        let () = Errors.static_outside_class p in
        N.CI (p, SN.Classes.cUnknown)
      else
        N.CIstatic
    | x when x = SN.SpecialIdents.this -> N.CIexpr (p, N.This)
    | x when x = SN.SpecialIdents.dollardollar ->
      (* We won't reach here for "new $$" because the parser creates a
       * proper Ast_defs.Dollardollar node, so make_class_id won't be called with
       * that node. In fact, the parser creates an Ast_defs.Dollardollar for all
       * "$$" except in positions where a classname is expected, like in
       * static member access. So, we only reach here for things
       * like "$$::someMethod()". *)
      N.CIexpr
        (p, N.Lvar (p, Local_id.make_unscoped SN.SpecialIdents.dollardollar))
    | x when x.[0] = '$' -> N.CIexpr (p, N.Lvar (Env.lvar env cid))
    | _ ->
      N.CI
        (Env.type_name
           ~context:Errors.ClassContext
           env
           cid
           ~allow_typedef:false
           ~allow_generics:true) )

and casel env l = List.map l (case env)

and case env c =
  match c with
  | Aast.Default (p, b) ->
    let b = branch env b in
    N.Default (p, b)
  | Aast.Case (e, b) ->
    let e = expr env e in
    let b = branch env b in
    N.Case (e, b)

and catchl env l = List.map l (catch env)

and catch env ((p1, lid1), (p2, lid2), b) =
  Env.scope env (fun env ->
      let name2 = Local_id.get_name lid2 in
      let x2 = Env.new_lvar env (p2, name2) in
      let b = branch env b in
      ( Env.type_name
          ~context:Errors.ClassContext
          env
          (p1, lid1)
          ~allow_typedef:true
          ~allow_generics:false,
        x2,
        b ))

and afield env field =
  match field with
  | Aast.AFvalue e -> N.AFvalue (expr env e)
  | Aast.AFkvalue (e1, e2) -> N.AFkvalue (expr env e1, expr env e2)

and afield_value env cname field =
  match field with
  | Aast.AFvalue e -> expr env e
  | Aast.AFkvalue (e1, _e2) ->
    Errors.unexpected_arrow (fst e1) cname;
    expr env e1

and afield_kvalue env cname field =
  match field with
  | Aast.AFvalue e ->
    Errors.missing_arrow (fst e) cname;
    ( expr env e,
      expr
        env
        ( fst e,
          Aast.Lvar (fst e, Local_id.make_unscoped "__internal_placeholder") )
    )
  | Aast.AFkvalue (e1, e2) -> (expr env e1, expr env e2)

and attrl env l = List.map ~f:(attr env) l

and attr env at =
  match at with
  | Aast.Xhp_simple (x, e) -> N.Xhp_simple (x, expr env e)
  | Aast.Xhp_spread e -> N.Xhp_spread (expr env e)

and string2 env idl = List.map idl (expr env)

and class_pu_enum env pu_enum =
  let make_tparam (sid, reified) def =
    Aast.
      {
        tp_variance = Ast_defs.Invariant;
        tp_name = sid;
        tp_constraints =
          (match def with
          | None -> []
          | Some hint -> [(Ast_defs.Constraint_eq, hint)]);
        tp_reified = reified;
        tp_user_attributes = [];
      }
  in
  let pu_case_types = pu_enum.Aast.pu_case_types in
  (* We create here an extended environment to type the abstract part
     of a PU enumeration (namely `case type` and `case` statement).
     Since we are typing the abstract part (`case type/ case`), we only
     add their name, without hints.
  *)
  let env_with_case_types =
    let (genv, lenv) = env in
    let make_tparam tp = make_tparam tp None in
    (extend_params genv (List.map ~f:make_tparam pu_case_types), lenv)
  in
  let pu_case_values =
    List.map
      ~f:(fun (sid, h) -> (sid, hint ~forbid_this:true env_with_case_types h))
      pu_enum.Aast.pu_case_values
  in

  (* Now when naming each member declaration, the environment can be
     updated more precisely:
     - type constraints (type t = foo) are available to have a more precise
       naming
     - each `case type` statement must be instantiated exactly once
     - each `case` statement must be instantiated exactly one
  *)
  (* For each member declaration, we gather:
     - its name
     - its type constraints (`type T = ...`)
     - its type expresssions (`foo = ...`)

     Since a member could be declared in several blocks (e.g.
       :@A(
          type T = ...
       )
       ...
       :@A(
          expr = 42
       )
       (because of extension via inheritance) we take care to update the
       lists/maps only once instead of creating duplicates.
  *)
  let (pu_members, pu_types, pu_exprs) =
    List.fold_left
      pu_enum.Aast.pu_members
      ~init:([], SMap.empty, SMap.empty)
      ~f:(fun (ids, types, exprs)
              Aast.{ pum_atom = id; pum_types; pum_exprs }
              ->
        (* add_id helps to decide if we need to create a new map/list
              or if we can update and existing one *)
        let (add_id, v) =
          match SMap.find_opt (snd id) types with
          | None -> (true, List.rev pum_types)
          | Some types' -> (false, List.rev_append pum_types types')
        in
        let (add_id, w) =
          match SMap.find_opt (snd id) exprs with
          | None -> (true, List.rev pum_exprs)
          | Some exprs' -> (add_id, List.rev_append pum_exprs exprs')
        in
        let ids =
          if add_id then
            id :: ids
          else
            ids
        and types = SMap.add (snd id) v types
        and exprs = SMap.add (snd id) w exprs in
        (ids, types, exprs))
  in
  let pu_members =
    let compute_mapping pum_atom =
      let pum_types = List.rev (SMap.find (snd pum_atom) pu_types) in
      (* Now that the abstract part is translated, we are going to do the same
         for each atom declaration (namely `:@A (type T = ..., foo = bar)`).
         This time, the original environment is extended with the PU types
         _and_ their specific hints since we have everything at hand
      *)
      let env_with_mapped_types =
        let (genv, lenv) = env in
        let make_tparam (tp, h) = make_tparam (tp, Aast.Erased) (Some h) in
        (extend_params genv (List.map ~f:make_tparam pum_types), lenv)
      in
      let pum_types =
        List.map pum_types ~f:(fun (id, h) ->
            (id, hint ~forbid_this:true env h))
      in
      let pum_exprs = List.rev (SMap.find (snd pum_atom) pu_exprs) in
      let pum_exprs =
        List.map ~f:(fun (s, e) -> (s, expr env_with_mapped_types e)) pum_exprs
      in
      { Aast.pum_atom; Aast.pum_types; Aast.pum_exprs }
    in
    List.map ~f:compute_mapping pu_members
  in
  {
    Aast.pu_annotation = ();
    Aast.pu_name = pu_enum.Aast.pu_name;
    Aast.pu_is_final = pu_enum.Aast.pu_is_final;
    Aast.pu_case_types = pu_enum.Aast.pu_case_types;
    pu_case_values;
    pu_members;
  }

let record_field env rf =
  let (id, h, e) = rf in
  let h = hint env h in
  let e = oexpr env e in
  (id, h, e)

let record_def ctx rd =
  let env = Env.make_top_level_env ctx in
  let rd =
    elaborate_namespaces#on_record_def
      (Naming_elaborate_namespaces_endo.make_env (fst env).namespace)
      rd
  in
  let attrs = user_attributes env rd.Aast.rd_user_attributes in
  let extends =
    match rd.Aast.rd_extends with
    | Some extends -> Some (hint ~context:Errors.RecordContext env extends)
    | None -> None
  in
  let fields = List.map rd.Aast.rd_fields ~f:(record_field env) in
  {
    N.rd_annotation = ();
    rd_name = rd.Aast.rd_name;
    rd_abstract = rd.Aast.rd_abstract;
    rd_extends = extends;
    rd_fields = fields;
    rd_user_attributes = attrs;
    rd_namespace = rd.Aast.rd_namespace;
    rd_span = rd.Aast.rd_span;
    rd_doc_comment = rd.Aast.rd_doc_comment;
    rd_emit_id = rd.Aast.rd_emit_id;
  }

(**************************************************************************)
(* Typedefs *)
(**************************************************************************)

let typedef ctx tdef =
  let cstrs = make_constraints tdef.Aast.t_tparams in
  let env = Env.make_typedef_env ctx cstrs tdef in
  let tdef =
    elaborate_namespaces#on_typedef
      (Naming_elaborate_namespaces_endo.make_env (fst env).namespace)
      tdef
  in
  let tconstraint = Option.map tdef.Aast.t_constraint (hint env) in
  let tparaml = type_paraml env tdef.Aast.t_tparams in
  let attrs = user_attributes env tdef.Aast.t_user_attributes in
  {
    N.t_annotation = ();
    t_name = tdef.Aast.t_name;
    t_tparams = tparaml;
    t_constraint = tconstraint;
    t_kind = hint env tdef.Aast.t_kind;
    t_user_attributes = attrs;
    t_mode = tdef.Aast.t_mode;
    t_namespace = tdef.Aast.t_namespace;
    t_vis = tdef.Aast.t_vis;
    t_emit_id = tdef.Aast.t_emit_id;
  }

(**************************************************************************)
(* Global constants *)
(**************************************************************************)

let global_const ctx cst =
  let env = Env.make_const_env ctx cst in
  let cst =
    elaborate_namespaces#on_gconst
      (Naming_elaborate_namespaces_endo.make_env (fst env).namespace)
      cst
  in
  let hint = Option.map cst.Aast.cst_type (hint env) in
  let e = constant_expr env cst.Aast.cst_value in
  {
    N.cst_annotation = ();
    cst_mode = cst.Aast.cst_mode;
    cst_name = cst.Aast.cst_name;
    cst_type = hint;
    cst_value = e;
    cst_namespace = cst.Aast.cst_namespace;
    cst_span = cst.Aast.cst_span;
    cst_emit_id = cst.Aast.cst_emit_id;
  }

(**************************************************************************)
(* The entry point to CHECK the program, and transform the program *)
(**************************************************************************)

let program ctx ast =
  let ast =
    elaborate_namespaces#on_program
      (Naming_elaborate_namespaces_endo.make_env
         Namespace_env.empty_with_default)
      ast
  in
  let top_level_env = ref (Env.make_top_level_env ctx) in
  let rec aux acc def =
    match def with
    | Aast.Fun f -> N.Fun (fun_ ctx f) :: acc
    | Aast.Class c -> N.Class (class_ ctx c) :: acc
    | Aast.Stmt (_, Aast.Noop)
    | Aast.Stmt (_, Aast.Markup _) ->
      acc
    | Aast.Stmt s -> N.Stmt (stmt !top_level_env s) :: acc
    | Aast.RecordDef rd -> N.RecordDef (record_def ctx rd) :: acc
    | Aast.Typedef t -> N.Typedef (typedef ctx t) :: acc
    | Aast.Constant cst -> N.Constant (global_const ctx cst) :: acc
    | Aast.Namespace (_ns, aast) -> List.fold_left ~f:aux ~init:[] aast @ acc
    | Aast.NamespaceUse _ -> acc
    | Aast.SetNamespaceEnv nsenv ->
      let (genv, lenv) = !top_level_env in
      let genv = { genv with namespace = nsenv } in
      top_level_env := (genv, lenv);
      acc
    | Aast.FileAttributes _ -> acc
  in
  let on_program aast =
    let nast = List.fold_left ~f:aux ~init:[] aast in
    List.rev nast
  in
  on_program ast
