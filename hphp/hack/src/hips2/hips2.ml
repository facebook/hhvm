(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
include Hips2_intf

module Make (Intra : Intra) = struct
  type intra_constraint_ = Intra.constraint_

  type inter_constraint_ = ClassInterConstraintProofOfConcept
  [@@deriving show { with_path = false }]

  type 'a lookup = {
    by_class: (string, 'a HashSet.t) Hashtbl.t;
    by_function: (string, 'a HashSet.t) Hashtbl.t;
  }

  type t = {
    intras: Intra.constraint_ lookup;
    inters: inter_constraint_ lookup;
  }

  type _ constraint_kind =
    | Intra : Intra.constraint_ constraint_kind
    | Inter : inter_constraint_ constraint_kind

  type id_kind =
    | Class
    | Function
  [@@deriving ord, show { with_path = false }]

  type id = id_kind * string [@@deriving ord, show { with_path = false }]

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

    let solve t = t
  end

  module Read = struct
    type nonrec t = t

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
  end
end
