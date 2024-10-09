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

let elab_class_prop (Aast.{ cv_id = (_, cv_name); _ } as cv) =
  let cv_xhp_attr =
    if is_xhp cv_name then
      Some Aast.{ xai_like = None; xai_tag = None; xai_enum_values = [] }
    else
      None
  in
  Aast.{ cv with cv_xhp_attr }

let elab_non_static_class_prop
    const_attr_opt (Aast.{ cv_user_attributes; _ } as cv) =
  let cv = elab_class_prop cv in
  let cv_user_attributes =
    match const_attr_opt with
    | Some ua
      when not
           @@ Naming_attributes.mem SN.UserAttributes.uaConst cv_user_attributes
      ->
      ua :: cv_user_attributes
    | _ -> cv_user_attributes
  in
  Aast.{ cv with cv_user_attributes }

let elab_xhp_attr (type_hint, cv, xhp_attr_tag_opt, enum_opt) =
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
      ~default:(None, None)
      ~f:(fun ((pos, hint_) as hint) ->
        match strip_like hint_ with
        | Aast.Hoption _ ->
          let err =
            if is_required then
              Some
                (Naming_phase_error.naming
                @@ Naming_error.Xhp_optional_required_attr
                     { pos; attr_name = snd @@ cv.Aast.cv_id })
            else
              None
          in
          (Some hint, err)
        | Aast.Hmixed -> (Some hint, None)
        | _ when is_required || has_default -> (Some hint, None)
        | _ -> (Some (pos, Aast.Hoption hint), None))
  in

  let hint_opt =
    Option.value ~default:hint_opt
    @@ Option.map2 hint_opt xai_like ~f:(fun hint pos ->
           Some (pos, Aast.Hlike hint))
  in
  let cv_type = (fst type_hint, hint_opt)
  and cv_xhp_attr =
    Some Aast.{ xai_like; xai_tag = xhp_attr_tag_opt; xai_enum_values }
  in
  let errs = List.filter_map ~f:Fn.id [req_attr_err] in
  (Aast.{ cv with cv_xhp_attr; cv_type; cv_user_attributes = [] }, errs)

let on_class_
    on_error
    (Aast.{ c_vars; c_xhp_attrs; c_methods; c_user_attributes; _ } as c)
    ~ctx =
  let (c, errs) =
    let (c_vars, err) =
      let (static_props, props) = Aast.split_vars c_vars in
      let const_attr_opt =
        Naming_attributes.find SN.UserAttributes.uaConst c_user_attributes
      in
      let static_props = List.map ~f:elab_class_prop static_props
      and props = List.map ~f:(elab_non_static_class_prop const_attr_opt) props
      and (xhp_attrs, xhp_attrs_err) =
        List.unzip @@ List.map ~f:elab_xhp_attr c_xhp_attrs
      in
      (static_props @ props @ xhp_attrs, List.concat xhp_attrs_err)
    in
    let c_methods =
      if Ast_defs.is_c_interface c.Aast.c_kind then
        List.map c_methods ~f:(fun m -> Aast.{ m with m_abstract = true })
      else
        c_methods
    in
    (Aast.{ c with c_methods; c_vars; c_xhp_attrs = [] }, err)
  in
  List.iter ~f:on_error errs;
  (ctx, Ok c)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up
    Aast.Pass.
      {
        id with
        on_ty_class_ = Some (fun elem ~ctx -> on_class_ on_error elem ~ctx);
      }
