(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = {
  tyvar_occurrences: Tvid.Set.t Tvid.Map.t;
      (** A map to track where each type variable occurs,
          more precisely in the type of which other type variables.
          E.g. if #1 is bound to (#2 | int), then this map contains the entry
            #2 -> { #1 }
          This is based on shallow binding, i.e. in the example above, if #2
          is mapped to #3, then tyvar_occurrences would be:
            #2 -> { #1 }
            #3 -> { #2 }
          but we would not record that #3 occurs in #1.
          When a type variable v gets solved or the type bound to it gets simplified,
          we simplify the unions and intersections of the types bound to the
          type variables associated to v in this map.
          So in our example, if #2 gets solved to int,
          we simplify #1 to (int | int) = int.
          There are only entries for variables that are unsolved or contain
          other unsolved type variables. Variables that are solved and contain
          no other unsolved type variables get removed from this map. *)
  tyvars_in_tyvar: Tvid.Set.t Tvid.Map.t;
      (** Mapping of type variables to the type variables contained in their
          types which are either unsolved or themselves contain unsolved type
          variables.
          This is the dual of tyvar_occurrences. *)
}

module Log = struct
  open Typing_log_value

  let iset_imap_as_value map =
    Map
      (Tvid.Map.fold
         (fun i vars m -> SMap.add (var_as_string i) (varset_as_value vars) m)
         map
         SMap.empty)

  let tyvar_occurrences_as_value = iset_imap_as_value

  let tyvars_in_tyvar_as_value = iset_imap_as_value

  let as_value x =
    let { tyvar_occurrences; tyvars_in_tyvar } = x in
    make_map
      [
        ("tyvar_occurrences", tyvar_occurrences_as_value tyvar_occurrences);
        ("tyvars_in_tyvar", tyvars_in_tyvar_as_value tyvars_in_tyvar);
      ]
end

let init =
  { tyvar_occurrences = Tvid.Map.empty; tyvars_in_tyvar = Tvid.Map.empty }

let get_tyvar_occurrences x v =
  Option.value (Tvid.Map.find_opt v x.tyvar_occurrences) ~default:Tvid.Set.empty

let get_tyvars_in_tyvar x v =
  Option.value (Tvid.Map.find_opt v x.tyvars_in_tyvar) ~default:Tvid.Set.empty

let set_tyvar_occurrences x v vars =
  { x with tyvar_occurrences = Tvid.Map.add v vars x.tyvar_occurrences }

(** Make v occur in v', i.e. add v' to the occurrences of v. *)
let add_tyvar_occurrence x v ~occurs_in:v' =
  let vars = get_tyvar_occurrences x v in
  let vars = Tvid.Set.add v' vars in
  let x = set_tyvar_occurrences x v vars in
  x

let add_tyvar_occurrences x vars ~occur_in:v =
  Tvid.Set.fold (fun v' x -> add_tyvar_occurrence x v' ~occurs_in:v) vars x

let set_tyvars_in_tyvar x v vars =
  { x with tyvars_in_tyvar = Tvid.Map.add v vars x.tyvars_in_tyvar }

let make_tyvars_occur_in_tyvar x vars ~occur_in:v =
  let x = add_tyvar_occurrences x vars ~occur_in:v in
  let x = set_tyvars_in_tyvar x v vars in
  x

let remove_tyvar_occurrence x v ~no_more_occurs_in:v' =
  let vars = get_tyvar_occurrences x v in
  let vars = Tvid.Set.remove v' vars in
  let x = set_tyvar_occurrences x v vars in
  x

let remove_tyvar_from_tyvar x ~from:v v' =
  let vars = get_tyvars_in_tyvar x v in
  let vars = Tvid.Set.remove v' vars in
  let x = set_tyvars_in_tyvar x v vars in
  x

let make_tyvar_no_more_occur_in_tyvar x v ~no_more_in:v' =
  let x = remove_tyvar_occurrence x v ~no_more_occurs_in:v' in
  let x = remove_tyvar_from_tyvar x ~from:v' v in
  x

let contains_unsolved_tyvars x v =
  not @@ Tvid.Set.is_empty (get_tyvars_in_tyvar x v)

(** Update the tyvar occurrences after unbinding a type variable. *)
let unbind_tyvar x v =
  let vars_in_v = get_tyvars_in_tyvar x v in
  (* update tyvar_occurrences. *)
  let x =
    Tvid.Set.fold
      (fun v' x -> remove_tyvar_occurrence x v' ~no_more_occurs_in:v)
      vars_in_v
      x
  in
  (* update tyvars_in_tyvar *)
  let x = set_tyvars_in_tyvar x v Tvid.Set.empty in
  x

let occurs_in x occuring_tv ~in_:containing_tv =
  let occurs =
    Tvid.Set.mem containing_tv (get_tyvar_occurrences x occuring_tv)
  in
  let contains =
    Tvid.Set.mem occuring_tv (get_tyvars_in_tyvar x containing_tv)
  in
  assert (Bool.equal occurs contains);
  occurs

let remove_var x v =
  let occurrences = get_tyvar_occurrences x v in
  let x =
    Tvid.Set.fold
      (fun occ x -> make_tyvar_no_more_occur_in_tyvar x v ~no_more_in:occ)
      occurrences
      x
  in
  let x =
    { x with tyvar_occurrences = Tvid.Map.remove v x.tyvar_occurrences }
  in
  let vars_in_v = get_tyvars_in_tyvar x v in
  let x =
    Tvid.Set.fold
      (fun var_in_v x ->
        make_tyvar_no_more_occur_in_tyvar x var_in_v ~no_more_in:v)
      vars_in_v
      x
  in
  let x = { x with tyvars_in_tyvar = Tvid.Map.remove v x.tyvars_in_tyvar } in
  x
