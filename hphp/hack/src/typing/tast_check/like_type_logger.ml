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

let supportdyn_of_mixed = Typing_make_type.supportdyn_mixed Typing_reason.Rnone

let is_exactly env ty ty' =
  Tast_env.is_sub_type env ty ty' && Tast_env.is_sub_type env ty' ty

let is_like_type env ty =
  let is_sub_type = Tast_env.is_sub_type env in
  is_sub_type dynamic ty
  && not (is_sub_type mixed ty || is_sub_type supportdyn_of_mixed ty)

let should_log env = not @@ Tast_env.is_hhi env

let like_type_log_level_of env =
  Tast_env.get_tcopt env
  |> TypecheckerOptions.log_levels
  |> SMap.find_opt "like_type"
  |> Option.value ~default:0

let locl_ty_of_hint (ty, _) = ty

module Counter : sig
  type t

  val zero : t

  val is_zero : t -> bool

  val plus : t -> t -> t

  val unit : Tast_env.t -> Typing_defs.locl_ty -> t

  val json_of : t -> JSON.json
end = struct
  module Key = struct
    type t =
      | Like
      | NonLike
      | Mixed
      | SupportdynOfMixed
      | Dynamic
    [@@deriving show { with_path = false }, ord]
  end

  open WrappedMap.Make (Key)

  type nonrec t = int t

  let zero = empty

  let is_zero = is_empty

  let plus =
    merge (fun _ c_opt c_opt' ->
        match (c_opt, c_opt') with
        | (Some c, Some c') -> Some (c + c')
        | (None, Some c)
        | (Some c, None) ->
          Some c
        | (None, None) -> None)

  let inc key =
    update key @@ function
    | None -> Some 1
    | Some c -> Some (c + 1)

  let unit env ty =
    empty
    |> begin
         if is_like_type env ty then
           inc Key.Like
         else
           inc Key.NonLike
       end
    |> begin
         if is_exactly env ty mixed then
           inc Key.Mixed
         else
           Fn.id
       end
    |> begin
         if is_exactly env ty supportdyn_of_mixed then
           inc Key.SupportdynOfMixed
         else
           Fn.id
       end
    |> begin
         if is_exactly env ty dynamic then
           inc Key.Dynamic
         else
           Fn.id
       end

  let json_of ctr =
    JSON.JSON_Object
      (bindings ctr |> List.map ~f:(fun (k, v) -> (Key.show k, JSON.int_ v)))
end

module Categories : sig
  module Key : sig
    type t =
      | Expression
      | Obj_get_receiver
      | Class_get_receiver
      | Class_const_receiver
      | Property
      | Parameter
      | Return
  end

  type t

  val zero : t

  val plus : t -> t -> t

  val singleton : Key.t -> Counter.t -> t

  val json_of : t -> JSON.json
end = struct
  module Key = struct
    type t =
      | Expression
      | Obj_get_receiver
      | Class_get_receiver
      | Class_const_receiver
      | Property
      | Parameter
      | Return
    [@@deriving show { with_path = false }, ord]
  end

  open WrappedMap.Make (Key)

  type nonrec t = Counter.t t

  let zero = empty

  let plus =
    merge (fun _ c_opt c_opt' ->
        match (c_opt, c_opt') with
        | (Some c, Some c') -> Some (Counter.plus c c')
        | (None, Some c)
        | (Some c, None) ->
          Some c
        | (None, None) -> None)

  let singleton k c =
    if Counter.is_zero c then
      empty
    else
      singleton k c

  let json_of ctr =
    JSON.JSON_Object
      (bindings ctr
      |> List.map ~f:(fun (k, v) -> (Key.show k, Counter.json_of v)))
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
    |> Categories.singleton Categories.Key.Parameter
  in
  let return =
    let ty = locl_ty_of_hint ret in
    Counter.unit env ty |> Categories.singleton Categories.Key.Return
  in
  Categories.(plus parameter return)

let partition_types =
  object
    inherit [_] Tast_visitor.reduce as super

    method zero = Categories.zero

    method plus = Categories.plus

    method! on_expr env ((ty, _, e_) as e) =
      let open Categories in
      Counter.unit env ty
      |> Categories.singleton Categories.Key.Expression
      |> plus
           begin
             match e_ with
             | Aast.Obj_get ((receiver_ty, _, _), _, _, _) ->
               Counter.unit env receiver_ty |> singleton Key.Obj_get_receiver
             | Aast.Class_get ((receiver_ty, _, _), _, _) ->
               Counter.unit env receiver_ty |> singleton Key.Class_get_receiver
             | Aast.Class_const ((receiver_ty, _, _), _) ->
               Counter.unit env receiver_ty
               |> singleton Key.Class_const_receiver
             | _ -> zero
           end
      |> plus (super#on_expr env e)

    method! on_method_ env m =
      let declaration =
        callable_decl_counter env m.Aast.m_params m.Aast.m_ret
      in
      let expression = super#on_method_ env m in
      Categories.plus declaration expression

    method! on_fun_ env f =
      let declaration =
        callable_decl_counter env f.Aast.f_params f.Aast.f_ret
      in
      let expression = super#on_fun_ env f in
      Categories.plus declaration expression

    method! on_class_ env c =
      let property =
        List.fold c.Aast.c_vars ~init:Counter.zero ~f:(fun acc prop ->
            let ty = locl_ty_of_hint prop.Aast.cv_type in
            Counter.plus acc (Counter.unit env ty))
        |> Categories.singleton Categories.Key.Property
      in
      let method_ = super#on_class_ env c in
      Categories.plus property method_
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

let create_handler _ctx =
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
