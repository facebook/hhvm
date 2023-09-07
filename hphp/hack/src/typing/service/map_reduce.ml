(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module type MapReducer = sig
  type t

  val is_enabled : TypecheckerOptions.t -> bool

  val map : Provider_context.t -> Relative_path.t -> Tast.by_names -> t

  val reduce : t -> t -> t

  val finalize :
    progress:(string -> unit) ->
    init_id:string ->
    recheck_id:string option ->
    t ->
    unit
end

[@@@warning "-32"] (* for ppxs *)

(* An enumeration of all supported map reducers.
   If you want to add a map-reducer, add a variant here and let the compiler
   guide you. *)
type map_reducer =
  | TastHashes
  | TypeCounter
  | ReasonCollector
[@@deriving eq, ord, show, enum]

[@@@warning "+32"]

(** Link each map-reducer to the type of its data. *)
type _ map_reducer_type =
  | TypeForTastHashes : Tast_hashes.t map_reducer_type
  | TypeForTypeCounter : Type_counter.t map_reducer_type
  | TypeForReasonCollector : Reason_collector.t map_reducer_type

(** Existential wrapper around the type for a map-reducer. *)
type any_map_reducer_type =
  | TypeForAny : 'a map_reducer_type -> any_map_reducer_type

(** A mapping from a supported map-reducer to a witness of its type.

Needs an entry when adding a map-reducer. *)
let type_for : map_reducer -> any_map_reducer_type = function
  | TastHashes -> TypeForAny TypeForTastHashes
  | TypeCounter -> TypeForAny TypeForTypeCounter
  | ReasonCollector -> TypeForAny TypeForReasonCollector

(** A mapping from a map-reducer type to a compatible implementation operating
on data for that type.

Needs an entry when adding a map-reducer. *)
let implementation_for (type t) (mr : t map_reducer_type) :
    (module MapReducer with type t = t) =
  match mr with
  | TypeForTastHashes -> (module Tast_hashes)
  | TypeForTypeCounter -> (module Type_counter)
  | TypeForReasonCollector -> (module Reason_collector)

module MRMap = WrappedMap.Make (struct
  type t = map_reducer

  let compare = compare_map_reducer
end)

(** Existential wrapper around any map-reducer with a value of its intermediate
data type attached. *)
type map_reducer_result =
  | MapReducerResult : ('a map_reducer_type * 'a) -> map_reducer_result

(** Witness that two values are of the same type. *

Needs an entry when adding a map-reducer. *)
let refine_map_reducer_result
    (type a b)
    (x : a map_reducer_type)
    (xv : a)
    (y : b map_reducer_type)
    (yv : b) : (a * a) option =
  match (x, y) with
  | (TypeForTastHashes, TypeForTastHashes) -> Some (xv, yv)
  | (TypeForTastHashes, _) -> None
  | (TypeForTypeCounter, TypeForTypeCounter) -> Some (xv, yv)
  | (TypeForTypeCounter, _) -> None
  | (TypeForReasonCollector, TypeForReasonCollector) -> Some (xv, yv)
  | (TypeForReasonCollector, _) -> None

let all_of_map_reducer : map_reducer list =
  List.init
    (max_map_reducer - min_map_reducer + 1)
    ~f:(fun i -> map_reducer_of_enum (i + min_map_reducer) |> Option.value_exn)

(** A list of all reducers with a function that can tell you whether they
are enabled.

Evaluated at module initialization time, because we don't want to do the
resolution from map-reducer to function everytime. *)
let all_map_reducers : ((TypecheckerOptions.t -> bool) * map_reducer) list =
  List.map all_of_map_reducer ~f:(fun mr ->
      match type_for mr with
      | TypeForAny typed_mr ->
        let (module MR) = implementation_for typed_mr in
        (MR.is_enabled, mr))

type t = map_reducer_result MRMap.t

let empty = MRMap.empty

let map ctx path tasts =
  let tcopt = Provider_context.get_tcopt ctx in
  let results =
    List.filter_map all_map_reducers ~f:(fun (is_enabled, mr) ->
        if is_enabled tcopt then
          match type_for mr with
          | TypeForAny typed_mr ->
            let (module MR) = implementation_for typed_mr in
            Some (mr, MapReducerResult (typed_mr, MR.map ctx path tasts))
        else
          None)
  in
  MRMap.of_list results

let reduce xs ys =
  let combine
      (_mr : map_reducer) (x : map_reducer_result) (y : map_reducer_result) =
    match (x, y) with
    | (MapReducerResult (typed_x, x_value), MapReducerResult (typed_y, y_value))
      ->
      let (x, y) =
        refine_map_reducer_result typed_x x_value typed_y y_value
        |> Option.value_exn ~message:"impossible, they have to be the same type"
      in
      let (module MR) = implementation_for typed_x in
      let z = MR.reduce x y in
      Some (MapReducerResult (typed_x, z))
  in
  (* This is lightning fast when we don't have any map-reducers enabled which
     is the common case. *)
  MRMap.union ~combine xs ys

let finalize ~progress ~init_id ~recheck_id xs =
  let do_ _key x =
    match x with
    | MapReducerResult (typed_x, x_value) ->
      let (module MR) = implementation_for typed_x in
      MR.finalize ~progress ~init_id ~recheck_id x_value
  in
  MRMap.iter do_ xs

let to_ffi xs =
  let f _key v s =
    let open Map_reduce_ffi in
    match v with
    | MapReducerResult (TypeForTastHashes, tast_hashes) ->
      { s with tast_hashes = Some tast_hashes }
    | MapReducerResult (TypeForTypeCounter, type_counter) ->
      { s with type_counter = Some type_counter }
    | MapReducerResult (TypeForReasonCollector, reason_collector) ->
      { s with reason_collector = Some reason_collector }
  in
  MRMap.fold f xs Map_reduce_ffi.empty

let of_ffi s =
  let Map_reduce_ffi.{ tast_hashes; type_counter; reason_collector } = s in
  let elems =
    List.filter_map
      ~f:Fn.id
      [
        Option.map tast_hashes ~f:(fun tast_hashes ->
            (TastHashes, MapReducerResult (TypeForTastHashes, tast_hashes)));
        Option.map type_counter ~f:(fun type_counter ->
            (TypeCounter, MapReducerResult (TypeForTypeCounter, type_counter)));
        Option.map reason_collector ~f:(fun reason_collector ->
            ( ReasonCollector,
              MapReducerResult (TypeForReasonCollector, reason_collector) ));
      ]
  in
  MRMap.of_list elems
