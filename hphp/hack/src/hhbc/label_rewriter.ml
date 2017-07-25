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
    | ILabel l -> (i, IMap.add (Label.id l) i m)
    | _        -> (i + 1, m)
    end)

let lookup_def l defs =
  match IMap.get l defs with
  | None -> failwith "lookup_def: label missing"
  | Some ix -> ix

(* Get any regular labels referenced by this instruction *)
let get_regular_labels instr =
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
  | IIterator (IterBreak (l, _))
  | ICall (DecodeCufIter (_, l))
  | IGenDelegation (YieldFromDelegate (_, l))
  | IContFlow (Jmp l | JmpNS l | JmpZ l | JmpNZ l) -> [l]
  | IContFlow (Switch (_, _, ls)) -> ls
  | IContFlow (SSwitch pairs) -> List.map pairs snd
  | _ -> []

(* Get any labels referred to in catch or fault handlers *)
let get_catch_or_fault_labels instr =
  match instr with
  | ITry (TryCatchLegacyBegin l | TryFaultBegin l) -> [l]
  | _ -> []

(* Generate new labels for all labels referenced in instructions and default
 * parameter values, in the same order as used by DumpHhas:
 *   1. First, labels referenced by normal control-flow (jumps, switches, etc)
 *   2. Next, labels referenced by catch or fault handlers
 *   3. Last, labels referenced by default parameter values
 *)
let create_label_ref_map defs params body =
  let process_ref (n, (used, refs) as acc) l =
    let l = Label.id l in
    let ix = lookup_def l defs in
    match IMap.get ix refs with
    (* This is the first time we've seen a reference to a label for
     * this instruction offset, so generate a new label *)
    | None -> (n + 1, (ISet.add l used, IMap.add ix n refs))
    (* We already have a label for this instruction offset *)
    | Some _ -> acc in
  let gather_using get_labels acc instrseq =
    InstrSeq.fold_left instrseq ~init:acc
    ~f:(fun acc instr ->
    List.fold_left (get_labels instr) ~init:acc ~f:process_ref) in
  let acc = (0, (ISet.empty, IMap.empty)) in
  let acc = gather_using get_regular_labels acc body in
  let acc = gather_using get_catch_or_fault_labels acc body in
  let acc =
    List.fold_left params ~init:acc
    ~f:(fun acc param ->
      match Hhas_param.default_value param with
       | None -> acc
       | Some (l, _) -> process_ref acc l) in
  snd acc

(* Relabel the instruction sequence and parameter values so that
 *   1. No instruction is preceded by more than one label
 *   2. No label is unreferenced
 *   3. References to labels occur in strict label number order, starting at 0
 *)
let rewrite_params_and_body defs used refs params body =
  let relabel_id l =
    let ix = lookup_def l defs in
    match IMap.get ix refs with
    | None -> failwith "relabel_instrseq: offset not in refs"
    | Some l' -> l' in
  (* Rewrite a label that's referenced by an instruction or parameter *)
  let relabel l = Label.map relabel_id l in
  (* Rewrite or remove a label definition *)
  let relabel_define_label_id id =
    if ISet.mem id used then IMap.get (lookup_def id defs) refs
    else None in
  (* Rewrite a single instruction *)
  let rewrite_instr instr =
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
    | IIterator (IterBreak (l, x)) ->
      Some (IIterator (IterBreak (relabel l, x)))
    | IGenDelegation (YieldFromDelegate (i, l)) ->
      Some (IGenDelegation (YieldFromDelegate (i, relabel l)))
    | ICall (DecodeCufIter (x, l)) ->
      Some (ICall (DecodeCufIter (x, relabel l)))
    | IContFlow (Jmp l)   -> Some (IContFlow (Jmp (relabel l)))
    | IContFlow (JmpNS l) -> Some (IContFlow (JmpNS (relabel l)))
    | IContFlow (JmpZ l)  -> Some (IContFlow (JmpZ (relabel l)))
    | IContFlow (JmpNZ l) -> Some (IContFlow (JmpNZ (relabel l)))
    | IContFlow (Switch (k, n, ll)) ->
      Some (IContFlow (Switch (k, n, List.map ll relabel)))
    | IContFlow (SSwitch pairs) ->
      Some (IContFlow (SSwitch
        (List.map pairs (fun (id,l) -> (id, relabel l)))))
    | ITry (TryCatchLegacyBegin l) ->
      Some (ITry (TryCatchLegacyBegin (relabel l)))
    | ITry (TryFaultBegin l) -> Some (ITry (TryFaultBegin (relabel l)))
    | ILabel l ->
      begin match Label.option_map relabel_define_label_id l with
      | None -> None
      | Some l' -> Some (ILabel l')
      end
    | _ -> Some instr in
    (* Rewrite any label referred to in a default value *)
    let rewrite_param param =
      let dv = Hhas_param.default_value param in
      match dv with
      | None -> param
      | Some (l, e) ->
        Hhas_param.make (Hhas_param.name param)
          (Hhas_param.is_reference param)
          (Hhas_param.is_variadic param)
          (Hhas_param.type_info param)
          (Some (relabel l, e)) in
    let params = List.map params rewrite_param in
    let body = InstrSeq.filter_map body ~f:rewrite_instr in
    (params, body)

