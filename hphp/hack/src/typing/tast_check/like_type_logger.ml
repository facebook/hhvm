(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module JSON = Hh_json

let dynamic = Typing_make_type.dynamic Typing_reason.Rnone

let mixed = Typing_make_type.mixed Typing_reason.Rnone

let support_dyn_of_mixed = Typing_make_type.supportdyn_mixed Typing_reason.Rnone

let is_like_type env ty =
  let is_sub_type = Tast_env.is_sub_type env in
  is_sub_type dynamic ty
  && not (is_sub_type mixed ty || is_sub_type support_dyn_of_mixed ty)

let should_log env = not @@ Tast_env.is_hhi env

let like_type_log_level_of env =
  Tast_env.get_tcopt env
  |> TypecheckerOptions.log_levels
  |> SMap.find_opt "like_type"
  |> Option.value ~default:0

let locl_ty_of_hint (ty, _) = ty

module Counter = struct
  type t = {
    like: int;
    non_like: int;
  }

  let zero = { like = 0; non_like = 0 }

  let plus r1 r2 =
    { like = r1.like + r2.like; non_like = r1.non_like + r2.non_like }

  let unit env ty =
    if is_like_type env ty then
      { like = 1; non_like = 0 }
    else
      { like = 0; non_like = 1 }

  let json_of ctr =
    JSON.JSON_Object
      [("like", JSON.int_ ctr.like); ("non_like", JSON.int_ ctr.non_like)]
end

module Categories = struct
  type t = {
    expression: Counter.t;
    property: Counter.t;
    parameter: Counter.t;
    return: Counter.t;
  }

  let zero =
    {
      expression = Counter.zero;
      parameter = Counter.zero;
      property = Counter.zero;
      return = Counter.zero;
    }

  let plus c1 c2 =
    {
      expression = Counter.plus c1.expression c2.expression;
      parameter = Counter.plus c1.parameter c2.parameter;
      return = Counter.plus c1.return c2.return;
      property = Counter.plus c1.property c2.property;
    }

  let json_of c =
    JSON.JSON_Object
      [
        ("expression", Counter.json_of c.expression);
        ("parameter", Counter.json_of c.parameter);
        ("return", Counter.json_of c.return);
        ("property", Counter.json_of c.property);
      ]
end

module Log = struct
  type t = {
    pos: Pos.t;
    categories: Categories.t;
  }

  let json_of l =
    let pos_str =
      let pos = l.pos in
      let filename = Pos.to_relative_string pos |> Pos.filename in
      let line = Pos.line pos in
      Format.sprintf "%s:%d" filename line
    in
    JSON.JSON_Object
      [
        ("pos", JSON.string_ pos_str);
        ("categories", Categories.json_of l.categories);
      ]

  let to_string ?(pretty = false) l =
    Format.sprintf
      "[Like_type_logger] %s"
      (json_of l |> JSON.json_to_string ~pretty)
end

let callable_decl_counter env params ret =
  let parameter =
    List.fold params ~init:Counter.zero ~f:(fun acc param ->
        let ty = locl_ty_of_hint param.Aast.param_type_hint in
        Counter.plus acc (Counter.unit env ty))
  in
  let return =
    let ty = locl_ty_of_hint ret in
    Counter.unit env ty
  in
  Categories.{ zero with return; parameter }

let partition_types =
  object (self)
    inherit [_] Tast_visitor.reduce as super

    method zero = Categories.zero

    method plus = Categories.plus

    method! on_expr env (ty, _, _) =
      Categories.{ (self#zero) with expression = Counter.unit env ty }

    method! on_method_ env m =
      let declaration =
        callable_decl_counter env m.Aast.m_params m.Aast.m_ret
      in
      let expression = super#on_method_ env m in
      self#plus declaration expression

    method! on_fun_ env f =
      let declaration =
        callable_decl_counter env f.Aast.f_params f.Aast.f_ret
      in
      let expression = super#on_fun_ env f in
      self#plus declaration expression

    method! on_class_ env c =
      let property =
        let property =
          List.fold c.Aast.c_vars ~init:Counter.zero ~f:(fun acc prop ->
              let ty = locl_ty_of_hint prop.Aast.cv_type in
              Counter.plus acc (Counter.unit env ty))
        in
        Categories.{ (self#zero) with property }
      in
      let method_ = super#on_class_ env c in
      self#plus property method_
  end

let log_type_partition env log =
  if like_type_log_level_of env > 1 then
    Hh_logger.log "%s" (Log.to_string log)
  else begin
    (* We don't use the logger when the log level is 1 so that we can write
       .exp style tests *)
    Format.sprintf "%s\n" (Log.to_string ~pretty:true log)
    |> Out_channel.output_string !Typing_log.out_channel;
    Out_channel.flush !Typing_log.out_channel
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_ env f =
      if should_log env then
        let categories = partition_types#on_fun_ env f in
        log_type_partition env Log.{ pos = f.Aast.f_span; categories }

    method! at_class_ env c =
      if should_log env then
        let categories = partition_types#on_class_ env c in
        log_type_partition env Log.{ pos = c.Aast.c_span; categories }
  end
