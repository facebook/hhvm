(* flowtypes.ml
   experimental toy model of a flow-sensitive typechecker
*)

type mytype =
  | Tint
  | Tbool
  | Tstring

let mytype_tostring t =
  match t with
  | Tint -> "int"
  | Tbool -> "bool"
  | Tstring -> "string"

(* this represents a disjunction of primitive types *)
module TypeSet = Set.Make (struct
  type t = mytype

  let compare = compare
end)

type dtype =
  | D of TypeSet.t
  | Top

let bottype = D TypeSet.empty

let onetype t = D (TypeSet.singleton t)

let booltype = onetype Tbool

let inttype = onetype Tint

let stringtype = onetype Tstring

let rec tltostring tl =
  match tl with
  | [] -> "F"
  | [t] -> mytype_tostring t
  | t :: rest -> mytype_tostring t ^ "|" ^ tltostring rest

let dtype_tostring dt =
  match dt with
  | Top -> "T"
  | D ts -> tltostring (TypeSet.elements ts)

let entails dt1 dt2 =
  match (dt2, dt1) with
  | (Top, _) -> true
  | (_, Top) -> false
  | (D ds2, D ds1) -> TypeSet.for_all (fun t1 -> TypeSet.mem t1 ds2) ds1

let equaltype dt1 dt2 = entails dt1 dt2 && entails dt2 dt1

let typejoin dt1 dt2 =
  match (dt1, dt2) with
  | (Top, _) -> Top
  | (_, Top) -> Top
  | (D ds1, D ds2) -> D (TypeSet.union ds1 ds2)

type vname = string

module VMap = Map.Make (struct
  type t = vname

  let compare = compare
end)

(* Used for Gamma *)
type env =
  | M of dtype VMap.t
  | Botenv

(* maps every variable to top, so classifies any environment *)
let topenv = M VMap.empty

(* an environment type mapping any variable to bottype is Botenv *)
let normenv e =
  match e with
  | Botenv -> Botenv
  | M e ->
    if VMap.exists (fun v t -> entails t bottype) e then
      Botenv
    else
      M e

(* this is an over-approximation of the union of e1 and e2 *)
let envjoin e1 e2 =
  match (normenv e1, normenv e2) with
  | (Botenv, e2) -> e2
  | (e1, Botenv) -> e1
  | (M e1, M e2) ->
    M (VMap.union (fun v ty1 ty2 -> Some (typejoin ty1 ty2)) e1 e2)

let envupdate e v t =
  match e with
  | Botenv -> Botenv
  | M e -> normenv (M (VMap.add v t e))

let env_tostring e =
  match e with
  | Botenv -> "Botenv"
  | M e ->
    "<"
    ^ VMap.fold (fun v t s -> v ^ ":" ^ dtype_tostring t ^ "," ^ s) e ""
    ^ ">"

(* record type for deltas *)
type delta = {
  skip: env;
  break: env;
  continue: env;
}

let delta_tostring d =
  "{skip="
  ^ env_tostring d.skip
  ^ " break="
  ^ env_tostring d.break
  ^ " continue="
  ^ env_tostring d.continue
  ^ "}"

let lookup v e =
  match normenv e with
  | Botenv -> bottype
  | M e ->
    begin
      match VMap.find_opt v e with
      | None -> Top
      | Some dt -> dt
    end

(* have to be careful about tops for missing vars so do something simple but a bit expensive *)
let enventails e1 e2 =
  match (normenv e1, normenv e2) with
  | (Botenv, _) -> true
  | (_, Botenv) -> false
  | (M e1m, M e2m) ->
    VMap.for_all (fun v t1 -> entails t1 (lookup v e2)) e1m
    && VMap.for_all (fun v t2 -> entails (lookup v e1) t2) e2m

type exp =
  | Ev of vname
  | Etrue
  | Efalse
  | En of int
  | Es of string
  | Eplus of exp * exp
  | Egreater of exp * exp

type com =
  | Cskip
  | Cseq of com * com
  | Cass of vname * exp
  | Cif of exp * com * com
  | Cloop of com
  | Cbreak
  | Ccontinue
  | Cifistype of vname * dtype * com * com

let rec infer_exp e t =
  match t with
  | Ev s -> lookup s e
  | Etrue
  | Efalse ->
    booltype
  | En _ -> inttype
  | Es _ -> stringtype
  | Eplus (t1, t2) ->
    let ty1 = infer_exp e t1 in
    let ty2 = infer_exp e t2 in
    if entails ty1 inttype && entails ty2 inttype then
      inttype
    else
      failwith "plus expression"
  | Egreater (t1, t2) ->
    let ty1 = infer_exp e t1 in
    let ty2 = infer_exp e t2 in
    if entails ty1 inttype && entails ty2 inttype then
      booltype
    else
      failwith "greater expression"

