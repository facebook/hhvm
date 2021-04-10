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

open Hh_prelude
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

type is_final = bool

type genv = {
  (* strict? decl? partial? *)
  in_mode: FileInfo.mode;
  (* various options that control the strictness of the typechecker *)
  ctx: Provider_context.t;
  (* In function foo<T1, ..., Tn> or class<T1, ..., Tn>, the field
   * type_params knows T1 .. Tn. It is able to find out about the
   * constraint on these parameters. *)
  type_params: SSet.t;
  (* The current class, None if we are in a function *)
  current_cls: (Ast_defs.id * Ast_defs.class_kind * is_final) option;
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

  val make_class_env : Provider_context.t -> Nast.class_ -> genv * lenv

  val make_typedef_env : Provider_context.t -> Nast.typedef -> genv * lenv

  val make_top_level_env : Provider_context.t -> genv * lenv

  val make_fun_decl_genv : Provider_context.t -> Nast.fun_ -> genv

  val make_file_attributes_env :
    Provider_context.t -> FileInfo.mode -> Aast.nsenv -> genv * lenv

  val make_const_env : Provider_context.t -> Nast.gconst -> genv * lenv

  val add_lvar : genv * lenv -> Ast_defs.id -> positioned_ident -> unit

  val add_param : genv * lenv -> Nast.fun_param -> genv * lenv

  val new_lvar : genv * lenv -> Ast_defs.id -> positioned_ident

  val lvar : genv * lenv -> Ast_defs.id -> positioned_ident

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
  }

  let get_tparam_names paraml =
    List.fold_right
      ~init:SSet.empty
      ~f:(fun { Aast.tp_name = (_, x); _ } acc -> SSet.add x acc)
      paraml

  let empty_local unbound_handler = { locals = ref SMap.empty; unbound_handler }

  let make_class_genv ctx tparams mode (cid, ckind) namespace final =
    {
      in_mode = mode;
      ctx;
      type_params = get_tparam_names tparams;
      current_cls = Some (cid, ckind, final);
      namespace;
    }

  let make_class_env ctx c =
    let genv =
      make_class_genv
        ctx
        c.Aast.c_tparams
        c.Aast.c_mode
        (c.Aast.c_name, c.Aast.c_kind)
        c.Aast.c_namespace
        c.Aast.c_final
    in
    let lenv = empty_local None in
    (genv, lenv)

  let make_typedef_genv ctx tparams tdef_namespace =
    {
      in_mode = FileInfo.Mstrict;
      ctx;
      type_params = get_tparam_names tparams;
      current_cls = None;
      namespace = tdef_namespace;
    }

  let make_typedef_env ctx tdef =
    let genv =
      make_typedef_genv ctx tdef.Aast.t_tparams tdef.Aast.t_namespace
    in
    let lenv = empty_local None in
    (genv, lenv)

  let make_fun_genv ctx params f_mode f_namespace =
    {
      in_mode = f_mode;
      ctx;
      type_params = get_tparam_names params;
      current_cls = None;
      namespace = f_namespace;
    }

  let make_fun_decl_genv ctx f =
    make_fun_genv ctx f.Aast.f_tparams f.Aast.f_mode f.Aast.f_namespace

  let make_const_genv ctx cst =
    {
      in_mode = cst.Aast.cst_mode;
      ctx;
      type_params = SSet.empty;
      current_cls = None;
      namespace = cst.Aast.cst_namespace;
    }

  let make_top_level_genv ctx =
    {
      in_mode = FileInfo.Mpartial;
      ctx;
      type_params = SSet.empty;
      current_cls = None;
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
      type_params = SSet.empty;
      current_cls = None;
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
        SN.Superglobals.is_superglobal x
        && FileInfo.equal_mode genv.in_mode FileInfo.Mpartial
      then
        (p, Local_id.make_unscoped x)
      else
        let lcl = SMap.find_opt x !(env.locals) in
        match lcl with
        | Some lcl -> (p, snd lcl)
        | None -> handle_undefined_variable (genv, env) (p, x)
    in
    (p, ident)

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
  if not (String.equal name SN.SpecialIdents.placeholder) then
    SSet.add name s
  else
    s

let check_name (p, name) =
  (* We perform this check here because currently, naming edits the AST to add
   * a parent node of this class to enums during the AST transform *)
  if
    ( String.equal name SN.Classes.cHH_BuiltinEnum
    || String.equal name SN.Classes.cHH_BuiltinEnumClass )
    && not (string_ends_with (Relative_path.suffix (Pos.filename p)) ".hhi")
  then
    Errors.using_internal_class p (strip_ns name)

let convert_shape_name env = function
  | Ast_defs.SFlit_int (pos, s) -> Ast_defs.SFlit_int (pos, s)
  | Ast_defs.SFlit_str (pos, s) -> Ast_defs.SFlit_str (pos, s)
  | Ast_defs.SFclass_const ((class_pos, class_name), (const_pos, const_name)) ->
    (* e.g. Foo::BAR or self::BAR. The first tuple is the use of Foo, second is the use of BAR *)
    (* We will resolve class-name 'self' *)
    let class_name =
      if String.equal class_name SN.Classes.cSelf then (
        match (fst env).current_cls with
        | Some ((_class_decl_pos, class_name), _, _) -> class_name
        | None ->
          Errors.self_outside_class class_pos;
          SN.Classes.cUnknown
      ) else
        let () = check_name (class_pos, class_name) in
        class_name
    in
    Ast_defs.SFclass_const ((class_pos, class_name), (const_pos, const_name))

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
    ?(forbid_this = false)
    ?(allow_retonly = false)
    ?(allow_wildcard = false)
    ?(allow_like = false)
    ?(in_where_clause = false)
    ?(ignore_hack_arr = false)
    ?(tp_depth = 0)
    env
    (hh : Aast.hint) =
  let (p, h) = hh in
  ( p,
    hint_
      ~forbid_this
      ~allow_retonly
      ~allow_wildcard
      ~allow_like
      ~in_where_clause
      ~ignore_hack_arr
      ~tp_depth
      env
      (p, h) )

and contexts env ctxs =
  let (pos, hl) = ctxs in
  let hl =
    List.map
      ~f:(fun h ->
        match h with
        | (p, Aast.Happly ((_, wildcard), []))
          when String.equal wildcard SN.Typehints.wildcard ->
          (* More helpful wildcard error for coeffects. We expect all valid
           * wildcard hints to be transformed into Hfun_context *)
          Errors.invalid_wildcard_context p;
          (p, N.Herr)
        | _ -> hint env h)
      hl
  in
  (pos, hl)

and hfun env ro hl il variadic_hint ctxs h readonly_ret =
  let variadic_hint = Option.map variadic_hint (hint env) in
  let hl = List.map ~f:(hint env) hl in
  let ctxs = Option.map ~f:(contexts env) ctxs in
  N.Hfun
    N.
      {
        hf_is_readonly = ro;
        hf_param_tys = hl;
        hf_param_info = il;
        hf_variadic_ty = variadic_hint;
        hf_ctxs = ctxs;
        hf_return_ty = hint ~allow_retonly:true env h;
        hf_is_readonly_return = readonly_ret;
      }

and hint_
    ~forbid_this
    ~allow_retonly
    ~allow_wildcard
    ~allow_like
    ~in_where_clause
    ~ignore_hack_arr
    ?(tp_depth = 0)
    env
    (p, x) =
  let tcopt = Provider_context.get_tcopt (fst env).ctx in
  let like_type_hints_enabled = TypecheckerOptions.like_type_hints tcopt in
  let hint = hint ~forbid_this ~allow_wildcard ~allow_like in
  match x with
  | Aast.Hunion hl -> N.Hunion (List.map hl ~f:(hint ~allow_retonly env))
  | Aast.Hintersection hl ->
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
    if TypecheckerOptions.interpret_soft_types_as_like_types tcopt then
      N.Hlike h
    else
      snd h
  | Aast.Hfun
      Aast.
        {
          hf_is_readonly = ro;
          hf_param_tys = hl;
          hf_param_info = il;
          hf_variadic_ty = variadic_hint;
          hf_ctxs = ctxs;
          hf_return_ty = h;
          hf_is_readonly_return = readonly_ret;
        } ->
    hfun env ro hl il variadic_hint ctxs h readonly_ret
  | Aast.Happly (((p, _x) as id), hl) ->
    let hint_id =
      hint_id
        ~forbid_this
        ~allow_retonly
        ~allow_wildcard
        ~ignore_hack_arr
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
      if not (List.is_empty hl) then Errors.unexpected_type_arguments p
    | _ -> ());
    hint_id
  | Aast.Haccess ((pos, root_id), ids) ->
    let root_ty =
      match root_id with
      | Aast.Happly ((pos, x), _) when String.equal x SN.Classes.cSelf ->
        begin
          match (fst env).current_cls with
          | None ->
            Errors.self_outside_class pos;
            N.Herr
          | Some (cid, _, _) -> N.Happly (cid, [])
        end
      | Aast.Happly ((pos, x), _)
        when String.equal x SN.Classes.cStatic
             || String.equal x SN.Classes.cParent ->
        Errors.invalid_type_access_root (pos, x);
        N.Herr
      | Aast.Happly (root, _) ->
        let h =
          hint_id
            ~forbid_this
            ~allow_retonly
            ~allow_wildcard:false
            ~ignore_hack_arr:false
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
      | Aast.Hvar n -> N.Hvar n
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
  | Aast.Hmixed -> N.Hmixed
  | Aast.Hfun_context n -> N.Hfun_context n
  | Aast.Hvar n -> N.Hvar n
  | Aast.Herr
  | Aast.Hany
  | Aast.Hnonnull
  | Aast.Habstr _
  | Aast.Hdarray _
  | Aast.Hvarray _
  | Aast.Hvarray_or_darray _
  | Aast.Hvec_or_dict _
  | Aast.Hprim _
  | Aast.Hthis
  | Aast.Hdynamic
  | Aast.Hnothing ->
    Errors.internal_error Pos.none "Unexpected hint not present on legacy AST";
    N.Herr

