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

open Hhbc_ast
open Local

(* an individual prop is an equation between local variables
  To start with this means that they are both defined and equal
  or that they are both undefined
  We can usefully use more refined relations, but let's try
  the simplest for now
  TODO: Restrict to unnamed locals only - this is important for
  soundness because of the presence of CGetN and friends!!
*)
type prop = Local.t * Local.t

(* along with pc, we track the current exception handler context
   this should be understood as a list of which exception handlers
   we are currently (dynamically) already within. The static part
   is dealt with via parents *)
type handlerstack = Label.t list

type epc = handlerstack * int
let succ ((hs,pc) : epc) = (hs,pc+1)
let ip_of_pc (_hs,pc) = pc
let hs_of_pc (hs,_pc) = hs

(* now an assertion is a list of props, read as conjunction *)
type assertion = prop list

(* compute an exception table. This maps each pc to its closest handler
   and also computes the containment (parent) relation between handlers.
   TODO: There's clearly something wrong with this, as I'm treating both kinds
   of handler the same, but I'm not yet sure what's right. This should at least
   suffice to get some necessary infrastructure in place, though.
   I think one difference is that a fault handler ends in unwind, so we'll
   have to use the parent relation to deal with that. A catch handler, on
   the other hand, doesn't auto-propagate - we analyze that more like a jump to
   the handler code and analyze that without reference to the parent relation.
 *)
