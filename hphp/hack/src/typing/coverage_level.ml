(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Coverage_level_defs
open Ide_api_types
open Typing_defs
open Utils
module TUtils = Typing_utils
module Reason = Typing_reason

(* This should be configurable by client command args... eventually*)
let sample_rate = 0

let display_limit = 10

let samples_limit = 5

let string_of_level = function
  | Checked -> "checked"
  | Partial -> "partial"
  | Unchecked -> "unchecked"

let empty_pos_stats_entry = { pos_count = 0; samples = [] }

let empty_level_stats_entry = { count = 0; reason_stats = SMap.empty }

let empty_counter =
  CLMap.empty
  |> CLMap.add Checked empty_level_stats_entry
  |> CLMap.add Partial empty_level_stats_entry
  |> CLMap.add Unchecked empty_level_stats_entry

(* This is highly unscientific and not really uniform sampling, but for
 * debugging purposes should be enough. *)
let merge_pos_stats_samples l1 l2 =
  let rec pick_n acc n m l =
    if n = 0 then
      acc
    else if m <= n then
      l @ acc
    else
      match l with
      | [] -> acc
      | h :: tl ->
        if Random.int m < n then
          pick_n (h :: acc) (n - 1) (m - 1) tl
        else
          pick_n acc n (m - 1) tl
  in
  pick_n [] samples_limit (List.length l1 + List.length l2) (l1 @ l2)

let add_sample_pos p samples = merge_pos_stats_samples samples [p]

let incr_reason_stats (ctx : Provider_context.t) r p reason_stats =
  if sample_rate = 0 || Random.int sample_rate <> 0 then
    reason_stats
  else
    let reason_pos = Reason.to_pos r |> Naming_provider.resolve_position ctx in
    let string_key = Reason.to_constructor_string r in
    let pos_stats_map =
      match SMap.find_opt string_key reason_stats with
      | Some x -> x
      | None -> Pos.Map.empty
    in
    let pos_stats =
      match Pos.Map.find_opt reason_pos pos_stats_map with
      | Some x -> x
      | None -> empty_pos_stats_entry
    in
    let pos_stats =
      {
        pos_count = pos_stats.pos_count + sample_rate;
        samples = add_sample_pos p pos_stats.samples;
      }
    in
    SMap.add
      string_key
      (Pos.Map.add reason_pos pos_stats pos_stats_map)
      reason_stats

let incr_counter (ctx : Provider_context.t) k (r, p, c) =
  let v = CLMap.find k c in
  CLMap.add
    k
    {
      count = v.count + 1;
      reason_stats = incr_reason_stats ctx r p v.reason_stats;
    }
    c

let merge_pos_stats p1 p2 =
  {
    pos_count = p1.pos_count + p2.pos_count;
    samples = merge_pos_stats_samples p1.samples p2.samples;
  }

let merge_reason_stats s1 s2 =
  SMap.merge
    (fun _ s1 s2 ->
      Option.merge s1 s2 ~f:(fun s1 s2 ->
          Pos.Map.merge
            (fun _ p1 p2 ->
              Option.merge p1 p2 ~f:(fun p1 p2 -> merge_pos_stats p1 p2))
            s1
            s2))
    s1
    s2

let merge_and_sum cs1 cs2 =
  CLMap.merge
    (fun _ c1 c2 ->
      Option.merge c1 c2 ~f:(fun c1 c2 ->
          {
            count = c1.count + c2.count;
            reason_stats = merge_reason_stats c1.reason_stats c2.reason_stats;
          }))
    cs1
    cs2

let rec is_tany env ty =
  let (env, ty) = Tast_env.expand_type env ty in
  match get_node ty with
  | Tany _
  | Terr ->
    (env, Some (get_reason ty))
  | Tunion [] -> (env, None)
  | Tunion (h :: tl) ->
    let (env, r_opt) = is_tany env h in
    (match r_opt with
    | Some r
      when List.for_all
             tl
             ~f:(compose Option.is_some (compose snd (is_tany env))) ->
      (env, Some r)
    | _ -> (env, None))
  | _ -> (env, None)

let level_of_type env fixme_map (pos, ty) =
  let (env, ty) = Tast_env.expand_type env ty in
  match get_node ty with
  | Tobject -> (env, Partial)
  | _ ->
    let (env, r_opt) = is_tany env ty in
    let level =
      match r_opt with
      | Some _ -> Unchecked
      | None ->
        (match TUtils.HasTany.check_why ty with
        | Some _ -> Partial
        | _ ->
          (* If the line has a HH_FIXME, then mark it as (at most) partially checked *)
          if IMap.mem (Pos.line pos) fixme_map then
            Partial
          else
            Checked)
    in
    (env, level)

class level_getter fixme_map =
  object
    inherit
      [coverage_level Pos.Map.t * checked_stats] Tast_visitor.reduce as super

    method zero = (Pos.Map.empty, { checked = 0; partial = 0; unchecked = 0 })

    method plus (pmap1, cmap1) (pmap2, cmap2) =
      ( Pos.Map.union pmap1 pmap2,
        {
          checked = cmap1.checked + cmap2.checked;
          partial = cmap1.partial + cmap2.partial;
          unchecked = cmap1.unchecked + cmap2.unchecked;
        } )

    method! on_expr env expr =
      (* This method performs the typical reduce operations on subsexpressions
       * yielding a map from positions to coverage levels of the expressions
       * at those positions, paired with a count of coverage level instances.
       * The map contains those positions we wish to track to show typing info.
       * Then it looks at the expression it is at and determines if it
       * something that should be tracked or not. Because
       * we want typing information to be informative, it only tracks
       * expressions that do not themselves contain worse-typed subexpressions.
       * This way large chunks of code that are poorly typed do not obscure
       * the root cause of their poor typing, which would be some poorly
       * typed subexpression. The count is then always incremented.
       *)
      let (pmap, cmap) = super#on_expr env expr in
      let ((pos, ty), _) = expr in
      let (_env, lvl) = level_of_type env fixme_map (pos, ty) in
      let should_update_pmap =
        match lvl with
        | Checked -> cmap.checked + cmap.partial + cmap.unchecked = 0
        | Partial -> cmap.partial + cmap.unchecked = 0
        | Unchecked -> cmap.unchecked = 0
      in
      let new_pmap =
        if should_update_pmap then
          Pos.Map.add pos lvl pmap
        else
          pmap
      in
      let new_cmap =
        match lvl with
        | Checked -> { cmap with checked = cmap.checked + 1 }
        | Partial -> { cmap with partial = cmap.partial + 1 }
        | Unchecked -> { cmap with unchecked = cmap.unchecked + 1 }
      in
      (new_pmap, new_cmap)
  end

let get_levels ctx tast check =
  let fixmes =
    Fixme_provider.get_hh_fixmes check |> Option.value ~default:IMap.empty
  in
  let lg = new level_getter fixmes in
  let (pmap, cmap) = lg#go ctx tast in
  Ok (Pos.Map.fold (fun p ty xs -> (Pos.to_absolute p, ty) :: xs) pmap [], cmap)

let get_percent counts =
  let nchecked = counts.checked in
  let nunchecked = counts.unchecked in
  let npartial = counts.partial in
  let ntotal = nchecked + nunchecked + npartial in
  if ntotal = 0 then
    100
  else
    ((nchecked * 100) + (npartial * 100)) / ntotal
