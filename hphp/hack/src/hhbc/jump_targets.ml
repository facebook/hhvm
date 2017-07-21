(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core

type iterator = (*is mutable*) bool * Iterator.t

type loop_labels =
{ label_break: Label.t
; label_continue: Label.t
; iterator: iterator option
}

type target =
  | Loop of loop_labels
  | Switch of (*end switch*) Label.t
  | TryFinally of (*finally start*) Label.t
  | Finally

type t = target list

let empty = []

type id_key = IdReturn | IdLabel of Label.t
module LabelIdMap: MyMap.S with type key = id_key = MyMap.Make(struct
  type t = id_key
  let compare = Pervasives.compare
end)

(* HHVM assigns ids to labels sequentially as break/continue to labels appear
   in the source code. When emitter is done with scope that contains
   break/continue - label ids associated with the scope are freed and can be reused.
   One exception from the rule is return - it is treated as if it is  jump to a label that
  finishes the function so once id for return is assigned - it is never released.
  Label ids are used in finally epilogue to determine how control flow got into
  the finally body.

  while (true) {
    try {
      if ($a) { continue; }
      if ($b) { return 100; }
    }
    finally {
      print "finally";
    }
  }

  will be emitted roughly as:

  While_Start:
    CGetL $a
    JmpZ L1

    Int 0 ; push id of the target label "continue label for while loop"
    SetL _1; save it in dedicated local
    PopC
    Jmp FinallyStart:

  L1:
    CGetL $b
    JmpZ FinallyStart
    Int 100 ; push return value
    Int 1; push id of the 'return' label
    SetL _1; save id in decicated local
    PopC
    SetL _2; save return value in dedicated local
    PopC
    Jmp FinallyStart

  FinallyStart:
    String "finally"
    Print
    PopC
    IssetL _1; if check if state local is set
    JmpZ FinallyEnd
    CGetL _1; load state
    Switch Unbounded 0 <Case0 Case1>

  Case0:
    UnsetL _1; new iteration - clean local for state id
    Jmp FinallyEnd
  Case1:
    CGetL _2; load return value
    RetC

  FinallyEnd:
    Jmp White_Start
  *)
let label_to_id = ref LabelIdMap.empty

let new_id k =
  match LabelIdMap.get k !label_to_id with
  | Some id -> id
  | None ->
  let rec aux n =
    if LabelIdMap.exists (fun _ id -> n = id) !label_to_id then aux (n + 1)
    else (label_to_id := LabelIdMap.add k n !label_to_id; n)
  in aux 0

let reset () =
  label_to_id := LabelIdMap.empty

let get_id_for_return () = new_id IdReturn
let get_id_for_label l = new_id (IdLabel l)

let release_id l =
  label_to_id := LabelIdMap.remove (IdLabel l) !label_to_id

(* runs a given function and then released label ids that were possibly assigned
   to labels at the head of the list *)
let run_and_release_ids f t =
  let r = f t in
  begin match t with
  | Loop { label_break; label_continue; _ } ::_ ->
    release_id label_break;
    release_id label_continue;
  | (Switch l | TryFinally l) :: _ ->
    release_id l
  | Finally :: _ -> ()
  | [] -> failwith "impossible"
  end;
  r

let with_loop label_break label_continue iterator t f =
  Loop { label_break; label_continue; iterator } :: t
  |> run_and_release_ids f

let with_switch end_label t f =
  Switch end_label :: t
  |> run_and_release_ids f

let with_try finally_label t f =
  TryFinally finally_label :: t
  |> run_and_release_ids f

let with_finally t f =
  Finally :: t
  |> run_and_release_ids f

type resolved_try_finally =
{ target_label: Label.t
; finally_label: Label.t
; adjusted_level: int
; iterators_to_release: iterator list
}

type resolved_jump_target =
  | NotFound
  | ResolvedTryFinally of resolved_try_finally
  | ResolvedRegular of Label.t * iterator list

let add_iterator it_opt iters =
  Option.value_map it_opt ~default:iters ~f:(fun v -> v :: iters)

(* Tries to find a target label given a level and a jump kind (break or continue) *)
let get_target_for_level ~is_break level t =
  (* skip_try_finally is true if we've already determined that we need to jump out of
     finally and now we are looking for the actual target label (break label of the
     while loop in the example below: )

     while (1) {
        try {
           break;
        }
        finally {
           ...
        }
     }
    *)
  let rec aux ~skip_try_finally n l iters =
    match l with
    | [] ->
      (* looked through the entires list of jump targets and still cannot find a
         target for a requested level - bad luck *)
      NotFound
    | TryFinally finally_label :: tl ->
      if skip_try_finally then aux ~skip_try_finally n tl iters
      else
      (* we need to jump out of try body in try/finally - in order to do this
         we should go through the finally block first.*)
      (* try to final target label where we would've jumped if there were no finally blocks *)
      let result = aux ~skip_try_finally:true n tl iters in
      begin match result with
      | NotFound -> NotFound
      | ResolvedRegular (target_label, _) ->
        ResolvedTryFinally
          { target_label = target_label
          ; finally_label
          ; adjusted_level = n
          ; iterators_to_release = iters }
      | _ -> failwith "impossible: TryFinally should be skipped"
      end
    | Switch end_label :: _ when n = 1 -> ResolvedRegular (end_label, iters)
    | Loop { label_break; label_continue; iterator } :: _ when n = 1 ->
        let label, iters =
          if is_break then label_break, add_iterator iterator iters
          else label_continue, iters
        in
        ResolvedRegular (label, iters)
    | Loop { iterator; _ } :: tl ->
      aux ~skip_try_finally (n - 1) tl (add_iterator iterator iters)
    | Switch _ :: tl ->
      aux ~skip_try_finally (n - 1) tl iters
    | Finally :: _ ->
      (* jumps out of finally body are disallowed *)
      NotFound
  in
  aux ~skip_try_finally:false level t []

let get_closest_enclosing_finally_label t =
  let rec aux l iters =
    match l with
    | [] -> None
    | TryFinally l :: _ -> Some (l, iters)
    | Loop { iterator; _ } :: tl -> aux tl (add_iterator iterator iters)
    | _ :: tl -> aux tl iters
  in aux t []

let collect_iterators t =
  List.filter_map t ~f:(function | Loop { iterator; _ } -> iterator | _ -> None)
