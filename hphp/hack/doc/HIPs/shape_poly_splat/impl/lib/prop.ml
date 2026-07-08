open StdLabels
open Repr

type atom =
  | Subtype_base of {
      sub: Base.t;
      super: Base.t;
    }
  | Subfield of {
      label: Label.t option;
      sub: Field_desc.t;
      super: Field_desc.t;
    }

type t =
  | Atom of atom
  | Disj of {
      failures: t list;
      props: t list;
    }
  | Conj of t list

let is_subtype_base ~sub ~super = Atom (Subtype_base { sub; super })

let is_subfield label ~sub ~super = Atom (Subfield { label; sub; super })

let valid = Conj []

let invalid failure = Disj { failures = [failure]; props = [] }

let invalids failures = Disj { failures; props = [] }

let is_invalid t =
  match t with
  | Disj { props = []; _ } -> true
  | _ -> false

let is_valid t =
  match t with
  | Conj [] -> true
  | _ -> false

let conj t1 t2 =
  if is_valid t1 then
    t2
  else if is_valid t2 then
    t1
  else
    match (t1, t2) with
    | (Conj ts1, Conj ts2) -> Conj (ts1 @ ts2)
    | (Disj { props = []; failures = fs1 }, Disj { props = _; failures = fs2 })
      ->
      Disj { props = []; failures = fs1 @ fs2 }
    | (Disj { props = _; failures = fs1 }, Disj { props = []; failures = fs2 })
      ->
      Disj { props = []; failures = fs1 @ fs2 }
    | (Disj { props = []; _ }, (Atom _ | Conj _)) -> t1
    | ((Atom _ | Conj _), Disj { props = []; _ }) -> t2
    | (Conj ts, t)
    | (t, Conj ts) ->
      Conj (ts @ [t])
    | ( (Disj { props = _ :: _; _ } | Atom _),
        (Disj { props = _ :: _; _ } | Atom _) ) ->
      Conj [t1; t2]

let conjs props = List.fold_left props ~init:valid ~f:conj

let disj t1 t2 =
  if is_valid t1 then
    t1
  else if is_valid t2 then
    t2
  else
    match (t1, t2) with
    | ((Atom _ | Conj _), (Atom _ | Conj _)) ->
      Disj { failures = []; props = [t1; t2] }
    | (Disj { failures = f1; props = p1 }, Disj { failures = f2; props = p2 })
      ->
      Disj { failures = f1 @ f2; props = p1 @ p2 }
    | (Disj { failures; props }, _) -> Disj { failures; props = t2 :: props }
    | (_, Disj { failures; props }) -> Disj { failures; props = t1 :: props }

let disjs props =
  match props with
  | [prop] -> prop
  | prop :: props -> List.fold_left props ~init:prop ~f:disj
  | [] -> failwith "Prop.disjs: expected non-empty list"
