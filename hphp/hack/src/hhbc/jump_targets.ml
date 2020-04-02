(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module T = Aast

let labels_in_function_ : bool SMap.t ref = ref SMap.empty

let function_has_goto_ = ref false

let get_labels_in_function () = !labels_in_function_

let get_function_has_goto () = !function_has_goto_

let set_labels_in_function s = labels_in_function_ := s

let set_function_has_goto f = function_has_goto_ := f

let rec collect_valid_target_labels_aux acc s =
  match snd s with
  | T.Block block -> collect_valid_target_labels_for_block_aux acc block
  (* can jump into the try block but not to finally *)
  | T.Try (block, cl, _) ->
    let acc = collect_valid_target_labels_for_block_aux acc block in
    List.fold_left cl ~init:acc ~f:(fun acc (_, _, block) ->
        collect_valid_target_labels_for_block_aux acc block)
  | T.GotoLabel (_, s) -> SSet.add s acc
  | T.If (_, then_block, else_block) ->
    let acc = collect_valid_target_labels_for_block_aux acc then_block in
    collect_valid_target_labels_for_block_aux acc else_block
  | T.Fallthrough
  | T.Expr _
  | T.Break
  | T.Continue
  | T.Throw _
  | T.Return _
  | T.Goto _
  | T.Awaitall _
  | T.Markup _
  | T.Noop
  | T.Foreach _
  | T.Do _
  | T.For _ ->
    acc
  (* jump to while loops/switches/usings are disallowed in php files
     and permitted in hh - assuming that they can only appear there
     as a result of source to source transformation that has validated
     correctness of the target *)
  | T.While (_, block)
  | T.Using { T.us_block = block; _ } ->
    collect_valid_target_labels_for_block_aux acc block
  | T.Switch (_, cl) -> collect_valid_target_labels_for_switch_cases_aux acc cl

and collect_valid_target_labels_for_block_aux acc block =
  List.fold_left block ~init:acc ~f:collect_valid_target_labels_aux

and collect_valid_target_labels_for_switch_cases_aux acc cl =
  List.fold_left cl ~init:acc ~f:(fun acc s ->
      match s with
      | T.Default (_, block)
      | T.Case (_, block) ->
        collect_valid_target_labels_for_block_aux acc block)

let rec collect_valid_target_labels_for_def_aux acc def =
  match def with
  | T.Stmt s -> collect_valid_target_labels_aux acc s
  | T.Namespace (_, defs) -> collect_valid_target_labels_for_defs_aux acc defs
  | _ -> acc

and collect_valid_target_labels_for_defs_aux acc defs =
  List.fold_left defs ~init:acc ~f:collect_valid_target_labels_for_def_aux

let collect_valid_target_labels_for_stmt s =
  if not !function_has_goto_ then
    SSet.empty
  else
    collect_valid_target_labels_aux SSet.empty s

let collect_valid_target_labels_for_block b =
  if not !function_has_goto_ then
    SSet.empty
  else
    collect_valid_target_labels_for_block_aux SSet.empty b

let collect_valid_target_labels_for_defs defs =
  if not !function_has_goto_ then
    SSet.empty
  else
    collect_valid_target_labels_for_defs_aux SSet.empty defs

let collect_valid_target_labels_for_switch_cases cl =
  if not !function_has_goto_ then
    SSet.empty
  else
    collect_valid_target_labels_for_switch_cases_aux SSet.empty cl

type loop_labels = {
  label_break: Label.t;
  label_continue: Label.t;
  iterator: Iterator.t option;
}

type region =
  | Loop of loop_labels * SSet.t
  | Switch of (*end switch*) Label.t * SSet.t
  | TryFinally of (*finally start*) Label.t * SSet.t
  | Finally of SSet.t
  | Function of SSet.t
  | Using of (*finally start*) Label.t * SSet.t

type t = region list

let empty = []

type id_key =
  | IdReturn
  | IdLabel of Label.t
[@@deriving ord]

module LabelIdMap : WrappedMap.S with type key = id_key =
WrappedMap.Make (struct
  type t = id_key

  let compare = compare_id_key
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
  match LabelIdMap.find_opt k !label_to_id with
  | Some id -> id
  | None ->
    let rec aux n =
      if LabelIdMap.exists (fun _ id -> n = id) !label_to_id then
        aux (n + 1)
      else (
        label_to_id := LabelIdMap.add k n !label_to_id;
        n
      )
    in
    aux 0

let reset () = label_to_id := LabelIdMap.empty

let get_id_for_return () = new_id IdReturn

let get_id_for_label l = new_id (IdLabel l)

let release_id l = label_to_id := LabelIdMap.remove (IdLabel l) !label_to_id

(* runs a given function and then released label ids that were possibly assigned
   to labels at the head of the list *)
let run_and_release_ids _labels f s t =
  let r = f t s in
  begin
    match t with
    | Loop ({ label_break; label_continue; _ }, _) :: _ ->
      release_id label_break;
      release_id label_continue
    | (Switch (l, _) | TryFinally (l, _) | Using (l, _)) :: _ -> release_id l
    | (Function _ | Finally _) :: _ -> ()
    | [] -> failwith "impossible"
  end;

  (* CONSIDER: now HHVM does not release state ids for named labels
     even after leaving the scope where labels are accessible
     Do the same for now for compatibility reasons *)
  (* SSet.iter (fun l -> release_id (Label.Named l)) labels; *)
  r

let with_loop label_break label_continue iterator t b f =
  let labels = collect_valid_target_labels_for_block b in
  Loop ({ label_break; label_continue; iterator }, labels) :: t
  |> run_and_release_ids labels f b

let with_switch end_label t cl f =
  let labels = collect_valid_target_labels_for_switch_cases cl in
  (* CONSIDER: now HHVM eagerly reserves state id for the switch end label
    which does not seem to be necessary - do it for now for HHVM compatibility *)
  let _ = get_id_for_label end_label in
  Switch (end_label, labels) :: t |> run_and_release_ids labels f ()

let with_try finally_label t b f =
  let labels = collect_valid_target_labels_for_block b in
  TryFinally (finally_label, labels) :: t |> run_and_release_ids labels f b

let with_finally t b f =
  let labels = collect_valid_target_labels_for_block b in
  Finally labels :: t |> run_and_release_ids labels f b

let with_function t s f =
  let labels = collect_valid_target_labels_for_defs s in
  Function labels :: t |> run_and_release_ids labels f s

let with_using finally_label t b f =
  let labels = collect_valid_target_labels_for_block b in
  Using (finally_label, labels) :: t |> run_and_release_ids labels f b

type resolved_try_finally = {
  target_label: Label.t;
  finally_label: Label.t;
  adjusted_level: int;
  iterators_to_release: Iterator.t list;
}

type resolved_jump_target =
  | NotFound
  | ResolvedTryFinally of resolved_try_finally
  | ResolvedRegular of Label.t * Iterator.t list

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
    | []
    | Function _ :: _ ->
      (* looked through the entires list of jump targets and still cannot find a
         target for a requested level - bad luck *)
      NotFound
    | (Using (finally_label, _) | TryFinally (finally_label, _)) :: tl ->
      if skip_try_finally then
        aux ~skip_try_finally n tl iters
      else
        (* we need to jump out of try body in try/finally - in order to do this
         we should go through the finally block first.*)
        (* try to final target label where we would've jumped if there were no finally blocks *)
        let result = aux ~skip_try_finally:true n tl iters in
        begin
          match result with
          | NotFound -> NotFound
          | ResolvedRegular (target_label, _) ->
            ResolvedTryFinally
              {
                target_label;
                finally_label;
                adjusted_level = n;
                iterators_to_release = iters;
              }
          | _ -> failwith "impossible: TryFinally should be skipped"
        end
    | Switch (end_label, _) :: _ when n = 1 -> ResolvedRegular (end_label, iters)
    | Loop ({ label_break; label_continue; iterator }, _) :: _ when n = 1 ->
      let (label, iters) =
        if is_break then
          (label_break, add_iterator iterator iters)
        else
          (label_continue, iters)
      in
      ResolvedRegular (label, iters)
    | Loop ({ iterator; _ }, _) :: tl ->
      aux ~skip_try_finally (n - 1) tl (add_iterator iterator iters)
    | Switch _ :: tl -> aux ~skip_try_finally (n - 1) tl iters
    | Finally _ :: _ ->
      (* jumps out of finally body are disallowed *)
      NotFound
  in
  aux ~skip_try_finally:false level t []

let get_closest_enclosing_finally_label t =
  let rec aux l iters =
    match l with
    | [] -> None
    | (Using (l, _) | TryFinally (l, _)) :: _ -> Some (l, iters)
    | Loop ({ iterator; _ }, _) :: tl -> aux tl (add_iterator iterator iters)
    | _ :: tl -> aux tl iters
  in
  aux t []

let collect_iterators t =
  List.filter_map t ~f:(function
      | Loop ({ iterator; _ }, _) -> iterator
      | _ -> None)

type resolved_goto_finally = {
  rgf_finally_start_label: Label.t;
  rgf_iterators_to_release: Iterator.t list;
}

type resolved_goto_target =
  | ResolvedGoto_label of Iterator.t list
  | ResolvedGoto_finally of resolved_goto_finally
  | ResolvedGoto_goto_from_finally
  | ResolvedGoto_goto_invalid_label

let find_goto_target t label =
  let rec aux t iters =
    match t with
    | Loop ({ iterator; _ }, labels) :: tl ->
      if SSet.mem label labels then
        ResolvedGoto_label iters
      else
        aux tl (add_iterator iterator iters)
    | Switch (_, labels) :: tl ->
      if SSet.mem label labels then
        ResolvedGoto_label iters
      else
        aux tl iters
    | (Using (finally_start, labels) | TryFinally (finally_start, labels)) :: _
      ->
      if SSet.mem label labels then
        ResolvedGoto_label iters
      else
        ResolvedGoto_finally
          {
            rgf_finally_start_label = finally_start;
            rgf_iterators_to_release = iters;
          }
    | Finally labels :: _ ->
      if SSet.mem label labels then
        ResolvedGoto_label iters
      else
        ResolvedGoto_goto_from_finally
    | Function labels :: _ ->
      if SSet.mem label labels then
        ResolvedGoto_label iters
      else
        ResolvedGoto_goto_invalid_label
    | [] -> failwith "impossible"
  in
  aux t []
