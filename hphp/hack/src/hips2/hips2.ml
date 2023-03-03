(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Store = Hips2_store
include Hips2_intf

let hashset_size = 0

let hashtbl_size = 0

module Make (Intra : Intra) :
  T with type intra_constraint_ = Intra.constraint_ = struct
  type intra_constraint_ = Intra.constraint_ [@@deriving hash, eq]

  type id =
    | ClassLike of string
    | Function of string
  [@@deriving hash, eq, ord, sexp, show { with_path = false }]

  type inter_constraint_ = Inherits of id
  [@@deriving eq, hash, ord, show { with_path = false }]

  let hash_id : id -> int = Obj.magic hash_id (* workaround for T92019055 *)

  let hash_inter_constraint_ : inter_constraint_ -> int =
    Obj.magic hash_inter_constraint_ (* workaround for T92019055 *)

  let hash_intra_constraint_ : intra_constraint_ -> int =
    Obj.magic hash_intra_constraint_ (* workaround for T92019055 *)

  (**
    internal. Mutable total map from ids to sets of constraints. Returns an empty set when an id is not found.
    Uses `Caml`'s.HashTbl instead of `Base`'s for marshallability, see T146711502.
  *)
  module DefaultTbl (SetOf : Caml.Hashtbl.HashedType) = struct
    module IdHash = struct
      type t = id

      let equal = equal_id

      let hash = hash_id
    end

    module TblImpl = Caml.Hashtbl.Make (IdHash)

    module Set = struct
      module SetImpl = Caml.Hashtbl.Make (SetOf)

      (* it's a Hashtbl where the values are always `unit` *)
      type t = unit SetImpl.t

      let create () = SetImpl.create hashset_size

      let add (t : t) v : bool =
        if SetImpl.mem t v then
          false
        else (
          SetImpl.add t v ();
          true
        )

      let union ~from_set ~to_set : bool =
        let fold_add c () has_changed = add to_set c || has_changed in
        SetImpl.fold fold_add from_set false

      let to_sequence t = SetImpl.to_seq_keys t |> Sequence.of_seq
    end

    type t = Set.t TblImpl.t

    let create () : t = TblImpl.create hashtbl_size

    let to_sequence_keys t : id Sequence.t =
      TblImpl.to_seq_keys t |> Sequence.of_seq

    let find t key : Set.t =
      match TblImpl.find_opt t key with
      | None ->
        let set : Set.t = Set.create () in
        TblImpl.add t key set;
        set
      | Some set -> set
  end

  module InterHash = struct
    type t = inter_constraint_

    let equal = equal_inter_constraint_

    let hash = hash_inter_constraint_
  end

  module InterTbl = DefaultTbl (InterHash)

  module IntraHash = struct
    type t = intra_constraint_

    let equal = equal_intra_constraint_

    let hash = hash_intra_constraint_
  end

  module IntraTbl = DefaultTbl (IntraHash)

  type t = {
    intras: IntraTbl.t;
    inters: InterTbl.t;
  }

  let get_inters_set { inters; _ } id = InterTbl.find inters id

  let get_inters t id = get_inters_set t id |> InterTbl.Set.to_sequence

  let get_intras_set { intras; _ } id = IntraTbl.find intras id

  let get_intras t id = get_intras_set t id |> IntraTbl.Set.to_sequence

  let get_keys { inters; _ } : id Sequence.t =
    (* OK to just check inters because add_keys adds keys to inters *)
    inters |> InterTbl.to_sequence_keys

  let all_inter_constraints (t : t) =
    get_keys t
    |> Sequence.bind ~f:(fun id ->
           get_inters t id |> Sequence.map ~f:(fun c -> (id, c)))

  let create () = { inters = InterTbl.create (); intras = IntraTbl.create () }

  module Write = struct
    type nonrec t = t

    type flush = unit -> unit

    (** Continue using existing in-memory storage where possible to reduce the number of distinct files we have to flush to *)
    let cached = ref None

    let create ~db_dir ~worker_id =
      match !cached with
      | Some (prev_db_dir, prev_worker_id, prev_flush, prev_t)
        when String.(equal prev_db_dir db_dir) && prev_worker_id = worker_id ->
        (prev_flush, prev_t)
      | Some _
      | None ->
        let t = create () in
        let flush () = Store.persist ~db_dir ~worker_id t in
        at_exit flush;
        cached := Some (db_dir, worker_id, flush, t);
        (flush, t)

    let add_id { inters; _ } id =
      (* We use `inters` to store ids.
       * The next line works because `InterTbl.find` creates a set for the key if it doesn't exist
       * *)
      ignore @@ InterTbl.find inters id

    let add_intra t id (c : Intra.constraint_) : unit =
      (* we use the keys of `inters` to represent seen ids *)
      add_id t id;
      let set = IntraTbl.find t.intras id in
      ignore (IntraTbl.Set.add set c : bool)

    let add_inter { inters; _ } id (c : inter_constraint_) : unit =
      let set = InterTbl.find inters id in
      ignore (InterTbl.Set.add set c : bool)
  end

  module Read = struct
    type nonrec t = t

    let get_intras = get_intras

    let get_inters = get_inters

    let get_keys = get_keys
  end

  let solve_t t : Read.t =
    let rec loop () =
      let at_inter_constraint_mut id = function
        | Inherits parent_id ->
          let from_set = get_intras_set t parent_id in
          let to_set = get_intras_set t id in
          let changed = IntraTbl.Set.union ~from_set ~to_set in
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

  let un_persist ~db_dir : t =
    let t_dst = create () in
    let merge_in t_src =
      get_keys t_src
      |> Sequence.iter ~f:(fun id ->
             Write.add_id t_dst id;
             get_inters_set t_src id
             |> InterTbl.Set.to_sequence
             |> Sequence.iter ~f:(Write.add_inter t_dst id);
             get_intras_set t_src id
             |> IntraTbl.Set.to_sequence
             |> Sequence.iter ~f:(Write.add_intra t_dst id))
    in
    Store.un_persist ~db_dir |> Sequence.iter ~f:merge_in;
    t_dst

  let debug_dump ~db_dir : Read.t = un_persist ~db_dir

  let solve ~db_dir : Read.t = solve_t @@ un_persist ~db_dir
end
