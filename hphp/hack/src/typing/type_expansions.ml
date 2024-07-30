(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Expansion = struct
  type t =
    | Enum of string
    | Type_alias of string
    | Type_constant of {
        receiver_name: string;
        type_const_name: string;
      }
  [@@deriving eq]

  let to_string : t -> string = function
    | Enum x
    | Type_alias x ->
      x
    | Type_constant { receiver_name; type_const_name } ->
      Printf.sprintf "%s::%s" receiver_name type_const_name
end

type t = {
  report_cycle: (Pos.t * Expansion.t) option;
      (** If we are expanding the RHS of a type definition, [report_cycle] contains
            the position and id of the LHS. This way, if the RHS expands at some point
            to the LHS id, we are able to report a cycle. *)
  expansions: (Pos_or_decl.t * Expansion.t) list;
      (** The type defs and type access we have expanded thus far, at what position. Used
            to prevent entering into a cycle when expanding these types.
            We preserve order (i.e. use a list instead of set) for error reporting. *)
  cyclic_expansion: bool;
      (** true if we've ever detected a cycle during add_and_check_cycles *)
}

let empty_w_cycle_report ~report_cycle =
  { report_cycle; expansions = []; cyclic_expansion = false }

let empty = empty_w_cycle_report ~report_cycle:None

let add ({ expansions; _ } as exps) exp =
  { exps with expansions = exp :: expansions }

(** Whether we've already expanded [x] *)
let has_expanded { report_cycle; expansions; _ } (x : Expansion.t) =
  match report_cycle with
  | Some (p, x') when Expansion.equal x x' -> Some (Some p)
  | Some _
  | None ->
    List.find_map expansions ~f:(function
        | (_, x') when Expansion.equal x x' -> Some None
        | _ -> None)

let add_and_check_cycles (exps : t) (p, (id : Expansion.t)) :
    t * Pos.t option option =
  let has_cycle = has_expanded exps id in
  let exps =
    if Option.is_some has_cycle then
      { exps with cyclic_expansion = true }
    else
      exps
  in
  let exps = add exps (p, id) in
  (exps, has_cycle)

let as_list { report_cycle; expansions; _ } =
  (report_cycle
  |> Option.map ~f:(Tuple2.map_fst ~f:Pos_or_decl.of_raw_pos)
  |> Option.to_list)
  @ List.rev expansions

let to_string_list exps =
  as_list exps |> List.map ~f:(fun x -> x |> snd |> Expansion.to_string)

let positions exps = as_list exps |> List.map ~f:fst

let cyclic_expansion exps = exps.cyclic_expansion
