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
open String_utils
open Naming_phase_sigs
module N = Aast
module SN = Naming_special_names

(*****************************************************************************)
(* The types *)
(*****************************************************************************)

type genv = {
  (* strict? decl?  *)
  in_mode: FileInfo.mode;
  (* various options that control the strictness of the typechecker *)
  ctx: Provider_context.t;
  (* Namespace environment, e.g., what namespace we're in and what use
   * declarations are in play. *)
  namespace: Namespace_env.env;
}

(* The primitives to manipulate the naming environment *)
module Env : sig
  val make_class_env : Provider_context.t -> Nast.class_ -> genv

  val make_typedef_env : Provider_context.t -> Nast.typedef -> genv

  val make_top_level_env : Provider_context.t -> genv

  val make_fun_decl_genv : Provider_context.t -> Nast.fun_def -> genv

  val make_const_env : Provider_context.t -> Nast.gconst -> genv

  val make_module_env : Provider_context.t -> Nast.module_def -> genv
end = struct
  let make_class_genv ctx _ mode _ namespace _ =
    { in_mode = mode; ctx; namespace }

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
    genv

  let make_typedef_genv ctx _ tdef_namespace =
    { in_mode = FileInfo.Mstrict; ctx; namespace = tdef_namespace }

  let make_typedef_env ctx tdef =
    let genv =
      make_typedef_genv ctx tdef.Aast.t_tparams tdef.Aast.t_namespace
    in
    genv

  let make_fun_genv ctx _ f_mode f_namespace =
    { in_mode = f_mode; ctx; namespace = f_namespace }

  let make_fun_decl_genv ctx f =
    make_fun_genv
      ctx
      f.Aast.fd_fun.Aast.f_tparams
      f.Aast.fd_mode
      f.Aast.fd_namespace

  let make_const_genv ctx cst =
    { in_mode = cst.Aast.cst_mode; ctx; namespace = cst.Aast.cst_namespace }

  let make_top_level_genv ctx =
    {
      in_mode = FileInfo.Mstrict;
      ctx;
      namespace = Namespace_env.empty_with_default;
    }

  let make_top_level_env ctx =
    let genv = make_top_level_genv ctx in
    genv

  let make_const_env ctx cst =
    let genv = make_const_genv ctx cst in
    genv

  let make_module_genv ctx _module =
    {
      in_mode = FileInfo.Mstrict;
      ctx;
      namespace = Namespace_env.empty_with_default;
    }

  let make_module_env ctx module_ =
    let genv = make_module_genv ctx module_ in
    genv
end

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)
let elaborate_namespaces =
  new Naming_elaborate_namespaces_endo.generic_elaborator

let invalid_expr_ (p : Pos.t) : Nast.expr_ =
  let throw : Nast.stmt =
    ( p,
      Aast.Throw
        ( (),
          p,
          Aast.New
            ( ((), p, Aast.CI (p, "\\Exception")),
              [],
              [((), p, Aast.String "invalid expression")],
              None,
              () ) ) )
  in
  Aast.Call
    ( ( (),
        p,
        Aast.Lfun
          ( {
              Aast.f_span = p;
              f_readonly_this = None;
              f_annotation = ();
              f_readonly_ret = None;
              f_ret = ((), None);
              f_name = (p, "invalid_expr");
              f_tparams = [];
              f_where_constraints = [];
              f_params = [];
              f_ctxs = None;
              f_unsafe_ctxs = None;
              f_body = { Aast.fb_ast = [throw] };
              f_fun_kind = Ast_defs.FSync;
              f_user_attributes = [];
              f_external = false;
              f_doc_comment = None;
            },
            [] ) ),
      [],
      [],
      None )

let ignored_expr_ p : Nast.expr_ = invalid_expr_ p

(**************************************************************************)
(* All the methods and static methods of an interface are "implicitly"
 * declared as abstract
 *)
(**************************************************************************)

let add_abstract m = { m with N.m_abstract = true }

let add_abstractl methods = List.map methods ~f:add_abstract

