(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Ppx_yojson_conv_lib.Yojson_conv.Primitives
module Reason = Typing_reason

(* This isn't a special name; I just need something that oxidize won't copy *)
module Internal = struct
  type t_info = {
    ty: Typing_defs.locl_ty;
    tast_env: Tast_env.env;
    info_position_kind: string;
    info_position: string;
  }
end

type t_ = {
  ty_json: string;
  when_called_dynamically: string option;
  position_kind: string;
  position: string;
}
[@@deriving yojson_of]

type t = t_ SMap.t [@@deriving yojson_of]

let is_enabled tcopt =
  TypecheckerOptions.log_levels tcopt
  |> SMap.find_opt "truthiness_collector"
  |> Option.map ~f:(fun level -> level >= 1)
  |> Option.value ~default:false

let get_types
    ctx path (tast_normal : Tast.def list) (tast_dynamic : Tast.def option list)
    : t =
  let tast_dynamic = List.filter_map ~f:Fun.id tast_dynamic in
  let visitor =
    let for_cond env cond kind : Internal.t_info list =
      let (ty, p, _e) = cond in
      [
        {
          ty;
          tast_env = env;
          info_position_kind = kind;
          info_position =
            (let (line, start_col, end_col) = Pos.info_pos p in
             Printf.sprintf
               "%s:%d:%d:%d"
               (Relative_path.to_absolute path)
               line
               start_col
               end_col);
        };
      ]
    in
    object
      inherit [Internal.t_info list] Tast_visitor.reduce

      method zero = []

      method plus = ( @ )

      method! on_If env cond _then_block _else_block = for_cond env cond "If"

      method! on_While env cond _block = for_cond env cond "While"

      method! on_Do env _block cond = for_cond env cond "Do"

      method! on_For env _init cond _incrs _block =
        match cond with
        | Some cond -> for_cond env cond "For"
        | None -> []

      method! on_Binop env { bop; lhs; rhs } =
        match bop with
        | Ampamp -> for_cond env lhs "&&" @ for_cond env rhs "&&"
        | Barbar -> for_cond env lhs "||" @ for_cond env rhs "||"
        | _ -> []

      method! on_Unop env uop expr =
        match uop with
        | Ast_defs.Unot -> for_cond env expr "!"
        | _ -> []

      method! on_Eif env cond _consq _alt = for_cond env cond "Ternary"
    end
  in
  let get_map tast : Internal.t_info SMap.t =
    visitor#go ctx tast
    |> List.map ~f:(fun (t_info : Internal.t_info) ->
           (t_info.info_position, t_info))
    |> SMap.of_list
  in
  let normal = get_map tast_normal in
  let dynamic = get_map tast_dynamic in
  let is_sub_like_bool env ty =
    Tast_env.is_sub_type
      env
      ty
      (Typing_make_type.union
         Reason.none
         [
           Typing_make_type.dynamic Reason.none;
           Typing_make_type.bool Reason.none;
         ])
  in
  SMap.merge
    (fun key
         (normal : Internal.t_info option)
         (dynamic : Internal.t_info option) : t_ option ->
      match (normal, dynamic) with
      | (Some normal_info, None) ->
        if is_sub_like_bool normal_info.tast_env normal_info.ty then
          None
        else
          Some
            {
              ty_json =
                Hh_json.json_to_string
                @@ Tast_env.ty_to_json
                     normal_info.tast_env
                     ~show_like_ty:true
                     normal_info.ty;
              when_called_dynamically = None;
              position_kind = normal_info.info_position_kind;
              position = normal_info.info_position;
            }
      | (None, Some { ty; tast_env; _ }) ->
        failwith
          ("Got type `"
          ^ Tast_env.print_ty tast_env ty
          ^ "` only under dynamic assumptions for "
          ^ key)
      | (None, None) -> None
      | (Some normal_info, Some dynamic_info) ->
        if
          is_sub_like_bool normal_info.tast_env normal_info.ty
          && is_sub_like_bool dynamic_info.tast_env dynamic_info.ty
        then
          None
        else
          let when_called_dynamically =
            if
              Tast_env.is_sub_type
                dynamic_info.tast_env
                dynamic_info.ty
                normal_info.ty
            then
              None
            else
              Some
                (Hh_json.json_to_string
                @@ Tast_env.ty_to_json
                     dynamic_info.tast_env
                     ~show_like_ty:true
                     dynamic_info.ty)
          in
          Some
            {
              ty_json =
                Hh_json.json_to_string
                @@ Tast_env.ty_to_json
                     normal_info.tast_env
                     ~show_like_ty:true
                     normal_info.ty;
              when_called_dynamically;
              position_kind = normal_info.info_position_kind;
              position = normal_info.info_position;
            })
    normal
    dynamic

let reduce =
  SMap.merge (fun key a_opt b_opt ->
      match (a_opt, b_opt) with
      | (Some v, None)
      | (None, Some v) ->
        Some v
      | (None, None) -> None
      | (Some _, Some _) -> failwith ("Unexpected merge conflict for " ^ key))

let map
    (ctx : Provider_context.t)
    (path : Relative_path.t)
    (tasts : Tast.by_names)
    _errors : t =
  let tasts = Tast.tasts_as_list tasts in
  let tasts_normal =
    List.map ~f:(fun t -> t.Tast_with_dynamic.under_normal_assumptions) tasts
  in
  let tasts_dynamic =
    List.map ~f:(fun t -> t.Tast_with_dynamic.under_dynamic_assumptions) tasts
  in
  get_types ctx path tasts_normal tasts_dynamic

let finalize ~progress:_ ~init_id:_ ~recheck_id:_ _counts = ()
