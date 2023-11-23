(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Reason = Typing_reason
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
  | Tany
[@@deriving ord, yojson_of]

type category =
  | Expression
  | Property
  | Parameter
  | Return
[@@deriving ord, yojson_of]

let dynamic = Typing_make_type.dynamic Typing_reason.Rnone

let mixed = Typing_make_type.mixed Typing_reason.Rnone

let supportdyn_of_mixed = Typing_make_type.supportdyn_mixed Typing_reason.Rnone

let is_like_type env ty =
  let is_sub_type = Tast_env.is_sub_type env in
  is_sub_type dynamic ty
  && not (is_sub_type mixed ty || is_sub_type supportdyn_of_mixed ty)

let is_tany = Typing_defs.is_any

let locl_ty_of_hint (ty, _) = ty

module Collection : sig
  type t

  val zero : t

  val is_zero : t -> bool

  val plus : t -> t -> t

  val unit : Tast_env.t -> Typing_defs.locl_ty -> t

  val unpack : t -> (logged_type * Reason.t) list
end = struct
  module Key = struct
    type t = logged_type [@@deriving ord]
  end

  open WrappedMap.Make (Key)

  type nonrec t = Reason.t list t

  let zero = empty

  let is_zero = is_empty

  let plus =
    merge (fun _ c_opt c_opt' ->
        match (c_opt, c_opt') with
        | (Some c, Some c') -> Some (c @ c')
        | (None, Some c)
        | (Some c, None) ->
          Some c
        | (None, None) -> None)

  let add key r =
    update key @@ function
    | None -> Some [r]
    | Some c -> Some (r :: c)

  let unit env ty =
    let r = Typing_defs.get_reason ty in
    empty
    |> begin
         if is_like_type env ty then
           add Like r
         else
           Fn.id
       end
    |> begin
         if is_tany ty then
           add Tany r
         else
           Fn.id
       end

  let unpack : t -> (logged_type * Reason.t) list =
   fun x ->
    elements x
    |> List.map ~f:(fun (lt, rs) -> List.map rs ~f:(fun r -> (lt, r)))
    |> List.concat
end

module Categories : sig
  module Key : sig
    type t = category
  end

  type t

  val zero : t

  val plus : t -> t -> t

  val singleton : Key.t -> Collection.t -> t

  val unpack : t -> (category * logged_type * Reason.t) list
end = struct
  module Key = struct
    type t = category [@@deriving ord]
  end

  open WrappedMap.Make (Key)

  type nonrec t = Collection.t t

  let zero = empty

  let plus =
    merge (fun _ c_opt c_opt' ->
        match (c_opt, c_opt') with
        | (Some c, Some c') -> Some (Collection.plus c c')
        | (None, Some c)
        | (Some c, None) ->
          Some c
        | (None, None) -> None)

  let singleton k c =
    if Collection.is_zero c then
      empty
    else
      singleton k c

  let unpack : t -> (category * logged_type * Reason.t) list =
   fun x ->
    map Collection.unpack x
    |> elements
    |> List.map ~f:(fun (cg, cnts) ->
           List.map cnts ~f:(fun (t, r) -> (cg, t, r)))
    |> List.concat
end

let callable_decl_counter env params ret =
  let parameter =
    List.fold params ~init:Collection.zero ~f:(fun acc param ->
        let ty = locl_ty_of_hint param.Aast.param_type_hint in
        Collection.plus acc (Collection.unit env ty))
    |> Categories.singleton Parameter
  in
  let return =
    let ty = locl_ty_of_hint ret in
    Collection.unit env ty |> Categories.singleton Return
  in
  Categories.(plus parameter return)

let partition_types =
  object
    inherit [_] Tast_visitor.reduce as super

    method zero = Categories.zero

    method plus = Categories.plus

    method! on_expr env ((ty, _, _e_) as e) =
      let open Categories in
      Collection.unit env ty
      |> Categories.singleton Expression
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
        List.fold c.Aast.c_vars ~init:Collection.zero ~f:(fun acc prop ->
            let ty = locl_ty_of_hint prop.Aast.cv_type in
            Collection.plus acc (Collection.unit env ty))
        |> Categories.singleton Property
      in
      let method_ = super#on_class_ env c in
      Categories.plus property method_
  end

type collected_reason = {
  entity_pos: entity_pos;
      (** The position of the entity for which this count holds *)
  counted_type: logged_type;  (** The type that this count is for *)
  category: category;  (** Program construct that produces this type *)
  reason_constructor: string;
      (** The constructor for the reason. We can't encode the actual
          reason, because it got lazy things in it and that's not serializable. *)
  reason_pos: pos;  (** Precomputed constructor string *)
}
[@@deriving yojson_of]

let collect_reasons ctx program =
  let reducer =
    object
      inherit [collected_reason list] Tast_visitor.reduce

      method zero = []

      method plus = ( @ )

      method! on_fun_def env f =
        let categories = partition_types#on_fun_def env f in
        let data = Categories.unpack categories in
        List.map data ~f:(fun (category, counted_type, reason) ->
            {
              entity_pos = (fst f.Aast.fd_name, Function (snd f.Aast.fd_name));
              counted_type;
              category;
              reason_constructor = Reason.to_constructor_string reason;
              reason_pos = Reason.to_pos reason |> Pos_or_decl.unsafe_to_raw_pos;
            })

      method! on_class_ env c =
        let categories = partition_types#on_class_ env c in
        let data = Categories.unpack categories in
        List.map data ~f:(fun (category, counted_type, reason) ->
            {
              entity_pos = (fst c.Aast.c_name, Class (snd c.Aast.c_name));
              counted_type;
              category;
              reason_constructor = Reason.to_constructor_string reason;
              reason_pos = Reason.to_pos reason |> Pos_or_decl.unsafe_to_raw_pos;
            })
    end
  in
  reducer#go ctx program

type t = collected_reason list Relative_path.Map.t [@@deriving yojson_of]

let of_collected_reasons (cnts : collected_reason list) =
  List.fold cnts ~init:Relative_path.Map.empty ~f:(fun m c ->
      let fn = Pos.filename (fst c.entity_pos) in
      Relative_path.Map.update
        fn
        (fun x -> Some (c :: Option.value x ~default:[]))
        m)

let is_enabled tcopt =
  TypecheckerOptions.log_levels tcopt
  |> SMap.find_opt "reason_collector"
  |> Option.map ~f:(fun level -> level = 1)
  |> Option.value ~default:false

let map
    (ctx : Provider_context.t)
    (_path : Relative_path.t)
    (tasts : Tast.by_names)
    _errors : t =
  Tast.tasts_as_list tasts
  |> List.map ~f:(fun t -> t.Tast_with_dynamic.under_normal_assumptions)
  |> collect_reasons ctx
  |> of_collected_reasons

let reduce = Relative_path.Map.union ~combine:(fun _key x y -> Some (x @ y))

let finalize ~progress:_ ~init_id:_ ~recheck_id:_ _counts = ()