let interface c constructor methods smethods =
  if not (Ast_defs.is_c_interface c.Aast.c_kind) then
    (constructor, methods, smethods)
  else
    let constructor = Option.map constructor ~f:add_abstract in
    let methods = add_abstractl methods in
    let smethods = add_abstractl smethods in
    (constructor, methods, smethods)

let make_xhp_attr = function
  | true -> Some { N.xai_like = None; N.xai_tag = None; N.xai_enum_values = [] }
  | false -> None

(**************************************************************************)
(* Classes *)
(**************************************************************************)

let class_prop_expr_is_xhp env cv =
  let expr = cv.Aast.cv_expr in
  let expr =
    if FileInfo.is_hhi env.in_mode && Option.is_none expr then
      let pos = fst cv.Aast.cv_id in
      Some ((), pos, ignored_expr_ (fst cv.Aast.cv_id))
    else
      expr
  in
  let is_xhp =
    try String.(sub (snd cv.Aast.cv_id) ~pos:0 ~len:1 = ":") with
    | Invalid_argument _ -> false
  in
  (expr, is_xhp)

let class_prop_static env cv =
  let (cv_expr, is_xhp) = class_prop_expr_is_xhp env cv in
  let cv_xhp_attr = make_xhp_attr is_xhp in
  Aast.{ cv with cv_xhp_attr; cv_expr }

let class_prop_non_static env ?(const = None) cv =
  (* if class is __Const, make all member fields __Const *)
  let cv_user_attributes =
    let attrs = cv.Aast.cv_user_attributes in
    match const with
    | Some c ->
      if not (Naming_attributes.mem SN.UserAttributes.uaConst attrs) then
        c :: attrs
      else
        attrs
    | None -> attrs
  in
  let (cv_expr, is_xhp) = class_prop_expr_is_xhp env cv in
  let cv_xhp_attr = make_xhp_attr is_xhp in
  Aast.{ cv with cv_xhp_attr; cv_expr; cv_user_attributes }

(* h cv is_required maybe_enum *)
let xhp_attribute_decl env (h, cv, tag, maybe_enum) =
  let (p, id) = cv.Aast.cv_id in
  let default = cv.Aast.cv_expr in
  let is_required = Option.is_some tag in
  if is_required && Option.is_some default then
    Errors.add_naming_error
    @@ Naming_error.Xhp_required_with_default { pos = p; attr_name = id };
  let hint_ =
    match maybe_enum with
    | Some (pos, items) ->
      let is_int item =
        match item with
        | (_, _, Aast.Int _) -> true
        | _ -> false
      in
      let contains_int = List.exists ~f:is_int items in
      let is_string item =
        match item with
        | (_, _, Aast.String _)
        | (_, _, Aast.String2 _) ->
          true
        | _ -> false
      in
      let contains_str = List.exists ~f:is_string items in
      if contains_int && not contains_str then
        Some (pos, Aast.(Hprim Tint))
      else if (not contains_int) && contains_str then
        Some (pos, Aast.(Hprim Tstring))
      else
        Some (pos, Aast.Hmixed)
    | _ -> Aast.hint_of_type_hint h
  in
  let strip_like h =
    match h with
    | Aast.Hlike h -> snd h
    | _ -> h
  in
  let hint_ =
    match hint_ with
    | Some (p, h) ->
      begin
        match strip_like h with
        | Aast.Hoption _ ->
          if is_required then
            Errors.add_naming_error
            @@ Naming_error.Xhp_optional_required_attr
                 { pos = p; attr_name = id };
          hint_
        | Aast.Hmixed -> hint_
        | _ ->
          let has_default =
            match default with
            | None
            | Some (_, _, Aast.Null) ->
              false
            | _ -> true
          in
          if is_required || has_default then
            hint_
          else
            Some (p, Aast.Hoption (p, h))
      end
    | None -> None
  in
  let (like, enum_values) =
    match cv.Aast.cv_xhp_attr with
    | Some xai -> (xai.Aast.xai_like, xai.Aast.xai_enum_values)
    | None -> (None, [])
  in
  let hint_ =
    Option.map
      ~f:(fun hint ->
        match like with
        | Some plike ->
          if
            not
              (TypecheckerOptions.like_type_hints
                 (Provider_context.get_tcopt env.ctx))
          then
            Errors.experimental_feature p "like-types";

          (plike, Aast.Hlike hint)
        | _ -> hint)
      hint_
  in
  let cv_type = ((), hint_) in
  let (cv_expr, _) = class_prop_expr_is_xhp env cv in
  let cv_xhp_attr =
    Some { N.xai_like = like; N.xai_tag = tag; N.xai_enum_values = enum_values }
  in
  Aast.{ cv with cv_xhp_attr; cv_type; cv_expr; cv_user_attributes = [] }

