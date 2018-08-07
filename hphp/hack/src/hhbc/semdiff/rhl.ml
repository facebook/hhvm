(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(*
  This is a pretty dumbed-down RHL prover for showing equivalence of two
  hhvm bytecode function bodies.
*)
open Hh_core
open Hhbc_ast
open Local
open Hhbc_destruct
module Loc = Srcloc_stats
module Log = Semdiff_logging
module Utils = Semdiff_utils

(* Ref keeping to-do set for pairs of classes that
   need to be compared. Originally just for closure classes, now
   for all corresponding pairs.
   Also comes with a permutation for matching up properties used in
   closure classes.
   A bit ugly to use refs here, but the structure of the original
   combinators didn't make it easy to plumb this info through when
   we discovered that we needed it - maybe refactor sometime.
*)
type perm = (int * int) list
module IntIntPermSet = Set.Make(struct type t = int*int*perm let compare = compare end)
module IntIntSet = Set.Make(struct type t = int*int let compare = compare end)
module StringStringSet = Set.Make(struct type t = string*string let compare = compare end)

let classes_to_check = ref IntIntPermSet.empty
let classes_checked = ref IntIntSet.empty

let functions_to_check = ref IntIntSet.empty
let functions_checked = ref IntIntSet.empty

let adata_to_check = ref StringStringSet.empty
let adata_checked = ref StringStringSet.empty

let typedefs_to_check = ref IntIntSet.empty
let typedefs_checked = ref IntIntSet.empty

(* comparison of string representation of doubles
   TODO: should be up to some precision?
*)
let compare_double_strings s s' =
 (* convert both to lowercase to handle INF in different casing *)
 let s = String.lowercase_ascii s in
 let s' = String.lowercase_ascii s' in
 if s=s' then true
 else match Scanf.sscanf s "%f" (fun x -> Some x),
            Scanf.sscanf s' "%f" (fun x -> Some x) with
      | Some f, Some f' -> f = f'
      | _, _ -> false
      | exception _ -> false

(* If `false`, then `UnsetL` instructions cannot be reordered. *)
let lax_unset = ref false

(* An individual prop is an equation between local variables. To start with,
  this means that they are both defined and equal or that they are both
  undefined.
  Note: We can usefully use more refined relations, but we try the simplest for
  now.
*)
type prop = Local.t * Local.t
module PropSet = Set.Make(struct type t = prop let compare = compare end)
module VarSet = Set.Make(struct type t = Local.t let compare = compare end)

(* Along with pc, we track the current exception handler context. This should be
  understood as a list of which exception handlers we are currently
  (dynamically) already within. *)
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

(* Assertions.
  An assertion tracks a conjunction of known equalities between local variables,
  and which ones *may* be set. Concretely, it is
  1. a set of `prop`s (a pair of local-ids), each denoting a known equality
    between the local variables;
  2. a set of local-ids that are *not* known to be unset for program A; and
  3. a set of local-ids that are *not* known to be unset for program B.

  An assertion is satisfied by a pair of states, i.e.
    s,s' \in [[props, vs, vs']]
  iff
    \forall (v,v') \in props, s v = s' v' /\
    \forall v  \notin vs,  s v   = unset /\
    \forall v' \notin vs', s' v' = unset
  *)
type assertion = PropSet.t * VarSet.t * VarSet.t
(* The assertion that holds at the beginning of a program: there are no known
  equalities (of interest) between local variables and all local-ids are unset.
*)
let (entry_assertion : assertion) = (PropSet.empty,VarSet.empty,VarSet.empty)
(* A set of assertions.
  Each pair of program counters maps to a set of assertions, each representing a
  possible state that may hold that those points. I.e. this is a disjunction of
  assertions. *)
module AsnSet = Set.Make(struct type t = assertion let compare=compare end)

exception Labelexn
module  LabelMap = MyMap.Make(struct type t = Label.t let compare = compare end)

(*
  Refactored version of exception table structures, to improve efficiency a bit
  and to cope with new try-catch-end structure, which doesn't have explicit
  labels for the handlers. First pass constructs labelmap and a map from the
  indices of TryCatchBegin instructions to the index of their matching
  TryCatchMiddle.
*)
type tcstackentry = Stack_label of Label.t | Stack_tc of int
let make_label_try_maps prog =
  let rec loop prog n trycatchstack labelmap trymap =
    match prog with
    | [] -> (labelmap, trymap)
    | i :: is -> begin match i with
      | ILabel l ->
        loop is (n+1) trycatchstack (LabelMap.add l n labelmap) trymap
      | ITry (TryCatchLegacyBegin l)
      | ITry (TryFaultBegin l) ->
        loop is (n+1) (Stack_label l :: trycatchstack) labelmap trymap
      | ITry TryCatchBegin ->
        loop is (n+1) (Stack_tc n :: trycatchstack) labelmap trymap
      | ITry TryCatchMiddle ->
        begin match trycatchstack with
        | Stack_tc m :: rest ->
          loop is (n+1) rest labelmap (IMap.add m n trymap)
        | _ -> raise Labelexn
        end
      | ITry TryCatchLegacyEnd
      | ITry TryFaultEnd ->
        begin match trycatchstack with
        | Stack_label _l :: rest ->
          loop is (n+1) rest labelmap trymap
        | _ -> raise Labelexn
        end
      | ITry TryCatchEnd
      | _ -> loop is (n+1) trycatchstack labelmap trymap
      end in
  loop prog 0 [] LabelMap.empty IMap.empty

(* Second pass constructs an exception table. Revised version maps instruction
  indices to a list of handler indices. Previous parent relation is removed, as
  it doesn't track the right information.

  Still a question mark over whether the whole stack needs to go into the
  handler lists or if it should be somehow truncated when we reach a catch
  handler. For now, I'm trying with remembering everything.
*)
let make_exntable prog labelmap trymap =
  let rec loop prog n trycatchstack exnmap = match prog with
  | [] -> exnmap
  | i::is ->
    let newexnmap = IMap.add n trycatchstack (* <- filter this? *) exnmap in
    begin match i with
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
      begin match trycatchstack with
      | Catch_handler _ :: rest -> loop is (n+1) rest newexnmap
      | _ -> raise Labelexn
      end

    | ITry TryFaultEnd ->
      begin match trycatchstack with
      | Fault_handler _ :: rest -> loop is (n+1) rest newexnmap
      | _ -> raise Labelexn
      end

    | ITry TryCatchLegacyEnd ->
      begin match trycatchstack with
      | Catch_handler _ :: rest -> loop is (n+1) rest newexnmap
      | _ -> raise Labelexn
      end

    | ITry TryCatchEnd
    | _ -> loop is (n+1) trycatchstack newexnmap
  end in
  loop prog 0 [] IMap.empty

(* lop off the tail after the first catch handler, if there is one *)
let rec to_first_catch hs = match hs with
  | [] -> []
  | Fault_handler x :: rest -> Fault_handler x :: to_first_catch rest
  | Catch_handler x :: _rest -> [Catch_handler x]

(* construct the new pc corresponding to throwing from given static
   and dynamic handler stacks
   This version assumes that the dynamic stacks do not include the "current" handler,
   just those that are still pending
   The result is an option to enable shortcircuiting throwing out of frame
*)
let throw_pc static dynamic =
  match (to_first_catch static) @ dynamic with
    | [] -> None (* really should be ([],-1) *)
    | Fault_handler x :: rest
    | Catch_handler x :: rest -> Some (rest, x)

