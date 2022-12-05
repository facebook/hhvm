(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
module Err = Naming_phase_error
module SN = Naming_special_names

module Hint_ctx = struct
  type t = {
    (* `this` is forbidden as a hint in this context *)
    forbid_this: bool;
    (* allow `void` and `noreturn` hints in this context *)
    allow_retonly: bool;
    (* allow wildcard hints in this context *)
    allow_wildcard: bool;
    (* allow like type hints in this context *)
    allow_like: bool;
    (* the current hint appears in a `where` clause *)
    in_where_clause: bool;
    (* the current hint appears in a coeffect `context` *)
    in_context: bool;
    (* the depth at which the hint appears; a top-level in has depth 0 *)
    tp_depth: int;
  }

  let empty =
    {
      forbid_this = false;
      allow_retonly = false;
      allow_wildcard = false;
      allow_like = false;
      in_where_clause = false;
      in_context = false;
      tp_depth = 0;
    }

  let create
      ?(forbid_this = false)
      ?(allow_retonly = false)
      ?(allow_wildcard = false)
      ?(allow_like = false)
      ?(in_where_clause = false)
      ?(in_context = false)
      ?(tp_depth = 0)
      () =
    {
      forbid_this;
      allow_retonly;
      allow_wildcard;
      allow_like;
      in_where_clause;
      in_context;
      tp_depth;
    }
end

module Env = struct
  type t = {
    (* the context in which a hint appears *)
    hint_ctx: Hint_ctx.t;
    (* the set of type parameters in scope, if any *)
    tparams: SSet.t;
    (* the name of the class , if any *)
    current_class: (Ast_defs.id * Ast_defs.classish_kind * bool) option;
    in_mode: FileInfo.mode;
  }

  let empty =
    {
      hint_ctx = Hint_ctx.empty;
      tparams = SSet.empty;
      current_class = None;
      in_mode = FileInfo.Mstrict;
    }

  let add_tparams ps init =
    List.fold
      ps
      ~f:(fun acc Aast.{ tp_name = (_, nm); _ } -> SSet.add nm acc)
      ~init

  let extend_tparams env ps =
    let tparams = add_tparams ps env.tparams in
    { env with tparams }

  let in_class Aast.{ c_mode; c_name; c_kind; c_tparams; c_final; _ } =
    {
      current_class = Some (c_name, c_kind, c_final);
      in_mode = c_mode;
      tparams = add_tparams c_tparams SSet.empty;
      hint_ctx = Hint_ctx.empty;
    }

  let in_fun_def Aast.{ fd_fun; fd_mode; _ } =
    {
      current_class = None;
      in_mode = fd_mode;
      tparams = add_tparams fd_fun.Aast.f_tparams SSet.empty;
      hint_ctx = Hint_ctx.empty;
    }

  let in_typedef Aast.{ t_tparams; _ } =
    {
      current_class = None;
      in_mode = FileInfo.Mstrict;
      tparams = add_tparams t_tparams SSet.empty;
      hint_ctx = Hint_ctx.empty;
    }

  let in_gconst Aast.{ cst_mode; _ } =
    {
      current_class = None;
      in_mode = cst_mode;
      tparams = SSet.empty;
      hint_ctx = Hint_ctx.empty;
    }
end

(* -- Helpers --------------------------------------------------------------- *)

(* We permit class constants to be used as shape field names. Here we replace
    uses of `self` with the class to which they refer or `unknown` if the shape
   is not defined within the context of a class *)
let canonical_shape_name current_class_opt = function
  (* TODO[mjt] int field names appear to be a parse error? *)
  | Ast_defs.SFlit_int (pos, s) -> (Ast_defs.SFlit_int (pos, s), None)
  | Ast_defs.SFlit_str (pos, s) -> (Ast_defs.SFlit_str (pos, s), None)
  | Ast_defs.SFclass_const ((class_pos, class_name), (const_pos, const_name)) ->
    (* e.g. Foo::BAR or self::BAR. The first tuple is the use of Foo, second is the use of BAR *)
    (* We will resolve class-name 'self' *)
    let (class_name, err) =
      if String.equal class_name SN.Classes.cSelf then
        match current_class_opt with
        | Some ((_class_decl_pos, class_name), _, _) -> (class_name, None)
        | None ->
          let err =
            Err.typing @@ Typing_error.Primary.Self_outside_class class_pos
          in
          (SN.Classes.cUnknown, Some err)
      else
        (class_name, None)
    in
    ( Ast_defs.SFclass_const ((class_pos, class_name), (const_pos, const_name)),
      err )

type canon_result =
  | Concrete of Aast.hint
  | This of Pos.t
  | Classname of Pos.t
  | Wildcard of Pos.t
  | Noreturn of Pos.t
  | Void of Pos.t
  | Tycon of Pos.t * string
  | Typaram of string
  | Varray of Pos.t
  | Darray of Pos.t
  | Vec_or_dict of Pos.t
  | Error of Naming_error.t

(* A number of hints are represented by `Happly` after lowering; we elaborate
   to the canonical representation here taking care to separate the result
   so we can apply subsequent validation of the hint based on where it appeared *)
let canonical_tycon typarams (pos, name) =
  if String.equal name SN.Typehints.int then
    Concrete (pos, Aast.(Hprim Tint))
  else if String.equal name SN.Typehints.bool then
    Concrete (pos, Aast.(Hprim Tbool))
  else if String.equal name SN.Typehints.float then
    Concrete (pos, Aast.(Hprim Tfloat))
  else if String.equal name SN.Typehints.string then
    Concrete (pos, Aast.(Hprim Tstring))
  else if String.equal name SN.Typehints.darray then
    Darray pos
  else if String.equal name SN.Typehints.varray then
    Varray pos
  (* TODO[mjt] `vec_or_dict` is currently special cased since the canonical representation
     requires us to have no arity mismatches or throw away info. We do not use that repr here
     to avoid having to do so. Ultimately, we should remove that special case *)
  else if
    String.(
      equal name SN.Typehints.varray_or_darray
      || equal name SN.Typehints.vec_or_dict)
  then
    Vec_or_dict pos
  else if String.equal name SN.Typehints.void then
    Void pos
  else if String.equal name SN.Typehints.noreturn then
    Noreturn pos
  else if String.equal name SN.Typehints.null then
    Concrete (pos, Aast.(Hprim Tnull))
  else if String.equal name SN.Typehints.num then
    Concrete (pos, Aast.(Hprim Tnum))
  else if String.equal name SN.Typehints.resource then
    Concrete (pos, Aast.(Hprim Tresource))
  else if String.equal name SN.Typehints.arraykey then
    Concrete (pos, Aast.(Hprim Tarraykey))
  else if String.equal name SN.Typehints.mixed then
    Concrete (pos, Aast.Hmixed)
  else if String.equal name SN.Typehints.nonnull then
    Concrete (pos, Aast.Hnonnull)
  else if String.equal name SN.Typehints.nothing then
    Concrete (pos, Aast.Hnothing)
  else if String.equal name SN.Typehints.dynamic then
    Concrete (pos, Aast.Hdynamic)
  else if String.equal name SN.Typehints.this then
    This pos
  else if String.equal name SN.Typehints.wildcard then
    Wildcard pos
  else if
    String.(
      equal name ("\\" ^ SN.Typehints.void)
      || equal name ("\\" ^ SN.Typehints.null)
      || equal name ("\\" ^ SN.Typehints.noreturn)
      || equal name ("\\" ^ SN.Typehints.int)
      || equal name ("\\" ^ SN.Typehints.bool)
      || equal name ("\\" ^ SN.Typehints.float)
      || equal name ("\\" ^ SN.Typehints.num)
      || equal name ("\\" ^ SN.Typehints.string)
      || equal name ("\\" ^ SN.Typehints.resource)
      || equal name ("\\" ^ SN.Typehints.mixed)
      || equal name ("\\" ^ SN.Typehints.nonnull)
      || equal name ("\\" ^ SN.Typehints.arraykey)
      || equal name ("\\" ^ SN.Typehints.nothing))
  then
    Error (Naming_error.Primitive_top_level pos)
  (* TODO[mjt] why wouldn't be have a fully qualified name here? *)
  else if String.(equal name SN.Classes.cClassname || equal name "classname")
  then
    Classname pos
  else if SSet.mem name typarams then
    Typaram name
  else
    Tycon (pos, name)

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    (* -- Function definitions ---------------------------------------------- *)
    method! on_fun_def _env fd =
      let env = Env.in_fun_def fd in
      super#on_fun_def env fd

    (* -- Module definitions ------------------------------------------------ *)
    method! on_module_def _env md =
      let env = Env.empty in
      super#on_module_def env md

    (* -- Global constants -------------------------------------------------- *)
    method! on_gconst _env gc =
      let env = Env.in_gconst gc in
      super#on_gconst env gc

    (* -- Type definitions -------------------------------------------------- *)
    method! on_typedef _env td =
      let env = Env.in_typedef td in
      super#on_typedef env td

    (* -- File attributes --------------------------------------------------- *)
    method! on_file_attribute _env fa =
      (* These are elaborated away in Namespaces.elaborate_toplevel_defs *)
      (fa, self#zero)

    (* -- Classes ----------------------------------------------------------- *)
    method! on_class_ outside_class_env c =
      (* Update the environment with class name, type parameters and mode *)
      let env = Env.in_class c in
      let (c_tparams, tparams_err) =
        let hint_ctx = Hint_ctx.create ~forbid_this:true () in
        let env = Env.{ env with hint_ctx } in
        super#on_list self#on_tparam env c.Aast.c_tparams
      in

      let (c_extends, extends_err) =
        let hint_ctx = Hint_ctx.create ~allow_retonly:false () in
        super#on_list self#on_hint Env.{ env with hint_ctx } c.Aast.c_extends
      in

      let (c_uses, uses_err) =
        let hint_ctx = Hint_ctx.empty in
        super#on_list self#on_hint Env.{ env with hint_ctx } c.Aast.c_uses
      in

      let (c_xhp_attrs, xhp_attrs_err) =
        let hint_ctx = Hint_ctx.empty in
        super#on_list
          super#on_xhp_attr
          Env.{ env with hint_ctx }
          c.Aast.c_xhp_attrs
      in

      let (c_xhp_attr_uses, xhp_attr_uses_err) =
        let hint_ctx = Hint_ctx.empty in
        super#on_list
          self#on_hint
          Env.{ env with hint_ctx }
          c.Aast.c_xhp_attr_uses
      in

      let (c_reqs, reqs_err) =
        let hint_ctx = Hint_ctx.empty in
        super#on_list
          (self#on_fst self#on_hint)
          Env.{ env with hint_ctx }
          c.Aast.c_reqs
      in

      let (c_implements, implements_err) =
        let hint_ctx = Hint_ctx.create ~allow_retonly:false () in
        super#on_list self#on_hint Env.{ env with hint_ctx } c.Aast.c_implements
      in

      let (c_where_constraints, where_constraints_err) =
        super#on_list
          self#on_where_constraint_hint
          env
          c.Aast.c_where_constraints
      in

      let (c_consts, consts_err) =
        super#on_list super#on_class_const env c.Aast.c_consts
      in

      let (c_typeconsts, typeconsts_err) =
        super#on_list super#on_class_typeconst_def env c.Aast.c_typeconsts
      in

      let (c_vars, vars_err) =
        super#on_list self#on_class_var env c.Aast.c_vars
      in

      let (c_enum, enum_err) =
        super#on_option super#on_enum_ env c.Aast.c_enum
      in

      let (c_methods, methods_err) =
        super#on_list self#on_method_ env c.Aast.c_methods
      in

      (* The attributes applied to a class exist outside the current class so
         references to `self` are invalid *)
      let (c_user_attributes, user_attributes_err) =
        super#on_list
          super#on_user_attribute
          outside_class_env
          c.Aast.c_user_attributes
      in
      let (c_file_attributes, file_attributes_err) =
        super#on_list super#on_file_attribute env c.Aast.c_file_attributes
      in
      let err =
        self#plus_all
          [
            file_attributes_err;
            user_attributes_err;
            methods_err;
            enum_err;
            vars_err;
            typeconsts_err;
            consts_err;
            where_constraints_err;
            implements_err;
            reqs_err;
            xhp_attr_uses_err;
            xhp_attrs_err;
            uses_err;
            extends_err;
            tparams_err;
          ]
      in
      let c =
        Aast.
          {
            c with
            c_tparams;
            c_extends;
            c_uses;
            c_xhp_attr_uses;
            c_reqs;
            c_implements;
            c_where_constraints;
            c_consts;
            c_typeconsts;
            c_vars;
            c_enum;
            c_methods;
            c_xhp_attrs;
            c_user_attributes;
            c_file_attributes;
          }
      in
      (c, err)

    method! on_class_var env cv =
      let (cv_user_attributes, user_attributes_err) =
        super#on_list super#on_user_attribute env cv.Aast.cv_user_attributes
      in
      let (cv_expr, expr_err) =
        super#on_option super#on_expr env cv.Aast.cv_expr
      in
      let (cv_type, type_err) =
        let hint_ctx =
          if cv.Aast.cv_is_static then
            let lsb =
              Naming_attributes.mem SN.UserAttributes.uaLSB cv_user_attributes
            in
            Hint_ctx.create ~forbid_this:(not lsb) ()
          else
            Hint_ctx.empty
        in
        super#on_type_hint Env.{ env with hint_ctx } cv.Aast.cv_type
      in
      let cv = Aast.{ cv with cv_user_attributes; cv_type; cv_expr } in
      let err = self#plus_all [expr_err; type_err; user_attributes_err] in
      (cv, err)

    method! on_method_ env m =
      let env = Env.extend_tparams env m.Aast.m_tparams in

      let (m_tparams, tparams_err) =
        super#on_list self#on_tparam env m.Aast.m_tparams
      in

      let (m_where_constraints, where_constraints_err) =
        super#on_list
          self#on_where_constraint_hint
          env
          m.Aast.m_where_constraints
      in

      let (m_params, params_err) =
        super#on_list self#on_fun_param env m.Aast.m_params
      in

      let (m_ctxs, ctxs_err) =
        super#on_option self#on_contexts env m.Aast.m_ctxs
      in

      let (m_unsafe_ctxs, unsafe_ctxs_err) =
        super#on_option self#on_contexts env m.Aast.m_unsafe_ctxs
      in

      let (m_body, body_err) = super#on_func_body env m.Aast.m_body in

      let (m_user_attributes, user_attributes_err) =
        super#on_list super#on_user_attribute env m.Aast.m_user_attributes
      in

      let (m_ret, ret_err) =
        let hint_ctx = Hint_ctx.create ~allow_retonly:true () in
        super#on_type_hint Env.{ env with hint_ctx } m.Aast.m_ret
      in

      let err =
        self#plus_all
          [
            ret_err;
            user_attributes_err;
            body_err;
            unsafe_ctxs_err;
            ctxs_err;
            params_err;
            where_constraints_err;
            tparams_err;
          ]
      and m =
        Aast.
          {
            m with
            m_tparams;
            m_where_constraints;
            m_params;
            m_ctxs;
            m_unsafe_ctxs;
            m_body;
            m_user_attributes;
            m_ret;
          }
      in
      (m, err)

    (* -- Functions --------------------------------------------------------- *)
    method! on_fun_ env f =
      let (f_ret, ret_err) =
        let hint_ctx = Hint_ctx.create ~allow_retonly:true () in
        super#on_type_hint Env.{ env with hint_ctx } f.Aast.f_ret
      in

      let (f_tparams, tparams_err) =
        super#on_list self#on_tparam env f.Aast.f_tparams
      in

      let (f_where_constraints, where_constraints_err) =
        super#on_list
          self#on_where_constraint_hint
          env
          f.Aast.f_where_constraints
      in

      let (f_params, params_err) =
        super#on_list self#on_fun_param env f.Aast.f_params
      in

      let (f_ctxs, ctxs_err) =
        super#on_option self#on_contexts env f.Aast.f_ctxs
      in

      let (f_unsafe_ctxs, unsafe_ctxs_err) =
        super#on_option self#on_contexts env f.Aast.f_unsafe_ctxs
      in

      let (f_body, body_err) = super#on_func_body env f.Aast.f_body in

      let (f_user_attributes, user_attributes_err) =
        super#on_list super#on_user_attribute env f.Aast.f_user_attributes
      in

      let err =
        self#plus_all
          [
            user_attributes_err;
            body_err;
            unsafe_ctxs_err;
            ctxs_err;
            params_err;
            where_constraints_err;
            tparams_err;
            ret_err;
          ]
      and f =
        Aast.
          {
            f with
            f_ret;
            f_tparams;
            f_where_constraints;
            f_params;
            f_ctxs;
            f_unsafe_ctxs;
            f_body;
            f_user_attributes;
          }
      in
      (f, err)

    (* -- Expressions ------------------------------------------------------- *)

    method! on_Cast env hint expr =
      let hint_ctx = Hint_ctx.create ~tp_depth:1 () in
      let env = Env.{ env with hint_ctx } in
      let (expr, expr_err) = self#on_expr env expr in
      let (hint, hint_err) = self#on_hint env hint in
      (* TODO[mjt] pull this out into a validation step *)
      let cast_hint_err =
        match hint with
        | (_, Aast.(Hprim (Tint | Tbool | Tfloat | Tstring))) -> self#zero
        | (_, Aast.(Happly ((_, tycon_nm), _)))
          when String.(
                 equal tycon_nm SN.Collections.cDict
                 || equal tycon_nm SN.Collections.cVec) ->
          self#zero
        | (_, Aast.Hvec_or_dict (_, _)) -> self#zero
        | (_, Aast.Hany) ->
          (* We end up with a `Hany` when we have an arity error for dict/vec
             - we don't error on this case to preserve behaviour
          *)
          self#zero
        | (pos, _) -> Err.naming @@ Naming_error.Object_cast pos
      in
      let err = self#plus_all [expr_err; hint_err; cast_hint_err] in
      (Aast.Cast (hint, expr), err)

    method! on_Is env expr hint =
      let hint_ctx = Hint_ctx.create ~allow_wildcard:true ~allow_like:true () in
      super#on_Is Env.{ env with hint_ctx } expr hint

    method! on_As env expr hint is_final =
      let hint_ctx = Hint_ctx.create ~allow_wildcard:true ~allow_like:true () in
      super#on_As Env.{ env with hint_ctx } expr hint is_final

    method! on_Upcast env expr hint =
      let hint_ctx =
        Hint_ctx.create ~allow_wildcard:false ~allow_like:true ()
      in
      super#on_Upcast Env.{ env with hint_ctx } expr hint

    method! on_Shape env fdl =
      let (name_err, fdl) =
        List.fold_map fdl ~init:self#zero ~f:(fun err (nm, expr) ->
            let (nm, nm_err_opt) =
              canonical_shape_name env.Env.current_class nm
            in
            let err =
              self#plus err @@ Option.value ~default:self#zero nm_err_opt
            in
            (err, (nm, expr)))
      in
      let (expr_, err) = super#on_Shape env fdl in
      (expr_, self#plus name_err err)

    (* -- Hints   ----------------------------------------------------------- *)
    method! on_hint env (pos, hint_) =
      let Hint_ctx.
            {
              forbid_this;
              allow_wildcard;
              allow_like;
              allow_retonly;
              tp_depth;
              _;
            } =
        env.Env.hint_ctx
      in
      match hint_ with
      | Aast.Hunion hints ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_like
            ~allow_retonly
            ()
        in
        let (hint_, err) = super#on_Hunion Env.{ env with hint_ctx } hints in
        ((pos, hint_), err)
      | Aast.Hintersection hints ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_like
            ~allow_retonly
            ()
        in
        let (hint_, err) =
          super#on_Hintersection Env.{ env with hint_ctx } hints
        in
        ((pos, hint_), err)
      | Aast.Htuple hints ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_like
            ~allow_retonly
            ~tp_depth:(tp_depth + 1)
            ()
        in
        let (hint_, err) = super#on_Htuple Env.{ env with hint_ctx } hints in
        ((pos, hint_), err)
      | Aast.Hoption hint ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_like
            ~allow_retonly
            ()
        in
        let (hint_, err) = super#on_Hoption Env.{ env with hint_ctx } hint in
        ((pos, hint_), err)
      | Aast.Hlike hint ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_like
            ~allow_retonly
            ()
        in
        let (hint_, hint_err) = super#on_Hlike Env.{ env with hint_ctx } hint in
        (* MT: note that we are not considering typechecker options here so we would need
           to remove errors which are subsequently allowed under those options - this should
           be straightforward since `like_type_hints` (and all other tc options) is global *)
        let like_err =
          if not allow_like then
            Err.like_type pos
          else
            self#zero
        in
        ((pos, hint_), self#plus hint_err like_err)
      | Aast.Hsoft hint ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_like
            ~allow_retonly
            ()
        in
        let (hint_, err) = super#on_Hsoft Env.{ env with hint_ctx } hint in
        ((pos, hint_), err)
      | Aast.Hfun hfun ->
        let (hint_, err) = self#on_Hfun env hfun in
        ((pos, hint_), err)
      | Aast.Happly (tycon, hints) ->
        self#canonicalise_happly env pos tycon hints
      | Aast.Haccess (hint, ids) ->
        let (hint_, err) = self#on_Haccess env hint ids in
        ((pos, hint_), err)
      | Aast.Hrefinement (subject, members) ->
        let hint_ctx =
          Hint_ctx.create ~forbid_this ~allow_wildcard ~allow_like ()
        in
        let (hint_, err) =
          super#on_Hrefinement Env.{ env with hint_ctx } subject members
        in
        ((pos, hint_), err)
      | Aast.Hshape nast_shape_info ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_like
            ~tp_depth
            ~allow_retonly
            ()
        in
        let (hint_, err) =
          super#on_Hshape Env.{ env with hint_ctx } nast_shape_info
        in
        ((pos, hint_), err)
      (* TODO[mjt] would we _ever_ expect to see these on a lowered AST? *)
      | Aast.Hmixed
      | Aast.Hfun_context _
      | Aast.Hvar _ ->
        ((pos, hint_), self#zero)
      (* The following should never appear on a lowered AST - we handle the cases
         that appear after canonicalizing `Happly` in `on_canonical_hint` *)
      | Aast.Herr
      | Aast.Hany
      | Aast.Hnonnull
      | Aast.Habstr _
      | Aast.Hvec_or_dict _
      | Aast.Hprim _
      | Aast.Hthis
      | Aast.Hdynamic
      | Aast.Hnothing ->
        let err = Err.unexpected_hint pos in
        ((pos, Aast.Herr), err)

    method private canonicalise_happly env hint_pos tycon hints =
      let Hint_ctx.{ forbid_this; allow_wildcard; allow_retonly; tp_depth; _ } =
        env.Env.hint_ctx
      in
      (* After lowering many hints are represented as `Happly(...,...)`. Here
         we canonicalise the representation of type constructor then handle
         errors and further elaboration *)
      match canonical_tycon env.Env.tparams tycon with
      (* The hint was malformed *)
      | Error err -> ((hint_pos, Aast.Herr), Err.naming err)
      (* The type constructors canonical representation is a concrete type *)
      | Concrete (pos, hint_) ->
        (* We can't represent a concrete type applied to other types
           so we raise an error here *)
        let err =
          if not @@ List.is_empty hints then
            Err.naming @@ Naming_error.Unexpected_type_arguments pos
          else
            self#zero
        in
        ((hint_pos, hint_), err)
      | Void pos ->
        if allow_retonly then
          let err =
            if not @@ List.is_empty hints then
              Err.naming @@ Naming_error.Unexpected_type_arguments pos
            else
              self#zero
          in
          ((hint_pos, Aast.(Hprim Tvoid)), err)
        else
          ( (hint_pos, Aast.Herr),
            Err.naming
            @@ Naming_error.Return_only_typehint { pos; kind = `void } )
      | Noreturn pos ->
        if allow_retonly then
          let err =
            if not @@ List.is_empty hints then
              Err.naming @@ Naming_error.Unexpected_type_arguments pos
            else
              self#zero
          in
          ((hint_pos, Aast.(Hprim Tnoreturn)), err)
        else
          ( (hint_pos, Aast.Herr),
            Err.naming
            @@ Naming_error.Return_only_typehint { pos; kind = `noreturn } )
      (* The type constructors corresponds to an in-scope type parameter *)
      | Typaram name ->
        let hint_ctx =
          Hint_ctx.create
            ~allow_wildcard
            ~forbid_this
            ~allow_retonly:true
            ~tp_depth:(tp_depth + 1)
            ()
        in
        let (hint_, err) =
          super#on_Habstr Env.{ env with hint_ctx } name hints
        in
        ((hint_pos, hint_), err)
      (* The type constructors canonical representation is `Happly` but
         additional elaboration / validation is required *)
      | This pos when not @@ forbid_this ->
        let err =
          if not @@ List.is_empty hints then
            Err.naming @@ Naming_error.This_no_argument hint_pos
          else
            self#zero
        in
        ((pos, Aast.Hthis), err)
      | This _ ->
        let err = Err.naming @@ Naming_error.This_type_forbidden hint_pos in
        ((hint_pos, Aast.Herr), err)
      | Wildcard pos ->
        if allow_wildcard && tp_depth >= 1 (* prevents 3 as _ *) then
          if not (List.is_empty hints) then
            let err =
              Err.naming
              @@ Naming_error.Tparam_applied_to_type
                   { pos = hint_pos; tparam_name = SN.Typehints.wildcard }
            in
            ((hint_pos, Aast.Herr), err)
          else
            ( (hint_pos, Aast.Happly ((pos, SN.Typehints.wildcard), [])),
              self#zero )
        else
          let err = Err.naming @@ Naming_error.Wildcard_hint_disallowed pos in
          ((hint_pos, Aast.Herr), err)
      | Classname pos ->
        (* TODO[mjt] currently if `classname` is not applied to exactly
           one type parameter, it canonicalizes to `Hprim Tstring`.
           Investigate why this happens and if we can delay treatment to
           typing *)
        (match hints with
        | [_] ->
          let hint_ctx =
            Hint_ctx.create
              ~allow_wildcard
              ~forbid_this
              ~allow_retonly:true
              ~tp_depth:(tp_depth + 1)
              ()
          in
          let (hint_, err) =
            super#on_Happly
              Env.{ env with hint_ctx }
              (pos, SN.Classes.cClassname)
              hints
          in
          ((hint_pos, hint_), err)
        | _ ->
          ( (hint_pos, Aast.(Hprim Tstring)),
            Err.naming @@ Naming_error.Classname_param pos ))
      | Darray pos -> self#canonicalise_darray env hint_pos pos hints
      | Varray pos -> self#canonicalise_varray env hint_pos pos hints
      | Vec_or_dict pos -> self#canonicalise_vec_or_dict env hint_pos pos hints
      (* The type constructors canonical representation is `Happly` *)
      | Tycon (pos, tycon) ->
        let hint_ctx =
          Hint_ctx.create
            ~allow_wildcard
            ~forbid_this
            ~allow_retonly:true
            ~tp_depth:(tp_depth + 1)
            ()
        in
        let (hint_, err) =
          super#on_Happly Env.{ env with hint_ctx } (pos, tycon) hints
        in
        ((hint_pos, hint_), err)

    (* TODO[mjt] should we really be special casing `darray`? *)
    method private canonicalise_darray env hint_pos pos hints =
      let Hint_ctx.{ forbid_this; allow_wildcard; tp_depth; _ } =
        env.Env.hint_ctx
      in
      match hints with
      | [] ->
        let err =
          if not @@ FileInfo.is_hhi env.Env.in_mode then
            Err.naming @@ Naming_error.Too_few_type_arguments hint_pos
          else
            self#zero
        in
        let any = (pos, Aast.Hany) in
        ((hint_pos, Aast.Happly ((pos, SN.Collections.cDict), [any; any])), err)
      | [_] ->
        let err =
          if not @@ FileInfo.is_hhi env.Env.in_mode then
            Err.naming @@ Naming_error.Too_few_type_arguments hint_pos
          else
            self#zero
        in
        ((hint_pos, Aast.Hany), err)
      | [key_hint; val_hint] ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_retonly:false
            ~tp_depth:(tp_depth + 1)
            ()
        in
        let env = Env.{ env with hint_ctx } in
        let (key_hint, key_err) = self#on_hint env key_hint in
        let (val_hint, val_err) = self#on_hint env val_hint in
        let err = self#plus key_err val_err in
        let hint =
          ( hint_pos,
            Aast.Happly ((pos, SN.Collections.cDict), [key_hint; val_hint]) )
        in
        (hint, err)
      | _ ->
        let err = Err.naming @@ Naming_error.Too_many_type_arguments hint_pos in
        ((hint_pos, Aast.Hany), err)

    (* TODO[mjt] should we really be special casing `varray`? *)
    method private canonicalise_varray env hint_pos pos hints =
      let Hint_ctx.{ forbid_this; allow_wildcard; tp_depth; _ } =
        env.Env.hint_ctx
      in
      match hints with
      | [] ->
        let err =
          if not @@ FileInfo.is_hhi env.Env.in_mode then
            Err.naming @@ Naming_error.Too_few_type_arguments hint_pos
          else
            self#zero
        in
        let any = (pos, Aast.Hany) in
        ((hint_pos, Aast.Happly ((pos, SN.Collections.cVec), [any])), err)
      | [val_hint] ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_retonly:false
            ~tp_depth:(tp_depth + 1)
            ()
        in
        let env = Env.{ env with hint_ctx } in
        let (val_hint, err) = self#on_hint env val_hint in
        let hint =
          (hint_pos, Aast.Happly ((pos, SN.Collections.cVec), [val_hint]))
        in
        (hint, err)
      | _ ->
        let err = Err.naming @@ Naming_error.Too_many_type_arguments hint_pos in
        ((hint_pos, Aast.Hany), err)

    (* TODO[mjt] should we really be special casing `vec_or_dict` both in
       its representation and error handling? *)
    method private canonicalise_vec_or_dict env hint_pos pos hints =
      let Hint_ctx.{ forbid_this; allow_wildcard; tp_depth; _ } =
        env.Env.hint_ctx
      in
      match hints with
      | [] ->
        let err =
          if not @@ FileInfo.is_hhi env.Env.in_mode then
            Err.naming @@ Naming_error.Too_few_type_arguments hint_pos
          else
            self#zero
        in
        let any = (pos, Aast.Hany) in
        ((hint_pos, Aast.Hvec_or_dict (None, any)), err)
      | [val_hint] ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_retonly:false
            ~tp_depth:(tp_depth + 1)
            ()
        in
        let env = Env.{ env with hint_ctx } in
        let (val_hint, err) = self#on_hint env val_hint in
        let hint = (hint_pos, Aast.Hvec_or_dict (None, val_hint)) in
        (hint, err)
      | [key_hint; val_hint] ->
        let hint_ctx =
          Hint_ctx.create
            ~forbid_this
            ~allow_wildcard
            ~allow_retonly:false
            ~tp_depth:(tp_depth + 1)
            ()
        in
        let env = Env.{ env with hint_ctx } in
        let (key_hint, key_err) = self#on_hint env key_hint in
        let (val_hint, val_err) = self#on_hint env val_hint in
        let err = self#plus key_err val_err in
        let hint = (hint_pos, Aast.Hvec_or_dict (Some key_hint, val_hint)) in
        (hint, err)
      | _ ->
        let err = Err.naming @@ Naming_error.Too_many_type_arguments hint_pos in
        ((hint_pos, Aast.Hany), err)

    method! on_shape_field_info env sfi =
      let Hint_ctx.{ allow_retonly; tp_depth; allow_wildcard; _ } =
        env.Env.hint_ctx
      in
      let hint_ctx =
        Hint_ctx.create
          ~allow_retonly
          ~allow_wildcard
          ~tp_depth:(tp_depth + 1)
          ()
      in
      let (sfi_hint, hint_err) =
        self#on_hint Env.{ env with hint_ctx } sfi.Aast.sfi_hint
      in
      let (sfi_name, name_err_opt) =
        canonical_shape_name env.Env.current_class sfi.Aast.sfi_name
      in
      let err =
        self#plus hint_err @@ Option.value ~default:self#zero name_err_opt
      in
      let sfi = Aast.{ sfi with sfi_hint; sfi_name } in
      (sfi, err)

    method! on_Haccess env (pos, hint_) ids =
      let (hint, err) =
        match hint_ with
        (* TODO[mjt] we appear to be discarding type parameters on `Happly` here
           - should we change the representation of `Haccess` or handle
           erroneous type parameters? *)
        | Aast.Happly ((tycon_pos, tycon_name), _)
          when String.equal tycon_name SN.Classes.cSelf ->
          begin
            match env.Env.current_class with
            | Some (cid, _, _) -> ((pos, Aast.Happly (cid, [])), self#zero)
            | _ ->
              ( (pos, Aast.Herr),
                Err.typing @@ Typing_error.Primary.Self_outside_class tycon_pos
              )
          end
          (* TODO[mjt] is this ever exercised? The cases is handles appear to
             be a parse errors *)
        | Aast.Happly ((tycon_pos, tycon_name), _)
          when String.(
                 equal tycon_name SN.Classes.cStatic
                 || equal tycon_name SN.Classes.cParent) ->
          ( (pos, Aast.Herr),
            Err.naming
            @@ Naming_error.Invalid_type_access_root
                 { pos = tycon_pos; id = tycon_name } )
        | Aast.Happly (tycon, hints) ->
          let (hint, hint_err) = self#canonicalise_happly env pos tycon hints in
          (match hint with
          | (_, Aast.(Hthis | Happly _)) -> (hint, hint_err)
          | (_, Aast.Habstr _)
            when env.Env.hint_ctx.Hint_ctx.in_where_clause
                 || env.Env.hint_ctx.Hint_ctx.in_context ->
            (hint, hint_err)
          | _ ->
            let (tycon_pos, tycon_name) = tycon in
            let err =
              Err.naming
              @@ Naming_error.Invalid_type_access_root
                   { pos = tycon_pos; id = tycon_name }
            in
            ((pos, Aast.Herr), self#plus hint_err err))
        (* TODO[mjt] why are we allow `Hvar`? *)
        | Aast.Hvar _ -> ((pos, hint_), self#zero)
        | _ -> ((pos, Aast.Herr), Err.malformed_access pos)
      in
      (Aast.Haccess (hint, ids), err)

    method! on_Hfun env hfun =
      let (hf_param_tys, param_tys_err) =
        super#on_list self#on_hint env hfun.Aast.hf_param_tys
      in
      let (hf_variadic_ty, variadic_ty_err) =
        super#on_option self#on_hint env hfun.Aast.hf_variadic_ty
      in
      let (hf_ctxs, contexts_err) =
        super#on_option self#on_contexts env hfun.Aast.hf_ctxs
      in
      let (hf_return_ty, return_ty_err) =
        let hint_ctx = Hint_ctx.create ~allow_retonly:true () in
        self#on_hint Env.{ env with hint_ctx } hfun.Aast.hf_return_ty
      in
      let err =
        self#plus_all
          [return_ty_err; contexts_err; variadic_ty_err; param_tys_err]
      in
      let hfun =
        Aast.{ hfun with hf_param_tys; hf_variadic_ty; hf_ctxs; hf_return_ty }
      in
      (Aast.Hfun hfun, err)

    method! on_targ env targ =
      let hint_ctx =
        Hint_ctx.create
          ~allow_wildcard:true
          ~forbid_this:false
          ~allow_retonly:true
          ~tp_depth:1
          ()
      in
      super#on_targ Env.{ env with hint_ctx } targ

    method! on_where_constraint_hint env cstr =
      let hint_ctx = Hint_ctx.create ~in_where_clause:true () in
      super#on_where_constraint_hint Env.{ env with hint_ctx } cstr

    method! on_contexts env (pos, hints) =
      let hint_ctx = Hint_ctx.create ~in_context:true () in
      let env = Env.{ env with hint_ctx } in
      let (hints, err) = super#on_list self#on_context env hints in
      ((pos, hints), err)

    method private on_context env hint =
      (* TODO[mjt] if a language element has a different hint context
         should we make a type alias? *)
      match hint with
      | (pos, Aast.Happly ((_, tycon_name), _))
        when String.equal tycon_name SN.Typehints.wildcard ->
        ( (pos, Aast.Herr),
          Err.naming @@ Naming_error.Invalid_wildcard_context pos )
      | _ -> self#on_hint env hint

    method! on_tparam env tp =
      (* TODO[mjt] do we want to maintain the HKT code? *)
      let env = Env.extend_tparams env tp.Aast.tp_parameters in
      let (tp_parameters, parameters_err) =
        super#on_list self#on_tparam env tp.Aast.tp_parameters
      in
      (* TODO[mjt] if a language element has a different hint context
         should we make a type alias? *)
      let (tp_constraints, constraints_err) =
        let forbid_this = env.Env.hint_ctx.Hint_ctx.forbid_this in
        let hint_ctx = Hint_ctx.create ~forbid_this () in
        super#on_list
          (self#on_snd self#on_hint)
          Env.{ env with hint_ctx }
          tp.Aast.tp_constraints
      in
      let (tp_user_attributes, user_attributes_err) =
        super#on_list super#on_user_attribute env tp.Aast.tp_user_attributes
      in
      let err =
        self#plus_all [parameters_err; constraints_err; user_attributes_err]
      in
      let tp =
        Aast.{ tp with tp_parameters; tp_constraints; tp_user_attributes }
      in
      (tp, err)

    (* -- Helpers ----------------------------------------------------------- *)
    method private on_fst f ctx (fst, snd) =
      let (fst, err) = f ctx fst in
      ((fst, snd), err)

    method private on_snd f ctx (fst, snd) =
      let (snd, err) = f ctx snd in
      ((fst, snd), err)

    method private plus_all errs =
      List.fold_right ~init:self#zero ~f:self#plus errs
  end

let elab_fun_def ?init ?(env = Env.empty) fd =
  let (fd, err) = visitor#on_fun_def env fd in
  let errors = Err.from_monoid ?init err in
  (fd, errors)

let elab_typedef ?init ?(env = Env.empty) td =
  let (td, err) = visitor#on_typedef env td in
  let errors = Err.from_monoid ?init err in
  (td, errors)

let elab_module_def ?init ?(env = Env.empty) md =
  let (md, err) = visitor#on_module_def env md in
  let errors = Err.from_monoid ?init err in
  (md, errors)

let elab_gconst ?init ?(env = Env.empty) gc =
  let (gc, err) = visitor#on_gconst env gc in
  let errors = Err.from_monoid ?init err in
  (gc, errors)

let elab_class ?init ?(env = Env.empty) cls =
  let (cls, err) = visitor#on_class_ env cls in
  let errors = Err.from_monoid ?init err in
  (cls, errors)

let elab_program ?init ?(env = Env.empty) prog =
  let (prog, err) = visitor#on_program env prog in
  let errors = Err.from_monoid ?init err in
  (prog, errors)