let relabel_function params body =
  let defs = create_label_to_offset_map body in
  let used, refs = create_label_ref_map defs params body in
  rewrite_params_and_body defs used refs params body

let clone_with_fresh_regular_labels block =
  let regular_labels, named_labels =
    InstrSeq.fold_left block ~init: (IMap.empty, SMap.empty) ~f:
      begin fun ((regular, named) as acc) i ->
        match i with
        | ILabel ((Label.Regular _) as l) ->
          IMap.add (Label.id l) (Label.next_regular ()) regular, named
        | ILabel (Label.Named name) ->
          regular, SMap.add name (Label.next_regular ()) named
        | _ -> acc
      end
  in
  if IMap.is_empty regular_labels && SMap.is_empty named_labels then
    block
  else
  let relabel l =
    Option.value ~default:l @@
      match l with
      | Label.Regular id -> IMap.get id regular_labels
      | Label.Named name -> SMap.get name named_labels
      | _ -> None
  in
  let rewrite_instr instr =
    match instr with
    | IIterator (IterInit (id, l, v)) ->
      IIterator (IterInit (id, relabel l, v))
    | IIterator (IterInitK (id, l, k, v)) ->
      IIterator (IterInitK (id, relabel l, k, v))
    | IIterator (WIterInit (id, l, v)) ->
      IIterator (WIterInit (id, relabel l, v))
    | IIterator (WIterInitK (id, l, k, v)) ->
      IIterator (WIterInitK (id, relabel l, k, v))
    | IIterator (MIterInit (id, l, v)) ->
      IIterator (MIterInit (id, relabel l, v))
    | IIterator (MIterInitK (id, l, k, v)) ->
      IIterator (MIterInitK (id, relabel l, k, v))
    | IIterator (IterNext (id, l, v)) ->
      IIterator (IterNext (id, relabel l, v))
    | IIterator (IterNextK (id, l, k, v)) ->
      IIterator (IterNextK (id, relabel l, k, v))
    | IIterator (WIterNext (id, l, v)) ->
      IIterator (WIterNext (id, relabel l, v))
    | IIterator (WIterNextK (id, l, k, v)) ->
      IIterator (WIterNextK (id, relabel l, k, v))
    | IIterator (MIterNext (id, l, v)) ->
      IIterator (MIterNext (id, relabel l, v))
    | IIterator (MIterNextK (id, l, k, v)) ->
      IIterator (MIterNextK (id, relabel l, k, v))
    | IIterator (IterBreak (l, x)) ->
      IIterator (IterBreak (relabel l, x))
    | ICall (DecodeCufIter (x, l)) ->
      ICall (DecodeCufIter (x, relabel l))
    | IContFlow (Jmp l)   -> IContFlow (Jmp (relabel l))
    | IContFlow (JmpNS l) -> IContFlow (JmpNS (relabel l))
    | IContFlow (JmpZ l)  -> IContFlow (JmpZ (relabel l))
    | IContFlow (JmpNZ l) -> IContFlow (JmpNZ (relabel l))
    | IContFlow (Switch (k, n, ll)) ->
      IContFlow (Switch (k, n, List.map ll relabel))
    | IContFlow (SSwitch pairs) ->
      IContFlow (SSwitch
        (List.map pairs (fun (id,l) -> (id, relabel l))))
    | ILabel l -> ILabel (relabel l)
    | _ -> instr
  in
  InstrSeq.map block rewrite_instr
