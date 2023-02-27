(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
include Hips2_intf

module Make (Intra : Intra) :
  T with type intra_constraint_ = Intra.constraint_ = struct
  type intra_constraint_ = Intra.constraint_

  type 'a lookup = {
    by_class: (string, 'a HashSet.t) Hashtbl.t;
    by_function: (string, 'a HashSet.t) Hashtbl.t;
  }

  type id_kind =
    | Class
    | Function
  [@@deriving ord, show { with_path = false }]

  type id = id_kind * string [@@deriving ord, show { with_path = false }]

  type inter_constraint_ = ClassExtends of id
  [@@deriving show { with_path = false }]

  type t = {
    intras: Intra.constraint_ lookup;
    inters: inter_constraint_ lookup;
  }

  type _ constraint_kind =
    | Intra : Intra.constraint_ constraint_kind
    | Inter : inter_constraint_ constraint_kind

  let id_of_class_sid sid = (Class, sid)

  let id_of_function_sid sid = (Function, sid)

  let create_lookup () : 'a lookup =
    {
      by_class = Hashtbl.Poly.create ~size:1000 ();
      by_function = Hashtbl.Poly.create ~size:1000 ();
    }

  let find_tbl { by_class; by_function } id_kind =
    match id_kind with
    | Class -> by_class
    | Function -> by_function

  (** preserves invariant that `intra` and `inter` have the same set of keys *)
  let find_set : type a. t -> a constraint_kind -> id -> a HashSet.t =
   fun { inters; intras } lookup_kind (id_kind, sid) ->
    let find' lookup =
      let tbl = find_tbl lookup id_kind in
      Hashtbl.find_or_add tbl sid ~default:(fun _ -> HashSet.create ())
    in
    let inter_set = find' inters in
    let intra_set = find' intras in
    match lookup_kind with
    | Inter -> inter_set
    | Intra -> intra_set

  let get_inters t id = find_set t Inter id

  let get_intras t id = find_set t Intra id

  let get_keys { intras = { by_function; by_class }; _ } : id Sequence.t =
    (* OK to only read `intras` due to invariant of `find_set` *)
    let keys_of_tbl id_of_sid tbl : id Sequence.t =
      tbl |> Hashtbl.keys |> Sequence.of_list |> Sequence.map ~f:id_of_sid
    in
    Sequence.append
      (keys_of_tbl id_of_function_sid by_function)
      (keys_of_tbl id_of_class_sid by_class)

  let all_inter_constraints t =
    get_keys t
    |> Sequence.bind ~f:(fun id ->
           get_inters t id
           |> HashSet.to_list
           |> Sequence.of_list
           |> Sequence.map ~f:(fun c -> (id, c)))

  module Write = struct
    type nonrec t = t

    let create () : t = { intras = create_lookup (); inters = create_lookup () }

    let add_id t id =
      (* OK to only update `intras` due to invariant of `find_set` *)
      ignore @@ find_set t Inter id

    let add_intra t id (c : Intra.constraint_) =
      let set = find_set t Intra id in
      HashSet.add set c

    let add_inter t id (c : inter_constraint_) =
      let set = find_set t Inter id in
      HashSet.add set c

    let debug_dump t = t

    let hashset_union ~from_set ~to_set : bool =
      let to_set_length_before = HashSet.length to_set in
      HashSet.union to_set ~other:from_set;
      let to_set_length_after = HashSet.length to_set in
      let has_changed = to_set_length_after <> to_set_length_before in
      has_changed

    let solve t : t =
      let rec loop () =
        let at_inter_constraint_mut id = function
          | ClassExtends parent_id ->
            let from_set = get_intras t parent_id in
            let to_set = get_intras t id in
            let changed = hashset_union ~from_set ~to_set in
            changed
        in
        let has_changes =
          all_inter_constraints t
          |> Sequence.map ~f:(Tuple2.uncurry at_inter_constraint_mut)
          |> Sequence.fold ~init:false ~f:( || )
        in
        if has_changes then loop ()
      in
      loop ();
      t
  end

  module Read = struct
    type nonrec t = t

    let get_intras = get_intras

    let get_inters = get_inters

    let get_keys = get_keys
  end
end
