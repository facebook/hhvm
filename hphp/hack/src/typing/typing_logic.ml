(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Typing_defs

(* Logical proposition about types *)
type subtype_prop =
(* Sub(ty1,ty2) if ty1 is a subtype of ty2, written ty1 <: ty2 *)
| Sub of locl ty * locl ty
(* Eq(ty1,ty2) if ty1 is equivalent to ty2. Should be same as ty1 <: ty2 /\ ty2 <: ty1 *)
| Eq of locl ty * locl ty
(* Conjunction. Conj [] means "true" *)
| Conj of subtype_prop list
(* Disjunction. Disj [] means "false", but use Unsat if you want specific error *)
| Disj of subtype_prop list
(* Equivalent to Disj [], but with an error message function attached.
 * TODO: actually store the error rather than a suspension *)
| Unsat of (unit -> unit)

let valid = Conj []

(* Smart constructor for binary conjunction *)
let rec conj p1 p2 =
  match p1, p2 with
  | Conj ps1, Conj ps2 -> conj_list (ps1 @ ps2)
  | Conj [], _ -> p2
  | _, Conj [] -> p1
  (* Preserve the order to maintain legacy behaviour. If two errors share the
   * same position then the first one to be emitted wins.
   * TODO: consider relaxing this behaviour *)
  | Conj ps, _ -> conj_list (ps @ [p2])
  | _, Conj ps -> conj_list (p1::ps)
  | _, _ -> Conj [p1; p2]

and conj_list ps =
  match ps with
  | [p] -> p
  | _ -> Conj ps

(* Smart constructor for binary disjunction *)
let disj p1 p2 =
  match p1, p2 with
  | _, (Unsat _ | Disj []) -> p1
  | (Unsat _ | Disj []), _ -> p2
  | Conj [], _
  | _, Conj [] -> Conj []
  | Disj ps1, Disj ps2 -> Disj (ps1 @ ps2)
  | _, Disj ps -> Disj (p1::ps)
  | Disj ps, _ -> Disj (p2::ps)
  | _, _ -> Disj [p1; p2]

let disj_list ps =
  match ps with
  | [p] -> p
  | _ -> Disj ps

(* Is this proposition always true? (e.g. Conj [] but also Disj [Conj []; Unsat _]
 * if not simplified
 *)
let rec is_valid p =
  match p with
  | Conj ps -> List.for_all ps is_valid
  | Disj ps -> List.exists ps is_valid
  | Unsat _ -> false
  | Sub (_, _) -> false
  | Eq (_, _) -> false

(* Is this proposition always false? e.g. Unsat _ but also Conj [Conj []; Unsat _]
 * if not simplified
 *)
and is_unsat p =
  match p with
  | Conj ps -> List.exists ps is_unsat
  | Disj ps -> List.for_all ps is_unsat
  | Unsat _ -> true
  | Sub (_, _) -> false
  | Eq (_, _) -> false

let rec to_string env p =
  match p with
  | Unsat _ -> "UNSAT"
  | Conj [] -> "TRUE"
  | Conj ps -> String.concat " & " (List.map ~f:(to_string env) ps)
  | Disj [] -> "FALSE"
  | Disj ps -> String.concat " | " (List.map ~f:(fun x -> "(" ^ to_string env x ^ ")") ps)
  | Sub(ty1, ty2) ->
    Typing_print.debug_with_tvars env ty1 ^ "<:" ^ Typing_print.debug_with_tvars env ty2
  | Eq(ty1, ty2) ->
    Typing_print.debug_with_tvars env ty1 ^ "=" ^ Typing_print.debug_with_tvars env ty2

let log_prop level p message env prop =
  if Typing_log.get_log_level() >= level then
    Typing_log.log_position p (fun () ->
      Typing_log.lprintf (Tty.Bold Tty.Green) "%s: " message;
      Typing_log.lprintf (Tty.Normal Tty.Green) "%s" (to_string env prop);
    Typing_log.lnewline ())
  else ()
