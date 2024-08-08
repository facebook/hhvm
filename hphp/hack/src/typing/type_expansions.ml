(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Expandable = struct
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

type expansion = {
  name: Expandable.t;
  use_pos: Pos_or_decl.t;
  def_pos: Pos_or_decl.t option;
}

type t = {
  report_cycle: (Pos.t * Expandable.t) option;
      (** If we are expanding the RHS of a type definition, [report_cycle] contains
          the position and id of the LHS. This way, if the RHS expands at some point
          to the LHS id, we are able to report a cycle. *)
  expansions: expansion list;
      (** The type defs and type access we have expanded thus far, at what position. Used
          to prevent entering into a cycle when expanding these types.
          We preserve order (i.e. use a list instead of set) for error reporting. *)
  cyclic_expansion: bool;
      (** true if we've ever detected a cycle during add_and_check_cycles *)
}

type cycle =
  | Not_to_report  (** There was a cycle but not a cycle we want to report. *)
  | To_report of {
      report_cycle: Pos.t * Expandable.t;
      expansions: expansion list;
      last_expansion: expansion;
    }

type cycle_reporter = cycle * Typing_error.Reasons_callback.t option

let empty_w_cycle_report ~report_cycle =
  { report_cycle; expansions = []; cyclic_expansion = false }

let empty = empty_w_cycle_report ~report_cycle:None

let add ({ expansions; _ } as exps) exp =
  { exps with expansions = exp :: expansions }

let add_and_check_cycles ({ report_cycle; expansions; _ } as t : t) expansion :
    (t, cycle) result =
  match report_cycle with
  | Some (p', x) when Expandable.equal expansion.name x ->
    Error
      (To_report
         { report_cycle = (p', x); expansions; last_expansion = expansion })
  | Some _
  | None ->
    if
      List.exists expansions ~f:(function exp ->
          Expandable.equal expansion.name exp.name)
    then
      Error Not_to_report
    else
      Ok (add t expansion)

let to_log_string { report_cycle; expansions; cyclic_expansion = _ } =
  let report_cycle_s =
    match report_cycle with
    | None -> ""
    | Some (_p, exp) ->
      Printf.sprintf "report on %s; " (Expandable.to_string exp)
  in
  let expansions_s =
    List.rev_map expansions ~f:(fun x -> Expandable.to_string x.name)
    |> String.concat ~sep:" -> "
  in
  Printf.sprintf "%sexpansions: %s" report_cycle_s expansions_s

let report (cycles : cycle_reporter list) : Typing_error.t option =
  List.filter_map cycles ~f:(fun (cycle, on_error) ->
      match cycle with
      | Not_to_report -> None
      | To_report
          { report_cycle = (pos, first_exp); expansions; last_expansion } ->
        (match first_exp with
        | Expandable.Enum _name ->
          Option.map
            on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  apply_reasons ~on_error
                  @@ Secondary.Cyclic_enum_constraint
                       (Pos_or_decl.of_raw_pos pos)
                (* TODO remove Pos_or_decl.of_raw_pos *))
        | Expandable.Type_alias _name ->
          Some
            Typing_error.(
              primary
              @@ Primary.Cyclic_typedef
                   { def_pos = pos; use_pos = last_expansion.use_pos })
        | Expandable.Type_constant _ ->
          let seen =
            (last_expansion :: expansions |> List.map ~f:(fun x -> x.name))
            @ [first_exp]
            |> List.map ~f:Expandable.to_string
          in
          Some
            Typing_error.(
              primary @@ Primary.Cyclic_typeconst { pos; tyconst_names = seen })))
  |> Typing_error.multiple_opt

let def_positions { report_cycle; expansions; _ } =
  (Option.map report_cycle ~f:(fun (p, _) -> Pos_or_decl.of_raw_pos p)
  |> Option.to_list)
  @ List.rev_filter_map expansions ~f:(fun x -> x.def_pos)

let cyclic_expansion exps = exps.cyclic_expansion