let class_help _ env c =
  let (sprops, props) = Aast.split_vars c.Aast.c_vars in
  let sprops = List.map ~f:(class_prop_static env) sprops in
  let const =
    Naming_attributes.find SN.UserAttributes.uaConst c.Aast.c_user_attributes
  in
  let props = List.map ~f:(class_prop_non_static ~const env) props in
  let xhp_attrs = List.map ~f:(xhp_attribute_decl env) c.Aast.c_xhp_attrs in
  (* These would be out of order with the old attributes, but that shouldn't matter? *)
  let c_vars = sprops @ props @ xhp_attrs in

  let (constructor, smethods, methods) = Aast.split_methods c.Aast.c_methods in
  let (constructor, methods, smethods) =
    interface c constructor methods smethods
  in
  let c_methods =
    match constructor with
    | None -> smethods @ methods
    | Some c -> c :: smethods @ methods
  in
  Aast.
    {
      c with
      c_vars;
      c_methods;
      (* Naming and typechecking shouldn't use these fields *)
      c_xhp_attrs = [];
    }

(**************************************************************************)
(* Programs *)
(**************************************************************************)

let program_help ctx env ast =
  let top_level_env = ref env in
  let rec aux acc def =
    match def with
    | Aast.Fun f -> N.Fun f :: acc
    | Aast.Class c ->
      let env = Env.make_class_env ctx c in
      N.Class (class_help ctx env c) :: acc
    | Aast.Stmt (_, Aast.Noop)
    | Aast.Stmt (_, Aast.Markup _) ->
      acc
    | Aast.Stmt s -> Aast.Stmt s :: acc
    | Aast.Typedef t -> N.Typedef t :: acc
    | Aast.Constant cst -> N.Constant cst :: acc
    | Aast.Namespace (_ns, aast) -> List.fold_left ~f:aux ~init:[] aast @ acc
    | Aast.NamespaceUse _ -> acc
    | Aast.SetNamespaceEnv nsenv ->
      let genv = !top_level_env in
      let genv = { genv with namespace = nsenv } in
      top_level_env := genv;
      acc
    | Aast.Module md -> N.Module md :: acc
    | Aast.SetModule sm -> N.SetModule sm :: acc
    (* These are elaborated away in Namespaces.elaborate_toplevel_defs *)
    | Aast.FileAttributes _ -> acc
  in
  let on_program aast =
    let nast = List.fold_left ~f:aux ~init:[] aast in
    List.rev nast
  in
  on_program ast

(**************************************************************************)
(* The entry points to CHECK the program, and transform the program *)
(**************************************************************************)

type 'elem pipeline = {
  elab_ns: 'elem -> 'elem;
  elab_hints: (Naming_elab_hints.Env.t, 'elem) elabidation;
  elab_collection: (Naming_elab_collection.Env.t, 'elem) elabidation;
  elab_call: (Naming_elab_call.Env.t, 'elem) elabidation;
  elab_tuple: (Naming_elab_tuple.Env.t, 'elem) elabidation;
  elab_invariant: (Naming_elab_invariant.Env.t, 'elem) elabidation;
  elab_const_expr: (Naming_elab_const_expr.Env.t, 'elem) elabidation;
  elab_user_attrs: (Naming_elab_user_attributes.Env.t, 'elem) elabidation;
  elab_import: (Naming_elab_import.Env.t, 'elem) elaboration;
  elab_lvar: (Naming_elab_lvar.Env.t, 'elem) elaboration;
  elab_as_expr: (Naming_elab_as_expr.Env.t, 'elem) elabidation;
  elab_block: (Naming_elab_block.Env.t, 'elem) elaboration;
  elab_pipe: (Naming_elab_pipe.Env.t, 'elem) elaboration;
  elab_func_body: (Naming_elab_func_body.Env.t, 'elem) elaboration;
  elab_lambda_captures: (Naming_captures.Env.t, 'elem) elaboration;
  elab_help: Provider_context.t -> genv -> 'elem -> 'elem;
  elab_soft: (Naming_elab_soft.Env.t, 'elem) elaboration;
  elab_everything_sdt: (Naming_elab_everything_sdt.Env.t, 'elem) elaboration;
  elab_hkt: (Naming_elab_hkt.Env.t, 'elem) elabidation;
  elab_enum_class: (Naming_elab_enum_class.Env.t, 'elem) elaboration;
  elab_class_id: (Naming_elab_class_id.Env.t, 'elem) elabidation;
  elab_dynamic_class_name:
    (Naming_elab_dynamic_class_name.Env.t, 'elem) elabidation;
  validate_fun_params: (Naming_validate_fun_params.Env.t, 'elem) validation;
  validate_xhp: (Naming_validate_xhp_name.Env.t, 'elem) validation;
  validate_builtin_enum: (Naming_validate_builtin_enum.Env.t, 'elem) validation;
  validate_enum_class_typeconst:
    (Naming_validate_enum_class_typeconst.Env.t, 'elem) validation;
  validate_supportdyn: (Naming_validate_supportdyn.Env.t, 'elem) validation;
  validate_module: (Naming_validate_module.Env.t, 'elem) validation;
  validate_class_req: (Naming_validate_class_req.Env.t, 'elem) validation;
  validate_consistent_construct:
    (Naming_validate_consistent_construct.Env.t, 'elem) validation;
}

(* Apply our elaboration and validation steps to a given ast element *)
let elab_elem
    elem
    ~ctx
    ~env
    ~filename
    {
      elab_ns;
      elab_hints;
      elab_collection;
      elab_call;
      elab_tuple;
      elab_invariant;
      elab_const_expr;
      elab_user_attrs;
      elab_import;
      elab_lvar;
      elab_as_expr;
      elab_block;
      elab_pipe;
      elab_func_body;
      elab_lambda_captures;
      elab_help;
      elab_soft;
      elab_everything_sdt;
      elab_hkt;
      elab_enum_class;
      elab_class_id;
      elab_dynamic_class_name;
      validate_fun_params;
      validate_xhp;
      validate_builtin_enum;
      validate_enum_class_typeconst;
      validate_supportdyn;
      validate_module;
      validate_class_req;
      validate_consistent_construct;
    } =
  let tcopt = Provider_context.get_tcopt ctx in
  (* Elaborate namespaces *)
  let elem = elab_ns elem in

  (* Elaborate hints, collect errors and report them *)
  let (elem, err) = elab_hints elem in
  (* We have to check if like-types are globally enabled and remove errors
     before reporting if so *)
  let err =
    if TypecheckerOptions.like_type_hints tcopt then
      Naming_phase_error.suppress_like_type_errors err
    else
      err
  in
  (* Check for invalid use of internal classes - note that we must have this
     validation pass _before_ we elaborate enum classes *)
  let err =
    if
      not
        (string_ends_with (Relative_path.suffix filename) ".hhi"
        || TypecheckerOptions.is_systemlib (Provider_context.get_tcopt ctx))
    then
      validate_builtin_enum ~init:err elem
    else
      err
  in

  (* Add implicit extends for enums & enum classes *)
  let elem = elab_enum_class elem in

  (* Use canonical class_ids and validate usage of special names outside classes *)
  let (elem, err) = elab_class_id ~init:err elem in

  (* Warn and rewrite dynamic class_id expressions to unknown class ident
     NB: this must be sequenced after class_id elaboration so that class_id's
     are in canonical form, specifically that any remaining `CIexpr` represent
     dynamic new's
  *)
  let (elem, err) = elab_dynamic_class_name ~init:err elem in

  (* Elaboration `Collection` to `ValCollection` or `KeyValCollection *)
  let (elem, err) = elab_collection ~init:err elem in

  let (elem, err) = elab_call ~init:err elem in

  let (elem, err) = elab_tuple ~init:err elem in

  let (elem, err) = elab_invariant ~init:err elem in

  let (elem, err) = elab_const_expr ~init:err elem in

  let (elem, err) = elab_user_attrs ~init:err elem in

  let elem = elab_import elem in

  let elem = elab_lvar elem in

  let (elem, err) = elab_as_expr ~init:err elem in

  let err = validate_fun_params ~init:err elem in

  let elem = elab_block elem in

  let elem = elab_pipe elem in

  let elem = elab_func_body elem in

  let elem = elab_lambda_captures elem in

  let err = validate_class_req ~init:err elem in

  let err =
    let level = TypecheckerOptions.explicit_consistent_constructors tcopt in
    if level > 0 then
      let env = Naming_validate_consistent_construct.Env.create level in
      validate_consistent_construct ~init:err ~env elem
    else
      err
  in

  (* General expression / statement / xhp elaboration & validation *)
  let elem = elab_help ctx env elem in

  (* Miscellaneous validation  *)
  (* TODO[mjt] move these to NAST checks*)
  (* Check for specific errors when referring to xhp classes *)
  let err = validate_xhp ~init:err elem in

  (* Apply elaboration / validation based on typechecker options *)
  (* Soft types *)
  let soft_as_like =
    TypecheckerOptions.interpret_soft_types_as_like_types tcopt
  in
  let elem = elab_soft elem ~env:soft_as_like in

  (* If HKTs are not enabled, we remove type parameter here and generate
     specific errors rather than arity errors in typing *)
  (* TODO[mjt] you do get an arity error from typing if you don't do
     this - we might consider specializing _that_ error to give info
     about HKTs rather than doing this from naming *)
  let hkt_enabled = TypecheckerOptions.higher_kinded_types tcopt in
  let (elem, err) =
    if not hkt_enabled then
      elab_hkt ~init:err elem
    else
      (elem, err)
  in

  (* SupportDyn *)
  let err =
    if
      (not
         (string_ends_with (Relative_path.suffix filename) ".hhi"
         || TypecheckerOptions.is_systemlib tcopt))
      && not
           (TypecheckerOptions.experimental_feature_enabled
              tcopt
              TypecheckerOptions.experimental_supportdynamic_type_hint)
    then
      validate_supportdyn ~init:err elem
    else
      err
  in

  (* Sound dynamic *)
  let elem =
    if TypecheckerOptions.everything_sdt tcopt then
      elab_everything_sdt elem
    else
      elem
  in

  (* enum class type constants *)
  (* we're still in the middle of developing type constants for enum classes
     so we gate them carefully for now:
     They must use the feature flag `type_constants_in_enum_class` AND
     be in a selected list of directories.

     For internal testing, we provide a global "enable" flag to just
     enable them. This is off by default except in hh_single_type_check.
  *)
  let file_str = Relative_path.suffix filename in
  let dir_str = Filename.dirname file_str in

  let err =
    if
      TypecheckerOptions.allow_all_locations_for_type_constant_in_enum_class
        tcopt
      || List.exists ~f:(String.equal dir_str)
         @@ TypecheckerOptions.allowed_locations_for_type_constant_in_enum_class
              tcopt
    then
      err
    else
      validate_enum_class_typeconst ~init:err elem
  in

  (* modules *)
  let err =
    if
      TypecheckerOptions.allow_all_files_for_module_declarations tcopt
      || List.exists
           ~f:(fun allowed_file ->
             let len = String.length allowed_file in
             if len > 0 then
               match allowed_file.[len - 1] with
               | '*' ->
                 let allowed_dir =
                   String.sub allowed_file ~pos:0 ~len:(len - 1)
                 in
                 String_utils.string_starts_with file_str allowed_dir
               | _ -> String.equal allowed_file file_str
             else
               false)
           (TypecheckerOptions.allowed_files_for_module_declarations tcopt)
    then
      err
    else
      validate_module ~init:err elem
  in

  Naming_phase_error.emit err;
  elem

let program_filename defs =
  let open Aast_defs in
  let rec aux = function
    | Fun fun_def :: _ -> Pos.filename fun_def.fd_fun.f_span
    | Class class_ :: _ -> Pos.filename class_.c_span
    | Stmt (pos, _) :: _ -> Pos.filename pos
    | Typedef typedef :: _ -> Pos.filename typedef.t_span
    | Constant gconst :: _ -> Pos.filename gconst.cst_span
    | Module module_def :: _ -> Pos.filename module_def.md_span
    | _ :: rest -> aux rest
    | _ -> Relative_path.default
  in
  aux defs

let program ctx ast =
  let env = Env.make_top_level_env ctx in
  let filename = program_filename ast in
  elab_elem
    ast
    ~ctx
    ~env
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_program
          (Naming_elaborate_namespaces_endo.make_env
             Namespace_env.empty_with_default);
      elab_hints = Naming_elab_hints.elab_program;
      elab_collection = Naming_elab_collection.elab_program;
      elab_call = Naming_elab_call.elab_program;
      elab_tuple = Naming_elab_tuple.elab_program;
      elab_invariant = Naming_elab_invariant.elab_program;
      elab_const_expr = Naming_elab_const_expr.elab_program;
      elab_user_attrs = Naming_elab_user_attributes.elab_program;
      elab_import = Naming_elab_import.elab_program;
      elab_lvar = Naming_elab_lvar.elab_program;
      elab_as_expr = Naming_elab_as_expr.elab_program;
      elab_block = Naming_elab_block.elab_program;
      elab_pipe = Naming_elab_pipe.elab_program;
      elab_func_body = Naming_elab_func_body.elab_program;
      elab_lambda_captures = Naming_captures.elab_program;
      elab_help = program_help;
      elab_soft = Naming_elab_soft.elab_program;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_program;
      elab_hkt = Naming_elab_hkt.elab_program;
      elab_enum_class = Naming_elab_enum_class.elab_program;
      elab_class_id = Naming_elab_class_id.elab_program;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_program;
      validate_fun_params = Naming_validate_fun_params.validate_program;
      validate_xhp = Naming_validate_xhp_name.validate_program;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_program;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_program;
      validate_supportdyn = Naming_validate_supportdyn.validate_program;
      validate_module = Naming_validate_module.validate_program;
      validate_class_req = Naming_validate_class_req.validate_program;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_program;
    }

let fun_def ctx fd =
  let env = Env.make_fun_decl_genv ctx fd in
  let filename = Pos.filename fd.Aast.fd_fun.Aast.f_span in
  elab_elem
    fd
    ~ctx
    ~env
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_fun_def
          (Naming_elaborate_namespaces_endo.make_env env.namespace);
      elab_hints = Naming_elab_hints.elab_fun_def;
      elab_collection = Naming_elab_collection.elab_fun_def;
      elab_call = Naming_elab_call.elab_fun_def;
      elab_tuple = Naming_elab_tuple.elab_fun_def;
      elab_invariant = Naming_elab_invariant.elab_fun_def;
      elab_const_expr = Naming_elab_const_expr.elab_fun_def;
      elab_user_attrs = Naming_elab_user_attributes.elab_fun_def;
      elab_import = Naming_elab_import.elab_fun_def;
      elab_lvar = Naming_elab_lvar.elab_fun_def;
      elab_as_expr = Naming_elab_as_expr.elab_fun_def;
      elab_block = Naming_elab_block.elab_fun_def;
      elab_pipe = Naming_elab_pipe.elab_fun_def;
      elab_func_body = Naming_elab_func_body.elab_fun_def;
      elab_lambda_captures = Naming_captures.elab_fun_def;
      elab_help = (fun _ _ elem -> elem);
      elab_soft = Naming_elab_soft.elab_fun_def;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_fun_def;
      elab_hkt = Naming_elab_hkt.elab_fun_def;
      elab_enum_class = Naming_elab_enum_class.elab_fun_def;
      elab_class_id = Naming_elab_class_id.elab_fun_def;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_fun_def;
      validate_fun_params = Naming_validate_fun_params.validate_fun_def;
      validate_xhp = Naming_validate_xhp_name.validate_fun_def;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_fun_def;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_fun_def;
      validate_supportdyn = Naming_validate_supportdyn.validate_fun_def;
      validate_module = Naming_validate_module.validate_fun_def;
      validate_class_req = Naming_validate_class_req.validate_fun_def;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_fun_def;
    }

let class_ ctx c =
  let env = Env.make_class_env ctx c in
  let filename = Pos.filename c.Aast.c_span in
  elab_elem
    c
    ~ctx
    ~env
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_class_
          (Naming_elaborate_namespaces_endo.make_env env.namespace);
      elab_hints = Naming_elab_hints.elab_class;
      elab_collection = Naming_elab_collection.elab_class;
      elab_call = Naming_elab_call.elab_class;
      elab_tuple = Naming_elab_tuple.elab_class;
      elab_invariant = Naming_elab_invariant.elab_class;
      elab_const_expr = Naming_elab_const_expr.elab_class;
      elab_user_attrs = Naming_elab_user_attributes.elab_class;
      elab_import = Naming_elab_import.elab_class;
      elab_lvar = Naming_elab_lvar.elab_class;
      elab_as_expr = Naming_elab_as_expr.elab_class;
      elab_block = Naming_elab_block.elab_class;
      elab_pipe = Naming_elab_pipe.elab_class;
      elab_func_body = Naming_elab_func_body.elab_class;
      elab_lambda_captures = Naming_captures.elab_class;
      elab_help = class_help;
      elab_soft = Naming_elab_soft.elab_class;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_class;
      elab_hkt = Naming_elab_hkt.elab_class;
      elab_enum_class = Naming_elab_enum_class.elab_class;
      elab_class_id = Naming_elab_class_id.elab_class;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_class;
      validate_fun_params = Naming_validate_fun_params.validate_class;
      validate_xhp = Naming_validate_xhp_name.validate_class;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_class;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_class;
      validate_supportdyn = Naming_validate_supportdyn.validate_class;
      validate_module = Naming_validate_module.validate_class;
      validate_class_req = Naming_validate_class_req.validate_class;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_class;
    }

let module_ ctx module_ =
  let env = Env.make_module_env ctx module_ in
  let filename = Pos.filename module_.Aast.md_span in
  elab_elem
    module_
    ~ctx
    ~env
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_module_def
          (Naming_elaborate_namespaces_endo.make_env env.namespace);
      elab_hints = Naming_elab_hints.elab_module_def;
      elab_collection = Naming_elab_collection.elab_module_def;
      elab_call = Naming_elab_call.elab_module_def;
      elab_tuple = Naming_elab_tuple.elab_module_def;
      elab_invariant = Naming_elab_invariant.elab_module_def;
      elab_const_expr = Naming_elab_const_expr.elab_module_def;
      elab_user_attrs = Naming_elab_user_attributes.elab_module_def;
      elab_import = Naming_elab_import.elab_module_def;
      elab_lvar = Naming_elab_lvar.elab_module_def;
      elab_as_expr = Naming_elab_as_expr.elab_module_def;
      elab_block = Naming_elab_block.elab_module_def;
      elab_pipe = Naming_elab_pipe.elab_module_def;
      elab_func_body = Naming_elab_func_body.elab_module_def;
      elab_lambda_captures = Naming_captures.elab_module_def;
      elab_help = (fun _ _ elem -> elem);
      elab_soft = Naming_elab_soft.elab_module_def;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_module_def;
      elab_hkt = Naming_elab_hkt.elab_module_def;
      elab_enum_class = Naming_elab_enum_class.elab_module_def;
      elab_class_id = Naming_elab_class_id.elab_module_def;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_module_def;
      validate_fun_params = Naming_validate_fun_params.validate_module_def;
      validate_xhp = Naming_validate_xhp_name.validate_module_def;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_module_def;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_module_def;
      validate_supportdyn = Naming_validate_supportdyn.validate_module_def;
      validate_module = Naming_validate_module.validate_module_def;
      validate_class_req = Naming_validate_class_req.validate_module_def;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_module_def;
    }

let global_const ctx cst =
  let env = Env.make_const_env ctx cst in
  let filename = Pos.filename cst.Aast.cst_span in
  elab_elem
    cst
    ~ctx
    ~env
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_gconst
          (Naming_elaborate_namespaces_endo.make_env env.namespace);
      elab_hints = Naming_elab_hints.elab_gconst;
      elab_collection = Naming_elab_collection.elab_gconst;
      elab_call = Naming_elab_call.elab_gconst;
      elab_tuple = Naming_elab_tuple.elab_gconst;
      elab_invariant = Naming_elab_invariant.elab_gconst;
      elab_const_expr = Naming_elab_const_expr.elab_gconst;
      elab_user_attrs = Naming_elab_user_attributes.elab_gconst;
      elab_import = Naming_elab_import.elab_gconst;
      elab_lvar = Naming_elab_lvar.elab_gconst;
      elab_as_expr = Naming_elab_as_expr.elab_gconst;
      elab_block = Naming_elab_block.elab_gconst;
      elab_pipe = Naming_elab_pipe.elab_gconst;
      elab_func_body = Naming_elab_func_body.elab_gconst;
      elab_lambda_captures = Naming_captures.elab_gconst;
      elab_help = (fun _ _ elem -> elem);
      elab_soft = Naming_elab_soft.elab_gconst;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_gconst;
      elab_hkt = Naming_elab_hkt.elab_gconst;
      elab_enum_class = Naming_elab_enum_class.elab_gconst;
      elab_class_id = Naming_elab_class_id.elab_gconst;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_gconst;
      validate_fun_params = Naming_validate_fun_params.validate_gconst;
      validate_xhp = Naming_validate_xhp_name.validate_gconst;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_gconst;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_gconst;
      validate_supportdyn = Naming_validate_supportdyn.validate_gconst;
      validate_module = Naming_validate_module.validate_gconst;
      validate_class_req = Naming_validate_class_req.validate_gconst;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_gconst;
    }

let typedef ctx tdef =
  let env = Env.make_typedef_env ctx tdef in
  let filename = Pos.filename @@ tdef.Aast.t_span in
  elab_elem
    tdef
    ~ctx
    ~env
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_typedef
          (Naming_elaborate_namespaces_endo.make_env env.namespace);
      elab_hints = Naming_elab_hints.elab_typedef;
      elab_collection = Naming_elab_collection.elab_typedef;
      elab_call = Naming_elab_call.elab_typedef;
      elab_tuple = Naming_elab_tuple.elab_typedef;
      elab_invariant = Naming_elab_invariant.elab_typedef;
      elab_const_expr = Naming_elab_const_expr.elab_typedef;
      elab_user_attrs = Naming_elab_user_attributes.elab_typedef;
      elab_import = Naming_elab_import.elab_typedef;
      elab_lvar = Naming_elab_lvar.elab_typedef;
      elab_as_expr = Naming_elab_as_expr.elab_typedef;
      elab_block = Naming_elab_block.elab_typedef;
      elab_pipe = Naming_elab_pipe.elab_typedef;
      elab_func_body = Naming_elab_func_body.elab_typedef;
      elab_lambda_captures = Naming_captures.elab_typedef;
      elab_help = (fun _ _ elem -> elem);
      elab_soft = Naming_elab_soft.elab_typedef;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_typedef;
      elab_hkt = Naming_elab_hkt.elab_typedef;
      elab_enum_class = Naming_elab_enum_class.elab_typedef;
      elab_class_id = Naming_elab_class_id.elab_typedef;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_typedef;
      validate_fun_params = Naming_validate_fun_params.validate_typedef;
      validate_xhp = Naming_validate_xhp_name.validate_typedef;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_typedef;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_typedef;
      validate_supportdyn = Naming_validate_supportdyn.validate_typedef;
      validate_module = Naming_validate_module.validate_typedef;
      validate_class_req = Naming_validate_class_req.validate_typedef;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_typedef;
    }
