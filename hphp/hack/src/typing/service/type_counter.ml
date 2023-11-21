(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* TODO(T170647909): In preparation to upgrading to ppx_yojson_conv.v0.16.X.
         Remove the suppress warning when the upgrade is done. *)
[@@@warning "-66"]

open Hh_prelude
open Ppx_yojson_conv_lib.Yojson_conv.Primitives

type entity =
  | Class of string
  | Function of string
[@@deriving ord, yojson_of]

type pos = Pos.t [@@deriving ord]

let yojson_of_pos pos =
  let filename = Pos.to_relative_string pos |> Pos.filename in
  let line = Pos.line pos in
  yojson_of_string @@ Format.sprintf "%s:%d" filename line

type entity_pos = pos * entity [@@deriving ord, yojson_of]

type logged_type =
  | Like
  | NonLike
  | Mixed
  | SupportdynOfMixed
  | Dynamic
  | Tany
[@@deriving ord, yojson_of]

type category =
  | Expression
  | Obj_get_receiver
  | Class_get_receiver
  | Class_const_receiver
  | Property
  | Parameter
  | Return
[@@deriving ord, yojson_of]

let dynamic = Typing_make_type.dynamic Typing_reason.Rnone

let mixed = Typing_make_type.mixed Typing_reason.Rnone

let supportdyn_of_mixed = Typing_make_type.supportdyn_mixed Typing_reason.Rnone

let is_exactly env ty ty' =
  Tast_env.is_sub_type env ty ty' && Tast_env.is_sub_type env ty' ty

let is_like_type env ty =
  let is_sub_type = Tast_env.is_sub_type env in
  is_sub_type dynamic ty
  && not (is_sub_type mixed ty || is_sub_type supportdyn_of_mixed ty)

let is_tany = Typing_defs.is_any

let locl_ty_of_hint (ty, _) = ty

module Counter : sig
  type t

  val zero : t

  val is_zero : t -> bool

  val plus : t -> t -> t

  val unit : Tast_env.t -> Typing_defs.locl_ty -> t

  val unpack : t -> (logged_type * int) list
end = struct
  module Key = struct
    type t = logged_type [@@deriving ord]
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
           inc Like
         else
           inc NonLike
       end
    |> begin
         if is_exactly env ty mixed then
           inc Mixed
         else
           Fn.id
       end
    |> begin
         if is_exactly env ty supportdyn_of_mixed then
           inc SupportdynOfMixed
         else
           Fn.id
       end
    |> begin
         if is_exactly env ty dynamic then
           inc Dynamic
         else
           Fn.id
       end
    |> begin
         if is_tany ty then
           inc Tany
         else
           Fn.id
       end

  let unpack = elements
end

module Categories : sig
  module Key : sig
    type t = category
  end

  type t

  val zero : t

  val plus : t -> t -> t

  val singleton : Key.t -> Counter.t -> t

  val unpack : t -> (category * logged_type * int) list
end = struct
  module Key = struct
    type t = category [@@deriving ord]
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

  let unpack : t -> (category * logged_type * int) list =
   fun x ->
    map Counter.unpack x
    |> elements
    |> List.map ~f:(fun (cg, cnts) ->
           List.map cnts ~f:(fun (t, c) -> (cg, t, c)))
    |> List.concat
end

let callable_decl_counter env params ret =
  let parameter =
    List.fold params ~init:Counter.zero ~f:(fun acc param ->
        let ty = locl_ty_of_hint param.Aast.param_type_hint in
        Counter.plus acc (Counter.unit env ty))
    |> Categories.singleton Parameter
  in
  let return =
    let ty = locl_ty_of_hint ret in
    Counter.unit env ty |> Categories.singleton Return
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
      |> Categories.singleton Expression
      |> plus
           begin
             match e_ with
             | Aast.Obj_get ((receiver_ty, _, _), _, _, _) ->
               Counter.unit env receiver_ty |> singleton Obj_get_receiver
             | Aast.Class_get ((receiver_ty, _, _), _, _) ->
               Counter.unit env receiver_ty |> singleton Class_get_receiver
             | Aast.Class_const ((receiver_ty, _, _), _) ->
               Counter.unit env receiver_ty |> singleton Class_const_receiver
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
        |> Categories.singleton Property
      in
      let method_ = super#on_class_ env c in
      Categories.plus property method_
  end

