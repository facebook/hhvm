(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* Destructors and simple parser combinators for arrays of hhbc_ast.instruct datatype
   Used for pattern-matching of special sequences in rhl
*)
open Hhbc_ast
open Core_kernel

(*  turn destructors into parsers on arrays *)
let rec pa p ?(ignore_srcloc=true) (a, n)=
  match Array.get a n with
  (* ignore srcloc *)
  | ISrcLoc _ when ignore_srcloc -> pa p ~ignore_srcloc (a, n+1)
  (* index out of bounds *)
  | exception (Invalid_argument _) -> None
  | x ->
    match p x with
    | Some v -> Some (v, (a,n+1))
    | None -> None

(* apply parser starting from previous instruction *)
let rec back p (a, n) =
  match Array.get a (n - 1) with
  | ISrcLoc _ -> back p (a, n - 1)
  | exception (Invalid_argument _) -> None
  | _ -> p (a, n - 1)

(* primitive parsers for instructions of interest *)
let uSetL = pa (function | IMutator (SetL lab) -> Some lab | _ -> None)
let uPopC = pa (function | IBasic PopC -> Some () | _ -> None)
let uPushL = pa (function | IGet (PushL lab) -> Some lab | _ -> None)
let uJmpZ = pa (function | IContFlow (JmpZ lab) -> Some lab | _ -> None)
let uNot = pa (function | IOp Not -> Some () | _ -> None)
let uJmpNZ = pa (function | IContFlow (JmpNZ lab) -> Some lab | _ -> None)
let uString = pa (function | ILitConst (String s) -> Some s | _ -> None)
let uConcat = pa (function | IOp Concat -> Some () | _ -> None)
let uCastString = pa (function | IOp CastString -> Some () | _ -> None)
let uFatal = pa (function | IOp (Fatal fop) -> Some fop | _ -> None)
let uCUGetL = pa (function | IGet (CUGetL loc) -> Some loc | _ -> None)
let uCreateCl = pa (function | IMisc (CreateCl (np,cn)) -> Some (np,cn) | _ -> None)
let uVGetL = pa (function | IGet (VGetL loc) -> Some loc | _ -> None)
let uBaseL = pa (function | IBase (BaseL (loc, op)) -> Some (loc,op) | _ -> None)
let uBindM = pa (function | IFinal (BindM (n,key)) -> Some (n,key) | _ -> None)
let uUnsetL = pa (function | IMutator (UnsetL loc) -> Some loc | _ -> None)
let uPopV = pa (function | IBasic PopV -> Some () | _ -> None)
let uBindN = pa (function | IMutator BindN -> Some () | _ -> None)
let uCGetL = pa (function | IGet (CGetL loc) -> Some loc | _ -> None)
let uCGetL2 = pa (function | IGet (CGetL2 loc) -> Some loc | _ -> None)
let uPrint = pa (function | IOp Print -> Some () | _ -> None)
let uBindS = pa (function | IMutator (BindS cid) -> Some cid | _ -> None)
let uIssetL = pa (function | IIsset (IssetL loc) -> Some loc | _ -> None)
let uInt0 = pa (function | ILitConst (Int 0L) -> Some () | _ -> None)
let uSub = pa (function | IOp (Sub | SubO) -> Some () | _ -> None)
let uIntOrDouble = pa (function
  | ILitConst ((Int _ | Double _) as v) -> Some v
  | ILitConst ((Cns s))
    when String.lowercase (Hhbc_id.Const.to_raw_string s) = "inf" -> Some (Double "inf")
  | _ -> None)
let uSrcLoc =
  pa (function ISrcLoc loc -> Some loc | _ -> None) ~ignore_srcloc:false
let uAnyInst = pa (function x -> Some x)
(* trivial parser, always succeds, reads nothing *)
let parse_any inp = Some ((),inp)

(* sequencing of parsers *)
let ($$) p1 p2 inp =
 match p1 inp with
  | None -> None
  | Some (v1,newinp) -> (match p2 newinp with
                          | None -> None
                          | Some (v2, newnewinp) -> Some ((v1,v2), newnewinp))

(* filtering - add a condition to a parser *)
let ($?) p f inp =
 match p inp with
  | Some (v, newinp) when f v -> Some (v, newinp)
  | _ -> None

(* compose parser with function *)
let ($>) p f inp =
 match p inp with
  | Some (v,newinp) -> Some (f v, newinp)
  | None -> None

(* compose with top-level function that gets the inp value too.
   Returns a thunk in an attempt to ensure tail-recursion.
*)
let ($>>) p f inp =
 match p inp with
  | Some (v, newinp) -> Some (fun () -> f v newinp)
  | None -> None

let rec bigmatch ps inp =
 match ps with
  | [] -> failwith "top level match failure"
  | p :: rest ->
    (match p inp with
      | None -> bigmatch rest inp
      | Some f -> f())

(* alternation *)
let ($|) p1 p2 inp =
 match p1 inp with
  | Some (v,newinp) -> Some (v, newinp)
  | None -> p2 inp

(* parsing pairs with pairs of parsers
   Do it sequentially, as it seems more efficient
*)
let ($*$) p p' (inp,inp') =
 match p inp with
  | Some (v,newinp) ->
     (match p' inp' with
       | Some (v', newinp') -> Some ((v,v'), (newinp,newinp'))
       | None -> None)
  | None -> None

let ($*$|) p p' =
  (p $*$ p') $| (p' $*$ p)

(* Parse a sequence of the same instruction.
   This isn't tail recursive, but that should be OK *)
let rec greedy_kleene p inp =
 match p inp with
  | Some (v,newinp) ->
      (match greedy_kleene p newinp with
        | Some (rest,endinp) -> Some (v::rest, endinp)
        | None -> None) (* can't happen *)
  | None -> Some ([],inp)
