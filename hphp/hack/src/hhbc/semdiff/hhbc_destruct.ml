(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Destructors and simple parser combinators for arrays of hhbc_ast.instruct datatype
   Used for pattern-matching of special sequences in rhl
*)
open Hhbc_ast

(*  turn destructors into parsers on arrays *)
let pa p (a, n) =
 match p (Array.get a n) with
  | Some v -> Some (v, (a,n+1))
  | None -> None
  | exception (Invalid_argument _) -> None (* Out of bounds *)

(* primitive parsers for instructions of interest *)
let uSetL = pa (function | IMutator (SetL lab) -> Some lab | _ -> None)
let uPopC = pa (function | IBasic PopC -> Some () | _ -> None)
let uPushL = pa (function | IGet (PushL lab) -> Some lab | _ -> None)
let uJmpZ = pa (function | IContFlow (JmpZ lab) -> Some lab | _ -> None)
let uNot = pa (function | IOp Not -> Some () | _ -> None)
let uJmpNZ = pa (function | IContFlow (JmpNZ lab) -> Some lab | _ -> None)
let uString = pa (function | ILitConst (String s) -> Some s | _ -> None)
let uConcat = pa (function | IOp Concat -> Some () | _ -> None)
let uFPassL = pa (function | ICall (FPassL (param, local, hint)) -> Some (param,local,hint) | _ -> None)
let uFPassC = pa (function | ICall (FPassC (param, hint)) -> Some (param,hint) | _ -> None)
let uFPassCW = pa (function | ICall (FPassCW (param, hint)) -> Some (param,hint) | _ -> None)
let uFPassCE = pa (function | ICall (FPassCE (param, hint)) -> Some (param,hint) | _ -> None)
let uFPassV = pa (function | ICall (FPassV (param, hint)) -> Some (param,hint) | _ -> None)
let uFPassVNop = pa (function | ICall (FPassVNop (param,hint)) -> Some (param,hint) | _ -> None)
let uFPassR = pa (function | ICall (FPassR (param, hint)) -> Some (param,hint) | _ -> None)
let uFPassN = pa (function | ICall (FPassN (param, hint)) -> Some (param,hint) | _ -> None)
let uFPassG = pa (function | ICall (FPassG (param, hint)) -> Some (param,hint) | _ -> None)
let uFPassS = pa (function | ICall (FPassS (param, cref, hint)) -> Some (param, cref, hint) | _ -> None)
let uFCall = pa (function | ICall (FCall np) -> Some np | _ -> None)
let uFCallD = pa (function | ICall (FCallD (np, clid, fid)) -> Some (np, clid, fid) | _ -> None)
let uFCallAwait =
  pa (function | ICall (FCallAwait (np, clid, fid)) -> Some (np, clid, fid) | _ -> None)
let uFCallUnpack = pa (function | ICall (FCallUnpack np) -> Some np | _ -> None)
let uFCallBuiltin =
  pa (function | ICall (FCallBuiltin (np1, np2, str)) -> Some (np1, np2, str) | _ -> None)
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
let uRetV = pa (function | IContFlow (RetV) -> Some () | _ -> None)

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
  | p :: rest -> (match p inp with
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

(* this isn't tail recursive, but that should be OK *)
let rec greedy_kleene p inp =
 match p inp with
  | Some (v,newinp) ->
      (match greedy_kleene p newinp with
        | Some (rest,endinp) -> Some (v::rest, endinp)
        | None -> None) (* can't happen *)
  | None -> Some ([],inp)

(* optional, always succeeds *)
let maybe p inp =
 match p inp with
  | Some (v,newinp) -> Some(Some v,newinp)
  | None -> Some(None,inp)
