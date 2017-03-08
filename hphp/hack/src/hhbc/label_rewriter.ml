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
open Hhbc_ast
open Instruction_sequence

(* Create a map from defined labels to instruction offset *)
let create_label_to_offset_map instrseq =
  snd @@
  InstrSeq.fold_left instrseq ~init:(0, IMap.empty) ~f:(fun (i, m) instr ->
    begin match instr with
    | ILabel (Label.Regular l) -> (i, IMap.add l i m)
    | _        -> (i + 1, m)
    end)

let lookup_def l defs =
  match IMap.get l defs with
  | None -> failwith "lookup_def: label missing"
  | Some ix -> ix

(* Generate new labels for all labels referenced in instructions, in the
 * order that the instructions appear. Also record which labels are
 *)
let create_label_ref_map defs instrseq =
  snd @@
  InstrSeq.fold_left instrseq ~init:(0, (ISet.empty, IMap.empty))
    ~f:(fun acc instr ->
    let process_ref (n, (used, refs) as acc) l =
      let l = Label.id l in
      let ix = lookup_def l defs in
      match IMap.get ix refs with
      (* This is the first time we've seen a reference to a label for
       * this instruction offset, so generate a new label *)
      | None -> (n + 1, (ISet.add l used, IMap.add ix n refs))
      (* We already have a label for this instruction offset *)
      | Some _ -> acc in
    match instr with
    | IIterator (IterInit (_, l, _))
    | IIterator (IterInitK (_, l, _, _))
    | IIterator (WIterInit (_, l, _))
    | IIterator (WIterInitK (_, l, _, _))
    | IIterator (MIterInit (_, l, _))
    | IIterator (MIterInitK (_, l, _, _))
    | IIterator (IterNext (_, l, _))
    | IIterator (IterNextK (_, l, _, _))
    | IIterator (WIterNext (_, l, _))
    | IIterator (WIterNextK (_, l, _, _))
    | IIterator (MIterNext (_, l, _))
    | IIterator (MIterNextK (_, l, _, _))
    | IContFlow (Jmp l | JmpNS l | JmpZ l | JmpNZ l) ->
      process_ref acc l
    | IContFlow (Switch (_, _, ls)) ->
      List.fold_left ls ~f:process_ref ~init:acc
    | IContFlow (SSwitch pairs) ->
      List.fold_left pairs ~f:(fun acc (_,l) -> process_ref acc l) ~init:acc
    (* TODO: other uses of Label.t in instructions:
      DecodeCufIter
      IterBreak
     *)
    | _ -> acc)

(* Relabel the instruction sequence so that
 *   1. No instruction is preceded by more than one label
 *   2. No label is unreferenced
 *   3. References to labels occur in strict label number order, starting at 0
 *)
let relabel_instrseq instrseq =
  let defs = create_label_to_offset_map instrseq in
  let used, refs = create_label_ref_map defs instrseq in
  let relabel l =
    let l = Label.id l in
    let ix = lookup_def l defs in
    match IMap.get ix refs with
    | None -> failwith "relabel_instrseq: offset not in refs"
    | Some l' -> Label.Regular l' in
  InstrSeq.filter_map instrseq ~f:(fun instr ->
    match instr with
    | IIterator (IterInit (id, l, v)) ->
      Some (IIterator (IterInit (id, relabel l, v)))
    | IIterator (IterInitK (id, l, k, v)) ->
      Some (IIterator (IterInitK (id, relabel l, k, v)))
    | IIterator (WIterInit (id, l, v)) ->
      Some (IIterator (WIterInit (id, relabel l, v)))
    | IIterator (WIterInitK (id, l, k, v)) ->
      Some (IIterator (WIterInitK (id, relabel l, k, v)))
    | IIterator (MIterInit (id, l, v)) ->
      Some (IIterator (MIterInit (id, relabel l, v)))
    | IIterator (MIterInitK (id, l, k, v)) ->
      Some (IIterator (MIterInitK (id, relabel l, k, v)))
    | IIterator (IterNext (id, l, v)) ->
      Some (IIterator (IterNext (id, relabel l, v)))
    | IIterator (IterNextK (id, l, k, v)) ->
      Some (IIterator (IterNextK (id, relabel l, k, v)))
    | IIterator (WIterNext (id, l, v)) ->
      Some (IIterator (WIterNext (id, relabel l, v)))
    | IIterator (WIterNextK (id, l, k, v)) ->
      Some (IIterator (WIterNextK (id, relabel l, k, v)))
    | IIterator (MIterNext (id, l, v)) ->
      Some (IIterator (MIterNext (id, relabel l, v)))
    | IIterator (MIterNextK (id, l, k, v)) ->
      Some (IIterator (MIterNextK (id, relabel l, k, v)))
    | IContFlow (Jmp l)   -> Some (IContFlow (Jmp (relabel l)))
    | IContFlow (JmpNS l) -> Some (IContFlow (JmpNS (relabel l)))
    | IContFlow (JmpZ l)  -> Some (IContFlow (JmpZ (relabel l)))
    | IContFlow (JmpNZ l) -> Some (IContFlow (JmpNZ (relabel l)))
    | IContFlow (Switch (k, n, ll)) ->
      Some (IContFlow (Switch (k, n, List.map ll relabel)))
    | IContFlow (SSwitch pairs) ->
      Some (IContFlow (SSwitch
        (List.map pairs (fun (id,l) -> (id, relabel l)))))
    (* TODO: other uses of Label.t in instructions:
      DecodeCufIter
      IterBreak
     *)
    | ILabel (Label.Regular l) ->
      (* TODO: Write test cases for things like catch and fault labels followed
      by loop start labels. *)
      if ISet.mem l used then
        let ix = lookup_def l defs in
        begin match IMap.get ix refs with
        | Some l' -> Some (ILabel (Label.Regular l'))
        | None -> None
        end
      else None
    | _ -> Some instr)