(* approximate difference of types *)
let tyminus dt1 dt2 =
  match dt2 with
  | Top -> bottype
  | D ds2 ->
    begin
      match dt1 with
      | Top -> Top (* hopelessly weak, but will do for now *)
      | D ds1 -> D (TypeSet.diff ds1 ds2)
    end

let tyintersect dt1 dt2 =
  match (dt1, dt2) with
  | (Top, _) -> dt2
  | (_, Top) -> dt1
  (* TODO: This is NOT generally right - relies on primitive types being disjoint *)
  | (D ds1, D ds2) -> D (TypeSet.inter ds1 ds2)

let rec infer_com e c =
  match c with
  | Cskip -> { skip = e; break = Botenv; continue = Botenv }
  | Cseq (c1, c2) ->
    let { skip = envs1; break = envb1; continue = envc1 } = infer_com e c1 in
    let { skip = envs2; break = envb2; continue = envc2 } =
      infer_com envs1 c2
    in
    {
      skip = envs2;
      break = envjoin envb1 envb2;
      continue = envjoin envc1 envc2;
    }
  | Cass (v, ex) ->
    let ety = infer_exp e ex in
    { skip = envupdate e v ety; break = Botenv; continue = Botenv }
  | Cif (ex, c1, c2) ->
    if entails (infer_exp e ex) booltype then
      let { skip = envs1; break = envb1; continue = envc1 } = infer_com e c1 in
      let { skip = envs2; break = envb2; continue = envc2 } = infer_com e c2 in
      {
        skip = envjoin envs1 envs2;
        break = envjoin envb1 envb2;
        continue = envjoin envc1 envc2;
      }
    else
      failwith "non bool in conditional"
  | Cloop c1 ->
    let { skip = envs; break = envb; continue = envc } = infer_com e c1 in
    if enventails envs e && enventails envc e then
      { skip = envb; break = Botenv; continue = Botenv }
    else
      infer_com (envjoin (envjoin e envs) envc) c
  | Cbreak -> { skip = Botenv; break = e; continue = Botenv }
  | Ccontinue -> { skip = Botenv; break = Botenv; continue = e }
  | Cifistype (v, t, c1, c2) ->
    let vty = lookup v e in
    let yesvty = tyintersect vty t in
    let novty = tyminus vty t in
    let { skip = envs1; break = envb1; continue = envc1 } =
      infer_com (envupdate e v yesvty) c1
    in
    let { skip = envs2; break = envb2; continue = envc2 } =
      infer_com (envupdate e v novty) c2
    in
    {
      skip = envjoin envs1 envs2;
      break = envjoin envb1 envb2;
      continue = envjoin envc1 envc2;
    }

(* slightly feeble testing *)

let teste e = dtype_tostring (infer_exp topenv e)

let testc c = delta_tostring (infer_com topenv c)

let e1 = Eplus (En 1, En 2)

(* skip=<x:int> *)
let c1 = Cass ("x", e1)

(* skip=<y:int,x:bool> *)
let c2 = Cseq (c1, Cseq (Cass ("y", Ev "x"), Cass ("x", Etrue)))

(* skip=<x:int> *)
let c3 = Cloop (Cseq (c1, Cbreak))

(* skip=<x:bool> *)
let c4 = Cseq (c1, Cifistype ("x", inttype, Cass ("x", Etrue), Cass ("y", En 6)))

(* this one ends up with skip:=<x:bool,y:int> *)
let c5 =
  Cseq
    ( Cif (Etrue, Cass ("x", Etrue), Cass ("x", En 4)),
      Cifistype ("x", inttype, Cass ("x", Etrue), Cass ("y", En 6)) )

(* thing that @catg observed was wrong before
   the thing after the break is type incorrect, but we shouldn't care
   and get skip=<x:bool>
*)
let c6 =
  Cloop
    (Cseq (Cseq (Cass ("x", Etrue), Cbreak), Cass ("y", Eplus (Ev "x", En 1))))

(* example needing fixpoints, due to @ajk
  $x = 0;
  $y = 1;
  $z = 2;
  for ($i = 0; $i < 3; $i++) {
    $x = $y;
    $y = $z;
    $z = true;
  }
yields:
{skip=<z:int|bool,y:int|bool,x:int|bool,i:int,> break=Botenv continue=Botenv}
*)
let thebody =
  Cif
    ( Egreater (En 4, Ev "i"),
      Cbreak,
      Cseq
        ( Cseq (Cseq (Cass ("x", Ev "y"), Cass ("y", Ev "z")), Cass ("z", Etrue)),
          Cass ("i", Eplus (Ev "i", En 1)) ) )

let theloop = Cloop thebody

let theprequel =
  Cseq
    ( Cseq (Cseq (Cass ("x", En 0), Cass ("y", En 1)), Cass ("z", En 2)),
      Cass ("i", En 0) )

let c7 = Cseq (theprequel, theloop)