and hint_id
    ~forbid_this
    ~allow_retonly
    ~allow_wildcard
    ~ignore_hack_arr
    ~tp_depth
    env
    ((p, x) as id)
    hl =
  let params = (fst env).type_params in
  (* some common Xhp screw ups *)
  if String.equal x "Xhp" || String.equal x ":Xhp" || String.equal x "XHP" then
    Errors.disallowed_xhp_type p x;
  match
    try_castable_hint
      ~forbid_this
      ~allow_wildcard
      ~ignore_hack_arr
      ~tp_depth
      env
      p
      x
      hl
  with
  | Some h -> h
  | None ->
    begin
      match x with
      | x when String.equal x SN.Typehints.wildcard ->
        if allow_wildcard && tp_depth >= 1 (* prevents 3 as _ *) then
          if not (List.is_empty hl) then (
            Errors.typaram_applied_to_type p x;
            N.Herr
          ) else
            N.Happly (id, [])
        else (
          Errors.wildcard_hint_disallowed p;
          N.Herr
        )
      | x
        when String.equal x ("\\" ^ SN.Typehints.void)
             || String.equal x ("\\" ^ SN.Typehints.null)
             || String.equal x ("\\" ^ SN.Typehints.noreturn)
             || String.equal x ("\\" ^ SN.Typehints.int)
             || String.equal x ("\\" ^ SN.Typehints.bool)
             || String.equal x ("\\" ^ SN.Typehints.float)
             || String.equal x ("\\" ^ SN.Typehints.num)
             || String.equal x ("\\" ^ SN.Typehints.string)
             || String.equal x ("\\" ^ SN.Typehints.resource)
             || String.equal x ("\\" ^ SN.Typehints.mixed)
             || String.equal x ("\\" ^ SN.Typehints.nonnull)
             || String.equal x ("\\" ^ SN.Typehints.arraykey) ->
        Errors.primitive_toplevel p;
        N.Herr
      | x when String.equal x ("\\" ^ SN.Typehints.nothing) ->
        Errors.primitive_toplevel p;
        N.Herr
      | x when String.equal x SN.Typehints.void && allow_retonly ->
        N.Hprim N.Tvoid
      | x when String.equal x SN.Typehints.void ->
        Errors.return_only_typehint p `void;
        N.Herr
      | x when String.equal x SN.Typehints.noreturn && allow_retonly ->
        N.Hprim N.Tnoreturn
      | x when String.equal x SN.Typehints.noreturn ->
        Errors.return_only_typehint p `noreturn;
        N.Herr
      | x when String.equal x SN.Typehints.null -> N.Hprim N.Tnull
      | x when String.equal x SN.Typehints.num -> N.Hprim N.Tnum
      | x when String.equal x SN.Typehints.resource -> N.Hprim N.Tresource
      | x when String.equal x SN.Typehints.arraykey -> N.Hprim N.Tarraykey
      | x when String.equal x SN.Typehints.mixed -> N.Hmixed
      | x when String.equal x SN.Typehints.nonnull -> N.Hnonnull
      | x when String.equal x SN.Typehints.dynamic -> N.Hdynamic
      | x when String.equal x SN.Typehints.nothing -> N.Hnothing
      | x when String.equal x SN.Typehints.this && not forbid_this ->
        if not (phys_equal hl []) then Errors.this_no_argument p;
        N.Hthis
      | x when String.equal x SN.Typehints.this ->
        Errors.this_type_forbidden p;
        N.Herr
      (* TODO: Duplicate of a Typing[4101] error if namespaced correctly
       * T56198838 *)
      | x
        when (String.equal x SN.Classes.cClassname || String.equal x "classname")
             && List.length hl <> 1 ->
        Errors.classname_param p;
        N.Hprim N.Tstring
      | _ when String.(lowercase x = SN.Typehints.this) ->
        Errors.lowercase_this p x;
        N.Herr
      | _ when SSet.mem x params ->
        let tcopt = Provider_context.get_tcopt (fst env).ctx in
        let hl =
          if
            (not (TypecheckerOptions.higher_kinded_types tcopt))
            && not (List.is_empty hl)
          then (
            Errors.typaram_applied_to_type p x;
            []
          ) else
            hl
        in

        N.Habstr
          ( x,
            hintl
              ~allow_wildcard
              ~forbid_this
              ~allow_retonly:true
              ~tp_depth:(tp_depth + 1)
              env
              hl )
      | _ ->
        let () = check_name id in
        N.Happly
          ( id,
            hintl
              ~allow_wildcard
              ~forbid_this
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
    ?(forbid_this = false)
    ?(allow_wildcard = false)
    ~ignore_hack_arr
    ~tp_depth
    env
    p
    x
    hl =
  let hint =
    hint
      ~forbid_this
      ~tp_depth:(tp_depth + 1)
      ~allow_wildcard
      ~allow_retonly:false
  in
  let unif env =
    (not ignore_hack_arr)
    && TypecheckerOptions.hack_arr_dv_arrs
         (Provider_context.get_tcopt (fst env).ctx)
  in
  let canon = String.lowercase x in
  let opt_hint =
    match canon with
    | nm when String.equal nm SN.Typehints.int -> Some (N.Hprim N.Tint)
    | nm when String.equal nm SN.Typehints.bool -> Some (N.Hprim N.Tbool)
    | nm when String.equal nm SN.Typehints.float -> Some (N.Hprim N.Tfloat)
    | nm when String.equal nm SN.Typehints.string -> Some (N.Hprim N.Tstring)
    | nm when String.equal nm SN.Typehints.darray ->
      Some
        (match hl with
        | [] ->
          if Partial.should_check_error (fst env).in_mode 2071 then
            Errors.too_few_type_arguments p;
          if unif env then
            N.Happly ((p, SN.Collections.cDict), [(p, N.Hany); (p, N.Hany)])
          else
            N.Hdarray ((p, N.Hany), (p, N.Hany))
        | [_] ->
          Errors.too_few_type_arguments p;
          N.Hany
        | [key_; val_] ->
          if unif env then
            N.Happly ((p, SN.Collections.cDict), [hint env key_; hint env val_])
          else
            N.Hdarray (hint env key_, hint env val_)
        | _ ->
          Errors.too_many_type_arguments p;
          N.Hany)
    | nm when String.equal nm SN.Typehints.varray ->
      Some
        (match hl with
        | [] ->
          if Partial.should_check_error (fst env).in_mode 2071 then
            Errors.too_few_type_arguments p;
          if unif env then
            N.Happly ((p, SN.Collections.cVec), [(p, N.Hany)])
          else
            N.Hvarray (p, N.Hany)
        | [val_] ->
          if unif env then
            N.Happly ((p, SN.Collections.cVec), [hint env val_])
          else
            N.Hvarray (hint env val_)
        | _ ->
          Errors.too_many_type_arguments p;
          N.Hany)
    | nm when String.equal nm SN.Typehints.varray_or_darray ->
      Some
        (match hl with
        | [] ->
          if Partial.should_check_error (fst env).in_mode 2071 then
            Errors.too_few_type_arguments p;

          if unif env then
            N.Hvec_or_dict (None, (p, N.Hany))
          else
            (* Warning: These Hanys are here because they produce subtle
                errors because of interaction with tco_experimental_isarray
                if you change them to Herr *)
            N.Hvarray_or_darray (None, (p, N.Hany))
        | [val_] ->
          if unif env then
            N.Hvec_or_dict (None, hint env val_)
          else
            N.Hvarray_or_darray (None, hint env val_)
        | [key; val_] ->
          if unif env then
            N.Hvec_or_dict (Some (hint env key), hint env val_)
          else
            N.Hvarray_or_darray (Some (hint env key), hint env val_)
        | _ ->
          Errors.too_many_type_arguments p;
          N.Hany)
    | nm when String.equal nm SN.Typehints.vec_or_dict ->
      Some
        (match hl with
        | [] ->
          if Partial.should_check_error (fst env).in_mode 2071 then
            Errors.too_few_type_arguments p;

          N.Hvec_or_dict (None, (p, N.Hany))
        | [val_] -> N.Hvec_or_dict (None, hint env val_)
        | [key; val_] -> N.Hvec_or_dict (Some (hint env key), hint env val_)
        | _ ->
          Errors.too_many_type_arguments p;
          N.Hany)
    | _ -> None
  in
  let () =
    match opt_hint with
    | Some _ when not (String.equal canon x) ->
      Errors.primitive_invalid_alias p x canon
    | _ -> ()
  in
  opt_hint

and hintl ~forbid_this ~allow_retonly ~allow_wildcard ~tp_depth env l =
  List.map ~f:(hint ~forbid_this ~allow_retonly ~allow_wildcard ~tp_depth env) l

let constraint_ ?(forbid_this = false) env (ck, h) =
  (ck, hint ~forbid_this env h)

let targ env (p, t) =
  ( p,
    hint
      ~allow_wildcard:true
      ~forbid_this:false
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
  if not (Ast_defs.is_c_interface c.Aast.c_kind) then
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
  let env = Env.make_class_env ctx c in
  let c =
    elaborate_namespaces#on_class_
      (Naming_elaborate_namespaces_endo.make_env (fst env).namespace)
      c
  in
  let where_constraints =
    type_where_constraints env c.Aast.c_where_constraints
  in
  let name = c.Aast.c_name in
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
  let (enum_bound, enum, in_enum_class) =
    match c.Aast.c_enum with
    | Some enum -> enum_ env name enum
    | None -> (None, None, false)
  in
  let parents = List.map c.Aast.c_extends (hint ~allow_retonly:false env) in
  let parents =
    match enum_bound with
    (* Make enums implicitly extend the BuiltinEnum/BuiltinEnumClass classes in
     * order to provide utility methods.
     *)
    | Some bound ->
      let pos = fst name in
      let builtin =
        if in_enum_class then
          SN.Classes.cHH_BuiltinEnumClass
        else
          SN.Classes.cHH_BuiltinEnum
      in
      let parent = (pos, N.Happly ((pos, builtin), [bound])) in
      parent :: parents
    | None -> parents
  in
  let methods = List.map ~f:(method_ (fst env)) methods in
  let uses = List.map ~f:(hint env) c.Aast.c_uses in
  let xhp_attr_uses = List.map ~f:(hint env) c.Aast.c_xhp_attr_uses in
  let (c_req_extends, c_req_implements) = Aast.split_reqs c in
  if
    (not (List.is_empty c_req_implements))
    && not (Ast_defs.is_c_trait c.Aast.c_kind)
  then
    Errors.invalid_req_implements (fst (List.hd_exn c_req_implements));
  let req_implements = List.map ~f:(hint env) c_req_implements in
  let req_implements = List.map ~f:(fun h -> (h, false)) req_implements in
  if
    (not (List.is_empty c_req_extends))
    && (not (Ast_defs.is_c_trait c.Aast.c_kind))
    && not (Ast_defs.is_c_interface c.Aast.c_kind)
  then
    Errors.invalid_req_extends (fst (List.hd_exn c_req_extends));
  let req_extends = List.map ~f:(hint env) c_req_extends in
  let req_extends = List.map ~f:(fun h -> (h, true)) req_extends in
  (* Setting a class type parameters constraint to the 'this' type is weird
   * so lets forbid it for now.
   *)
  let tparam_l = type_paraml ~forbid_this:true env c.Aast.c_tparams in
  let consts = List.map ~f:(class_const env ~in_enum_class) c.Aast.c_consts in
  let typeconsts = List.map ~f:(typeconst env) c.Aast.c_typeconsts in
  let implements =
    List.map ~f:(hint ~allow_retonly:false env) c.Aast.c_implements
  in
  let constructor = Option.map constructor (method_ (fst env)) in
  let (constructor, methods, smethods) =
    interface c constructor methods smethods
  in
  let file_attributes =
    file_attributes ctx c.Aast.c_mode c.Aast.c_file_attributes
  in
  let c_tparams = tparam_l in
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
    N.c_xhp_attr_uses = xhp_attr_uses;
    N.c_xhp_category = c.Aast.c_xhp_category;
    N.c_reqs = req_extends @ req_implements;
    N.c_implements = implements;
    N.c_implements_dynamic = c.Aast.c_implements_dynamic;
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
    let () = check_name ua_name in
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
    | Some (pos, items) ->
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
    N.cv_readonly = cv.Aast.cv_readonly;
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

and enum_ env enum_name e =
  let open Aast in
  let pos = fst enum_name in
  let enum_hint = (pos, Happly (enum_name, [])) in
  let is_enum_class = e.e_enum_class in
  let old_base = e.e_base in
  let new_base = hint env old_base in
  let bound =
    if is_enum_class then
      (* Turn the base type of the enum class into MemberOf<E, base> *)
      let elt = (pos, SN.Classes.cMemberOf) in
      let h = (pos, Happly (elt, [enum_hint; old_base])) in
      hint env h
    else
      enum_hint
  in
  let enum =
    {
      N.e_base = new_base;
      N.e_constraint = Option.map e.e_constraint (hint env);
      N.e_includes = List.map ~f:(hint env) e.e_includes;
      N.e_enum_class = is_enum_class;
    }
  in
  (Some bound, Some enum, is_enum_class)

and type_paraml ?(forbid_this = false) env tparams =
  List.map tparams ~f:(type_param ~forbid_this env)

(*
  We need to be careful regarding the scoping of type variables:
  Type parameters are always in scope simultaneously: Given
  class C<T1 ... , T2 ... , Tn ...>,
  all type parameters are in scope in the constraints of all other ones (and the where constraints,
  in case of functions).
  For consitency, the same holds for nested type parameters (i.e., type parameters of type
  parameters). Given
  class Foo<T<T1 ... , ...., Tn ... > ... >
  every Ti is in scope of the constraints of all other Tj, and in the constraints on T itself.
*)
and type_param ~forbid_this (genv, lenv) t =
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
        let (def_pos, _) = GEnv.get_type_full_pos genv.ctx (def_pos, name) in
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
  let hk_types_enabled =
    TypecheckerOptions.higher_kinded_types (Provider_context.get_tcopt genv.ctx)
  in
  ( if (not hk_types_enabled) && (not @@ List.is_empty t.Aast.tp_parameters) then
    let (pos, name) = t.Aast.tp_name in
    Errors.tparam_with_tparam pos name );

  (* Bring all type parameters into scope at once before traversing nested tparams,
    as per the note above *)
  let env = (extend_tparams genv t.Aast.tp_parameters, lenv) in
  let tp_parameters =
    if hk_types_enabled then
      List.map t.Aast.tp_parameters (type_param ~forbid_this env)
    else
      []
  in
  (* Use the env with all nested tparams still in scope *)
  let tp_constraints =
    List.map t.Aast.tp_constraints (constraint_ ~forbid_this env)
  in
  {
    N.tp_variance = t.Aast.tp_variance;
    tp_name = t.Aast.tp_name;
    tp_parameters;
    tp_constraints;
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
    if
      FileInfo.equal_mode (fst env).in_mode FileInfo.Mhhi && Option.is_none expr
    then
      Some (fst cv.Aast.cv_id, N.Any)
    else
      expr
  in
  let is_xhp =
    try String.(sub (snd cv.Aast.cv_id) 0 1 = ":")
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
    N.cv_readonly = cv.Aast.cv_readonly;
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
    N.cv_readonly = cv.Aast.cv_readonly;
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

and check_constant_expression env ~in_enum_class (pos, e) =
  if not in_enum_class then
    check_constant_expr env (pos, e)
  else
    true

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
  | Aast.Darray (_, l) ->
    List.for_all l ~f:(fun (e1, e2) ->
        check_constant_expr env e1 && check_constant_expr env e2)
  | Aast.Varray (_, l) -> List.for_all l ~f:(check_constant_expr env)
  | Aast.Shape fdl ->
    (* Only check the values because shape field names are always legal *)
    List.for_all fdl ~f:(fun (_, e) -> check_constant_expr env e)
  | Aast.Call ((_, Aast.Id (_, cn)), _, el, unpacked_element)
    when String.equal cn SN.AutoimportedFunctions.fun_
         || String.equal cn SN.AutoimportedFunctions.class_meth
         || String.equal cn SN.StdlibFunctions.array_mark_legacy
         (* Tuples are not really function calls, they are just parsed that way*)
         || String.equal cn SN.SpecialFunctions.tuple ->
    arg_unpack_unexpected unpacked_element;
    List.for_all el ~f:(check_constant_expr env)
  | Aast.FunctionPointer ((Aast.FP_id _ | Aast.FP_class_const _), _) -> true
  | Aast.Collection (id, _, l) ->
    let (p, cn) = NS.elaborate_id (fst env).namespace NS.ElaborateClass id in
    (* Only vec/keyset/dict are allowed because they are value types *)
    if
      String.equal cn SN.Collections.cVec
      || String.equal cn SN.Collections.cKeyset
      || String.equal cn SN.Collections.cDict
    then
      List.for_all l ~f:(check_afield_constant_expr env)
    else (
      Errors.illegal_constant p;
      false
    )
  | Aast.As (e, (_, Aast.Hlike _), _) -> check_constant_expr env e
  | Aast.As (e, (_, Aast.Happly (id, [_])), _) ->
    let (p, cn) = NS.elaborate_id (fst env).namespace NS.ElaborateClass id in
    if String.equal cn SN.FB.cIncorrectType then
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

and constant_expr env ~in_enum_class e =
  let valid_constant_expression =
    check_constant_expression env ~in_enum_class e
  in
  if valid_constant_expression then
    expr env e
  else
    (fst e, N.Any)

and class_const env ~in_enum_class cc =
  let h = Option.map cc.Aast.cc_type (hint env) in
  let e = Option.map cc.Aast.cc_expr (constant_expr env ~in_enum_class) in
  {
    N.cc_type = h;
    N.cc_id = cc.Aast.cc_id;
    N.cc_expr = e;
    N.cc_doc_comment = cc.Aast.cc_doc_comment;
  }

and typeconst env t =
  let open Aast in
  let tconst =
    match t.c_tconst_kind with
    | TCAbstract { c_atc_as_constraint; c_atc_super_constraint; c_atc_default }
      ->
      TCAbstract
        {
          c_atc_as_constraint = Option.map ~f:(hint env) c_atc_as_constraint;
          c_atc_super_constraint =
            Option.map ~f:(hint env) c_atc_super_constraint;
          c_atc_default = Option.map ~f:(hint env) c_atc_default;
        }
    | TCConcrete { c_tc_type } -> TCConcrete { c_tc_type = hint env c_tc_type }
    | TCPartiallyAbstract { c_patc_constraint; c_patc_type } ->
      TCPartiallyAbstract
        {
          c_patc_constraint = hint env c_patc_constraint;
          c_patc_type = hint env c_patc_type;
        }
  in
  let attrs = user_attributes env t.Aast.c_tconst_user_attributes in
  N.
    {
      c_tconst_user_attributes = attrs;
      c_tconst_name = t.Aast.c_tconst_name;
      c_tconst_kind = tconst;
      c_tconst_span = t.Aast.c_tconst_span;
      c_tconst_doc_comment = t.Aast.c_tconst_doc_comment;
      c_tconst_is_ctx = t.Aast.c_tconst_is_ctx;
    }

and method_ genv m =
  let genv = extend_tparams genv m.Aast.m_tparams in
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
    | FileInfo.Mhhi ->
      { N.fb_ast = []; fb_annotation = Nast.NamedWithUnsafeBlocks }
    | FileInfo.Mstrict
    | FileInfo.Mpartial ->
      if Nast.is_body_named m.Aast.m_body then
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
        { N.fb_ast = fub_ast; fb_annotation = annotation }
      else
        failwith "ast_to_nast error unnamedbody in method_"
  in
  let attrs = user_attributes env m.Aast.m_user_attributes in
  let m_ctxs = Option.map ~f:(contexts env) m.Aast.m_ctxs in
  let m_unsafe_ctxs = Option.map ~f:(contexts env) m.Aast.m_unsafe_ctxs in
  {
    N.m_annotation = ();
    N.m_span = m.Aast.m_span;
    N.m_final = m.Aast.m_final;
    N.m_visibility = m.Aast.m_visibility;
    N.m_abstract = m.Aast.m_abstract;
    N.m_readonly_this = m.Aast.m_readonly_this;
    N.m_static = m.Aast.m_static;
    N.m_name = m.Aast.m_name;
    N.m_tparams = tparam_l;
    N.m_where_constraints = where_constraints;
    N.m_params = paraml;
    N.m_ctxs;
    N.m_unsafe_ctxs;
    N.m_body = body;
    N.m_fun_kind = m.Aast.m_fun_kind;
    N.m_readonly_ret = m.Aast.m_readonly_ret;
    N.m_ret = ret;
    N.m_variadic = variadicity;
    N.m_user_attributes = attrs;
    N.m_external = m.Aast.m_external;
    N.m_doc_comment = m.Aast.m_doc_comment;
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
    param_readonly = param.Aast.param_readonly;
    param_user_attributes = user_attributes env param.Aast.param_user_attributes;
    param_visibility = param.Aast.param_visibility;
  }

and extend_tparams genv paraml =
  let params =
    List.fold_right
      paraml
      ~init:genv.type_params
      ~f:(fun { Aast.tp_name = (_, x); _ } acc -> SSet.add x acc)
  in
  { genv with type_params = params }

and fun_ ctx f =
  let genv = Env.make_fun_decl_genv ctx f in
  let lenv = Env.empty_local None in
  let env = (genv, lenv) in
  let f =
    elaborate_namespaces#on_fun_def
      (Naming_elaborate_namespaces_endo.make_env (fst env).namespace)
      f
  in
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
    | FileInfo.Mhhi ->
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
        { N.fb_ast; fb_annotation = annotation }
      else
        failwith "ast_to_nast error unnamedbody in fun_"
  in
  let f_ctxs = Option.map ~f:(contexts env) f.Aast.f_ctxs in
  let f_unsafe_ctxs = Option.map ~f:(contexts env) f.Aast.f_unsafe_ctxs in
  let file_attributes =
    file_attributes ctx f.Aast.f_mode f.Aast.f_file_attributes
  in
  let named_fun =
    {
      N.f_annotation = ();
      f_readonly_this = f.Aast.f_readonly_this;
      f_span = f.Aast.f_span;
      f_mode = f.Aast.f_mode;
      f_readonly_ret = f.Aast.f_readonly_ret;
      f_ret = h;
      f_name = f.Aast.f_name;
      f_tparams;
      f_where_constraints = where_constraints;
      f_params = paraml;
      (* TODO(T70095684) double-check f_ctxs *)
      f_ctxs;
      f_unsafe_ctxs;
      f_body = body;
      f_fun_kind = f_kind;
      f_variadic = variadicity;
      f_user_attributes = user_attributes env f.Aast.f_user_attributes;
      f_file_attributes = file_attributes;
      f_external = f.Aast.f_external;
      f_namespace = f.Aast.f_namespace;
      f_doc_comment = f.Aast.f_doc_comment;
    }
  in
  named_fun

and get_using_vars es =
  List.concat_map es (fun (_, e) ->
      match e with
      (* Simple assignment to local of form `$lvar = e` *)
      | Aast.Binop (Ast_defs.Eq None, (_, Aast.Lvar (p, lid)), _) ->
        [(p, Local_id.get_name lid)]
      (* Arbitrary expression. This will be assigned to a temporary *)
      | _ -> [])

and stmt env (pos, st) =
  let stmt =
    match st with
    | Aast.Block _ -> failwith "stmt block error"
    | Aast.Fallthrough -> N.Fallthrough
    | Aast.Noop -> N.Noop
    | Aast.Markup _ -> N.Noop
    | Aast.AssertEnv _ -> N.Noop
    | Aast.Break -> Aast.Break
    | Aast.Continue -> Aast.Continue
    | Aast.Throw e -> N.Throw (expr env e)
    | Aast.Return e -> N.Return (Option.map e (expr env))
    | Aast.Yield_break -> N.Yield_break
    | Aast.Awaitall (el, b) -> awaitall_stmt env el b
    | Aast.If (e, b1, b2) -> if_stmt env e b1 b2
    | Aast.Do (b, e) -> do_stmt env b e
    | Aast.While (e, b) -> N.While (expr env e, block env b)
    | Aast.Using s ->
      using_stmt env s.Aast.us_has_await s.Aast.us_exprs s.Aast.us_block
    | Aast.For (st1, e, st2, b) -> for_stmt env st1 e st2 b
    | Aast.Switch (e, cl) -> switch_stmt env e cl
    | Aast.Foreach (e, ae, b) -> foreach_stmt env e ae b
    | Aast.Try (b, cl, fb) -> try_stmt env b cl fb
    | Aast.Expr (cp, Aast.Call ((p, Aast.Id (fp, fn)), hl, el, unpacked_element))
      when String.equal fn SN.AutoimportedFunctions.invariant ->
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
                ( (p, Aast.Id (fp, SN.AutoimportedFunctions.invariant_violation)),
                  hl,
                  el,
                  unpacked_element ) )
          in
          (match cond with
          | Aast.False ->
            (* a false <condition> means unconditional invariant_violation *)
            N.Expr (expr env violation)
          | _ ->
            let (b1, b2) =
              ([(cp, Aast.Expr violation)], [(Pos.none, Aast.Noop)])
            in
            let cond = (cond_p, Aast.Unop (Ast_defs.Unot, (cond_p, cond))) in
            if_stmt env cond b1 b2)
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
and using_stmt env has_await (loc, e) b =
  let vars = get_using_vars e in
  let e = List.map ~f:(expr env) e in
  let b = block ~new_scope:false env b in
  Env.remove_locals env vars;
  N.Using
    N.
      {
        us_is_block_scoped = false;
        (* This isn't used for naming so provide a default *)
        us_has_await = has_await;
        us_exprs = (loc, e);
        us_block = b;
      }

and for_stmt env e1 e2 e3 b =
  (* The initialization and condition expression should be in the outer scope,
   * as they are always executed. *)
  let e1 = exprl env e1 in
  let e2 = oexpr env e2 in
  Env.scope env (fun env ->
      (* The third expression (iteration step) should have the same scope as the
       * block, as it is not always executed. *)
      let b = block ~new_scope:false env b in
      let e3 = exprl env e3 in
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
      | x when String.equal x SN.Collections.cPair ->
        let ta =
          match tal with
          | Some (Aast.CollectionTV _) ->
            Errors.naming_too_few_arguments p;
            None
          | Some (Aast.CollectionTKV (tk, tv)) -> Some (targ env tk, targ env tv)
          | None -> None
        in
        begin
          match l with
          | [] ->
            Errors.naming_too_few_arguments p;
            N.Any
          | [e1; e2] ->
            let pn = SN.Collections.cPair in
            N.Pair (ta, afield_value env pn e1, afield_value env pn e2)
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
  | Aast.Id x -> N.Id x
  | Aast.Lvar (_, x)
    when String.equal (Local_id.to_string x) SN.SpecialIdents.this ->
    N.This
  | Aast.Lvar (p, x)
    when String.equal (Local_id.to_string x) SN.SpecialIdents.dollardollar ->
    N.Dollardollar (p, Local_id.make_unscoped SN.SpecialIdents.dollardollar)
  | Aast.Lvar (p, x)
    when String.equal (Local_id.to_string x) SN.SpecialIdents.placeholder ->
    N.Lplaceholder p
  | Aast.Lvar x ->
    let x = (fst x, Local_id.to_string @@ snd x) in
    N.Lvar (Env.lvar env x)
  | Aast.Obj_get (e1, e2, nullsafe, in_parens) ->
    (* If we encounter Obj_get(_,_,true) by itself, then it means "?->"
       is being used for instance property access; see the case below for
       handling nullsafe instance method calls to see how this works *)
    N.Obj_get (expr env e1, expr_obj_get_name env e2, nullsafe, in_parens)
  | Aast.Array_get ((p, Aast.Lvar x), None) ->
    let x = (fst x, Local_id.to_string @@ snd x) in
    let id = (p, N.Lvar (Env.lvar env x)) in
    N.Array_get (id, None)
  | Aast.Array_get (e1, e2) -> N.Array_get (expr env e1, oexpr env e2)
  | Aast.Class_get
      ((_, Aast.CIexpr (_, Aast.Id x1)), Aast.CGstring x2, in_parens) ->
    N.Class_get (make_class_id env x1, N.CGstring x2, in_parens)
  | Aast.Class_get
      ((_, Aast.CIexpr (_, Aast.Lvar (p, lid))), Aast.CGstring x2, in_parens) ->
    let x1 = (p, Local_id.to_string lid) in
    N.Class_get (make_class_id env x1, N.CGstring x2, in_parens)
  | Aast.Class_get ((_, Aast.CIexpr x1), Aast.CGstring _, _) ->
    ensure_name_not_dynamic env x1;
    N.Any
  | Aast.Class_get ((_, Aast.CIexpr x1), Aast.CGexpr x2, _) ->
    ensure_name_not_dynamic env x1;
    ensure_name_not_dynamic env x2;
    N.Any
  | Aast.Class_get _ -> failwith "Error in Ast_to_nast module for Class_get"
  | Aast.Class_const ((_, Aast.CIexpr (_, Aast.Id x1)), ((_, str) as x2))
    when String.equal str "class" ->
    N.Class_const (make_class_id env x1, x2)
  | Aast.Class_const ((_, Aast.CIexpr (_, Aast.Id x1)), x2) ->
    N.Class_const (make_class_id env x1, x2)
  | Aast.Class_const ((_, Aast.CIexpr (_, Aast.Lvar (p, lid))), x2) ->
    let x1 = (p, Local_id.to_string lid) in
    N.Class_const (make_class_id env x1, x2)
  | Aast.Class_const _ -> (* TODO: report error in strict mode *) N.Any
  | Aast.Call ((_, Aast.Id (p, pseudo_func)), tal, el, unpacked_element)
    when String.equal pseudo_func SN.SpecialFunctions.echo ->
    arg_unpack_unexpected unpacked_element;
    N.Call ((p, N.Id (p, pseudo_func)), targl env p tal, exprl env el, None)
  | Aast.Call ((p, Aast.Id (_, cn)), tal, el, _)
    when String.equal cn SN.StdlibFunctions.call_user_func ->
    Errors.deprecated_use
      p
      ( "The builtin "
      ^ Markdown_lite.md_codify (Utils.strip_ns cn)
      ^ " is deprecated." );
    begin
      match el with
      | [] ->
        Errors.naming_too_few_arguments p;
        N.Any
      | f :: el -> N.Call (expr env f, targl env p tal, exprl env el, None)
    end
  | Aast.Call ((p, Aast.Id (_, cn)), _, el, unpacked_element)
    when String.equal cn SN.AutoimportedFunctions.fun_ ->
    arg_unpack_unexpected unpacked_element;
    begin
      match el with
      | [] ->
        Errors.naming_too_few_arguments p;
        N.Any
      | [(p, Aast.String x)] -> N.Fun_id (p, x)
      | [(p, _)] ->
        Errors.illegal_fun p;
        N.Any
      | _ ->
        Errors.naming_too_many_arguments p;
        N.Any
    end
  | Aast.Call ((p, Aast.Id (_, cn)), _, el, unpacked_element)
    when String.equal cn SN.AutoimportedFunctions.inst_meth ->
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
  | Aast.Call ((p, Aast.Id (_, cn)), _, el, unpacked_element)
    when String.equal cn SN.AutoimportedFunctions.meth_caller ->
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
            let () = check_name (pc, cl) in
            N.Method_caller ((pc, cl), (pm, meth))
          | ((_, N.Class_const ((_, N.CI cl), (_, mem))), (pm, N.String meth))
            when String.equal mem SN.Members.mClass ->
            let () = check_name cl in
            N.Method_caller (cl, (pm, meth))
          | ((p, _), _) ->
            Errors.illegal_meth_caller p;
            N.Any
        end
      | _ ->
        Errors.naming_too_many_arguments p;
        N.Any
    end
  | Aast.Call ((p, Aast.Id (_, cn)), _, el, unpacked_element)
    when String.equal cn SN.AutoimportedFunctions.class_meth ->
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
            let () = check_name (pc, cl) in
            let cid = N.CI (pc, cl) in
            N.Smethod_id ((pc, cid), (pm, meth))
          | ((_, N.Id (pc, const)), (pm, N.String meth))
            when String.equal const SN.PseudoConsts.g__CLASS__ ->
            (* All of these that use current_cls aren't quite correct
             * inside a trait, as the class should be the using class.
             * It's sufficient for typechecking purposes (we require
             * subclass to be compatible with the trait member/method
             * declarations).
             *)
            (match (fst env).current_cls with
            | Some (cid, _, true) ->
              let cid = N.CI (pc, snd cid) in
              N.Smethod_id ((p, cid), (pm, meth))
            | Some (cid, kind, false) ->
              let is_trait = Ast_defs.is_c_trait kind in
              Errors.class_meth_non_final_CLASS p is_trait (snd cid);
              N.Any
            | None ->
              Errors.illegal_class_meth p;
              N.Any)
          | ((_, N.Class_const ((pc, N.CI cl), (_, mem))), (pm, N.String meth))
            when String.equal mem SN.Members.mClass ->
            let () = check_name cl in
            let cid = N.CI cl in
            N.Smethod_id ((pc, cid), (pm, meth))
          | ((p, N.Class_const ((pc, N.CIself), (_, mem))), (pm, N.String meth))
            when String.equal mem SN.Members.mClass ->
            (match (fst env).current_cls with
            | Some (_cid, _, true) -> N.Smethod_id ((pc, N.CIself), (pm, meth))
            | Some (cid, _, false) ->
              Errors.class_meth_non_final_self p (snd cid);
              N.Any
            | None ->
              Errors.illegal_class_meth p;
              N.Any)
          | ( (p, N.Class_const ((pc, N.CIstatic), (_, mem))),
              (pm, N.String meth) )
            when String.equal mem SN.Members.mClass ->
            (match (fst env).current_cls with
            | Some (_cid, _, _) -> N.Smethod_id ((pc, N.CIstatic), (pm, meth))
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
  | Aast.Call ((p, Aast.Id (_, cn)), _, el, unpacked_element)
    when String.equal cn SN.SpecialFunctions.tuple ->
    arg_unpack_unexpected unpacked_element;
    (match el with
    | [] ->
      Errors.naming_too_few_arguments p;
      N.Any
    | el -> N.Tuple (exprl env el))
  | Aast.Call ((p, Aast.Id f), tal, el, unpacked_element) ->
    N.Call
      ((p, N.Id f), targl env p tal, exprl env el, oexpr env unpacked_element)
  (* match *)
  (* Handle nullsafe instance method calls here. Because Obj_get is used
     for both instance property access and instance method calls, we need
     to match the entire "Call(Obj_get(..), ..)" pattern here so that we
     only match instance method calls *)
  | Aast.Call
      ( (p, Aast.Obj_get (e1, e2, Aast.OG_nullsafe, in_parens)),
        tal,
        el,
        unpacked_element ) ->
    N.Call
      ( ( p,
          N.Obj_get
            (expr env e1, expr_obj_get_name env e2, N.OG_nullsafe, in_parens) ),
        targl env p tal,
        exprl env el,
        oexpr env unpacked_element )
  (* Handle all kinds of calls that weren't handled by any of the cases above *)
  | Aast.Call (e, tal, el, unpacked_element) ->
    N.Call
      (expr env e, targl env p tal, exprl env el, oexpr env unpacked_element)
  | Aast.FunctionPointer (Aast.FP_id fid, targs) ->
    N.FunctionPointer (N.FP_id fid, targl env p targs)
  | Aast.FunctionPointer
      (Aast.FP_class_const ((_, Aast.CIexpr (_, Aast.Id x1)), x2), targs) ->
    N.FunctionPointer
      (N.FP_class_const (make_class_id env x1, x2), targl env p targs)
  | Aast.FunctionPointer
      (Aast.FP_class_const ((_, Aast.CIexpr (_, Aast.Lvar (p, lid))), x2), targs)
    ->
    let x1 = (p, Local_id.to_string lid) in
    N.FunctionPointer
      (N.FP_class_const (make_class_id env x1, x2), targl env p targs)
  | Aast.FunctionPointer _ -> N.Any
  | Aast.Yield e -> N.Yield (afield env e)
  | Aast.Await e -> N.Await (expr env e)
  | Aast.Tuple el -> N.Tuple (exprl env el)
  | Aast.List el -> N.List (exprl env el)
  | Aast.Cast (ty, e2) ->
    let ((p, x), hl) =
      match ty with
      | (_, Aast.Happly (id, hl)) -> (id, hl)
      | _ -> assert false
    in
    let ty =
      match try_castable_hint ~tp_depth:1 ~ignore_hack_arr:false env p x hl with
      | Some ty -> (p, ty)
      | None ->
        let h = hint env ty in
        Errors.object_cast p;
        h
    in
    N.Cast (ty, expr env e2)
  | Aast.ExpressionTree et ->
    N.ExpressionTree
      N.
        {
          et_hint = hint env et.et_hint;
          et_splices = block env et.et_splices;
          et_virtualized_expr = expr env et.et_virtualized_expr;
          et_runtime_expr = expr env et.et_runtime_expr;
        }
  | Aast.ET_Splice e -> N.ET_Splice (expr env e)
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
    N.Is
      ( expr env e,
        hint ~allow_wildcard:true ~allow_like:true ~ignore_hack_arr:true env h
      )
  | Aast.As (e, h, b) ->
    N.As
      ( expr env e,
        hint ~allow_wildcard:true ~allow_like:true ~ignore_hack_arr:true env h,
        b )
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
  | Aast.Record (id, l) ->
    let () = check_name id in
    let l = List.map l (fun (e1, e2) -> (expr env e1, expr env e2)) in
    N.Record (id, l)
  | Aast.Efun (f, idl) ->
    let idl =
      List.fold_right idl ~init:[] ~f:(fun ((p, x) as id) acc ->
          if String.equal (Local_id.to_string x) SN.SpecialIdents.this then (
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
    let () = check_name x in
    N.Xml (x, attrl env al, exprl env el)
  | Aast.Shape fdl ->
    let shp =
      List.map fdl ~f:(fun (pname, value) ->
          (convert_shape_name env pname, expr env value))
    in
    N.Shape shp
  | Aast.Import _ -> N.Any
  | Aast.Omitted -> N.Omitted
  | Aast.Callconv (kind, e) -> N.Callconv (kind, expr env e)
  | Aast.EnumAtom x -> N.EnumAtom x
  | Aast.ReadonlyExpr e -> N.ReadonlyExpr (expr env e)
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
  | Aast.Any
  | Aast.Hole _ ->
    Errors.internal_error
      p
      "Malformed expr: Expr not found on legacy AST: T39599317";
    Aast.Any

and expr_lambda env f =
  let h =
    Aast.type_hint_option_map ~f:(hint ~allow_retonly:true env) f.Aast.f_ret
  in
  let (variadicity, paraml) = fun_paraml env f.Aast.f_params in
  (* The bodies of lambdas go through naming in the containing local
   * environment *)
  let body_nast = f_body env f.Aast.f_body in
  let annotation = Nast.Named in
  let f_ctxs = Option.map ~f:(contexts env) f.Aast.f_ctxs in
  let f_unsafe_ctxs = Option.map ~f:(contexts env) f.Aast.f_unsafe_ctxs in
  (* These could all be probably be replaced with a {... where ...} *)
  let body = { N.fb_ast = body_nast; fb_annotation = annotation } in
  {
    N.f_annotation = ();
    f_readonly_this = f.Aast.f_readonly_this;
    f_span = f.Aast.f_span;
    f_mode = (fst env).in_mode;
    f_readonly_ret = f.Aast.f_readonly_ret;
    f_ret = h;
    f_name = f.Aast.f_name;
    f_params = paraml;
    f_tparams = [];
    f_ctxs;
    f_unsafe_ctxs;
    f_where_constraints = [];
    f_body = body;
    f_fun_kind = f.Aast.f_fun_kind;
    f_variadic = variadicity;
    f_file_attributes = [];
    f_user_attributes = user_attributes env f.Aast.f_user_attributes;
    f_external = f.Aast.f_external;
    f_namespace = f.Aast.f_namespace;
    f_doc_comment = f.Aast.f_doc_comment;
  }

and f_body env f_body =
  if Nast.is_body_named f_body then
    block env f_body.Aast.fb_ast
  else
    failwith "Malformed f_body: unexpected UnnamedBody from ast_to_nast"

and make_class_id env ((p, x) as cid) =
  ( p,
    match x with
    | x when String.equal x SN.Classes.cParent ->
      if Option.is_none (fst env).current_cls then
        let () = Errors.parent_outside_class p in
        N.CI (p, SN.Classes.cUnknown)
      else
        N.CIparent
    | x when String.equal x SN.Classes.cSelf ->
      if Option.is_none (fst env).current_cls then
        let () = Errors.self_outside_class p in
        N.CI (p, SN.Classes.cUnknown)
      else
        N.CIself
    | x when String.equal x SN.Classes.cStatic ->
      if Option.is_none (fst env).current_cls then
        let () = Errors.static_outside_class p in
        N.CI (p, SN.Classes.cUnknown)
      else
        N.CIstatic
    | x when String.equal x SN.SpecialIdents.this -> N.CIexpr (p, N.This)
    | x when String.equal x SN.SpecialIdents.dollardollar ->
      (* We won't reach here for "new $$" because the parser creates a
       * proper Ast_defs.Dollardollar node, so make_class_id won't be called with
       * that node. In fact, the parser creates an Ast_defs.Dollardollar for all
       * "$$" except in positions where a classname is expected, like in
       * static member access. So, we only reach here for things
       * like "$$::someMethod()". *)
      N.CIexpr
        (p, N.Lvar (p, Local_id.make_unscoped SN.SpecialIdents.dollardollar))
    | x when Char.equal x.[0] '$' -> N.CIexpr (p, N.Lvar (Env.lvar env cid))
    | _ ->
      let () = check_name cid in
      N.CI cid )

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
      let () = check_name (p1, lid1) in
      ((p1, lid1), x2, b))

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
  | Aast.Xhp_simple { Aast.xs_name; xs_type; xs_expr = e } ->
    N.Xhp_simple { Aast.xs_name; xs_type; xs_expr = expr env e }
  | Aast.Xhp_spread e -> N.Xhp_spread (expr env e)

and string2 env idl = List.map idl (expr env)

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
    | Some extends -> Some (hint env extends)
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
  let env = Env.make_typedef_env ctx tdef in
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
    t_span = tdef.Aast.t_span;
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
  let e = constant_expr env false cst.Aast.cst_value in
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
