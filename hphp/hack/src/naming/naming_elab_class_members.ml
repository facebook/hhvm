(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names
module Err = Naming_phase_error

module Env = struct
  type t = FileInfo.mode

  let empty = FileInfo.Mstrict
end

let exists_both p1 p2 xs =
  let rec aux (b1, b2) = function
    | _ when b1 && b2 -> (b1, b2)
    | [] -> (b1, b2)
    | next :: rest -> aux (b1 || p1 next, b2 || p2 next) rest
  in
  aux (false, false) xs

let xhp_attr_hint items =
  let is_int = function
    | (_, _, Aast.Int _) -> true
    | _ -> false
  and is_string = function
    | (_, _, Aast.(String _ | String2 _)) -> true
    | _ -> false
  in
  match exists_both is_int is_string items with
  | (true, false) -> Aast.(Hprim Tint)
  | (_, true) -> Aast.(Hprim Tstring)
  | _ -> Aast.Hmixed

let strip_like = function
  | Aast.Hlike (_, hint_) -> hint_
  | hint_ -> hint_

(* TODO[mjt] is there no other way of determining this? *)
let is_xhp cv_name =
  try String.(sub cv_name ~pos:0 ~len:1 = ":") with
  | Invalid_argument _ -> false

let visitor =
  object (self)
    inherit [_] Naming_visitors.mapreduce as super

    method! on_typedef _ t = super#on_typedef t.Aast.t_mode t

    method! on_gconst _ cst = super#on_gconst cst.Aast.cst_mode cst

    method! on_fun_def _ fd = super#on_fun_def fd.Aast.fd_mode fd

    method! on_module_def _ md = super#on_module_def md.Aast.md_mode md

    method! on_class_
        _
        (Aast.{ c_vars; c_xhp_attrs; c_methods; c_user_attributes; c_mode; _ }
        as c) =
      let env = c_mode in

      let (c, err) =
        let (c_vars, err) =
          let (static_props, props) = Aast.split_vars c_vars in
          let const_attr_opt =
            Naming_attributes.find SN.UserAttributes.uaConst c_user_attributes
          in
          let static_props = List.map ~f:(self#elab_class_prop env) static_props
          and props =
            List.map
              ~f:(self#elab_non_static_class_prop const_attr_opt env)
              props
          and (xhp_attrs, xhp_attrs_err) =
            super#on_list self#elab_xhp_attr env c_xhp_attrs
          in
          (static_props @ props @ xhp_attrs, xhp_attrs_err)
        in
        let c_methods =
          if Ast_defs.is_c_interface c.Aast.c_kind then
            List.map c_methods ~f:(fun m -> Aast.{ m with m_abstract = true })
          else
            c_methods
        in
        (Aast.{ c with c_methods; c_vars; c_xhp_attrs = [] }, err)
      in
      let (c, super_err) = super#on_class_ env c in
      (c, self#plus super_err err)

    method private elab_cv_expr env pos cv_expr =
      match cv_expr with
      | None when FileInfo.is_hhi env -> Some ((), pos, Err.invalid_expr_ pos)
      | cv_expr -> cv_expr

    method private elab_class_prop
        env (Aast.{ cv_expr; cv_id = (pos, cv_name); _ } as cv) =
      let cv_expr = self#elab_cv_expr env pos cv_expr in
      let cv_xhp_attr =
        if is_xhp cv_name then
          Some Aast.{ xai_like = None; xai_tag = None; xai_enum_values = [] }
        else
          None
      in
      Aast.{ cv with cv_expr; cv_xhp_attr }

    method private elab_non_static_class_prop
        const_attr_opt env (Aast.{ cv_user_attributes; _ } as cv) =
      let cv = self#elab_class_prop env cv in
      let cv_user_attributes =
        match const_attr_opt with
        | Some ua
          when not
               @@ Naming_attributes.mem
                    SN.UserAttributes.uaConst
                    cv_user_attributes ->
          ua :: cv_user_attributes
        | _ -> cv_user_attributes
      in
      Aast.{ cv with cv_user_attributes }

    method private elab_xhp_attr env (type_hint, cv, xhp_attr_tag_opt, enum_opt)
        =
      let is_required = Option.is_some xhp_attr_tag_opt
      and has_default =
        Option.value_map
          ~default:false
          ~f:(function
            | (_, _, Aast.Null) -> false
            | _ -> true)
          cv.Aast.cv_expr
      in
      let (xai_like, xai_enum_values) =
        match cv.Aast.cv_xhp_attr with
        | Some xai -> (xai.Aast.xai_like, xai.Aast.xai_enum_values)
        | None -> (None, [])
      in
      let hint_opt =
        Option.value_map
          enum_opt
          ~default:(Aast.hint_of_type_hint type_hint)
          ~f:(fun (pos, items) -> Some (pos, xhp_attr_hint items))
      in
      let (hint_opt, req_attr_err) =
        Option.value_map
          hint_opt
          ~default:(None, self#zero)
          ~f:(fun ((pos, hint_) as hint) ->
            match strip_like hint_ with
            | Aast.Hoption _ ->
              let err =
                if is_required then
                  Err.naming
                  @@ Naming_error.Xhp_optional_required_attr
                       { pos; attr_name = snd @@ cv.Aast.cv_id }
                else
                  self#zero
              in
              (Some hint, err)
            | Aast.Hmixed -> (Some hint, self#zero)
            | _ when is_required || has_default -> (Some hint, self#zero)
            | _ -> (Some (pos, Aast.Hoption hint), self#zero))
      in

      let (hint_opt, like_err) =
        Option.value ~default:(hint_opt, self#zero)
        @@ Option.map2 hint_opt xai_like ~f:(fun hint pos ->
               (Some (pos, Aast.Hlike hint), Err.like_type pos))
      in
      let cv_type = ((), hint_opt)
      and cv_xhp_attr =
        Some Aast.{ xai_like; xai_tag = xhp_attr_tag_opt; xai_enum_values }
      and cv_expr = self#elab_cv_expr env (fst cv.Aast.cv_id) cv.Aast.cv_expr in
      let err = self#plus req_attr_err like_err in
      ( Aast.{ cv with cv_xhp_attr; cv_type; cv_expr; cv_user_attributes = [] },
        err )
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