let exntable prog =
 let rec exntable' p n currentstack mapsofar parents =
   match p with
   | [] -> (mapsofar, parents)
   | (i::is) -> (match i with
      | ITry (TryCatchLegacyBegin l)
      | ITry (TryFaultBegin l) ->
        (match currentstack with
          | [] -> exntable' is (n+1) [l] mapsofar parents
          | h::_ -> exntable' is (n+1) (l::currentstack) ((n,h)::mapsofar)
            ((l,h)::parents))
      | ITry TryCatchLegacyEnd
      | ITry TryFaultEnd ->
        exntable' is (n+1) (List.tl currentstack)
                  ((n, List.hd currentstack)::mapsofar)
                  parents
      | _ ->
       (match currentstack with
         | [] -> exntable' is (n+1) currentstack mapsofar parents
         | h::_ -> exntable' is (n+1) currentstack ((n,h)::mapsofar) parents)
    )
  in
  exntable' prog 0 [] [] []


(* inefficiently return the list position where label l occurs
   should really turn everything into arrays with a separate label
   map.
*)
exception Labelexn
let rec findlabel p l = match p with
  | [] -> raise Labelexn
  | i::is -> if i = (ILabel l) then 0 else 1+findlabel is l

(* add equality between v1 and v2 to an assertion
   removing any existing relation between them *)
let addeq_props v1 v2 props =
  let stripped = List.filter (fun (x1,x2) -> x1 != v1 && x2 != v2) props in
      (v1,v2) :: stripped
(*
let addeq_assertion v1 v2 (er,props) =
      (er, addeq_props props)
*)

(* really dumb entailment *)
let entails_props props2 props1 =
  (List.for_all (fun prop -> List.mem prop props2) props1)
(*
let entails_asn (er2, props2) (er1,props1) =
  (er2 = er1) && (entails_props props2 props1)
*)

(* need to deal with the many local-manipulating instructions
   Want to know when two instructions are equal up to an assertion
   and also to return a modified assertion in case that holds
   Modifying this to only track unnamed locals
*)
let reads props l l' =
 match l, l' with
  | Named s, Named s' -> if s=s' then Some props else None
  | Unnamed _, Unnamed _ ->
  if entails_props props [(l,l')]
                 then Some props
                 else None
  | _, _ -> None

let check_instruct_get props i i' =
match i, i' with
 | CGetL l, CGetL l'
 | CGetQuietL l, CGetQuietL l'
 | CGetL2 l, CGetL2 l'
 | CGetL3 l, CGetL3 l'
 | CUGetL l, CUGetL l'
 | PushL l, PushL l' (* this also unsets but don't track that yet *)
    -> reads props l l' (* these instructions read locals *)
 | ClsRefGetL (l,cr), ClsRefGetL (l',cr') ->
   if cr = cr' then reads props l l' else None
 | VGetL _, _
 | _, VGetL _ -> None (* can't handle the possible aliasing here, so bail *)
 (* default case, require literally equal instructions *)
 | _, _ -> if i = i' then Some props else None

 let check_instruct_isset props i i' =
 match i, i' with
 | IssetL l, IssetL l'
 | EmptyL l, EmptyL l'
  -> reads props l l'
 | IsTypeL (l,op), IsTypeL (l',op') ->
   if op = op' then reads props l l' else None
 | _,_ -> if i=i' then Some props else None

(* TODO: allow one-sided writes to dead variables - this shows up
  in one of the tests *)
let writes props l l' =
match l, l' with
 | Named s, Named s' -> if s=s' then Some props else None
 | Unnamed _, Unnamed _ ->
    Some (addeq_props l l' props) (* TODO: tidy order, redundancy *)
 | _, _ -> None

let check_instruct_mutator props i i' =
 match i, i' with
  | SetL l, SetL l'
  | BindL l, BindL l'
  | UnsetL l, UnsetL l'  (* Note, we just record equality, not undefinedness
                            for now *)
    -> writes props l l'
  | SetOpL (l,op), SetOpL (l',op') ->
     if op=op' then
      match reads props l l' with
       | None -> None
       | Some newprops -> writes newprops l l'
     else None
     (* that's something that both reads and writes *)
  | IncDecL (l,op), IncDecL (l',op') ->
    if op=op' then
    match reads props l l' with
     | None -> None
     | Some newprops -> writes newprops l l'
    else None
  | _,_ -> if i=i' then Some props else None

let check_instruct_call props i i' =
 match i, i' with
  | FPassL (_,_), _
  | _, FPassL (_,_) -> None (* if this is pass by reference, might get aliasing
                               so just wimp out for now *)
  | _,_ -> if i=i' then Some props else None

let check_instruct_base props i i' =
 match i,i' with
  | BaseNL (l,op), BaseNL (l',op') ->
    if op=op' then reads props l l'
    else None
    (* All these depend on the string names of locals never being the ones
    we're tracking with the analysis *)
  | FPassBaseNL (n,l), FPassBaseNL (n',l') ->
     if n=n' then reads props l l'
     else None
  | BaseGL (l,mode), BaseGL(l',mode') ->
     if mode = mode' then reads props l l'
     else None (* don't really know if this is right *)
  | FPassBaseGL (n,l), FPassBaseGL (n',l') ->
     if n=n' then reads props l l'
     else None
  | BaseSL (l,n), BaseSL (l',n') ->
     if n=n' then reads props l l'
     else None
  | BaseL (l,mode), BaseL (l',mode') ->
     if mode=mode' then reads props l l'
     else None
  | FPassBaseL (n,l), FPassBaseL (n',l') ->
    if n = n' then reads props l l'
    else None
  | _, _ -> if i=i' then Some props else None

let check_instruct_final props i i' =
  match i, i' with
   | SetWithRefLML (l1,l2), SetWithRefLML (l1',l2') ->
      (match reads props l1 l1' with
        | None -> None
        | Some newprops -> reads newprops l2 l2')
  (* I'm guessing wildly here! *)
   | SetWithRefRML l, SetWithRefRML l' -> reads props l l'
   | _, _ -> if i=i' then Some props else None

(* Iterators. My understanding is that the initializers either jump to the
specified label with no access to locals, or write the first value of the
iteration to the locals given in the instruction. Since this is control-flow,
we need to return further stuff to check, rather than just the newprops that
will hold for the next instruction
*)
exception IterExn (* just a sanity check *)
let check_instruct_iterator props i i' =
 match i, i' with
  | IterInit (it,lab,l), IterInit (it',lab',l')
  | WIterInit (it,lab,l), WIterInit (it',lab',l')
  | MIterInit (it,lab,l), MIterInit (it',lab',l')
  | IterNext (it,lab,l), IterNext (it',lab',l')
  | WIterNext (it,lab,l), WIterNext (it',lab',l')
  | MIterNext (it,lab,l), MIterNext (it',lab',l')  ->
    if it = it' (* not tracking correspondence between iterators yet *)
    then (writes props l l', (* next instruction's state *)
          [((lab,lab'),props)])  (* additional assertions to check *)
    else (None,[]) (* fail *)
  | IterInitK (it,lab,l1,l2), IterInitK (it',lab',l1',l2')
  | WIterInitK (it,lab,l1,l2), WIterInitK (it',lab',l1',l2')
  | MIterInitK (it,lab,l1,l2), MIterInitK (it',lab',l1',l2')
  | IterNextK (it,lab,l1,l2), IterNextK (it',lab',l1',l2')
  | WIterNextK (it,lab,l1,l2), WIterNextK (it',lab',l1',l2')
  | MIterNextK (it,lab,l1,l2), MIterNextK (it',lab',l1',l2')  ->
    if it = it'
    then match writes props l1 l1' with
           | None -> (None,[])
           | Some newprops ->
             (writes newprops l2 l2', (* wrong if same local?? *)
              [((lab,lab'),props)])
    else (None,[]) (* fail *)
  | IterBreak (_,_) , _
  | _ , IterBreak (_,_) -> raise IterExn (* should have been dealt with
                                            along with other control flow *)
  | _ , _ -> if i=i' then (Some props,[]) else (None,[])

let check_instruct_misc props i i' =
 match i,i' with
  | InitThisLoc l, InitThisLoc l' ->
      writes props l l'
  | StaticLoc (l,str), StaticLoc (l',str')
  | StaticLocInit (l,str), StaticLocInit (l',str') ->
     if str=str' then writes props l l'
     else None
  | AssertRATL (_l,_rat), AssertRATL (_l',_rat') ->
     Some props (* Think this is a noop for us, could do something different *)
  | Silence (l, Start), Silence(l',Start) ->
     writes props l l'
  | Silence (l, End), Silence(l',End) ->
     reads props l l'
  (* TODO: Check whether GetMemoKeyL etc. are indeed semantically
     dubious. If they effectively take a reference to an unnamed
     local then they are, 'cos we can't track reads/writes, but they
     may well be OK *)
  | GetMemoKeyL _, _
  | _, GetMemoKeyL _ -> None (* wimp out *)
  | MemoSet (_,_,_), _
  | MemoGet (_,_,_), _
  | _, MemoSet(_,_,_)
  | _, MemoGet(_,_,_) -> None (* wimp out again *)
  | _, _ -> if i=i' then Some props else None

(* main function for comparing two lists of instructions
   this deals with control flow instructions explicitly
   and otherwise uses the subsidiary functions above.
   Currently just uses association lists everywhere, which is kind
   of inefficient.
*)
let rec drop n l =
 match n with | 0 -> l | _ -> drop (n-1) (List.tl l)

let myassoc v l = try Some (List.assoc v l) with _ -> None

let equiv prog prog' =
 let (exnmap,exnparents) = exntable prog in
 let (exnmap',exnparents') = exntable prog' in
 let rec check pc pc' props assumed todo =
   let try_specials () = specials pc pc' props assumed todo in

   let nextins () =
    match myassoc (ip_of_pc pc) exnmap, myassoc (ip_of_pc pc') exnmap' with
     | None, None ->
        check (succ pc) (succ pc') props (((pc,pc'),props)::assumed) todo
     | Some l, Some l'-> let epc = (l:: hs_of_pc pc, findlabel prog l) in
                         let epc' = (l' :: hs_of_pc pc', findlabel prog' l') in
                         let newtodo = ((epc,epc'),props) :: todo in
         check (succ pc) (succ pc') props (((pc,pc'),props)::assumed) newtodo
     | _,_ -> try_specials () in

   (* TODO: remove copy-paste between these two functions *)
   let nextinsnewprops newprops =
    match myassoc (ip_of_pc pc) exnmap, myassoc (ip_of_pc pc') exnmap' with
     | None, None ->
         check (succ pc) (succ pc') newprops (((pc,pc'),props)::assumed) todo
     | Some l, Some l'-> let epc = (l:: hs_of_pc pc, findlabel prog l) in
                         let epc' = (l':: hs_of_pc pc', findlabel prog' l') in
                         let newtodo = ((epc,epc'),props) :: todo in
      check (succ pc) (succ pc') newprops (((pc,pc'),props)::assumed) newtodo
     | _,_ -> try_specials () in

    match myassoc (pc,pc') assumed with
     | Some ass -> if entails_props props ass then donext assumed todo
                   else try_specials () (* failure *)
     | None -> (
       let i = List.nth prog (ip_of_pc pc) in
       let i' = List.nth prog' (ip_of_pc pc') in
       match i, i' with
        (* one-sided stuff for jumps, labels, comments *)
        | IContFlow(Jmp lab), _
        | IContFlow(JmpNS lab), _ ->
         check (hs_of_pc pc, findlabel prog lab) pc' props
               (((pc,pc'),props)::assumed) todo
        | ILabel _, _
        | IComment _, _ ->
           check (succ pc) pc' props (((pc,pc'),props)::assumed) todo

        | _, IContFlow(Jmp lab')
        | _, IContFlow(JmpNS lab') ->
              check pc (hs_of_pc pc', findlabel prog' lab') props
                    (((pc,pc'),props)::assumed) todo
        | _, ILabel _
        | _, IComment _ ->
              check pc (succ pc') props (((pc,pc'),props)::assumed) todo
        | IContFlow (JmpZ lab), IContFlow (JmpZ lab')
        | IContFlow (JmpNZ lab), IContFlow (JmpNZ lab') ->
           check (succ pc) (succ pc') props
             (((pc,pc'),props)::assumed)
             ((( (hs_of_pc pc, findlabel prog lab),
                (hs_of_pc pc', findlabel prog' lab') ),props)::todo)
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
               | [],[] -> try_specials () (* unwind not in handler! *)
               | (h::hs),(h'::hs') ->
                 let leftside = match myassoc h exnparents with
                   | None -> hs
                   | Some h2 -> h2::hs in
                 let  rightside = match myassoc h' exnparents' with
                   | None -> hs'
                   | Some h2' -> h2'::hs' in
                 (match leftside, rightside with
                   | [],[] -> donext assumed todo (* both jump out*)
                   | (hh::_), (hh'::_) ->
                     check (leftside,findlabel prog hh)
                           (rightside,findlabel prog' hh')
                           props
                           (((pc,pc'),props)::assumed)
                           todo
                   | _,_ -> try_specials ())
               | _,_ -> try_specials ()) (* mismatch *)
        | IContFlow Throw, IContFlow Throw ->
            (match myassoc (ip_of_pc pc) exnmap,
                   myassoc (ip_of_pc pc') exnmap' with
             | None, None ->  donext assumed todo (* both leave *)
             | Some h, Some h' ->
                  let hip = findlabel prog h in
                  let hes = h :: (hs_of_pc pc) in
                  let hip' = findlabel prog' h' in
                  let hes' = h' :: (hs_of_pc pc') in
                   check (hes,hip) (hes',hip') props
                   (((pc,pc'),props)::assumed)
                   todo
             | _,_ -> try_specials ()) (* leaves/stays -> mismatch *)
        | IContFlow _, IContFlow _ -> try_specials ()
        (* next block have no interesting controls flow or local
           variable effects
           TODO: Some of these are not actually in this class!
        *)
        | IBasic ins, IBasic ins' ->
           if ins = ins' then nextins()
           else try_specials ()
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
        | ICall ins, ICall ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        | ITry ins, ITry ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        | IAsync ins, IAsync ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        | IGenerator ins, IGenerator ins' ->
           if ins = ins' then nextins()
           else try_specials ()
        | IIncludeEvalDefine ins, IIncludeEvalDefine ins' ->
           if ins = ins' then nextins()
           else try_specials ()

        | IGet ins, IGet ins' ->
           (match check_instruct_get props ins ins' with
             | None -> try_specials ()
             | Some newprops -> nextinsnewprops newprops)
        | IIsset ins, IIsset ins' ->
           (match check_instruct_isset props ins ins' with
             | None -> try_specials ()
             | Some newprops -> nextinsnewprops newprops)
        | IMutator ins, IMutator ins' ->
           (match check_instruct_mutator props ins ins' with
             | None -> try_specials ()
             | Some newprops -> nextinsnewprops newprops)
        | IBase ins, IBase ins' ->
           (match check_instruct_base props ins ins' with
             | None -> try_specials ()
             | Some newprops -> nextinsnewprops newprops)
        | IFinal ins, IFinal ins' ->
           (match check_instruct_final props ins ins' with
             | None -> try_specials ()
             | Some newprops -> nextinsnewprops newprops)
        | IMisc ins, IMisc ins' ->
           (match check_instruct_misc props ins ins' with
             | None -> try_specials ()
             | Some newprops -> nextinsnewprops newprops)
        (* iterator instructions have multiple exit points, so have
           to add to todos as well as looking at next instruction
           TODO: exceptional exits from here *)
        | IIterator ins, IIterator ins' ->
           (match check_instruct_iterator props ins ins' with
             | (None, _) -> try_specials ()
             | (Some newprops, newtodos) ->
                let striptodos = List.map (fun ((l,l'),props) ->
                 ( ((hs_of_pc pc, findlabel prog l),
                    (hs_of_pc pc', findlabel prog' l'))
                     ,props)) newtodos in
                check (succ pc) (succ pc') newprops
                 (((pc,pc'),props)::assumed) (striptodos @ todo))
        (* if they're different classes altogether, give up *)
        | _, _ -> try_specials ()
       )
and donext assumed todo =
 match todo with
  | [] -> None (* success *)
  | ((pc,pc'),props)::rest -> check pc pc' props assumed rest

  (* check is more or less uniform - it deals with matching instructions
     modulo local variable matching, and simple control-flow differences.
     specials deals with slightly deeper, ad hoc properties of particular
     instructions, or sequences.
     We assume we've already called check on the two pcs, so don't have
     an appropriate assumed assertion, and the instructions aren't the same
     *)
and specials pc pc' props assumed todo =
  let i = List.nth prog (ip_of_pc pc) in
  let i' = List.nth prog' (ip_of_pc pc') in
  match i, i' with
   (* first, special case of unset on one side *)
   | IMutator (UnsetL l), _ ->
      let newprops = List.filter (fun (x1,_x2) -> x1 != l) props in
        check (succ pc) pc' newprops (((pc,pc'),props)::assumed) todo
   | _, IMutator (UnsetL l') ->
      let newprops = List.filter (fun (_x1,x2) -> x2 != l') props in
      check pc (succ pc') newprops (((pc,pc'),props)::assumed) todo
   | _, _ -> (
     (* having looked at individual instructions, try some known special
        sequences. This is *really* hacky, and I should work out how
        to do it more generally, but it'll do for now
        Remark - really want a nicer way of doing rule-based programming
        without nasty nested matches. Order of testing is annoyingly delicate
        at the moment
     *)
        let prog_from_pc = drop (ip_of_pc pc) prog in
        let prog_from_pc' = drop (ip_of_pc pc') prog' in
        (* a funny almost no-op that shows up sometimes *)
        match prog_from_pc, prog_from_pc' with
         | (IMutator (SetL l1) :: IBasic PopC :: IGet (PushL l2) :: _), _
           when l1 = l2 ->
             let newprops = List.filter (fun (x1,_x2) -> x1 != l1) props in
               check (succ (succ (succ pc))) pc' newprops
                     (((pc,pc'),props)::assumed) todo
         | _, (IMutator (SetL l1) :: IBasic PopC :: IGet (PushL l2) :: _)
            when l1 = l2 ->
             let newprops = List.filter (fun (_x1,x2) -> x2 != l1) props in
               check pc (succ (succ (succ pc'))) newprops
                        (((pc,pc'),props)::assumed) todo
        (* OK, we give up *)
         | _, _ -> Some (pc, pc', props, assumed, todo)
     )
in
 check ([],0) ([],0) [] [] []