(* Moving string functions into rhl so that I can use them in debugging *)
let propstostring props =
  String.concat " " (List.map
    ~f:(fun (v,v') ->
      "(" ^ (Hhbc_hhas.string_of_local_id v) ^ "," ^
      (Hhbc_hhas.string_of_local_id v') ^ ")")
    (PropSet.elements props))

let varsettostring vs =
  "{" ^
  String.concat "," (List.map
    ~f:(fun v -> Hhbc_hhas.string_of_local_id v)
    (VarSet.elements vs)) ^
  "}"

let asntostring (props,vs,vs') =
  propstostring props ^ varsettostring vs ^ varsettostring vs'

let asnsettostring asns =
  "<" ^
  String.concat "," (List.map ~f:asntostring (AsnSet.elements asns)) ^ ">"

let string_of_pc (hs,ip) =
  String.concat " " (List.map ~f:(fun h -> string_of_int (ip_of_emv h)) hs) ^
  ";" ^
  string_of_int ip
let labasnstostring ((l1,l2),asns) =
  "[" ^
  (string_of_pc l1) ^ "," ^
  (string_of_pc l2) ^ "->" ^
  (asnsettostring asns) ^
  "]\n"
let labasntostring ((l1,l2),asns) =
  "[" ^
  (string_of_pc l1) ^ "," ^
  (string_of_pc l2) ^ "->" ^
  (asntostring asns) ^
  "]\n"
let labasnlisttostring l = String.concat "" (List.map ~f:labasntostring l)
let labasnsmaptostring asnmap =
  String.concat "" (List.map ~f:labasnstostring (PcpMap.bindings asnmap))

let string_of_nth_instruction l pc =
  let i = ip_of_pc pc in
  if i= -1 then "THROWN"
  else Utils.string_of_instruction @@ List.nth_exn l i

(* Add equality between v1 and v2 to an assertion, removing any existing
  relation involving them. Mark v1 and v2 as being possibly set. *)
let addeq_asn v1 v2 (props, vs, vs') =
  let stripped = PropSet.filter (fun (x1,x2) -> x1 <> v1 && x2 <> v2) props in
  (PropSet.add (v1,v2) stripped, VarSet.add v1 vs, VarSet.add v2 vs')

(* Simple-minded entailment between assertions.
  In general, asn2 entails asn1 iff (s,s') \in asn2 implies (s,s') \in asn1 for
  all states s and s'.
*)
let entails_asns (props2,vs2,vs2') (props1,vs1,vs1') =
  (* Every local variable relation in props1 is either in props2, or else
    both variables are known to be unset: *)
  (PropSet.for_all (fun ((v,v') as prop) ->
    PropSet.mem prop props2
    || not (VarSet.mem v vs2 || VarSet.mem v' vs2')
  ) props1)
  (* Assertion 2 has more specific knowledge of unset variables: *)
  && VarSet.subset vs2 vs1
  && VarSet.subset vs2' vs1'

(* Need to deal with the many local-manipulating instructions.
   Want to know when two instructions are equal up to an assertion
   and also to return a modified assertion in case that holds.
   Note that we only track unnamed locals
*)

(* An assertion entails that locals l and l' are equal either if it has a
  relation between them or knows that both are unset. *)
let asn_entails_equal (props,vs,vs') l l' =
  PropSet.mem (l,l') props || not (VarSet.mem l vs || VarSet.mem l' vs')

(* A pair of instructions that read from locals l and l', respectively, will
  have equivalent behavior (assuming all else equal) if l and l' are "equal".
  Tracking named variables is hard because they can be accessed by dynamically-
  computed strings, so we require the names to be equal. Unnamed variable
  equivalences are decided by the assertion.
*)
let reads asn l l' =
  match l, l' with
  | Named s, Named s' -> if s=s' then Some asn else None
  | Unnamed _, Unnamed _ ->
    if asn_entails_equal asn l l'
    then Some asn
    else None
  | Named _, _ | Unnamed _, _ -> None

let check_instruct_get asn i i' =
  match i, i' with
  | CGetL l, CGetL l'
  | CGetQuietL l, CGetQuietL l'
  | CGetL2 l, CGetL2 l'
  | CUGetL l, CUGetL l'
  | VGetL l, VGetL l'
  | PushL l, PushL l' (* TODO: this also unsets but don't track that yet *)
    -> reads asn l l' (* these instructions read locals *)
  | ClsRefGetL (l,cr), ClsRefGetL (l',cr') ->
    if cr = cr' then reads asn l l' else None
  | VGetL _, _
  | _, VGetL _ ->
    (* can't handle the possible  aliasing here, so bail *)
    None
  | CGetL _, _ | CGetQuietL _, _ | CGetL2 _, _ | CUGetL _, _
  | PushL _, _ | ClsRefGetL _, _ -> None
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | CGetN, _ | CGetQuietN, _ | CGetG, _ | CGetQuietG, _ | CGetS _, _ | VGetN, _
  | VGetG, _ | VGetS _, _ | ClsRefGetC _, _ ->
    if i = i' then Some asn else None

let check_instruct_isset asn i i' =
  match i, i' with
  | IssetL l, IssetL l'
  | EmptyL l, EmptyL l'
    -> reads asn l l'
  | IsTypeL (l,op), IsTypeL (l',op') ->
    if op = op' then reads asn l l' else None
  | IssetL _, _ | EmptyL _, _ | IsTypeL _, _ -> None
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | IssetC, _ | IssetN, _ | IssetG, _ | IssetS _, _ | EmptyN, _ | EmptyG, _
  | EmptyS _, _ | IsTypeC _, _ ->
    if i=i' then Some asn else None

(* TODO: allow one-sided writes to dead variables - this shows up
  in one of the tests *)
let writes asn l l' =
  match l, l' with
  | Named s, Named s' -> if s=s' then Some asn else None
  | Unnamed _, Unnamed _ ->
    Some (addeq_asn l l' asn)
  | Named _, _ | Unnamed _, _ -> None

(* We could be a bit more refined in tracking set/unset status of named locals
   but it might not make much difference, so leaving it out for now
*)
let writesunset ((props,vs,vs') as asn) l l' =
  match l, l' with
  | Named s, Named s' -> if s=s' then Some asn else None
  | Unnamed _, Unnamed _ ->
    let stripped = PropSet.filter (fun (x,x') -> x <> l && x' <> l') props in
      Some (stripped, VarSet.remove l vs, VarSet.remove l' vs')
  | Named _, _ | Unnamed _, _ -> None

let readswrites asn l l' =
  match reads asn l l' with
  | None -> None
  | Some newasn -> writes newasn l l' (* actually, newasn=asn, of course *)

let check_instruct_mutator asn i i' =
  match i, i' with
  | SetL l, SetL l'
  | PopL l, PopL l'
  | BindL l, BindL l'
    -> writes asn l l'
  | UnsetL l, UnsetL l'
    -> writesunset asn l l'
  | SetOpL (l,op), SetOpL (l',op') ->
    if op=op' then readswrites asn l l' else None
  | IncDecL (l,op), IncDecL (l',op') ->
    if op=op' then readswrites asn l l' else None
  | SetL _, _ | PopL _ , _ | BindL _, _ | UnsetL _, _ | SetOpL _, _
  | IncDecL _, _ -> None
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | SetN, _ | SetG, _ | SetS _, _ | SetOpN _, _ | SetOpG _, _ | SetOpS _, _
  | IncDecN _, _ | IncDecG _, _ | IncDecS _, _ | BindN, _ | BindG, _
  | BindS _, _ | UnsetN, _ | UnsetG, _ | CheckProp _, _ | InitProp _, _  ->
    if i=i' then Some asn else None

let check_instruct_call asn i i' =
  match i, i' with
  | DecodeCufIter _ , DecodeCufIter _ ->
    (* should be handled in check *)
    None
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | FPushFunc _, _ | FPushFuncD _, _ | FPushFuncU _, _ | FPushObjMethod _, _
  | FPushObjMethodD _, _ | FPushClsMethod _, _
  | FPushClsMethodS _, _ | FPushClsMethodSD _, _
  | FPushClsMethodD _, _ | FPushCtor _, _ | FPushCtorD _, _ | FPushCtorI _, _
  | FPushCtorS _, _ | FPushCufIter _, _ | FIsParamByRef _, _
  | FThrowOnRefMismatch _, _ | FCall _, _ | FCallAwait _, _
  | FCallBuiltin _, _ ->
    if i=i' then Some asn else None
  | _, _ -> None

(* Asserts equivalence for reading from member keys. For example, checks
  equivalence for reading an element or property via a local variable. *)
let reads_member_key asn m m' =
  match m, m' with
  | MemberKey.EL l, MemberKey.EL l'
  | MemberKey.PL l, MemberKey.PL l'
    -> reads asn l l'
  (* Whitelist the keys where equality implies equivalence
    (e.g. they do not access locals). *)
  | MemberKey.EC _, _ | MemberKey.ET _, _ | MemberKey.EI _, _
  | MemberKey.PC _, _ | MemberKey.PT _, _ | MemberKey.QT _, _ | MemberKey.W, _
    -> if m=m' then Some asn else None
  | MemberKey.EL _, _ | MemberKey.PL _, _ -> None

let check_instruct_base asn i i' =
  match i,i' with
  | BaseNL (l,op), BaseNL (l',op') ->
    if op=op' then reads asn l l'
    else None
    (* All these depend on the string names of locals never being the ones
    we're tracking with the analysis *)
  | BaseGL (l,mode), BaseGL(l',mode') ->
    if mode = mode' then reads asn l l'
    else None (* don't really know if this is right *)
  | BaseSL (l,n,_), BaseSL (l',n',_) ->
    if n=n' then reads asn l l'
    else None
  | BaseL (l,mode), BaseL (l',mode') ->
    if mode=mode' then reads asn l l'
    else None
  | Dim (mode, mk), Dim (mode', mk') ->
    if mode=mode' then reads_member_key asn mk mk' else None
  | BaseNL _, _ | BaseGL _, _ | BaseSL _, _ | BaseL _, _ | Dim _, _ ->
    None
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | BaseNC _, _ | BaseGC _, _ | BaseSC _, _ | BaseC _, _ | BaseR _, _
  | BaseH, _ ->
    if i=i' then Some asn else None

let check_instruct_final asn i i' =
  match i, i' with
  | SetWithRefLML (l1,l2), SetWithRefLML (l1',l2') ->
    begin match reads asn l1 l1' with
    | None -> None
    | Some newasn -> reads newasn l2 l2'
    end
   (* TODO: abstraction, we've heard of it *)
  | QueryM (n,op,mk), QueryM (n',op',mk')
    when n=n' && op=op' -> reads_member_key asn mk mk'
  | VGetM (n, mk), VGetM (n', mk')
    when n=n' -> reads_member_key asn mk mk'
  | SetM (n, mk), SetM (n', mk')
    when n=n' -> reads_member_key asn mk mk'
  | IncDecM (m,op,mk), IncDecM (m',op',mk')
    when m=m' && op=op' -> reads_member_key asn mk mk'
  | SetOpM (m,op,mk), SetOpM (m',op',mk')
    when m=m' && op=op' -> reads_member_key asn mk mk'
  | BindM (n, mk), BindM (n', mk')
    when n=n' -> reads_member_key asn mk mk'
  | UnsetM (n, mk), UnsetM (n', mk')
    when n=n' -> reads_member_key asn mk mk'
  | SetWithRefRML _, SetWithRefRML _ ->
    (* COMPLETENESS: HackC/HHVM do not generate this instruction, so reject it
      for now. *)
    None
  | SetWithRefLML _, _ | QueryM _, _ | VGetM _, _ | SetM _, _ | IncDecM _, _
  | SetOpM _, _ | BindM _, _ | UnsetM _, _ | SetWithRefRML _, _ ->
    None

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
    (* COMPLETENESS: not tracking correspondence between iterators yet *)
    if it = it' then
      (* next instruction's state *)
      let nextAsn = writes asn l l' in
      (* additional assertions to check *)
      let todoAsns = [((lab,lab'),asn)] in
      (nextAsn, todoAsns)
    else (None,[])
  | IterInitK (it,lab,l1,l2), IterInitK (it',lab',l1',l2')
  | WIterInitK (it,lab,l1,l2), WIterInitK (it',lab',l1',l2')
  | MIterInitK (it,lab,l1,l2), MIterInitK (it',lab',l1',l2')
  | IterNextK (it,lab,l1,l2), IterNextK (it',lab',l1',l2')
  | WIterNextK (it,lab,l1,l2), WIterNextK (it',lab',l1',l2')
  | MIterNextK (it,lab,l1,l2), MIterNextK (it',lab',l1',l2')  ->
    if it = it' then
      match writes asn l1 l1' with
      | None -> (None,[])
      | Some newasn ->
        (* SOUNDNESS: wrong if same local?? i.e. l1=l2 or l1'=l2'. *)
        let nextAsn = writes newasn l2 l2' in
        let todoAsns = [((lab,lab'),asn)] in
        (nextAsn, todoAsns)
    else (None,[]) (* fail *)
  | IterBreak (_,_) , _
  | _ , IterBreak (_,_) ->
    (* This case should have been handled along with other control flow. *)
    (None,[])
  | IterInit _, _ | WIterInit _, _ | MIterInit _, _ | IterNext _, _
  | LIterInit _, _ | LIterInitK _, _ | LIterNext _, _ | LIterNextK _, _
  | WIterNext _, _ | MIterNext _, _ | IterInitK _, _ | WIterInitK _, _
  | MIterInitK _, _ | IterNextK _, _ | WIterNextK _, _ | MIterNextK _, _ ->
    (None, [])
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | IterFree _, _ | MIterFree _, _ | CIterFree _, _ | LIterFree _, _ ->
    if i=i' then (Some asn,[]) else (None,[])

let check_instruct_misc asn i i' =
 match i,i' with
  | InitThisLoc l, InitThisLoc l' ->
    writes asn l l'
  | StaticLocCheck (l,str), StaticLocCheck (l',str')
  | StaticLocDef (l,str), StaticLocDef (l',str')
  | StaticLocInit (l,str), StaticLocInit (l',str') ->
    if str=str'
    then writes asn l l'
    else None
  | AssertRATL (_l,_rat), AssertRATL (_l',_rat') ->
    (* SOUNDNESS: Think this is a noop for us, could do something different. *)
    Some asn
  | Silence (l, Start), Silence(l',Start) ->
    writes asn l l'
  | Silence (l, End), Silence(l',End) ->
    reads asn l l'
  | GetMemoKeyL (Local.Named s), GetMemoKeyL (Local.Named s')
    when s = s' -> Some asn
  | GetMemoKeyL _, _
  | _, GetMemoKeyL _ -> None (* wimp out if not same named local *)

  | MemoSet (Some (Local.Unnamed first, local_count)),
    MemoSet (Some (Local.Unnamed first', local_count'))
  | MemoGet (_, Some (Local.Unnamed first, local_count)),
    MemoGet (_, Some (Local.Unnamed first', local_count'))
    when local_count = local_count' ->
      let rec loop loop_asn local local' count =
        match reads loop_asn (Local.Unnamed local) (Local.Unnamed local') with
        | None -> None
        | Some new_asn ->
          if count = 1 then Some new_asn
          else loop new_asn (local + 1) (local' + 1) (count - 1)
        in
      loop asn first first' local_count
  | MemoSet _, _
  | _, MemoSet _
  | MemoGet _, _
  | _, MemoGet _ ->
    (* COMPLETENESS: wimp out again *)
    None
  | CreateCl(npars,cln), CreateCl(npars',cln') ->
    if npars = npars' then begin
      classes_to_check := IntIntPermSet.add (cln,cln',[]) (!classes_to_check);
      Some asn
    end else
      None
  | InitThisLoc _, _ | StaticLocCheck _, _ | StaticLocDef _, _
  | StaticLocInit _, _ | AssertRATL _, _ | Silence _, _ | CreateCl _, _ ->
    None
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | This, _ | BareThis _, _ | CheckThis, _ | Catch, _ | ChainFaults, _
  | OODeclExists _, _
  | VerifyParamType _, _ | VerifyRetTypeC, _ | VerifyRetTypeV, _ | Self _, _
  | Parent _, _ | LateBoundCls _, _ | ClsRefName _, _ | NativeImpl, _
  | VerifyOutType _, _
  | AKExists, _ | Idx, _ | ArrayIdx, _
  | AssertRATStk _, _ | BreakTraceHint, _
  | CGetCUNop, _ | UGetCUNop, _ ->
    if i=i' then Some asn else None

let check_instruct_basic i i' =
  match i, i' with
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | Nop, _ | EntryNop, _ | PopC, _ | PopV, _ | PopR, _ | PopU, _ | Dup, _
  | Box, _ | Unbox, _ | BoxR, _ | BoxRNop, _ | UnboxR, _ | UnboxRNop, _ | RGetCNop, _ ->
    if i=i' then Some () else None

let check_instruct_lit_const asn i i' =
  match i, i' with
  | Array id, Array id'
  | Dict id, Dict id'
  | Vec id, Vec id'
  | Keyset id, Keyset id' ->
    adata_to_check := StringStringSet.add (id,id') !adata_to_check;
    Some asn
  | Double s, Double s' ->
    if compare_double_strings s s' then Some asn else None
  | NewLikeArrayL (l, n), NewLikeArrayL (l', n') ->
    if n=n'
    then reads asn l l'
    else None
  | Array _, _ | Dict _, _ | Vec _, _ | Keyset _, _ | Double _, _
  | NewLikeArrayL _, _ -> None
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | Null, _ | True, _ | False, _ | NullUninit, _ | Int _, _
  | String _, _ | TypedValue _, _ | NewArray _, _
  | NewMixedArray _, _ | NewDictArray _, _ | NewPackedArray _, _
  | NewStructArray _, _ | NewStructDict _, _
  | NewVecArray _, _ | NewKeysetArray _, _
  | NewVArray _, _ | NewDArray _, _ | NewStructDArray _, _ | NewPair, _
  | AddElemC, _ | AddElemV, _ | AddNewElemC, _ | AddNewElemV, _ | NewCol _, _
  | ColFromArray _, _ | Cns _, _ | CnsE _, _ | CnsU _, _
  | ClsCns _, _ | ClsCnsD _, _ | File, _ | Dir, _ | Method, _  (*| NameA, _ *)->
    if i=i' then Some asn else None

(* Returns true if the instruction terminates the program. *)
let check_instruct_operator i i' =
  match i, i' with
  | Hhbc_ast.Exit, Hhbc_ast.Exit ->
    Some true
  | Fatal op, Fatal op' ->
    if op=op'
    then Some true
    else None
  | Hhbc_ast.Exit, _ | Fatal _, _ ->
    None
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | Concat, _ | ConcatN _, _ | Abs, _ | Add, _ | Sub, _ | Mul, _ | AddO, _ | SubO, _ | MulO, _
  | Div, _ | Mod, _ | Pow, _ | Sqrt, _ | Xor, _ | Not, _ | Same, _ | NSame, _
  | Eq, _ | Neq, _ | Lt, _ | Lte, _ | Gt, _ | Gte, _ | Cmp, _ | BitAnd, _
  | BitOr, _ | BitXor, _ | BitNot, _ | Shl, _ | Shr, _ | Floor, _ | Ceil, _
  | CastBool, _ | CastInt, _ | CastDouble, _ | CastString, _ | CastArray, _
  | CastObject, _ | CastVec, _ | CastDict, _ | CastKeyset, _ | CastVArray, _
  | CastDArray, _ | InstanceOf, _ | InstanceOfD _, _ | IsTypeStruct _, _
  | Print, _ | ResolveFunc _, _ | ResolveObjMethod, _ | ResolveClsMethod, _
  | AsTypeStruct _, _ | Clone, _ ->
    if i=i' then Some false else None

let check_instruct_special_flow i i' =
  match i, i' with
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | Continue _, _ | Break _, _ | Goto _, _->
    if i = i' then Some () else None

let check_instruct_async_functions asn i i' =
  match i, i' with
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | WHResult, _ | Await, _ ->
    if i = i' then Some asn else None
  | AwaitAll Some (Local.Unnamed first1, count1),
    AwaitAll Some (Local.Unnamed first2, count2) when count1 = count2 ->
    let rec loop loop_asn local local' count =
      match reads loop_asn (Local.Unnamed local) (Local.Unnamed local') with
      | None -> None
      | Some new_asn ->
        if count = 1 then Some new_asn
        else loop new_asn (local + 1) (local' + 1) (count - 1)
      in
    loop asn first1 first2 count1
  | AwaitAll _, AwaitAll _ -> failwith "AwaitAll requires unnamed locals"
  | _ -> None

let check_instruct_gen_creation_execution i i' =
  match i, i' with
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | CreateCont, _ | ContEnter, _ | ContRaise, _ | Yield, _ | YieldK, _
  | ContCheck _, _ | ContValid, _ | ContStarted, _ | ContKey, _ | ContCurrent, _
  | ContGetReturn, _ ->
    if i=i' then Some () else None

let check_instruct_eval_defined i i' =
  match i, i' with
  | DefCls cid, DefCls cid'
  | DefClsNop cid, DefClsNop cid' ->
    classes_to_check := IntIntPermSet.add (cid,cid',[]) !classes_to_check;
    Some ()
  | DefFunc fid, DefFunc fid' ->
    functions_to_check := IntIntSet.add (fid,fid') !functions_to_check;
    Some ()
  | DefTypeAlias tid, DefTypeAlias tid' ->
    typedefs_to_check := IntIntSet.add (tid, tid') !typedefs_to_check;
    Some ()
  | DefCls _, _ | DefClsNop _, _ | DefFunc _, _ | DefTypeAlias _, _ ->
    None
  (* Whitelist the instructions where equality implies equivalence
    (e.g. they do not access locals). *)
  | Incl, _ | InclOnce, _ | Req, _ | ReqOnce, _ | ReqDoc, _ | Eval, _
  | AliasCls _, _ | DefCns _, _ ->
    if i=i' then Some() else None

(* abstracting this out in case we want to change it from a list later *)
let add_todo (pc,pc') asn todo =
  ((pc,pc'),asn) :: todo

let lookup_assumption (pc,pc') assumed =
  match PcpMap.get (pc,pc') assumed with
  | None -> AsnSet.empty (* empty disjunction = false *)
  | Some asns -> asns

(* this returns assumed[pc,pc' -> assumed(pc,pc') \/ asn] *)
let add_assumption (pc,pc') asn assumed =
  let prev = lookup_assumption (pc,pc') assumed in
  if AsnSet.exists (fun a -> entails_asns asn a) prev
  then assumed
  else let filtered = AsnSet.filter (fun a -> not (entails_asns a asn)) prev in
       let updated = AsnSet.add asn filtered in
         PcpMap.add (pc,pc') updated assumed

(* Compute the permutation mapping. Not very efficiently, but lists should
  always be very small. *)
let findperm l1 l2 =
  if List.contains_dup l1 || (List.length l1 <> List.length l2)
  then None
  else let rec loop n l p =
    match l with
    | [] -> Some p
    | x :: xs ->
      begin match List.findi l2 (fun _i y -> x = y) with
      | Some (j,_) -> loop (n+1) xs ((n,j)::p)
      | None -> None
      end in
    loop 0 l1 []

(* Main entry point for equivalence checking *)
let equiv prog prog' startlabelpairs =
  let (labelmap, trymap) = make_label_try_maps prog in
  let exnmap = make_exntable prog labelmap trymap in
  let (labelmap', trymap') = make_label_try_maps prog' in
  let exnmap' = make_exntable prog' labelmap' trymap' in
  let prog_array = Array.of_list prog in
  let prog_array' = Array.of_list prog' in

  (* Revising for better treatment of loops, this now returns an optional
     next step on the left, which is a located assertion.
     We don't bother about assumptions or todos at this point, as they
     were all updated in the same way anyway.
     We include the initial located assertion as an arg, as we might
     call this several times when looking for loops, and one-sided transitions
     can validly change the assertion.
  *)
  let one_side_left (pc,pc', ((props,vs,vs') as asn)) =
  if ip_of_pc pc <> -1
  then begin match prog_array.(ip_of_pc pc) with
  | IContFlow (Jmp lab)
  | IContFlow (JmpNS lab) ->
     Some ((hs_of_pc pc, LabelMap.find lab labelmap), pc', asn)

  (* TryCatchMiddle moves exception info from exn stack to evaluation stack
     so *shouldn't* be treated as a dynamic nop here *)
  | ITry TryCatchBegin
  | ITry TryCatchEnd
  | ITry TryFaultBegin _
  | ITry TryFaultEnd
  | ITry TryCatchLegacyBegin _
  | ITry TryCatchLegacyEnd
  | ILabel _
  | IComment _
  | IBasic Nop
  | IBasic EntryNop ->
     Some (succ pc, pc', asn)

  | ISrcLoc p when p.line_begin < 0 ->
      Some (succ pc, pc', asn)

  | IContFlow Unwind ->
    begin match hs_of_pc pc with
      | [] -> (* empty now means we're unwinding out of frame *)
         Some (([], -1), pc', asn)
      | Fault_handler next :: hs ->
         Some ((hs, next), pc', asn)
      | Catch_handler next :: hs ->
         Some ((hs, next), pc', asn)
    end
  | IMutator (UnsetL l)
    when (!lax_unset) || (not (VarSet.mem l vs)) ->
     let newprops = PropSet.filter (fun (x1,_x2) -> x1 <> l) props in
     let newasn = (newprops, VarSet.remove l vs, vs') in
       Some (succ pc, pc', newasn)
  | _ -> None
  end
  else None in

  (* likewise, this now returns an option *)
  let one_side_right (pc,pc', ((props,vs,vs') as asn)) =
  if ip_of_pc pc' <> -1
  then begin match prog_array'.(ip_of_pc pc') with
  | IContFlow(Jmp lab')
  | IContFlow(JmpNS lab') ->
     Some (pc, (hs_of_pc pc', LabelMap.find lab' labelmap'), asn)

  | ITry TryCatchBegin
  | ITry TryCatchEnd
  | ITry TryFaultBegin _
  | ITry TryFaultEnd
  | ITry TryCatchLegacyBegin _
  | ITry TryCatchLegacyEnd
  | ILabel _
  | IComment _
  | IBasic Nop
  | IBasic EntryNop ->
     Some (pc, succ pc', asn)

  | ISrcLoc p when p.line_begin < 0 ->
     Some (pc, succ pc', asn)

  | IContFlow Unwind ->
    begin match hs_of_pc pc' with
      | [] ->
         Some (pc, ([],-1), asn)
      | Fault_handler next' :: hs' ->
         Some (pc, (hs', next'), asn)
      | Catch_handler next' :: hs' ->
         Some (pc, (hs', next'), asn)
    end
  | IMutator (UnsetL l')
    when (!lax_unset) || (not (VarSet.mem l' vs')) ->
     let newprops = PropSet.filter (fun (_x1,x2) -> x2 <> l') props in
     let newasn = (newprops, vs, VarSet.remove l' vs') in
       Some (pc, succ pc', newasn)
  | _ -> None
  end
  else None in

  let rec check pc pc' asn assumed todo =



    (* both_sides_now does the normal thing of matching instructions.
       Note that returning None here means overall success, unlike the one-sided
       things, which return None if they can't make a move.
     *)
    let both_sides_now (pc,pc',asn) =
    let try_specials () = specials pc pc' asn assumed todo in

    let exceptional_pc pc exnmap =
      let static =  match IMap.get (ip_of_pc pc) exnmap with
       | None
       | Some [] -> []
       | Some statichandlers -> statichandlers in
      let dynamic = hs_of_pc pc in
      throw_pc static dynamic in

    (* update todo with throwing from both sides *)
    let exceptional_todo () =
      match exceptional_pc pc exnmap, exceptional_pc pc' exnmap' with
      | None, None -> todo
      | Some epc, Some epc' -> add_todo (epc,epc') asn todo
      | Some epc, None -> add_todo (epc,([],-1)) asn todo
      | None, Some epc' -> add_todo (([],-1), epc') asn todo in

    (* Check the next instruction; no change to the assertion. *)
    let nextins () =
      let newtodo = exceptional_todo () in
        check (succ pc) (succ pc') asn
          (add_assumption (pc,pc') asn assumed) newtodo in

    (* Check the next instruction; the assertion has changed.
       We assume it's not had its stepcount incremented yet
       so do that here. *)
    let nextinsnewasn newasn =
      let newtodo = exceptional_todo () in
        check (succ pc) (succ pc') newasn
          (add_assumption (pc,pc') asn assumed) newtodo in

    (* real body of both_sides_now *)
    if ip_of_pc pc = -1 && ip_of_pc pc' = -1
    then donext assumed todo
    else if ip_of_pc pc = -1 || ip_of_pc pc' = -1
    then Some (pc, pc', asn, assumed, todo)
    (* fail if one of them has thrown out and we haven't been able to progress
      the other one independently *)
    else begin match prog_array.(ip_of_pc pc), prog_array'.(ip_of_pc pc') with
    | IIterator (IterBreak (lab, it_list)),
      IIterator (IterBreak (lab', it_list')) ->
      if it_list = it_list'
      then check (hs_of_pc pc, LabelMap.find lab labelmap)
        (hs_of_pc pc', LabelMap.find lab' labelmap') asn
        (add_assumption (pc,pc') asn assumed) todo
      else try_specials ()
    | IContFlow (SSwitch _), _
    | _, IContFlow (SSwitch _) ->
      failwith "SSwitch not implemented"

    (* Catch instructions have to match up because they affect the stack *)
    | ITry TryCatchMiddle, ITry TryCatchMiddle -> nextins()

    (* NativeImpl behaves like return - we don't want to look at the next instruction *)
    | IMisc NativeImpl, IMisc NativeImpl ->
          donext assumed todo

    | ICall (DecodeCufIter (i1, l1)), ICall (DecodeCufIter (i2, l2))
      when i1 = i2 ->
      (* This instruction looks up $1 as a callable, and writes enough information to
         iterator %1 for FPushCufIter to be able to push an actrec, as if it had been
         given the callable. If the function is not successfully decoded, branches to
         the given offset without raising a warning.
         Since this instruction can either transfer control to label or fallthrough
         treat it as conditional jump *)
      check (succ pc) (succ pc') asn
        (add_assumption (pc,pc') asn assumed)
        (add_todo ((hs_of_pc pc, LabelMap.find l1 labelmap),
          (hs_of_pc pc', LabelMap.find l2 labelmap')) asn todo)

    | IContFlow ins, IContFlow ins' ->
        begin match ins, ins' with
        | JmpZ lab, JmpZ lab'
        | JmpNZ lab, JmpNZ lab' ->
          check (succ pc) (succ pc') asn
            (add_assumption (pc,pc') asn assumed)
            (add_todo ((hs_of_pc pc, LabelMap.find lab labelmap),
              (hs_of_pc pc', LabelMap.find lab' labelmap')) asn todo)
        | RetM n, RetM m ->
          if n=m then donext assumed todo else try_specials ()
        | RetC, RetC
        | RetV, RetV ->
          donext assumed todo

      (* Two-sided treatment of throw. Here we need the instructions to be the
         same, but we allow the handler stacks to vary
         Don't think this can be one-sided because it affects the stack *)
       | Throw, Throw ->
         begin match exceptional_pc pc exnmap, exceptional_pc pc' exnmap' with
         | None, None ->  donext assumed todo (* both leave the frame*)
         | Some epc, Some epc' ->
           check epc epc' asn (add_assumption (pc,pc') asn assumed) todo
         | None, Some epc' ->
           check ([], -1) epc' asn (add_assumption (pc,pc') asn assumed) todo
         | Some epc, None ->
           check epc ([], -1) asn (add_assumption (pc,pc') asn assumed) todo
        end

        | Switch (kind, offset, labs), Switch (kind', offset', labs')
          when kind=kind' && offset=offset' ->
          begin match List.zip labs labs' with
          | None -> try_specials () (* feebly, give up if different lengths *)
          | Some lab_pairs ->
            let hs = hs_of_pc pc in
            let hs' = hs_of_pc pc' in
            let newtodo = List.fold_right
              ~f:(fun (lab,lab') accum ->
                add_todo (
                  (hs, LabelMap.find lab labelmap),
                  (hs', LabelMap.find lab' labelmap')
                ) asn accum)
              ~init:todo lab_pairs in
            donext (add_assumption (pc,pc') asn assumed) newtodo
          end
        | _, _ -> try_specials ()
        end

    (* The next block has no interesting control flow or local variable
      effects. *)
    | IBasic ins, IBasic ins' ->
      begin match check_instruct_basic ins ins' with
      | None -> try_specials ()
      | Some () -> nextins ()
      end
    | ILitConst ins, ILitConst ins' ->
      begin match check_instruct_lit_const asn ins ins' with
      | None -> try_specials ()
      | Some newasn -> nextinsnewasn newasn
      end
    (* special cases for exiting the whole program *)
    | IOp ins, IOp ins' ->
      begin match check_instruct_operator ins ins' with
      | None -> try_specials ()
      | Some true -> donext assumed todo (* termination *)
      | Some false -> nextins ()
      end
    | ISpecialFlow ins, ISpecialFlow ins' ->
      begin match check_instruct_special_flow ins ins' with
      | None -> try_specials ()
      | Some () -> nextins ()
      end
    | IAsync ins, IAsync ins' ->
      begin match check_instruct_async_functions asn ins ins' with
      | None -> try_specials ()
      | Some newasn -> nextinsnewasn newasn
      end
    | IGenerator ins, IGenerator ins' ->
      begin match check_instruct_gen_creation_execution ins ins' with
      | None -> try_specials ()
      | Some () -> nextins ()
      end
    | IIncludeEvalDefine ins, IIncludeEvalDefine ins' ->
      begin match check_instruct_eval_defined ins ins' with
      | None -> try_specials ()
      | Some () -> nextins ()
      end
    | ICall ins, ICall ins' ->
      begin match check_instruct_call asn ins ins' with
      | None -> try_specials()
      | Some newasn -> nextinsnewasn newasn
      end
    | IGet ins, IGet ins' ->
      begin match check_instruct_get asn ins ins' with
      | None -> try_specials ()
      | Some newasn -> nextinsnewasn newasn
      end
    | IIsset ins, IIsset ins' ->
      begin match check_instruct_isset asn ins ins' with
      | None -> try_specials ()
      | Some newasn -> nextinsnewasn newasn
      end
    | IMutator ins, IMutator ins' ->
      begin match check_instruct_mutator asn ins ins' with
      | None -> try_specials ()
      | Some newasn -> nextinsnewasn newasn
      end
    | IBase ins, IBase ins' ->
      begin match check_instruct_base asn ins ins' with
      | None -> try_specials ()
      | Some newasn -> nextinsnewasn newasn
      end
    | IFinal ins, IFinal ins' ->
      begin match check_instruct_final asn ins ins' with
      | None -> try_specials ()
      | Some newasn -> nextinsnewasn newasn
      end
    | IMisc ins, IMisc ins' ->
      begin match check_instruct_misc asn ins ins' with
      | None -> try_specials ()
      | Some newasn -> nextinsnewasn newasn
      end
    (* Iterator instructions have multiple exit points, so have to add to
      todos as well as looking at next instruction.
      TODO: exceptional exits from here.
      TODO: this looks like it can be simplified considerably, as there's
            only a maximum of one extra exit, in fact. So the list
            stuff is overkill - just use an option, or maybe even
            move the calculation of the new todo list back into
            check_instruct_iterator *)
    | IIterator ins, IIterator ins' ->
      begin match check_instruct_iterator asn ins ins' with
      | (None, _) -> try_specials ()
      | (Some newasn, newtodos) ->
        let label_to_pc_with_hs pc l lm = (hs_of_pc pc, LabelMap.find l lm) in
        let rewritten_todos = List.map newtodos (fun ((l,l'),asn) ->
          let pc = label_to_pc_with_hs pc l labelmap in
          let pc' = label_to_pc_with_hs pc' l' labelmap' in
          ((pc, pc'), asn)
        ) in
        (* Add the rewritten new todos to list to be processed *)
        let check_todos = List.fold_left rewritten_todos ~init:todo
          ~f:(fun td ((pc,pc'),asn) -> add_todo (pc,pc') asn td) in
        check (succ pc) (succ pc') newasn
          (add_assumption (pc,pc') asn assumed)
          check_todos
      end
    | _, _ -> try_specials ()
    end in

    (* eagerly constructing the strings here turns out to be expensive ! *)
    let logstate () =
      if !Log.verbosity_level > 2
      then begin
      Log.trace (Tty.Normal Tty.White) @@ Printf.sprintf
        "pc=%s, pc'=%s, i=%s i'=%s asn=%s\nAssumed=\n%s\nTodo=%s"
        (string_of_pc pc) (string_of_pc pc')
        (string_of_nth_instruction prog pc)
        (string_of_nth_instruction prog' pc')
        (asntostring asn) (labasnsmaptostring assumed)
        (labasnlisttostring todo);
      Log.trace (Tty.Normal Tty.Blue) "*******"
      end
      else () in

    (* main body of checking function here *)
    if List.length (hs_of_pc pc) > 10 (* arbitrary limit *)
    then begin
      Log.error ~level:0 (Tty.Normal Tty.Red) ("runaway: " ^ string_of_pc pc);
      (* COMPLETENESS: fail, dump state
       *)
      Some (pc, pc', asn, assumed, todo)
    end
    else
      let () = logstate () in
      let previous_assumptions = lookup_assumption (pc,pc') assumed in
      if AsnSet.exists (fun assasn -> entails_asns asn assasn)
          previous_assumptions then
        (* that's a clumsy attempt at entailment asn => \bigcup prev_asses *)
        donext assumed todo
      else if AsnSet.cardinal previous_assumptions > 7 (* arbitrary bound *)
          then (
        Log.error ~level:0 (Tty.Normal Tty.Blue) ("disjunction limit exceeded");
        specials pc pc' asn assumed todo
      ) else (
        let (pc, pc') = Loc.check_srcloc prog_array prog_array' pc pc' in
        match one_side_left (pc,pc',asn) with
         | None ->
           let rec possible_right_loop sofar (pc,pc',asn) =
             match one_side_right (pc,pc',asn) with
              | None -> both_sides_now (pc,pc',asn)
              | Some (newpc,newpc',newasn) ->
                if List.exists sofar (fun (apc,apc',anasn) ->
                                      apc=newpc && apc'=newpc' && entails_asns anasn newasn)
                then begin (* got a loop against a stuck, so fail *)
                  Log.debug (Tty.Normal Tty.Blue) "Right loop detected";
                  Some (pc,pc',asn,assumed,todo)
                end else possible_right_loop ((pc,pc',asn)::sofar) (newpc,newpc',newasn)
            in possible_right_loop [] (pc, pc',asn)
         | Some (newpc,newpc',newasn) -> begin
            match one_side_right (newpc,newpc',newasn) with
             | Some (bothnewpc,bothnewpc',bothnewasn) -> (* both stepped, so progress *)
                check bothnewpc bothnewpc' bothnewasn
                      (add_assumption (pc,pc') asn assumed) todo
             | None ->
               let rec possible_left_loop sofar (pc,pc',asn) =
                 match one_side_left (pc,pc',asn) with
                  | None -> both_sides_now (pc,pc',asn)
                  | Some (newpc,newpc',newasn) ->
                    if List.exists sofar (fun (apc,apc',anasn) ->
                                          apc=newpc && apc'=newpc' && entails_asns anasn newasn)
                    then begin
                      Log.debug (Tty.Normal Tty.Blue) "Left loop detected";
                      Some (pc,pc',asn,assumed,todo)
                    end else
                      possible_left_loop ((pc,pc',asn)::sofar) (newpc,newpc',newasn)
                in possible_left_loop [(pc,pc',asn)] (newpc,newpc',newasn)
          end
        )

  and donext assumed todo =
    match todo with
    | [] -> None (* success *)
    | ((pc,pc'),asn)::rest -> check pc pc' asn assumed rest

  (* Check for ad-hoc equivalences.

    Check is more or less uniform - it deals with matching instructions modulo
    local variable matching, and simple control-flow differences. `specials` deals
    with slightly deeper, ad hoc properties of particular instructions, or
    sequences. We assume we've already called check on the two pcs, so don't have
    an appropriate assumed assertion, and the instructions aren't the same.
  *)
  and specials pc pc' ((props,vs,vs') as asn) assumed todo =
    (* a funny almost no-op that shows up sometimes *)
    let set_pop_get_pattern =
      (uSetL $$ uPopC $$ uPushL)
      $? (fun ((l1,_),l2) -> l1=l2)
      $> (fun ((l,_),_) -> l) in

    let set_pop_get_action_left =
      (set_pop_get_pattern $*$ parse_any)
      $>> (fun (l, _) ((_,n),(_,n')) ->
        let newprops = PropSet.filter (fun (x1,_x2) -> x1 <> l) props in
        let newasn = (newprops, VarSet.remove l vs, vs') in
        let newpc = (hs_of_pc pc, n) in
        let newpc' = (hs_of_pc pc', n') in (* always = pc' in fact *)
        check newpc newpc' newasn (add_assumption (pc,pc') asn assumed) todo) in

    let set_pop_get_action_right =
      (parse_any $*$ set_pop_get_pattern)
      $>> (fun (_, l) ((_,n),(_,n')) ->
        let newprops = PropSet.filter (fun (_x1,x2) -> x2 <> l) props in
        let newasn = (newprops, vs, VarSet.remove l vs') in
        let newpc = (hs_of_pc pc, n) in
        let newpc' = (hs_of_pc pc', n') in
        check newpc newpc' newasn (add_assumption (pc,pc') asn assumed) todo) in

    let not_jmpnz_pattern = uNot $$ uJmpNZ $> (fun (_,l) -> l) in
    let not_jmpz_pattern = uNot $$ uJmpZ $> (fun (_,l) -> l) in
    let jmpz_not_jmpnz_pattern = uJmpZ $*$ not_jmpnz_pattern in
    let not_jmpnz_jmpz_pattern = not_jmpnz_pattern $*$ uJmpZ in
    let jmpnz_not_jmpz_pattern = uJmpNZ $*$ not_jmpz_pattern in
    let not_jmpz_jmpnz_pattern = not_jmpz_pattern $*$ uJmpNZ in
    let notjmp_action =
      (jmpz_not_jmpnz_pattern
        $| not_jmpnz_jmpz_pattern
        $| not_jmpz_jmpnz_pattern
        $| jmpnz_not_jmpz_pattern)
      $>> (fun (lab,lab') ((_,n),(_,n')) ->
        let newpc = (hs_of_pc pc, n) in
        let newpc' = (hs_of_pc pc', n') in
        let newTodo = (
          (hs_of_pc pc, LabelMap.find lab labelmap),
          (hs_of_pc pc', LabelMap.find lab' labelmap')) in
        check newpc newpc' asn (add_assumption (pc,pc') asn assumed)
          (add_todo newTodo asn todo)) in


    (* associativity of string concatenation *)
    let string_concat_concat_pattern =
      (uString $$ uConcat $$ uConcat)
      $> (fun ((s,_),_) -> s) in
    let concat_string_concat_pattern =
      (uConcat $$ uString $$ uConcat)
      $> (fun ((_,s),_) -> s) in
    let concat_string_either_pattern =
      (string_concat_concat_pattern $*$ concat_string_concat_pattern) $|
      (concat_string_concat_pattern $*$ string_concat_concat_pattern) $?
      (fun (s,s') -> s=s') in
    let concat_string_either_action =
      concat_string_either_pattern
      $> (fun (s,_) -> s)
      $>> (fun _s ((_,n), (_,n')) ->
        let newpc = (hs_of_pc pc, n) in
        let newpc' = (hs_of_pc pc', n') in
          check newpc newpc' asn (add_assumption (pc,pc') asn assumed) todo) in

    let concat_or_caststring_or_string_pattern =
      (uConcat $> ignore) $|
      (uCastString $> ignore) $|
      (uString $> ignore) in

    let previous_instruction_pushes_string_pattern =
      back concat_or_caststring_or_string_pattern in

    (* string + "" is no op *)
    let previous_instruction_pushes_string_concat_empty_string_pattern =
      previous_instruction_pushes_string_pattern $$
      (uString $? (fun s -> s = "")) $$
      uConcat
      $> ignore in

    let string_concat_empty_string_is_noop_pattern =
      previous_instruction_pushes_string_concat_empty_string_pattern $*$|
      previous_instruction_pushes_string_pattern in

    let string_concat_empty_string_is_noop_action =
      string_concat_empty_string_is_noop_pattern
      $>> (fun _ ((_,n), (_,n')) ->
      let newpc = (hs_of_pc pc, n) in
      let newpc' = (hs_of_pc pc', n') in
      check newpc newpc' asn (add_assumption (pc,pc') asn assumed) todo) in

    let empty_string_cgetl2_concat_pattern =
      (uString $? (fun s -> s = "") $$ uCGetL2 $$ uConcat)
      $> (fun ((_empty_string, loc), _) -> loc) in
    let cgetl_caststring_pattern =
      (uCGetL $$ uCastString)
      $> (fun (loc, _) -> loc) in
    let concat_caststring_pattern =
      empty_string_cgetl2_concat_pattern $*$| cgetl_caststring_pattern
      $? (fun (loc, loc') -> loc = loc')
      $> (fun (loc, _loc') -> loc) in
    let concat_caststring_action =
      concat_caststring_pattern
      $>> (fun _loc ((_, n), (_, n')) ->
        let newpc = (hs_of_pc pc, n) in
        let newpc' = (hs_of_pc pc', n') in
          check newpc newpc' asn (add_assumption (pc,pc') asn assumed) todo) in

    let negative_int_or_double = uIntOrDouble $? begin function
      | Int v -> v < 0L
      | Double s -> String.get s 0 = '-'
      | _ -> assert false
    end in

    let positive_int_or_double = uIntOrDouble $? begin function
      | Int v -> v >= 0L
      | Double s -> String.get s 0 <> '-'
      | _ -> assert false
    end in

    let zero_minus_number =
      uInt0 $$ positive_int_or_double $$ uSub
      $> (fun ((_, v), _) -> match v with
          | Int v -> Int (Int64.sub 0L v)
          | Double s -> Double ("-" ^ s)
          | _ -> assert false) in

    let negative_number_vs_zero_minus_action =
      negative_int_or_double $*$| zero_minus_number
      $? (fun (l1, l2) -> match l1, l2 with
          | Int v1, Int v2 -> v1 = v2
          | Double s1, Double s2 -> compare_double_strings s1 s2
          | _ -> false)
      $> (fun (s,_) -> s)
      $>> (fun _s ((_,n), (_,n')) ->
        let newpc = (hs_of_pc pc, n) in
        let newpc' = (hs_of_pc pc', n') in
          check newpc newpc' asn (add_assumption (pc,pc') asn assumed) todo) in

     (* genuinely flipped conditional branches *)
     let flipped_jump_pattern = (uJmpZ $*$ uJmpNZ) $| (uJmpNZ $*$ uJmpZ) in
     let flipped_jump_action =
       flipped_jump_pattern $>> (fun (lab,lab') ((_,n), (_,n')) ->
         let newpc = (hs_of_pc pc, n) in
         let newpc' = (hs_of_pc pc', n') in
         let brpc = (hs_of_pc pc, LabelMap.find lab labelmap) in
         let brpc' = (hs_of_pc pc', LabelMap.find lab' labelmap') in
         check newpc brpc' asn (add_assumption (pc, pc') asn assumed)
             (add_todo (brpc, newpc') asn todo)) in

    let print_constant_strings_pattern =
      greedy_kleene (uString $$ uPrint $$ uPopC) in

    let two_print_constant_strings_action =
      (print_constant_strings_pattern $*$ print_constant_strings_pattern)
      $? (function
        | [], _ | _, [] -> false
        | l1, l2 ->
          let s1 = List.map l1 (fun ((s, _), _) -> s) |> String.concat "" in
          let s2 = List.map l2 (fun ((s, _), _) -> s) |> String.concat "" in
          s1 = s2)
      $> (fun (s,_) -> s)
      $>> (fun _s ((_,n), (_,n')) ->
        let newpc = (hs_of_pc pc, n) in
        let newpc' = (hs_of_pc pc', n') in
          check newpc newpc' asn (add_assumption (pc,pc') asn assumed) todo) in

    (* recognize closure creation *)
    let named_filter foo =
      foo
      $? (function | Local.Unnamed _ -> false | Local.Named _ -> true)
      $> (function
        | Local.Unnamed _ -> failwith "unnamed can't happen"
        | Local.Named s -> s) in

    let cugetl_named_pattern = named_filter uCUGetL in

    let cugetl_list_pattern = greedy_kleene cugetl_named_pattern in
    let cugetl_list_createcl_pattern =
      cugetl_list_pattern
      $$ uCreateCl
      $? (fun (cugets,(np,_cn)) -> np >= List.length cugets) in

    (* Dealing with equivalence of closure creation *)
    let two_cugetl_list_createcl_pattern =
      cugetl_list_createcl_pattern $*$ cugetl_list_createcl_pattern
      $> (fun ((cugets,(np,cn)), (cugets',(np',cn'))) ->
        findperm cugets cugets', np, cn, np', cn')
      $? (function | (Some _perm, np, _cn, np', _cn') -> np=np' | _ -> false)
      $> (function
        | (Some perm,_np,cn,_np',cn') -> (perm,cn,cn')
        | _ -> failwith "perms can't happen") in
    let two_cugetl_list_createcl_action =
      two_cugetl_list_createcl_pattern
      $>> (fun (perm,cn,cn') ((_,n),(_,n')) ->
        Log.debug (Tty.Normal Tty.Blue) @@
          Printf.sprintf "create cl pattern at lines %d, %d" n n';
        let newpc = (hs_of_pc pc, n) in
        let newpc' = (hs_of_pc pc', n') in
        classes_to_check := IntIntPermSet.add (cn,cn',perm) (!classes_to_check);
        check newpc newpc' (props, vs, vs') assumed todo) in

    let string_fatal_pattern = uString $$ uFatal in
    let two_string_fatal_pattern =
      (string_fatal_pattern $*$ string_fatal_pattern)
      $? (fun ((_s,fop),(_s',fop')) -> fop = fop') in
    let two_string_fatal_action =
      two_string_fatal_pattern
      (* success, nothing more to check here *)
      $>> (fun _ _ -> donext assumed todo) in

    let unnamed_filter foo =
      foo
      $? (function | Local.Unnamed _ -> true | Local.Named _ -> false)
      $> (function
        | Local.Unnamed n -> n
        | Local.Named _ -> failwith "named can't happen") in

    (* patterns for cases in which VGetL _n is generated by unnatural foreach
        statements. Although we briefly take a reference, it behaves like a read of
        that local followed by unsetting, so the ref doesn't escape.
      In the basel-bindm case we also read the local that goes into base and
      possibly a local that's used as a memberkey *)
    let vget_unnamed_pattern = unnamed_filter uVGetL in
    let unsetl_unnamed_pattern = unnamed_filter uUnsetL in
    let vget_base_pattern =
      (vget_unnamed_pattern
        $$ uBaseL
        $$ uBindM
        $$ uPopV
        $$ unsetl_unnamed_pattern)
      $? (fun ((((n1,(_loc,_op)),(_n,_key)),_),n2) -> n1=n2)
      $> (fun ((((n1,(loc,op)),(n,key)),_),_n2) -> (n1,loc,op,n,key)) in
    let two_vget_base_pattern =
      vget_base_pattern $*$ vget_base_pattern
      $? (fun ((_n1,_loc,op,n,_key),(_n1',_loc',op',n',_key')) ->
        op=op' && n=n')
      $> (fun ((n1,loc,_op,_n,key),(n1',loc',_op',_n',key')) ->
        ((n1,loc,key),(n1',loc',key'))) in
    let two_vget_base_action =
      two_vget_base_pattern
      $>> (fun ((n1,loc,key),(n1',loc',key')) ((_,ip),(_,ip')) ->
        Log.debug (Tty.Normal Tty.Blue) @@
          Printf.sprintf "vget base pattern %d to %d" n1 n1';
        match reads asn (Local.Unnamed n1) (Local.Unnamed n1') with
        | None -> Some (pc, pc', asn, assumed, todo) (* fail *)
        | Some new_asn ->
          begin match reads new_asn loc loc' with
          | None -> Some (pc, pc', asn, assumed, todo)
          | Some new_asn2 ->
            let continuation (some_props,some_vs,some_vs') =
              let newpc =(hs_of_pc pc, ip) in
              let newpc' = (hs_of_pc pc', ip') in
              let newprops = PropSet.filter (fun (x,x') ->
                x <> Local.Unnamed n1 && x' <> Local.Unnamed n1') some_props in
              let final_asn = (
                newprops,
                VarSet.remove (Local.Unnamed n1) some_vs,
                VarSet.remove (Local.Unnamed n1') some_vs'
                ) in
              check newpc newpc' final_asn
                (add_assumption (pc,pc') asn assumed) todo in
            begin match key, key' with
            | MemberKey.EL l, MemberKey.EL l'
            | MemberKey.PL l, MemberKey.PL l' ->
              begin match reads new_asn2 l l' with
              | None -> Some (pc,pc',asn,assumed,todo) (*fail *)
              | Some new_asn3 -> continuation new_asn3
              end
            | _,_ ->
              if key=key'
              then continuation new_asn2
              else Some (pc,pc',asn,assumed,todo)
            end
          end
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
      two_vget_cget_bind_pattern
      $>> (fun ((n,optl),(n',optl')) ((_,ip),(_,ip')) ->
        Log.debug (Tty.Normal Tty.Blue)
          @@ Printf.sprintf "vget cget pattern %d to %d" n n';
        match reads asn (Local.Unnamed n) (Local.Unnamed n') with
        | None -> Some(pc,pc',asn,assumed,todo)
        | Some new_asn ->
          let continuation (some_props,some_vs,some_vs') =
            let newpc =(hs_of_pc pc, ip) in
            let newpc' = (hs_of_pc pc', ip') in
            let newprops = PropSet.filter (fun (x,x') ->
              x <> Local.Unnamed n && x' <> Local.Unnamed n') some_props in
            let final_asn = (
              newprops,
              VarSet.remove (Local.Unnamed n) some_vs,
              VarSet.remove (Local.Unnamed n') some_vs') in
            check newpc newpc' final_asn
              (add_assumption (pc,pc') asn assumed) todo in
          begin match optl,optl' with
          | None, None -> continuation new_asn
          | Some l, Some l' ->
            begin match reads new_asn l l' with
            | None -> Some(pc,pc',asn,assumed,todo)
            | Some new_asn2 -> continuation new_asn2
            end
          | _,_ -> failwith "vget cget can't happen"
          end) in

    let vget_binds_pattern =
      (vget_unnamed_pattern $$ uBindS $$ uPopV $$ uUnsetL)
      $? (fun (((n1,_cn),_),n2) -> Local.Unnamed n1 = n2)
      $> (fun (((n,cn),_),_) -> (n,cn)) in
    let two_vget_binds_pattern =
      vget_binds_pattern $*$ vget_binds_pattern in
    let two_vget_binds_action =
      two_vget_binds_pattern
     $>> (fun ((n,cn), (n',cn')) ((_,ip),(_,ip'))->
      begin
       Log.debug (Tty.Normal Tty.Blue)
         @@ Printf.sprintf "vget binds pattern %d to %d, class_refs %d and %d" n n' cn cn';
       match reads asn (Local.Unnamed n) (Local.Unnamed n') with
        | None -> Some(pc,pc',asn,assumed,todo)
        | Some (new_props, new_vs, new_vs') ->
           let newpc = (hs_of_pc pc, ip) in
           let newpc' =(hs_of_pc pc', ip') in
           let finalprops = PropSet.filter (fun (x,x') ->
             x <> Local.Unnamed n && x' <> Local.Unnamed n') new_props in
             let final_asn = (
               finalprops,
               VarSet.remove (Local.Unnamed n) new_vs,
               VarSet.remove (Local.Unnamed n') new_vs') in
         begin
          classes_to_check := IntIntPermSet.add (cn,cn',[]) (!classes_to_check);
          check newpc newpc' final_asn
            (add_assumption (pc,pc') asn assumed) todo
         end
      end) in

    let vget_retv_pattern =
      vget_unnamed_pattern
      $$ uRetV
      $> (fun (n,_) -> n) in
    let two_vget_retv_pattern = vget_retv_pattern $*$ vget_retv_pattern in
    let two_vget_retv_pattern_action =
      two_vget_retv_pattern
      $>> (fun (n,n') (_,_) ->
        match reads asn (Local.Unnamed n) (Local.Unnamed n') with
        | None -> Some(pc,pc',asn,assumed,todo)
        | Some _new_asn ->
          donext (add_assumption (pc,pc') asn assumed) todo
      ) in

    (* Isset and branch where we know statically that a variable is
      unset. This should morally be in one_side_x, but we don't currently do
      any non-trivial pattern-matching there, so I'm putting it here, even
      though it then can't participate in the loop-detecting code *)
    let issetl_jmpz_pattern varset =
      ((unnamed_filter uIssetL) $$ uJmpZ)
      $? (fun (n, _lab) -> not (VarSet.mem (Local.Unnamed n) varset)) in

    let issetl_jmpz_action_left =
      ((issetl_jmpz_pattern vs) $*$ parse_any)
     $>> (fun ((_local,lab),_) (_,_) ->
           let newpc = (hs_of_pc pc, LabelMap.find lab labelmap) in
           check newpc pc' asn (add_assumption (pc,pc') asn assumed) todo) in

    let issetl_jmpz_action_right =
      (parse_any $*$ (issetl_jmpz_pattern vs'))
     $>> (fun (_, (_local',lab')) (_,_) ->
       let newpc' = (hs_of_pc pc', LabelMap.find lab' labelmap') in
       check pc newpc' asn (add_assumption (pc,pc') asn assumed) todo) in

   (* last, failure, case for use in bigmatch *)
   let failure_pattern_action =
     parse_any
     $>> (fun _ _ -> Some (pc, pc', asn, assumed, todo)) in

    let bigmatch_action = bigmatch [
      set_pop_get_action_left;
      set_pop_get_action_right;
      notjmp_action;
      flipped_jump_action;
      concat_string_either_action;
      concat_caststring_action;
      two_print_constant_strings_action;
      two_string_fatal_action;
      two_cugetl_list_createcl_action;
      two_vget_base_action;
      two_vget_cget_bind_action;
      two_vget_retv_pattern_action;
      two_vget_binds_action;
      issetl_jmpz_action_left;
      issetl_jmpz_action_right;
      negative_number_vs_zero_minus_action;
      string_concat_empty_string_is_noop_action;

      failure_pattern_action; (* This one should stay last *)
      ] in
    bigmatch_action ((prog_array, ip_of_pc pc),(prog_array', ip_of_pc pc'))
  in

  (* We always start from ip,ip'=0 for the top entry to the function/method, but
    also take startlabelpairs, which is  list of pairs of labels from the two
    programs, as alternative entry points. These are used for default param
    values *)
  let initialtodo = List.map startlabelpairs
    ~f:(fun (lab,lab') -> (
      (([],LabelMap.find lab labelmap), ([],LabelMap.find lab' labelmap')),
      entry_assertion
    )) in

  check ([],0) ([],0) entry_assertion PcpMap.empty initialtodo
