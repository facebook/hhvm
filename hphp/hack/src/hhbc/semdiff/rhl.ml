(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(*
This is a really dumbed-down RHL prover for showing equivalence of two
bytecode function bodies. The assertion language is just conjunctions of
equalities between local variables on the two sides, and there's no proper
fixed point iteration at all. Still, it should cope with different labels,
different uses of locals and some simple variations in control-flow
*)
open Core
open Hhbc_ast
open Local
open Hhbc_destruct
module Log = Semdiff_logging

(* Refs storing the adata for the two programs; they're written in semdiff
   and accessed in equiv
*)
let adata1_ref = ref ([] : Hhas_adata.t list)
let adata2_ref = ref ([] : Hhas_adata.t list)

(* Ref keeping to-do set for pairs of classes that
   need to be compared. Originally just for closure classes, now
   for all corresponding pairs
   Also comes with a permutation for matching up properties used in
   closure classes*)
type perm = (int * int) list
module IntIntPermSet = Set.Make(struct type t = int*int*perm let compare = compare end)
module IntIntSet = Set.Make(struct type t = int*int let compare = compare end)
let classes_to_check = ref IntIntPermSet.empty
let classes_checked = ref IntIntSet.empty

let functions_to_check = ref IntIntSet.empty
let functions_checked = ref IntIntSet.empty

let rec lookup_adata id data_dict =
match data_dict with
 | [] -> failwith "adata lookup failed"
 | ad :: rest -> if Hhas_adata.id ad = id
                 then Hhas_adata.value ad
                 else lookup_adata id rest

(* an individual prop is an equation between local variables
  To start with this means that they are both defined and equal
  or that they are both undefined
  We can usefully use more refined relations, but let's try
  the simplest for now
*)
type prop = Local.t * Local.t
module PropSet = Set.Make(struct type t = prop let compare = compare end)
module VarSet = Set.Make(struct type t = Local.t let compare = compare end)

(* along with pc, we track the current exception handler context
   this should be understood as a list of which exception handlers
   we are currently (dynamically) already within. The static part
   is dealt with via parents *)
type exnmapvalue = Fault_handler of int | Catch_handler of int
let ip_of_emv emv = match emv with
  | Fault_handler n
  | Catch_handler n -> n

module EMVMap = MyMap.Make(struct type t = exnmapvalue let compare = compare end)
type handlerstack = exnmapvalue list

type epc = handlerstack * int
let succ ((hs,pc) : epc) = (hs,pc+1)
let ip_of_pc (_hs,pc) = pc
let hs_of_pc (hs,_pc) = hs

module PcpMap = MyMap.Make(struct type t = epc*epc let compare=compare end)

(* now an assertion is a set of props, read as conjunction
   paired with two sets of local variables
   The reading is
   s,s' \in [[props, vs, vs']] iff
    \forall (v,v')\in props, s v = s' v' /\
    \forall v\in Vars\vs, s v = unset /\
    \forall v'\in Vars\vs', s' v' = unset
   *)
type assertion = PropSet.t * VarSet.t * VarSet.t
let (entry_assertion : assertion) = (PropSet.empty,VarSet.empty,VarSet.empty)
module AsnSet = Set.Make(struct type t = assertion let compare=compare end)

exception Labelexn
module  LabelMap = MyMap.Make(struct type t = Label.t let compare = compare end)

(* Refactored version of exception table structures, to improve efficiency a bit
   and to cope with new try-catch-end structure, which doesn't have explicit
   labels for the handlers
   First pass constructs labelmap and a map from the indices of TryCatchBegin
   instructions to the index of their matching TryCatchMiddle
*)
type tcstackentry = Stack_label of Label.t | Stack_tc of int
let make_label_try_maps prog =
 let rec loop p n trycatchstack labelmap trymap =
  match p with
  | [] -> (labelmap, trymap)
  | i :: is -> (match i with
     | ILabel l ->
         loop is (n+1) trycatchstack (LabelMap.add l n labelmap) trymap
     | ITry (TryCatchLegacyBegin l)
     | ITry (TryFaultBegin l) ->
         loop is (n+1) (Stack_label l :: trycatchstack) labelmap trymap
     | ITry TryCatchBegin ->
         loop is (n+1) (Stack_tc n :: trycatchstack) labelmap trymap
     | ITry TryCatchMiddle ->
         (match trycatchstack with
           | Stack_tc m :: rest ->
             loop is (n+1) rest labelmap (IMap.add m n trymap)
           | _ -> raise Labelexn
           )
     | ITry TryCatchLegacyEnd
     | ITry TryFaultEnd ->
         (match trycatchstack with
           | Stack_label _l :: rest ->
              loop is (n+1) rest labelmap trymap
           | _ -> raise Labelexn)
     (* Note that we do nothing special for TryCatchEnd, which seems a useless
        instruction *)
     | _ -> loop is (n+1) trycatchstack labelmap trymap) in
  loop prog 0 [] LabelMap.empty IMap.empty

(* Second pass constructs exception table. Revised version maps instruction indices to
   a list of handler indices. Previous parent relation is removed, as it doesn't track
   the right information.
   Still a question mark over whether the whole stack needs to go into the handler lists
   or if it should be somehow truncated when we reach a catch handler. For now, I'm trying
   with remembering everything, though I suspect that might lead to runaway stacks.
*)

let make_exntable prog labelmap trymap =
 let rec loop p n trycatchstack exnmap =
  match p with
  | [] -> exnmap
  | i::is ->
    let newexnmap = IMap.add n trycatchstack (* <- filter this? *) exnmap in
   (match i with
    | ITry (TryCatchLegacyBegin (Label.Catch _ as l)) ->
       let nl = LabelMap.find l labelmap in
       loop is (n+1) (Catch_handler nl :: trycatchstack) newexnmap

    | ITry (TryCatchLegacyBegin _) -> raise Labelexn

    | ITry (TryFaultBegin (Label.Fault _ as l)) ->
       let nl = LabelMap.find l labelmap in
       loop is (n+1) (Fault_handler nl :: trycatchstack) newexnmap

    | ITry (TryFaultBegin _) -> raise Labelexn

    | ITry TryCatchBegin ->
       let nl = IMap.find n trymap in (* find corresponding middle *)
       loop is (n+1) (Catch_handler nl :: trycatchstack) newexnmap

    | ITry TryCatchMiddle ->
        (match trycatchstack with
          | Catch_handler _ :: rest -> loop is (n+1) rest newexnmap
          | _ -> raise Labelexn)

    | ITry TryFaultEnd ->
        (match trycatchstack with
          | Fault_handler _ :: rest -> loop is (n+1) rest newexnmap
          | _ -> raise Labelexn)

    | ITry TryCatchLegacyEnd ->
        (match trycatchstack with
          | Catch_handler _ :: rest -> loop is (n+1) rest newexnmap
          | _ -> raise Labelexn)
   | _ -> loop is (n+1) trycatchstack newexnmap)
   in loop prog 0 [] IMap.empty

