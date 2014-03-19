(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils
open Nast

(* Module coded with an exception, if we find a terminal statement we
 * throw the exception Exit.
*)
module Terminal: sig
  val case: case -> bool
  val block: block -> bool

end = struct

  let rec terminal inside_case stl =
    List.iter (terminal_ inside_case) stl

  and terminal_ inside_case = function
    | Break -> if inside_case then () else raise Exit
    | Continue
    | Throw _
    | Return _
    | Expr (_, Yield_break)
    | Expr (_, Assert (
            AE_assert (_, False) |
            AE_invariant ((_, False), _, _) |
            AE_invariant_violation _)) -> raise Exit
    | Expr (_, Yield (_, Special_func sf)) -> special_func sf
    | If (_, b1, b2) ->
      (try terminal inside_case b1; () with Exit ->
        terminal inside_case b2)
    | Switch (_, cl) ->
      terminal_cl cl
    | Try (b, catch_list, fb) ->
      (* Note: return inside a finally block is allowed in PHP and
       * overrides any return in try or catch. It is an error in <?hh,
       * however. The only way that a finally block can thus be
       * terminal is if it throws unconditionally -- however, there's
       * no good case I (eletuchy) could think of for why one would
       * write *always* throwing code inside a finally block.
       *)
      (try terminal inside_case b; () with Exit ->
        terminal_catchl inside_case catch_list)
    | Do _
    | While _
    | For _
    | Foreach _
    | Noop
    | Fallthrough
    | Expr _
    | Static_var _ -> ()

  and special_func = function
    | Gena _
    | Genva _
    | Gen_array_rec _
    | Gen_array_va_rec _ -> ()

  and terminal_catchl inside_case = function
    | [] -> raise Exit
    | (_, _, x) :: rl ->
      (try
         terminal inside_case x
       with Exit ->
         terminal_catchl inside_case rl
      )

  and terminal_cl = function
    (* Empty list case should only be when switch statement is malformed and has
       no case or default blocks *)
    | [] -> ()
    | [Case (_, b)] | [Default b] -> terminal true b
    | Case (_, b) :: rl ->
      (try
         terminal true b;
          (* TODO check this *)
         if List.exists (function Break -> true | _ -> false) b
         then ()
         else raise Exit
       with Exit -> terminal_cl rl)
    | Default b :: rl ->
      (try terminal true b with Exit -> terminal_cl rl)

  and terminal_case = function
    | Case (_, b) | Default b -> terminal true b

  let block stl =
    try terminal false stl; false with Exit -> true

  let case c =
    try terminal_case c; false with Exit -> true

end

(* TODO jwatzman #3076304 convert this and Terminal to visitor pattern to
 * remove copy-pasta *)
module SafeCase: sig
  val check: Pos.t -> case list -> unit
end = struct

  let rec terminal stl =
    List.iter (terminal_) stl

  and terminal_ = function
    | Fallthrough
    | Break
    | Continue
    | Throw _
    | Return _
    | Expr (_, Yield_break)
    | Expr (_, Assert (
            AE_assert (_, False) |
            AE_invariant ((_, False), _, _) |
            AE_invariant_violation _)) -> raise Exit
    | Expr (_, Yield (_, Special_func sf)) -> special_func sf
    | If (_, b1, b2) ->
      (try terminal b1; () with Exit ->
        terminal b2)
    | Switch (_, cl) ->
      terminal_cl cl
    | Try (b, catches, fb) ->
      (* NOTE: contents of fb are not executed in normal flow, so they
       * cannot contribute to terminality *)
      (try terminal b; () with Exit -> terminal_catchl catches)
    | Do _
    | While _
    | For _
    | Foreach _
    | Noop
    | Expr _
    | Static_var _ -> ()

  and special_func = function
    | Gena _
    | Genva _
    | Gen_array_rec _
    | Gen_array_va_rec _ -> ()

  and terminal_catchl = function
    | [] -> raise Exit
    | (_, _, x) :: rl ->
      (try
         terminal x
       with Exit ->
         terminal_catchl rl
      )

  and terminal_cl = function
    (* Empty list case should only be when switch statement is malformed and has
       no case or default blocks *)
    | [] -> ()
    | [Case (_, b)] | [Default b] -> terminal b
    | Case (_, b) :: rl ->
      (try
         terminal b;
          (* TODO check this *)
         if List.exists (function Break -> true | _ -> false) b
         then ()
         else raise Exit
       with Exit -> terminal_cl rl)
    | Default b :: rl ->
      (try terminal b with Exit -> terminal_cl rl)

  let check p = function
    | [] -> () (* Skip empty cases so we can use tl below *)
    | cl -> List.iter begin fun c ->
      try match c with
        (* Allow empty cases to fall through *)
        | Case (_, [])
        | Default [] -> ()
        | Case (e, b) -> begin
          terminal b;
          error_l [
            p, ("This switch has a case that implicitly falls through and is "^
                "not annotated with // FALLTHROUGH");
            fst e, "This case implicitly falls through"
          ]
        end
        | Default b -> begin
          terminal b;
          error p ("This switch has a default case that implicitly falls "^
                   "through and is not annotated with // FALLTHROUGH")
        end
      with Exit -> ()
    end (List.tl (List.rev cl)) (* Skip the last case *)
end
