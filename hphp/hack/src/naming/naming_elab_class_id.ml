(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  type t = bool

  let empty = false
end

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    method! on_class_ _ c =
      let env = true in
      let (c_tparams, tparams_err) =
        super#on_list self#on_tparam env c.Aast.c_tparams
      in

      let (c_extends, extends_err) =
        super#on_list self#on_hint env c.Aast.c_extends
      in

      let (c_uses, uses_err) = super#on_list self#on_hint env c.Aast.c_uses in

      let (c_xhp_attrs, xhp_attrs_err) =
        super#on_list super#on_xhp_attr env c.Aast.c_xhp_attrs
      in

      let (c_xhp_attr_uses, xhp_attr_uses_err) =
        super#on_list self#on_hint env c.Aast.c_xhp_attr_uses
      in

      let (c_reqs, reqs_err) =
        super#on_list (self#on_fst self#on_hint) env c.Aast.c_reqs
      in

      let (c_implements, implements_err) =
        super#on_list self#on_hint env c.Aast.c_implements
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
        super#on_list super#on_user_attribute false c.Aast.c_user_attributes
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

    (* The lowerer will give us CIexpr (Id  _ | Lvar _ ); here we:
       - convert CIexpr(_,_,Id _) to CIparent, CIself, CIstatic and CI.
       - convert CIexpr(_,_,Lvar $this) to CIexpr(_,_,This)

       If there is a CIexpr with anything other than an Lvar or This after this
       elaboration step, it is an error and will be raised in subsequent
       validation passes

       TODO[mjt] We're overriding `on_class` rather than `on_class_` since
       the legacy code mangles positions by using the inner `class_id_` position
       in the output `class_id` tuple. This looks to be erroneous.
    *)
    method! on_class_id in_class (_, _, class_id_) =
      match class_id_ with
      (* TODO[mjt] if we don't expect these from lowering should we refine the
         NAST repr? *)
      | Aast.(CIparent | CIself | CIstatic | CI _) ->
        failwith "Error in Ast_to_nast module for Class_get"
      | Aast.(CIexpr (_, expr_pos, Id (id_pos, cname))) ->
        if String.equal cname SN.Classes.cParent then
          if not in_class then
            ( ((), expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
              Err.typing @@ Typing_error.Primary.Parent_outside_class id_pos )
          else
            (((), expr_pos, Aast.CIparent), self#zero)
        else if String.equal cname SN.Classes.cSelf then
          if not in_class then
            ( ((), expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
              Err.typing @@ Typing_error.Primary.Self_outside_class id_pos )
          else
            (((), expr_pos, Aast.CIself), self#zero)
        else if String.equal cname SN.Classes.cStatic then
          if not in_class then
            ( ((), expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
              Err.typing @@ Typing_error.Primary.Static_outside_class id_pos )
          else
            (((), expr_pos, Aast.CIstatic), self#zero)
        else
          (((), expr_pos, Aast.CI (expr_pos, cname)), self#zero)
      | Aast.(CIexpr (_, expr_pos, Lvar (lid_pos, lid)))
        when String.equal (Local_id.to_string lid) SN.SpecialIdents.this ->
        (* TODO[mjt] why is `$this` valid outside a class? *)
        (Aast.((), expr_pos, CIexpr ((), lid_pos, This)), self#zero)
      | Aast.(CIexpr (_, expr_pos, _)) -> (((), expr_pos, class_id_), self#zero)

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