(* Moving string functions into rhl so that I can use them in debugging *)

let propstostring props = String.concat " "
(List.map ~f:(fun (v,v') -> "(" ^ (Hhbc_hhas.string_of_local_id v) ^ "," ^
 (Hhbc_hhas.string_of_local_id v') ^ ")") (PropSet.elements props))

let varsettostring vs = "{" ^ String.concat ","
  (List.map ~f:(fun v -> Hhbc_hhas.string_of_local_id v) (VarSet.elements vs)) ^
  "}"

let asntostring (props,vs,vs') = propstostring props ^ varsettostring vs ^ varsettostring vs'

let asnsettostring asns = "<" ^ String.concat ","
 (List.map ~f:asntostring (AsnSet.elements asns)) ^ ">"

let string_of_pc (hs,ip) = String.concat " "
 (List.map ~f:(fun h -> string_of_int (ip_of_emv h)) hs)
                           ^ ";" ^ string_of_int ip
let labasnstostring ((l1,l2),asns) = "[" ^ (string_of_pc l1) ^ "," ^
  (string_of_pc l2) ^ "->" ^ (asnsettostring asns) ^ "]\n"
let labasntostring ((l1,l2),asns) = "[" ^ (string_of_pc l1) ^ "," ^
    (string_of_pc l2) ^ "->" ^ (asntostring asns) ^ "]\n"
let labasnlisttostring l = String.concat "" (List.map ~f:labasntostring l)
let labasnsmaptostring asnmap = String.concat ""
 (List.map ~f:labasnstostring (PcpMap.bindings asnmap))

(* add equality between v1 and v2 to an assertion
   removing any existing relation between them *)
let addeq_asn v1 v2 (props,vs,vs') =
  let stripped = PropSet.filter (fun (x1,x2) -> x1 != v1 && x2 != v2) props in
      (PropSet.add (v1,v2) stripped, VarSet.add v1 vs, VarSet.add v2 vs')

(* Unset both v1 and v2, could remove overlap with above and make one-sided *)
let addunseteq_asn v1 v2 (props,vs,vs') =
  let stripped = PropSet.filter (fun (x1,x2) -> x1 != v1 && x2 != v2) props in
    (stripped, VarSet.add v1 vs, VarSet.add v2 vs')

(* simple-minded entailment between assertions *)
let entails_asns (props2,vs2,vs2') (props1,vs1,vs1') =
  (PropSet.for_all (fun ((v,v') as prop) -> PropSet.mem prop props2
                    || not (VarSet.mem v vs2 || VarSet.mem v' vs2')) props1)
  &&
  VarSet.subset vs2 vs1
  &&
  VarSet.subset vs2' vs1'


(* need to deal with the many local-manipulating instructions
   Want to know when two instructions are equal up to an assertion
   and also to return a modified assertion in case that holds
   Note that we only track unnamed locals
*)
let asn_entails_equal (props,vs,vs') l l' =
 PropSet.mem (l,l') props
 || not (VarSet.mem l vs || VarSet.mem l' vs')

let reads asn l l' =
 match l, l' with
  | Named s, Named s' -> if s=s' then Some asn else None
  | Unnamed _, Unnamed _ ->
  if asn_entails_equal asn l l'
                 then Some asn
                 else None
  | _, _ -> None

let check_instruct_get asn i i' =
match i, i' with
 | CGetL l, CGetL l'
 | CGetQuietL l, CGetQuietL l'
 | CGetL2 l, CGetL2 l'
 | CGetL3 l, CGetL3 l'
 | CUGetL l, CUGetL l'
 | PushL l, PushL l' (* TODO: this also unsets but don't track that yet *)
    -> reads asn l l' (* these instructions read locals *)
 | ClsRefGetL (l,cr), ClsRefGetL (l',cr') ->
   if cr = cr' then reads asn l l' else None
 | VGetL (Local.Named s), VGetL (Local.Named s')
   when s=s' -> Some asn
 | VGetL _, _
 | _, VGetL _ -> None (* can't handle the possible  aliasing here, so bail *)
 (* default case, require literally equal instructions *)
 | _, _ -> if i = i' then Some asn else None

 let check_instruct_isset asn i i' =
 match i, i' with
 | IssetL l, IssetL l'
 | EmptyL l, EmptyL l'
  -> reads asn l l'
 | IsTypeL (l,op), IsTypeL (l',op') ->
   if op = op' then reads asn l l' else None
 | _,_ -> if i=i' then Some asn else None

(* TODO: allow one-sided writes to dead variables - this shows up
  in one of the tests *)
let writes asn l l' =
match l, l' with
 | Named s, Named s' -> if s=s' then Some asn else None
 | Unnamed _, Unnamed _ ->
    Some (addeq_asn l l' asn)
 | _, _ -> None

(* We could be a bit more refined in tracking set/unset status of named locals
   but it might not make much difference, so leaving it out for now
*)
 let writesunset asn l l' =
 match l, l' with
  | Named s, Named s' -> if s=s' then Some asn else None
  | Unnamed _, Unnamed _ ->
     Some (addunseteq_asn l l' asn)
  | _, _ -> None

let check_instruct_mutator asn i i' =
 match i, i' with
  | SetL l, SetL l'
  | BindL l, BindL l'
   -> writes asn l l'
  | UnsetL l, UnsetL l'
    -> writesunset asn l l'
  | SetOpL (l,op), SetOpL (l',op') ->
     if op=op' then
      match reads asn l l' with
       | None -> None
       | Some newasn -> writes newasn l l' (* actually, newasn=asn, of course *)
     else None
     (* that's something that both reads and writes *)
  | IncDecL (l,op), IncDecL (l',op') ->
    if op=op' then
    match reads asn l l' with
     | None -> None
     | Some newasn -> writes newasn l l'
    else None
  | _,_ -> if i=i' then Some asn else None

let check_instruct_call asn i i' =
 match i, i' with
  | FPassL (param_id,Local.Named s), FPassL (param_id', Local.Named s')
    when param_id=param_id' && s=s' -> Some asn
  | FPassL (_,_), _
  | _, FPassL (_,_) -> None (* if this is pass by reference, might get aliasing
                               so just wimp out for now *)
  | _,_ -> if i=i' then Some asn else None

let check_instruct_base asn i i' =
 match i,i' with
  | BaseNL (l,op), BaseNL (l',op') ->
    if op=op' then reads asn l l'
    else None
    (* All these depend on the string names of locals never being the ones
    we're tracking with the analysis *)
  | FPassBaseNL (n,l), FPassBaseNL (n',l') ->
     if n=n' then reads asn l l'
     else None
  | BaseGL (l,mode), BaseGL(l',mode') ->
     if mode = mode' then reads asn l l'
     else None (* don't really know if this is right *)
  | FPassBaseGL (n,l), FPassBaseGL (n',l') ->
     if n=n' then reads asn l l'
     else None
  | BaseSL (l,n), BaseSL (l',n') ->
     if n=n' then reads asn l l'
     else None
  | BaseL (l,mode), BaseL (l',mode') ->
     if mode=mode' then reads asn l l'
     else None
  | FPassBaseL (n,l), FPassBaseL (n',l') ->
    if n = n' then reads asn l l'
    else None
  | Dim (mode,MemberKey.EL l), Dim (mode',MemberKey.EL l')
  | Dim (mode,MemberKey.PL l), Dim (mode',MemberKey.PL l')
    when mode=mode' -> reads asn l l'
    | FPassDim(n,MemberKey.EL l), FPassDim(n',MemberKey.EL l')
  | FPassDim(n,MemberKey.PL l), FPassDim(n',MemberKey.PL l')
    when n=n' -> reads asn l l'
  | _, _ -> if i=i' then Some asn else None

let check_instruct_final asn i i' =
  match i, i' with
   | SetWithRefLML (l1,l2), SetWithRefLML (l1',l2') ->
      (match reads asn l1 l1' with
        | None -> None
        | Some newasn -> reads newasn l2 l2')
   (* TODO: abstraction, we've heard of it *)
   | QueryM (n,op,MemberKey.EL l), QueryM (n',op',MemberKey.EL l')
   | QueryM (n,op,MemberKey.PL l), QueryM (n',op',MemberKey.PL l')
    when n=n' && op=op' -> reads asn l l'
   | VGetM (n, MemberKey.EL l), VGetM (n', MemberKey.EL l')
   | VGetM (n, MemberKey.PL l), VGetM (n', MemberKey.PL l')
    when n=n' -> reads asn l l'
   | FPassM (m,n,MemberKey.EL l), FPassM (m',n',MemberKey.EL l')
   | FPassM (m,n,MemberKey.PL l), FPassM (m',n',MemberKey.PL l')
    when m=m' && n=n' -> reads asn l l'
   | SetM (n, MemberKey.EL l), SetM (n', MemberKey.EL l')
   | SetM (n, MemberKey.PL l), SetM (n', MemberKey.PL l')
    when n=n' -> reads asn l l'
   | IncDecM (m,op,MemberKey.EL l), IncDecM (m',op',MemberKey.EL l')
   | IncDecM (m,op,MemberKey.PL l), IncDecM (m',op',MemberKey.PL l')
    when m=m' && op=op' -> reads asn l l'
   | SetOpM (m,op,MemberKey.EL l), SetOpM (m',op',MemberKey.EL l')
   | SetOpM (m,op,MemberKey.PL l), SetOpM (m',op',MemberKey.PL l')
    when m=m' && op=op' -> reads asn l l'
   | BindM (n, MemberKey.EL l), BindM (n', MemberKey.EL l')
   | BindM (n, MemberKey.PL l), BindM (n', MemberKey.PL l')
    when n=n' -> reads asn l l'
   | UnsetM (n, MemberKey.EL l), UnsetM (n', MemberKey.EL l')
   | UnsetM (n, MemberKey.PL l), UnsetM (n', MemberKey.PL l')
    when n=n' -> reads asn l l'
    (* I'm guessing a bit here! *)
   | SetWithRefRML l, SetWithRefRML l' -> reads asn l l'
   | _, _ -> if i=i' then Some asn else None

(* Iterators. My understanding is that the initializers either jump to the
specified label with no access to locals, or write the first value of the
iteration to the locals given in the instruction. Since this is control-flow,
we need to return further stuff to check, rather than just the newprops that
will hold for the next instruction
*)
let check_instruct_iterator asn i i' =
 match i, i' with
  | IterInit (it,lab,l), IterInit (it',lab',l')
  | WIterInit (it,lab,l), WIterInit (it',lab',l')
  | MIterInit (it,lab,l), MIterInit (it',lab',l')
  | IterNext (it,lab,l), IterNext (it',lab',l')
  | WIterNext (it,lab,l), WIterNext (it',lab',l')
  | MIterNext (it,lab,l), MIterNext (it',lab',l')  ->
    if it = it' (* not tracking correspondence between iterators yet *)
    then (writes asn l l', (* next instruction's state *)
          [((lab,lab'),asn)])  (* additional assertions to check *)
    else (None,[]) (* fail *)
  | IterInitK (it,lab,l1,l2), IterInitK (it',lab',l1',l2')
  | WIterInitK (it,lab,l1,l2), WIterInitK (it',lab',l1',l2')
  | MIterInitK (it,lab,l1,l2), MIterInitK (it',lab',l1',l2')
  | IterNextK (it,lab,l1,l2), IterNextK (it',lab',l1',l2')
  | WIterNextK (it,lab,l1,l2), WIterNextK (it',lab',l1',l2')
  | MIterNextK (it,lab,l1,l2), MIterNextK (it',lab',l1',l2')  ->
    if it = it'
    then match writes asn l1 l1' with
           | None -> (None,[])
           | Some newasn ->
             (writes newasn l2 l2', (* wrong if same local?? *)
              [((lab,lab'),asn)])
    else (None,[]) (* fail *)
  | IterBreak (_,_) , _
  | _ , IterBreak (_,_) -> (None,[]) (* should have been dealt with
                                        along with other control flow *)
  | _ , _ -> if i=i' then (Some asn,[]) else (None,[])

let check_instruct_misc asn i i' =
 match i,i' with
  | InitThisLoc l, InitThisLoc l' ->
      writes asn l l'
  | StaticLocCheck (l,str), StaticLocCheck (l',str')
  | StaticLocDef (l,str), StaticLocDef (l',str')
  | StaticLocInit (l,str), StaticLocInit (l',str') ->
     if str=str' then writes asn l l'
     else None
  | AssertRATL (_l,_rat), AssertRATL (_l',_rat') ->
     Some asn (* Think this is a noop for us, could do something different *)
  | Silence (l, Start), Silence(l',Start) ->
     writes asn l l'
  | Silence (l, End), Silence(l',End) ->
     reads asn l l'
  | GetMemoKeyL (Local.Named s), GetMemoKeyL (Local.Named s')
    when s = s' -> Some asn
  | GetMemoKeyL _, _
  | _, GetMemoKeyL _ -> None (* wimp out if not same named local *)
  | MemoSet (count, Local.Unnamed first, local_count),
    MemoSet(count', Local.Unnamed first', local_count')
    when count=count' && local_count = local_count' ->
      let rec loop loop_asn local local' count =
       match reads loop_asn (Local.Unnamed local) (Local.Unnamed local') with
        | None -> None
        | Some new_asn ->
           if count = 1 then Some new_asn
           else loop new_asn (local+1) (local' + 1) (count - 1)
       in loop asn first first' local_count
  | MemoSet (_,_,_), _
  | _, MemoSet (_,_,_) -> None
  | MemoGet (count, Local.Unnamed first, local_count),
    MemoGet(count', Local.Unnamed first', local_count')
    when count=count' && local_count = local_count' ->
      let rec loop loop_asn local local' count =
       match reads loop_asn (Local.Unnamed local) (Local.Unnamed local') with
        | None -> None
        | Some new_asn ->
           if count = 1 then Some new_asn
           else loop new_asn (local+1) (local' + 1) (count - 1)
       in loop asn first first' local_count
   (* yes, I did just copy-paste there. Should combine patterns !*)
  | MemoGet (_,_,_), _
  | _, MemoGet(_,_,_) -> None (* wimp out again *)
  | CreateCl(npars,cln), CreateCl(npars',cln') ->
   if npars = npars' then
     (classes_to_check := IntIntPermSet.add (cln,cln',[]) (!classes_to_check);
     Some asn)
   else None (* fail in this case *)
  | _, _ -> if i=i' then Some asn else None

(* abstracting this out in case we want to change it from a list later *)
let add_todo (pc,pc') asn todo = ((pc,pc'),asn) :: todo

let lookup_assumption (pc,pc') assumed =
 match PcpMap.get (pc,pc') assumed with
  | None -> AsnSet.empty
  | Some asns -> asns

let add_assumption (pc,pc') asn assumed =
  let prev = lookup_assumption (pc,pc') assumed in
  let updated = AsnSet.add asn prev in (* this is a clumsy union *)
  PcpMap.add (pc,pc') updated assumed

(* compute permutation, not very efficiently, but lists should always be very small *)
let findperm l1 l2 =
 if List.contains_dup l1 || (List.length l1 != List.length l2)
 then None
 else let rec loop n l p =
        match l with
         | [] -> Some p
         | x :: xs -> (match List.findi l2 (fun _i y -> x = y) with
                        | Some (j,_) -> loop (n+1) xs ((n,j)::p)
                        | None -> None)
      in loop 0 l1 []

(* Main entry point for equivalence checking *)
let equiv prog prog' startlabelpairs =
 let (labelmap, trymap) = make_label_try_maps prog in
 let exnmap = make_exntable prog labelmap trymap in
 let (labelmap', trymap') = make_label_try_maps prog' in
 let exnmap' = make_exntable prog' labelmap' trymap' in
 let prog_array = Array.of_list prog in
 let prog_array' = Array.of_list prog' in

 let rec check pc pc' ((props,vs,vs') as asn) assumed todo =
   let try_specials () = specials pc pc' asn assumed todo in

   (* This could be more one-sided, but can't allow one side to leave the
      frame and the other to go to a handler.
      Still, seems no real reason to require the same kind of
      handler on both sides
    *)
   let exceptional_todo () =
    match IMap.get (ip_of_pc pc) exnmap, IMap.get (ip_of_pc pc') exnmap' with
     | None, None -> Some todo (* should the imap be total? *)
     | Some [], Some [] -> Some todo (* no local handlers, so nothing to do *)
     | Some (Fault_handler h :: rest), Some (Fault_handler h' :: rest') ->
        let epc = (Fault_handler h :: (rest @ hs_of_pc pc), h) in
        let epc' = (Fault_handler h' :: (rest' @ hs_of_pc pc'), h') in
         Some (add_todo (epc,epc') asn todo)
     | Some (Catch_handler h :: _rest), Some (Catch_handler h' :: _rest') ->
       (* TODO: I *think* dropping everything except the catch handler at this point is right
                but am not entirely sure - hs_of_pc pc is just gone
       *)
        let epc = ([Catch_handler h], h) in
        let epc' = ([Catch_handler h'], h') in
         Some (add_todo (epc,epc') asn todo)
     | _,_ -> None
      (* here we've got a mismatch between the handlers on the two sides
         so we return None so that our caller can decide what to do
      *)
    in

   let nextins () =
    match exceptional_todo () with
     | Some newtodo ->
        check (succ pc) (succ pc') asn
              (add_assumption (pc,pc') asn assumed) newtodo
     | None -> try_specials ()
   in

   let nextinsnewasn newasn =
    match exceptional_todo () with
     | Some newtodo ->
        check (succ pc) (succ pc') newasn
              (add_assumption (pc,pc') asn assumed) newtodo
     | None -> try_specials ()
   in

   if List.length (hs_of_pc pc) > 10 (* arbitrary limit *)
   then (Log.error ~level:0 (Tty.Normal Tty.Red) ("runaway: " ^ string_of_pc pc);
        Some (pc, pc', asn, assumed, todo)) (* fail, dump state *)
   else
    let previous_assumptions = lookup_assumption (pc,pc') assumed in
    if AsnSet.exists (fun assasn -> entails_asns asn assasn) previous_assumptions
    (* that's a clumsy attempt at entailment asn => \bigcup prev_asses *)
    then donext assumed todo
    else
      if AsnSet.cardinal previous_assumptions > 2 (* arbitrary bound *)
      then try_specials ()
      else
       let i = prog_array.(ip_of_pc pc) in
       let i' = prog_array'.(ip_of_pc pc') in
       match i, i' with
        (* one-sided stuff for jumps, labels, comments *)
        | IContFlow(Jmp lab), _
        | IContFlow(JmpNS lab), _ ->
         check (hs_of_pc pc, LabelMap.find lab labelmap) pc' asn
               (add_assumption (pc,pc') asn assumed) todo
        | ITry _, _
        | ILabel _, _
        | IComment _, _ ->
           check (succ pc) pc' asn (add_assumption (pc,pc') asn assumed) todo

        | _, IContFlow(Jmp lab')
        | _, IContFlow(JmpNS lab') ->
              check pc (hs_of_pc pc', LabelMap.find lab' labelmap') asn
                    (add_assumption (pc,pc') asn assumed) todo
        | _, ITry _
        | _, ILabel _
        | _, IComment _ ->
              check pc (succ pc') asn (add_assumption (pc,pc') asn assumed) todo
        (* For IterBreak, the lists have to be the same as we're not tracking correspondences
           between iterators at the moment, and need the same freeing behaviour so exceptions
           match up if we try to use them later *)
        | IIterator (IterBreak (lab, it_list)), IIterator (IterBreak (lab', it_list')) ->
              if it_list = it_list'
              then check (hs_of_pc pc, LabelMap.find lab labelmap)
                         (hs_of_pc pc', LabelMap.find lab' labelmap') asn
                         (add_assumption (pc,pc') asn assumed) todo
              else try_specials ()
        | IContFlow (JmpZ lab), IContFlow (JmpZ lab')
        | IContFlow (JmpNZ lab), IContFlow (JmpNZ lab') ->
           check (succ pc) (succ pc') asn
             (add_assumption (pc,pc') asn assumed)
             (add_todo ((hs_of_pc pc, LabelMap.find lab labelmap),
                (hs_of_pc pc', LabelMap.find lab' labelmap')) asn todo)
        | IContFlow RetC, IContFlow RetC
        | IContFlow RetV, IContFlow RetV ->
           donext assumed todo
        | IContFlow Unwind, IContFlow Unwind ->
           (* TODO: this should be one side at a time, except it's
            a bit messy to deal with the case where we leave this
            frame altogether, in which case the two should agree, so
            I'm only dealing with the matching case
            *)
            (match hs_of_pc pc, hs_of_pc pc' with
               | [],[] -> (Log.debug (Tty.Normal Tty.Red) "unwind not in handler"; try_specials ())
                                               (* unwind not in handler! should be hard failure? *)
               | [Fault_handler _h], [Fault_handler _h'] -> donext assumed todo (* both jump out *)
               | (Fault_handler _h :: Fault_handler next :: hs),
                 (Fault_handler _h' :: Fault_handler next' :: hs') ->
                 check (Fault_handler next :: hs, next) (Fault_handler next' :: hs', next') asn
                       (add_assumption (pc,pc') asn assumed) todo
               | (Fault_handler _h :: Catch_handler next :: _hs),
                 (Fault_handler _h' :: Catch_handler next' :: _hs') ->
                  check ([Catch_handler next] , next) ([Catch_handler next'], next') asn
                        (add_assumption (pc,pc') asn assumed) todo
               | _, _ -> try_specials ())
        | IContFlow Throw, IContFlow Throw ->
            (match IMap.get (ip_of_pc pc) exnmap,
                   IMap.get (ip_of_pc pc') exnmap' with
             | None, None ->  donext assumed todo (* both leave *)
             | Some [], Some [] -> donext assumed todo
             | Some (Fault_handler h as handler :: rest),
               Some (Fault_handler h' as handler' :: rest') ->
              let newstack = handler :: (rest @ hs_of_pc pc) in
              let newstack' = handler' :: (rest' @ hs_of_pc pc') in
              check (newstack, h) (newstack', h')
                    asn (add_assumption (pc,pc') asn assumed)
                    todo
             | Some (Catch_handler h as handler :: _rest),
               Some (Catch_handler h' as handler' :: _rest') ->
               check ([handler],h) ([handler'], h')
                     asn (add_assumption (pc,pc') asn assumed)
                     todo
             | _,_ -> try_specials ())
        | IContFlow (Switch (kind, offset, labs)), IContFlow (Switch (kind', offset', labs'))
          when kind=kind' && offset=offset' ->
           (match List.zip labs labs' with
             | None -> try_specials () (* feebly, give up if different lengths *)
             | Some lab_pairs ->
                let hs = hs_of_pc pc in
                let hs' = hs_of_pc pc' in
                let newtodo = List.fold_right ~f:(fun (lab,lab') accum ->
                 add_todo ((hs, LabelMap.find lab labelmap), (hs', LabelMap.find lab' labelmap'))
                          asn accum) ~init:todo lab_pairs in
                donext (add_assumption (pc,pc') asn assumed) newtodo)
        | IContFlow (SSwitch _), _
        | _, IContFlow (SSwitch _) ->
          failwith "SSwitch not implemented"
        | IContFlow _, IContFlow _ -> try_specials ()
        (* Special treatment for Unset instructions *)
        | IMutator (UnsetL l), _ ->
           let newprops = PropSet.filter (fun (x1,_x2) -> x1 != l) props in
           let newasn = (newprops, VarSet.remove l vs, vs') in
             check (succ pc) pc' newasn (add_assumption (pc,pc') asn assumed) todo
        | _, IMutator (UnsetL l') ->
           let newprops = PropSet.filter (fun (_x1,x2) -> x2 != l') props in
           let newasn = (newprops, vs, VarSet.remove l' vs') in
             check pc (succ pc') newasn (add_assumption (pc,pc') asn assumed) todo
        (* next block have no interesting controls flow or local
           variable effects
        *)
        | IBasic ins, IBasic ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        | ILitConst (Array id), ILitConst (Array id')
        | ILitConst (Dict id), ILitConst (Dict id')
        | ILitConst (Vec id), ILitConst (Vec id')
        | ILitConst (Keyset id), ILitConst (Keyset id') ->
          let tv = lookup_adata id (!adata1_ref) in
          let tv' = lookup_adata id' (!adata2_ref) in
          if tv = tv' then nextins()
          else try_specials () (* TODO: Log the differences somewhere *)
        | ILitConst (Double s), ILitConst (Double s') ->
          (match Scanf.sscanf s "%f" (fun x -> Some x),
                 Scanf.sscanf s' "%f" (fun x -> Some x) with
           Some f, Some f' -> if f = f' then nextins() else try_specials ()
           | exception _ -> try_specials ()
           | _ -> try_specials ())
        | ILitConst ins, ILitConst ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        (* special cases for exiting the whole program *)
        | IOp Hhbc_ast.Exit, IOp Hhbc_ast.Exit ->
           donext assumed todo
        | IOp (Fatal op), IOp (Fatal op') ->
           if op=op' then donext assumed todo
           else try_specials ()
        | IOp ins, IOp ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        | ISpecialFlow ins, ISpecialFlow ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        | IAsync ins, IAsync ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        | IGenerator ins, IGenerator ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        | IIncludeEvalDefine (DefCls cid), IIncludeEvalDefine (DefCls cid') ->
            classes_to_check := IntIntPermSet.add (cid,cid',[]) !classes_to_check;
            nextins ()
        | IIncludeEvalDefine (DefFunc fid), IIncludeEvalDefine (DefFunc fid') ->
            functions_to_check := IntIntSet.add (fid,fid') !functions_to_check;
            nextins ()
        | IIncludeEvalDefine ins, IIncludeEvalDefine ins' ->
           if ins = ins' then nextins()
           else try_specials ()

        | ICall ins, ICall ins' ->
            (match check_instruct_call asn ins ins' with
              | None -> try_specials()
              | Some newasn -> nextinsnewasn newasn)
        | IGet ins, IGet ins' ->
           (match check_instruct_get asn ins ins' with
             | None -> try_specials ()
             | Some newasn -> nextinsnewasn newasn)
        | IIsset ins, IIsset ins' ->
           (match check_instruct_isset asn ins ins' with
             | None -> try_specials ()
             | Some newasn -> nextinsnewasn newasn)
        | IMutator ins, IMutator ins' ->
           (match check_instruct_mutator asn ins ins' with
             | None -> try_specials ()
             | Some newasn -> nextinsnewasn newasn)
        | IBase ins, IBase ins' ->
           (match check_instruct_base asn ins ins' with
             | None -> try_specials ()
             | Some newasn -> nextinsnewasn newasn)
        | IFinal ins, IFinal ins' ->
           (match check_instruct_final asn ins ins' with
             | None -> try_specials ()
             | Some newasn -> nextinsnewasn newasn)
        | IMisc ins, IMisc ins' ->
           (match check_instruct_misc asn ins ins' with
             | None -> try_specials ()
             | Some newasn -> nextinsnewasn newasn)
        (* iterator instructions have multiple exit points, so have
           to add to todos as well as looking at next instruction
           TODO: exceptional exits from here *)
        | IIterator ins, IIterator ins' ->
           (match check_instruct_iterator asn ins ins' with
             | (None, _) -> try_specials ()
             | (Some newasn, newtodos) ->
                let striptodos = List.map newtodos (fun ((l,l'),asn) ->
                 ( ((hs_of_pc pc, LabelMap.find l labelmap),
                    (hs_of_pc pc', LabelMap.find l' labelmap'))
                     ,asn)) in
                check (succ pc) (succ pc') newasn
                 (add_assumption (pc,pc') asn assumed)
                 (List.fold_left striptodos ~init:todo
                   ~f:(fun td ((pc,pc'),asn) -> add_todo (pc,pc') asn td)))
        (* if they're different classes altogether, give up *)
        | _, _ -> try_specials ()

and donext assumed todo =
 match todo with
  | [] -> None (* success *)
  | ((pc,pc'),asn)::rest -> check pc pc' asn assumed rest

  (* check is more or less uniform - it deals with matching instructions
     modulo local variable matching, and simple control-flow differences.
     specials deals with slightly deeper, ad hoc properties of particular
     instructions, or sequences.
     We assume we've already called check on the two pcs, so don't have
     an appropriate assumed assertion, and the instructions aren't the same
     *)
and specials pc pc' ((props,vs,vs') as asn) assumed todo =
        (* a funny almost no-op that shows up sometimes *)
        let set_pop_get_pattern =
         (uSetL $$ uPopC $$ uPushL) $? (fun ((l1,_),l2) -> l1=l2) $> (fun ((l,_),_) -> l) in

        let set_pop_get_action_left =
         (set_pop_get_pattern $*$ parse_any) $>> (fun (l, _) ((_,n),(_,n')) ->
           let newprops = PropSet.filter (fun (x1,_x2) -> x1 != l) props in
           let newasn = (newprops, VarSet.remove l vs, vs') in
           let newpc = (hs_of_pc pc, n) in
           let newpc' = (hs_of_pc pc', n') in (* always = pc' in fact *)
             check newpc newpc' newasn
                   (add_assumption (pc,pc') asn assumed) todo) in

        let set_pop_get_action_right =
         (parse_any $*$ set_pop_get_pattern) $>> (fun (_, l) ((_,n),(_,n')) ->
         let newprops = PropSet.filter (fun (_x1,x2) -> x2 != l) props in
         let newasn = (newprops, vs, VarSet.remove l vs') in
         let newpc = (hs_of_pc pc, n) in
         let newpc' = (hs_of_pc pc', n') in
           check newpc newpc' newasn
                    (add_assumption (pc,pc') asn assumed) todo) in

        let not_jmpnz_pattern = uNot $$ uJmpNZ $> (fun (_,l) -> l) in
        let not_jmpz_pattern = uNot $$ uJmpZ $> (fun (_,l) -> l) in
        let jmpz_not_jmpnz_pattern = uJmpZ $*$ not_jmpnz_pattern in
        let not_jmpnz_jmpz_pattern = not_jmpnz_pattern $*$ uJmpZ in
        let jmpnz_not_jmpz_pattern = uJmpNZ $*$ not_jmpz_pattern in
        let not_jmpz_jmpnz_pattern = not_jmpz_pattern $*$ uJmpNZ in
        let notjmp_action =
        ( jmpz_not_jmpnz_pattern $|
          not_jmpnz_jmpz_pattern $|
          not_jmpz_jmpnz_pattern $|
          jmpnz_not_jmpz_pattern) $>> (fun (lab,lab') ((_,n),(_,n')) ->
           let newpc = (hs_of_pc pc, n) in
           let newpc' = (hs_of_pc pc', n') in
           check newpc newpc' asn (add_assumption (pc,pc') asn assumed)
                (add_todo ((hs_of_pc pc, LabelMap.find lab labelmap),
                           (hs_of_pc pc', LabelMap.find lab' labelmap')) asn todo)) in

        let string_concat_concat_pattern =
          (uString $$ uConcat $$ uConcat) $> (fun ((s,_),_) -> s) in
        let concat_string_concat_pattern =
          (uConcat $$ uString $$ uConcat) $> (fun ((_,s),_) -> s) in
        let concat_string_either_pattern =
          (string_concat_concat_pattern $*$ concat_string_concat_pattern) $|
          (concat_string_concat_pattern $*$ string_concat_concat_pattern) $?
          (fun (s,s') -> s=s') in
        let concat_string_either_action =
          concat_string_either_pattern $> (fun (s,_) -> s) $>>
          (fun _s ((_,n), (_,n')) ->
          let newpc = (hs_of_pc pc, n) in
          let newpc' = (hs_of_pc pc', n') in
           check newpc newpc' asn (add_assumption (pc,pc') asn assumed) todo) in

        let fpassl_fpassl_pattern =
          uFPassL $*$ uFPassL $? (fun ((pn,_l),(pn',_l')) -> pn=pn')
                              $> (fun ((_pn,l),(_pn',l')) -> (l,l')) in
        let any_pass_pattern =
          uFPassC $| uFPassCW $| uFPassCE $| uFPassV $| uFPassVNop $| uFPassR $|
          uFPassN $| uFPassG $| (uFPassL $> (fun (param,_) -> param)) $|
          (uFPassS $> (fun (param,_) -> param)) in
        let any_pass_list_pattern = greedy_kleene any_pass_pattern in
        let any_call_pattern =
          uFCall $| (uFCallD $> (fun (np,_,_) -> np)) $|
          (uFCallAwait $> (fun (np,_,_) -> np)) $| uFCallUnpack $|
          (uFCallBuiltin $> (fun (_,np2,_) -> np2)) in
        let passes_call_pattern = any_pass_list_pattern $$ any_call_pattern in
        let two_passes_call_pattern =
         (passes_call_pattern $*$ passes_call_pattern) $? (fun ((pass_list,np),(pass_list',np')) ->
            pass_list = pass_list' && np = np' && np > List.length pass_list) in
        let fpassl_action =
        (fpassl_fpassl_pattern $$ two_passes_call_pattern) $>>
          (fun ((l,l'),_) _ ->
            match reads asn l l' with
             | None -> Some (pc, pc', asn, assumed, todo) (* really bail, no subsequent patterns *)
             | Some newasn ->
               (match writes newasn l l' with
                 | None -> Some (pc, pc', asn, assumed, todo)
                 | Some newnewasn ->
                    check (succ pc) (succ pc') newnewasn
                          (add_assumption (pc,pc') asn assumed) todo)) in

        let named_filter foo =
           foo $? (function | Local.Unnamed _ -> false
                            | Local.Named _ -> true)
               $> (function | Local.Unnamed _ -> failwith "unnamed can't happen"
                            | Local.Named s -> s) in

        let cugetl_named_pattern = named_filter uCUGetL in

        let cugetl_list_pattern = greedy_kleene cugetl_named_pattern in
        let cugetl_list_createcl_pattern = cugetl_list_pattern $$ uCreateCl
          $? (fun (cugets,(np,_cn)) -> np >= List.length cugets) in
        let two_cugetl_list_createcl_pattern =
           cugetl_list_createcl_pattern $*$ cugetl_list_createcl_pattern $>
             (fun ((cugets,(np,cn)), (cugets',(np',cn'))) ->
               (findperm cugets cugets', np, cn, np', cn'))
           $? (function | (Some _perm, np, _cn, np', _cn') -> np=np' | _ -> false)
           $> (function | (Some perm,_np,cn,_np',cn') -> (perm,cn,cn')
                        | _ -> failwith "perms can't happen") in
        let two_cugetl_list_createcl_action =
          two_cugetl_list_createcl_pattern $>>
           (fun (perm,cn,cn') ((_,n),(_,n')) ->
             Log.debug (Tty.Normal Tty.Blue) @@
               Printf.sprintf "create cl pattern at lines %d, %d" n n';
             let newpc = (hs_of_pc pc, n) in
             let newpc' = (hs_of_pc pc', n') in
               classes_to_check := IntIntPermSet.add (cn,cn',perm) (!classes_to_check);
               check newpc newpc' asn assumed todo) in

        let string_fatal_pattern = uString $$ uFatal in
        let two_string_fatal_pattern =
          (string_fatal_pattern $*$ string_fatal_pattern) $? (fun ((_s,fop),(_s',fop')) ->
            fop = fop') in
        let two_string_fatal_action = two_string_fatal_pattern $>>
          (fun _ _ -> donext assumed todo) in (* success, nothing more to check here *)

        let unnamed_filter foo =
          foo $? (function | Local.Unnamed _ -> true | Local.Named _ -> false)
              $> (function | Local.Unnamed n -> n
                           | Local.Named _ -> failwith "named can't happen") in

        (* patterns for cases in which VGetL _n is generated by unnatural foreach
           statements. Although we briefly take a reference, it behaves like a read of
           that local followed by unsetting, so the ref doesn't escape.
          In the basel-bindm case we also read the local that goes into base and
          possibly a local that's used as a memberkey *)
        let vget_unnamed_pattern = unnamed_filter uVGetL in
        let unsetl_unnamed_pattern = unnamed_filter uUnsetL in
        let vget_base_pattern =
          (vget_unnamed_pattern $$
           uBaseL $$
           uBindM $$
           uPopV $$
           unsetl_unnamed_pattern)
           $? (fun ((((n1,(_loc,_op)),(_n,_key)),_),n2) -> n1=n2)
           $> (fun ((((n1,(loc,op)),(n,key)),_),_n2) -> (n1,loc,op,n,key)) in
        let two_vget_base_pattern =
         vget_base_pattern $*$ vget_base_pattern
         $? (fun ((_n1,_loc,op,n,_key),(_n1',_loc',op',n',_key')) -> op=op' && n=n')
         $> (fun ((n1,loc,_op,_n,key),(n1',loc',_op',_n',key')) ->((n1,loc,key),(n1',loc',key'))) in
        let two_vget_base_action = two_vget_base_pattern $>>
         (fun ((n1,loc,key),(n1',loc',key')) ((_,ip),(_,ip')) ->
           Log.debug (Tty.Normal Tty.Blue) @@ Printf.sprintf "vget base pattern %d to %d" n1 n1';
           match reads asn (Local.Unnamed n1) (Local.Unnamed n1') with
            | None -> Some (pc, pc', asn, assumed, todo) (* fail *)
            | Some new_asn ->
            (match reads new_asn loc loc' with
               | None -> Some (pc, pc', asn, assumed, todo)
               | Some new_asn2 ->
                let continuation ((some_props,some_vs,some_vs') as _some_asn) =
                  let newpc =(hs_of_pc pc, ip) in
                  let newpc' = (hs_of_pc pc', ip') in
                  let newprops = PropSet.filter
                   (fun (x,x') -> x != Local.Unnamed n1 && x' != Local.Unnamed n1') some_props in
                  let final_asn = (newprops, VarSet.remove (Local.Unnamed n1) some_vs,
                                   VarSet.remove (Local.Unnamed n1') some_vs') in
                  check newpc newpc' final_asn (add_assumption (pc,pc') asn assumed) todo in
                (match key, key' with
                  | MemberKey.EL l, MemberKey.EL l'
                  | MemberKey.PL l, MemberKey.PL l' ->
                     (match reads new_asn2 l l' with
                       | None -> Some (pc,pc',asn,assumed,todo) (*fail *)
                       | Some new_asn3 -> continuation new_asn3)
                  | _,_ -> if key=key' then continuation new_asn2
                           else Some (pc,pc',asn,assumed,todo))
           )
         ) in

        let vget_cget_pattern =
            (vget_unnamed_pattern $$ uCGetL2 $> (fun (n,loc) -> (n, Some loc)))
         $| (uCGetL $$ vget_unnamed_pattern $> (fun (loc,n) -> (n, Some loc)))
         $| (vget_unnamed_pattern $> (fun n -> (n, None))) in
        let vget_cget_bind_pattern =
          (vget_cget_pattern
           $$ uBindN
           $$ uPopV
           $$ uUnsetL)
         $? (fun ((((n1,_optl),_),_),n2) -> Local.Unnamed n1 = n2)
         $> (fun ((((n,optl),_),_),_) -> (n,optl)) in
        let two_vget_cget_bind_pattern =
          vget_cget_bind_pattern $*$ vget_cget_bind_pattern
          $? (fun ((_n,optl),(_n',optl')) ->
               match optl,optl' with
                | None, None
                | Some _, Some _ -> true
                | _,_ -> false) in
        let two_vget_cget_bind_action =
          two_vget_cget_bind_pattern $>>
         (fun ((n,optl),(n',optl')) ((_,ip),(_,ip')) ->
           Log.debug (Tty.Normal Tty.Blue) @@ Printf.sprintf "vget cget pattern %d to %d" n n';
           match reads asn (Local.Unnamed n) (Local.Unnamed n') with
            | None -> Some(pc,pc',asn,assumed,todo)
            | Some new_asn ->
            let continuation ((some_props,some_vs,some_vs') as _some_asn) =
              let newpc =(hs_of_pc pc, ip) in
              let newpc' = (hs_of_pc pc', ip') in
              let newprops = PropSet.filter
               (fun (x,x') -> x != Local.Unnamed n && x' != Local.Unnamed n') some_props in
              let final_asn = (newprops, VarSet.remove (Local.Unnamed n) some_vs,
                               VarSet.remove (Local.Unnamed n') some_vs') in
              check newpc newpc' final_asn (add_assumption (pc,pc') asn assumed) todo in
             match optl,optl' with
              | None, None -> continuation new_asn
              | Some l, Some l' ->
              (match reads new_asn l l' with
                | None -> Some(pc,pc',asn,assumed,todo)
                | Some new_asn2 -> continuation new_asn2)
              | _,_ -> failwith "vget cget can't happen") in

        let vget_retv_pattern =
          vget_unnamed_pattern $$ uRetV $> (fun (n,_) -> n) in
        let two_vget_retv_pattern = vget_retv_pattern $*$ vget_retv_pattern in
        let two_vget_retv_pattern_action =
          two_vget_retv_pattern $>>
         (fun (n,n') (_,_) ->
           match reads asn (Local.Unnamed n) (Local.Unnamed n') with
            | None -> Some(pc,pc',asn,assumed,todo)
            | Some _new_asn ->
               donext (add_assumption (pc,pc') asn assumed) todo
         ) in

        (* last, failure, case for use in bigmatch *)
        let failure_pattern_action =
         parse_any $>> (fun _ _ -> Some (pc, pc', asn, assumed, todo)) in

        let bigmatch_action = bigmatch [
          set_pop_get_action_left;
          set_pop_get_action_right;
          notjmp_action;
          concat_string_either_action;
          fpassl_action;
          two_string_fatal_action;
          two_cugetl_list_createcl_action;
          two_vget_base_action;
          two_vget_cget_bind_action;
          two_vget_retv_pattern_action;
          failure_pattern_action;
         ] in
        bigmatch_action ((prog_array, ip_of_pc pc),(prog_array', ip_of_pc pc'))
in
 (* We always start from ip,ip'=0 for the top entry to the function/method, but
  also take startlabelpairs, which is  list of pairs of labels from the two
  programs, as alternative entry points. These are used for default param
  values *)
  let initialtodo = List.map ~f:(fun (lab,lab') ->
   ((([],LabelMap.find lab labelmap), ([],LabelMap.find lab' labelmap')),
     entry_assertion )) startlabelpairs in
 check ([],0) ([],0)  entry_assertion PcpMap.empty initialtodo