type count = {
  entity_pos: entity_pos;
      (** The position of the entity for which this count holds *)
  counted_type: logged_type;  (** The type that this count is for *)
  category: category;  (** Program construct that produces this type *)
  value: int;  (** The actual count *)
}
[@@deriving yojson_of]

let count ctx program =
  let reducer =
    object
      inherit [count list] Tast_visitor.reduce

      method zero = []

      method plus = ( @ )

      method! on_fun_def env f =
        let categories = partition_types#on_fun_def env f in
        let data = Categories.unpack categories in
        List.map data ~f:(fun (category, counted_type, value) ->
            {
              entity_pos = (fst f.Aast.fd_name, Function (snd f.Aast.fd_name));
              counted_type;
              category;
              value;
            })

      method! on_class_ env c =
        let categories = partition_types#on_class_ env c in
        let data = Categories.unpack categories in
        List.map data ~f:(fun (category, counted_type, value) ->
            {
              entity_pos = (fst c.Aast.c_name, Class (snd c.Aast.c_name));
              counted_type;
              category;
              value;
            })
    end
  in
  reducer#go ctx program

type summary = {
  num_like_types: int;
  num_non_like_types: int;
  num_mixed: int;
  num_supportdyn_of_mixed: int;
  num_dynamic: int;
  num_tany: int;
}
[@@deriving yojson_of]

type t = summary Relative_path.Map.t [@@deriving yojson_of]

let empty_summary =
  {
    num_like_types = 0;
    num_non_like_types = 0;
    num_mixed = 0;
    num_supportdyn_of_mixed = 0;
    num_dynamic = 0;
    num_tany = 0;
  }

let summary_of_count (cnt : count) =
  let { counted_type; value; entity_pos = _; category = _ } = cnt in
  match counted_type with
  | Like -> { empty_summary with num_like_types = value }
  | NonLike -> { empty_summary with num_non_like_types = value }
  | Mixed -> { empty_summary with num_mixed = value }
  | SupportdynOfMixed -> { empty_summary with num_supportdyn_of_mixed = value }
  | Dynamic -> { empty_summary with num_dynamic = value }
  | Tany -> { empty_summary with num_tany = value }

let plus_summary s t =
  let {
    num_like_types;
    num_non_like_types;
    num_mixed;
    num_supportdyn_of_mixed;
    num_dynamic;
    num_tany;
  } =
    s
  in
  {
    num_like_types = num_like_types + t.num_like_types;
    num_non_like_types = num_non_like_types + t.num_non_like_types;
    num_mixed = num_mixed + t.num_mixed;
    num_supportdyn_of_mixed =
      num_supportdyn_of_mixed + t.num_supportdyn_of_mixed;
    num_dynamic = num_dynamic + t.num_dynamic;
    num_tany = num_tany + t.num_tany;
  }

let summary_of_counts (cnts : count list) =
  List.fold cnts ~init:Relative_path.Map.empty ~f:(fun m c ->
      let fn = Pos.filename (fst c.entity_pos) in
      let c = summary_of_count c in
      Relative_path.Map.update
        fn
        (fun x -> Some (plus_summary (Option.value x ~default:empty_summary) c))
        m)

let is_enabled tcopt =
  TypecheckerOptions.log_levels tcopt
  |> SMap.find_opt "type_counter"
  |> Option.map ~f:(fun level -> level = 1)
  |> Option.value ~default:false

let map
    (ctx : Provider_context.t)
    (_path : Relative_path.t)
    (tasts : Tast.by_names)
    _errors : t =
  Tast.tasts_as_list tasts
  |> List.map ~f:(fun t -> t.Tast_with_dynamic.under_normal_assumptions)
  |> count ctx
  |> summary_of_counts

let reduce =
  Relative_path.Map.union ~combine:(fun _key x y -> Some (plus_summary x y))

let finalize ~progress:_ ~init_id:_ ~recheck_id:_ _counts = ()
