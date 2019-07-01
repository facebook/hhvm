(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
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
  | IIterator (LIterInit (_, _, l, _))
  | IIterator (LIterInitK (_, _, l, _, _))
  | IIterator (IterNext (_, l, _))
  | IIterator (IterNextK (_, l, _, _))
  | IIterator (LIterNext (_, _, l, _))
  | IIterator (LIterNextK (_, _, l, _, _))
  | IIterator (IterBreak (l, _))
  | ICall (FCall ((_, _, _, _, Some l)))
  | ICall (FCallObjMethod ((_, _, _, _, Some l), _, _))
  | ICall (FCallObjMethodD ((_, _, _, _, Some l), _, _))
  | ICall (FCallObjMethodRD ((_, _, _, _, Some l), _, _))
  | IGenDelegation (YieldFromDelegate (_, l))
  | IMisc (MemoGet (l, _))
  | IContFlow (Jmp l | JmpNS l | JmpZ l | JmpNZ l) -> [l]
  | IContFlow (Switch (_, _, ls)) -> ls
  | IContFlow (SSwitch pairs) -> List.map pairs snd
  | IMisc (MemoGetEager (l1, l2, _)) -> [l1; l2]
  | _ -> []

(* Generate new labels for all labels referenced in instructions and default
 * parameter values, in the same order as used by DumpHhas:
 *   1. First, labels referenced by normal control-flow (jumps, switches, etc)
 *   2. Last, labels referenced by default parameter values
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
  let acc =
    List.fold_left params ~init:acc
      ~f:(fun acc param ->
        match Hhas_param.default_value param with
        | None -> acc
        | Some (l, _) -> process_ref acc l) in
  snd acc

let relabel_instr instr relabel =
  match instr with
  | IIterator (IterInit (id, l, v)) ->
    IIterator (IterInit (id, relabel l, v))
  | IIterator (IterInitK (id, l, k, v)) ->
    IIterator (IterInitK (id, relabel l, k, v))
  | IIterator (LIterInit (id, b, l, v)) ->
    IIterator (LIterInit (id, b, relabel l, v))
  | IIterator (LIterInitK (id, b, l, k, v)) ->
    IIterator (LIterInitK (id, b, relabel l, k, v))
  | IIterator (IterNext (id, l, v)) ->
    IIterator (IterNext (id, relabel l, v))
  | IIterator (IterNextK (id, l, k, v)) ->
    IIterator (IterNextK (id, relabel l, k, v))
  | IIterator (LIterNext (id, b, l, v)) ->
    IIterator (LIterNext (id, b, relabel l, v))
  | IIterator (LIterNextK (id, b, l, k, v)) ->
    IIterator (LIterNextK (id, b, relabel l, k, v))
  | IIterator (IterBreak (l, x)) ->
    IIterator (IterBreak (relabel l, x))
  | IGenDelegation (YieldFromDelegate (i, l)) ->
    IGenDelegation (YieldFromDelegate (i, relabel l))
  | ICall (FCall ((fl, na, nr, br, Some l))) ->
    ICall (FCall ((fl, na, nr, br, Some (relabel l))))
  | ICall (FCallObjMethod ((fl, na, nr, br, Some l), f, p)) ->
    ICall (FCallObjMethod ((fl, na, nr, br, Some (relabel l)), f, p))
  | ICall (FCallObjMethodD ((fl, na, nr, br, Some l), f, m)) ->
    ICall (FCallObjMethodD ((fl, na, nr, br, Some (relabel l)), f, m))
  | ICall (FCallObjMethodRD ((fl, na, nr, br, Some l), f, m)) ->
    ICall (FCallObjMethodRD ((fl, na, nr, br, Some (relabel l)), f, m))
  | IContFlow (Jmp l)   -> IContFlow (Jmp (relabel l))
  | IContFlow (JmpNS l) -> IContFlow (JmpNS (relabel l))
  | IContFlow (JmpZ l)  -> IContFlow (JmpZ (relabel l))
  | IContFlow (JmpNZ l) -> IContFlow (JmpNZ (relabel l))
  | IContFlow (Switch (k, n, ll)) ->
    IContFlow (Switch (k, n, List.map ll relabel))
  | IContFlow (SSwitch pairs) ->
    IContFlow (SSwitch
     (List.map pairs (fun (id,l) -> (id, relabel l))))
  | IMisc (MemoGet (l, r)) ->
    IMisc (MemoGet (relabel l, r))
  | IMisc (MemoGetEager (l1, l2, r)) ->
    IMisc (MemoGetEager (relabel l1, relabel l2, r))
  | ILabel l -> ILabel (relabel l)
  | _ -> instr

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
    | ILabel l ->
      begin match Label.option_map relabel_define_label_id l with
      | None -> None
      | Some l' -> Some (ILabel l')
      end
    | _ -> Some (relabel_instr instr relabel)
  in
  (* Rewrite any label referred to in a default value *)
  let rewrite_param param =
    let dv = Hhas_param.default_value param in
    match dv with
    | None -> param
    | Some (l, e) ->
      Hhas_param.make (Hhas_param.name param)
        (Hhas_param.is_reference param)
        (Hhas_param.is_variadic param)
        (Hhas_param.is_inout param)
        (Hhas_param.user_attributes param)
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
  let rewrite_instr instr = relabel_instr instr relabel in
  InstrSeq.map block rewrite_instr
