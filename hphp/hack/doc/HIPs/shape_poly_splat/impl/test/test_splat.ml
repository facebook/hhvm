module Base = Repr.Base
module Row = Repr.Row
module Field_desc = Repr.Field_desc
module Field_req = Repr.Field_req
module Prim = Repr.Prim

let env0 = Env.empty

(* All query variables are minted through the env so each is recorded (empty bounds) once;
   the strict Env accessors then flag any unminted var as a bug.  [env_q] carries them. *)
let (tv0, env_q) = Env.fresh_ty_var env0

let (tv1, env_q) = Env.fresh_ty_var env_q

let (rv0, env_q) = Env.fresh_ty_var env_q

let (rv1, env_q) = Env.fresh_ty_var env_q

(* a query is ACCEPTED when its closed proposition isn't invalid *)
let accepts (_, p) = not (Prop.is_invalid p)

let holds ~sub ~super = accepts (Splat.tell_subtype ~sub ~super env0)

(* == an INDEPENDENT brute per-field subtype oracle (ground types only) ======
   Re-implements the per-field structural subtype relation directly, so the
   differential cross-checks the engine instead of restating it. *)
let rec gsub (a : Base.t) (b : Base.t) : bool =
  match (a, b) with
  | (Base.Bottom, _) -> true
  | (_, Base.Bottom) -> false
  | (_, Base.Top) -> true
  | (Base.Top, _) -> false
  | (Base.Union (a1, a2), _) -> gsub a1 b && gsub a2 b
  | (_, Base.Union (b1, b2)) -> gsub a b1 || gsub a b2
  | (Base.Prim p, Base.Prim q) -> Prim.equal p q
  | (Base.Shape r, Base.Shape s) -> gsubrow r s
  | _ -> false (* Prim vs Shape (disjoint), or a stray Flex (ground only) *)

and gsubrow (r : Row.t) (s : Row.t) : bool =
  match (Row.canon r, Row.canon s) with
  | (Row.Row_simple rs, Row.Row_simple ss) ->
    let labels =
      None
      :: List.map
           (fun l -> Some l)
           (Label.Set.elements
              (Label.Set.union
                 (Row.Simple.label_set rs)
                 (Row.Simple.label_set ss)))
    in
    List.for_all
      (fun l -> gsub_field (Row.Simple.proj rs l) (Row.Simple.proj ss l))
      labels
  | _ -> false (* ground rows canonicalize to simple rows *)

and gsub_field (sub : Field_desc.t) (super : Field_desc.t) : bool =
  (match (sub.req, super.req) with
  | (Field_req.Opt, Field_req.Req) -> false
  | _ -> true)
  && gsub sub.base super.base

(* == generators ============================================================ *)

(* ground base types (no flex variables) over the full lattice *)
let rec gen_base fuel =
  let leaves = QCheck.Gen.oneofl [Base.top; Base.bot; Base.nat; Base.bool] in
  if fuel <= 0 then
    leaves
  else
    QCheck.Gen.frequency
      [
        (4, leaves);
        ( 1,
          QCheck.Gen.map2 Base.union (gen_base (fuel - 1)) (gen_base (fuel - 1))
        );
        (1, QCheck.Gen.map Base.shape (gen_row (fuel - 1)));
      ]

and gen_field fuel =
  QCheck.Gen.map2
    (fun is_req b ->
      if is_req then
        Field_desc.req b
      else
        Field_desc.opt b)
    QCheck.Gen.bool
    (gen_base fuel)

and gen_row fuel =
  QCheck.Gen.map3
    (fun fa fb u ->
      let knowns =
        List.filter_map
          (fun x -> x)
          [Option.map (fun f -> ("a", f)) fa; Option.map (fun f -> ("b", f)) fb]
      in
      Row.simple knowns u)
    (QCheck.Gen.opt (gen_field fuel))
    (QCheck.Gen.opt (gen_field fuel))
    (gen_base fuel)

let pp = Base.to_string

let arb_base = QCheck.make ~print:pp (gen_base 3)

let arb_pair =
  QCheck.make
    ~print:(fun (a, b) -> pp a ^ " , " ^ pp b)
    QCheck.Gen.(pair (gen_base 3) (gen_base 3))

let arb_triple =
  QCheck.make
    ~print:(fun (a, b, c) -> pp a ^ " , " ^ pp b ^ " , " ^ pp c)
    QCheck.Gen.(triple (gen_base 3) (gen_base 3) (gen_base 3))

(* == flex differential: a CHAIN lattice so the finite witness universe is
   complete (⊥ < nat < ⊤; unions of chain elements stay in the chain). ======= *)
let chain = [Base.bot; Base.nat; Base.top]

let rec gen_chain fuel =
  let leaves = QCheck.Gen.oneofl (chain @ [Base.flex tv0; Base.flex tv1]) in
  if fuel <= 0 then
    leaves
  else
    QCheck.Gen.frequency
      [
        (3, leaves);
        ( 1,
          QCheck.Gen.map2
            Base.union
            (gen_chain (fuel - 1))
            (gen_chain (fuel - 1)) );
      ]

let rec flex_ids = function
  | Base.Flex i -> [i]
  | Base.Union (a, b) -> flex_ids a @ flex_ids b
  | Base.Top
  | Base.Bottom
  | Base.Prim _
  | Base.Shape _
  | Base.Rigid _ ->
    []

let rec subst asn = function
  | Base.Flex i ->
    (match List.assoc_opt i asn with
    | Some t -> t
    | None -> Base.flex i)
  | Base.Union (a, b) -> Base.union (subst asn a) (subst asn b)
  | (Base.Top | Base.Bottom | Base.Prim _ | Base.Shape _ | Base.Rigid _) as t ->
    t

(* exists an assignment of the flex variables (from the chain) making sub <: super *)
let exists_witness t1 t2 =
  let ids = List.sort_uniq compare (flex_ids t1 @ flex_ids t2) in
  let rec assigns = function
    | [] -> [[]]
    | i :: rest ->
      List.concat_map
        (fun v -> List.map (fun a -> (i, v) :: a) (assigns rest))
        chain
  in
  List.exists (fun a -> gsub (subst a t1) (subst a t2)) (assigns ids)

let arb_chain_pair =
  QCheck.make
    ~print:(fun (a, b) -> pp a ^ " <: " ^ pp b)
    QCheck.Gen.(pair (gen_chain 2) (gen_chain 2))

(* the query vars are already minted (and recorded) in [env_q]; alias for the differentials *)
let env_hi = env_q

(* SOLUTION test: single-flex queries (?t0 on at most one side, no var-var) so the
   solver yields a CONCRETE value we can substitute back and check. *)
let rec gen_ground_chain fuel =
  let leaves = QCheck.Gen.oneofl chain in
  if fuel <= 0 then
    leaves
  else
    QCheck.Gen.frequency
      [
        (3, leaves);
        ( 1,
          QCheck.Gen.map2
            Base.union
            (gen_ground_chain (fuel - 1))
            (gen_ground_chain (fuel - 1)) );
      ]

let gen_solution_pair =
  QCheck.Gen.(
    let g = gen_ground_chain 2 in
    oneof
      [
        map (fun x -> (Base.flex tv0, x)) g;
        map (fun x -> (x, Base.flex tv0)) g;
        map2 (fun a b -> (a, b)) g g;
      ])

let arb_solution =
  QCheck.make ~print:(fun (a, b) -> pp a ^ " <: " ^ pp b) gen_solution_pair

(* ?t0's solved value from a closed env; an unconstrained var can be anything (⊥) *)
let solution env =
  match Env.solve_ty_var env tv0 with
  | Base.Flex _ -> Base.bot
  | t -> t

(* == ROW-VARIABLE differential ==============================================
   A single flex row variable on one side, the other side ground; inline rows
   over the chain {⊥,nat,⊤} at label "a" only.  The known field "a" ranges over
   every chain field-desc; the UNKNOWN tail ranges over the chain BASES (it is
   opt-only — a required unknown is unrepresentable).  So the row universe below
   is COMPLETE for the opt-only semantics: any witness lives in it.  This
   exercises subrow_infer_sub / subrow_infer_super + the dispatch row closure,
   which nothing else touches. *)
let chain_fd =
  [
    Field_desc.req Base.bot;
    Field_desc.req Base.nat;
    Field_desc.req Base.top;
    Field_desc.opt Base.bot;
    Field_desc.opt Base.nat;
    Field_desc.opt Base.top;
  ]

(* Unknown tails are opt-only, so they are just bases. *)
let chain_base = [Base.bot; Base.nat; Base.top]

let simple_a fa (tail : Base.t) =
  Row.Simple.of_fields
    (match fa with
    | Some f -> [("a", f)]
    | None -> [])
    tail

let row_universe =
  List.concat_map
    (fun fa -> List.map (fun tail -> simple_a fa tail) chain_base)
    (None :: List.map (fun f -> Some f) chain_fd)

let subst_row s0 (r : Row.t) : Row.t =
  match r with
  | Row.Row_simple _ -> r
  | Row.Row_splat { elems } ->
    Row.canon
      (Row.splat
         (List.map
            (function
              | Row.Splat_elem.Spread (Base.Flex _) -> Row.Splat_elem.simple s0
              | e -> e)
            elems))

let exists_witness_row sub super =
  List.exists
    (fun s0 -> gsubrow (subst_row s0 sub) (subst_row s0 super))
    row_universe

let gen_simple_a =
  QCheck.Gen.map2
    simple_a
    (QCheck.Gen.opt (QCheck.Gen.oneofl chain_fd))
    (QCheck.Gen.oneofl chain_base)

let gen1_of gen_simple =
  QCheck.Gen.(
    let inline = map (fun s -> Row.Splat_elem.simple s) gen_simple in
    let var_side =
      map2
        (fun pre post ->
          Row.splat
            (Option.to_list pre
            @ [Row.Splat_elem.spread (Base.flex rv0)]
            @ Option.to_list post))
        (opt inline)
        (opt inline)
    in
    let ground = map (fun s -> Row.Row_simple s) gen_simple in
    oneof
      [
        map2 (fun v g -> (v, g)) var_side ground;
        map2 (fun v g -> (g, v)) var_side ground;
      ])

let gen_row_query = gen1_of gen_simple_a

(* opt-only fields: no requiredness decision anywhere ⇒ the keep-both / field-variable
   axis is structurally absent, so any over-reject is the empty-floor axis alone. *)
let chain_opt =
  [Field_desc.opt Base.bot; Field_desc.opt Base.nat; Field_desc.opt Base.top]

let gen_simple_opt =
  QCheck.Gen.map2
    simple_a
    (QCheck.Gen.opt (QCheck.Gen.oneofl chain_opt))
    (QCheck.Gen.oneofl chain_base)

let gen_row_query_opt = gen1_of gen_simple_opt

let arb_row_query =
  QCheck.make
    ~print:(fun (a, b) -> Row.to_string a ^ " <: " ^ Row.to_string b)
    gen_row_query

(* TWO row variables — the fragment that actually exercises non-focused-var elimination
   (a focused var's obligation substitutes the OTHER vars by their current solution,
   which defaults to [empty]).  Single-var tests can't reach this. *)
let subst_vars asn (r : Row.t) : Row.t =
  match r with
  | Row.Row_simple _ -> r
  | Row.Row_splat { elems } ->
    Row.canon
      (Row.splat
         (List.map
            (function
              | Row.Splat_elem.Spread (Base.Flex i) ->
                Row.Splat_elem.simple (List.assoc i asn)
              | e -> e)
            elems))

let exists_witness2 sub super =
  List.exists
    (fun s0 ->
      List.exists
        (fun s1 ->
          let asn = [(rv0, s0); (rv1, s1)] in
          gsubrow (subst_vars asn sub) (subst_vars asn super))
        row_universe)
    row_universe

let gen2_of gen_simple =
  QCheck.Gen.(
    let inline = map (fun s -> Row.Splat_elem.simple s) gen_simple in
    let oi = opt inline in
    let ground = map (fun s -> Row.Row_simple s) gen_simple in
    (* both vars on one side, interleaved with optional inlines *)
    let two_var_side =
      map3
        (fun a b c ->
          Row.splat
            (Option.to_list a
            @ [Row.Splat_elem.spread (Base.flex rv0)]
            @ Option.to_list b
            @ [Row.Splat_elem.spread (Base.flex rv1)]
            @ Option.to_list c))
        oi
        oi
        oi
    in
    let var_side i =
      map2
        (fun a b ->
          Row.splat
            (Option.to_list a
            @ [Row.Splat_elem.spread (Base.flex i)]
            @ Option.to_list b))
        oi
        oi
    in
    oneof
      [
        map2 (fun v g -> (v, g)) two_var_side ground (* ?ρ0,?ρ1 both on sub *);
        map2 (fun v g -> (g, v)) two_var_side ground (* both on super *);
        map2 (fun s0 s1 -> (s0, s1)) (var_side rv0) (var_side rv1)
        (* one var each side *);
        map2 (fun v s -> (v, s)) two_var_side (var_side rv0)
        (* vars on BOTH sides with a multi-var side ⇒ the DECOUPLED solver *);
        map2 (fun v s -> (s, v)) two_var_side (var_side rv0);
      ])

let gen_row_query2 = gen2_of gen_simple_a

let gen_row_query2_opt = gen2_of gen_simple_opt

let arb_row_query2 =
  QCheck.make
    ~print:(fun (a, b) -> Row.to_string a ^ " <: " ^ Row.to_string b)
    gen_row_query2

(* Ground any field left as an unsolved flex (an unconstrained var can be anything; ⊥ is
   the safe minimal witness — the same choice [solution] makes for a single type var). *)
let rec ground_base b =
  match b with
  | Base.Flex _ -> Base.bot
  | Base.Union (a, c) -> Base.union (ground_base a) (ground_base c)
  | Base.Shape r -> Base.shape (ground_row r)
  | Base.Top
  | Base.Bottom
  | Base.Prim _
  | Base.Rigid _ ->
    b

and ground_row r =
  match r with
  | Row.Row_simple s -> Row.Row_simple (ground_simple s)
  | Row.Row_splat _ -> r

and ground_simple (s : Row.Simple.t) : Row.Simple.t =
  let g (fd : Field_desc.t) = { fd with base = ground_base fd.base } in
  { known = Label.Map.map g s.known; unknown = ground_base s.unknown }

(* Recover ?ρ0's solved row from a closed env, grounded to a concrete simple row. *)
let recover env =
  match
    Env.resolve_row env (Row.splat [Row.Splat_elem.spread (Base.flex rv0)])
  with
  | Row.Row_simple s -> ground_simple s
  | Row.Row_splat _ as r ->
    (match Row.canon r with
    | Row.Row_simple s -> ground_simple s
    | Row.Row_splat _ -> Row.Simple.empty)

(* == unit cases ============================================================ *)

let shape fields = Base.shape (Row.closed fields)

let case name expected ~sub ~super =
  Alcotest.test_case name `Quick (fun () ->
      Alcotest.(check bool) name expected (holds ~sub ~super))

let unit_tests =
  [
    case "nat <: nat" true ~sub:Base.nat ~super:Base.nat;
    case "nat <: bool" false ~sub:Base.nat ~super:Base.bool;
    case "bot <: top" true ~sub:Base.bot ~super:Base.top;
    case "top <: nat" false ~sub:Base.top ~super:Base.nat;
    case
      "{a:nat} <: {a:top}"
      true
      ~sub:(shape [("a", Field_desc.req Base.nat)])
      ~super:(shape [("a", Field_desc.req Base.top)]);
    case
      "{a:top} <: {a:nat}"
      false
      ~sub:(shape [("a", Field_desc.req Base.top)])
      ~super:(shape [("a", Field_desc.req Base.nat)]);
    case
      "nat <: nat|bool"
      true
      ~sub:Base.nat
      ~super:(Base.union Base.nat Base.bool);
    case
      "nat|bool <: nat"
      false
      ~sub:(Base.union Base.nat Base.bool)
      ~super:Base.nat;
    (* recording + transitive closure across two queries: ?t0 gets upper [nat] then
       lower [bool]; the implied bool <: nat is dispatched and fails. *)
    Alcotest.test_case "flex transitive closure" `Quick (fun () ->
        let (env1, p1) =
          Splat.tell_subtype ~sub:(Base.flex tv0) ~super:Base.nat env_q
        in
        Alcotest.(check bool)
          "?t0 <: nat accepted"
          true
          (not (Prop.is_invalid p1));
        let (_, p2) =
          Splat.tell_subtype ~sub:Base.bool ~super:(Base.flex tv0) env1
        in
        Alcotest.(check bool)
          "bool <: ?t0 <: nat rejected"
          true
          (Prop.is_invalid p2));
  ]

(* == property tests ======================================================== *)

let qc = QCheck_alcotest.to_alcotest

let property_tests =
  [
    qc
      (QCheck.Test.make ~count:1000 ~name:"reflexive: t <: t" arb_base (fun t ->
           holds ~sub:t ~super:t));
    qc
      (QCheck.Test.make ~count:1000 ~name:"bottom is least" arb_base (fun t ->
           holds ~sub:Base.bot ~super:t));
    qc
      (QCheck.Test.make ~count:1000 ~name:"top is greatest" arb_base (fun t ->
           holds ~sub:t ~super:Base.top));
    (* SOUND + COMPLETE on ground types: the engine agrees with the brute oracle *)
    qc
      (QCheck.Test.make
         ~count:25000
         ~name:"ground differential: check = brute"
         arb_pair
         (fun (a, b) -> Bool.equal (holds ~sub:a ~super:b) (gsub a b)));
    qc
      (QCheck.Test.make
         ~count:2000
         ~name:"transitive"
         arb_triple
         (fun (a, b, c) ->
           (not (holds ~sub:a ~super:b && holds ~sub:b ~super:c))
           || holds ~sub:a ~super:c));
    qc
      (QCheck.Test.make
         ~count:2000
         ~name:"meet is a lower bound"
         arb_pair
         (fun (a, b) ->
           let m = Base.meet a b in
           holds ~sub:m ~super:a && holds ~sub:m ~super:b));
    qc
      (QCheck.Test.make
         ~count:2000
         ~name:"meet is the greatest lower bound"
         arb_triple
         (fun (a, b, c) ->
           (not (holds ~sub:c ~super:a && holds ~sub:c ~super:b))
           || holds ~sub:c ~super:(Base.meet a b)));
    qc
      (QCheck.Test.make
         ~count:2000
         ~name:"join is an upper bound"
         arb_pair
         (fun (a, b) ->
           let j = Base.join a b in
           holds ~sub:a ~super:j && holds ~sub:b ~super:j));
    qc
      (QCheck.Test.make
         ~count:2000
         ~name:"join is the least upper bound"
         arb_triple
         (fun (a, b, c) ->
           (not (holds ~sub:a ~super:c && holds ~sub:b ~super:c))
           || holds ~sub:(Base.join a b) ~super:c));
    (* SOUND + COMPLETE closure: a query with flex variables is accepted iff some
       chain assignment of those variables makes the original hold. *)
    qc
      (QCheck.Test.make
         ~count:25000
         ~name:"flex differential: check = exists-witness"
         arb_chain_pair
         (fun (a, b) ->
           Bool.equal
             (accepts (Splat.tell_subtype ~sub:a ~super:b env_hi))
             (exists_witness a b)));
    (* SOLUTION soundness: when a single-flex query is accepted, the value the solver
       assigns to ?t0 actually makes the original hold (the recorded bounds + the
       solver produce a real witness). *)
    qc
      (QCheck.Test.make
         ~count:15000
         ~name:"solution is a witness"
         arb_solution
         (fun (a, b) ->
           let (env, p) = Splat.tell_subtype ~sub:a ~super:b env_hi in
           Prop.is_invalid p
           ||
           let s0 = solution env in
           gsub (subst [(0, s0)] a) (subst [(0, s0)] b)));
    (* SOUND + COMPLETE row closure (single row var): accepted iff some row assignment
       satisfies the original subrow.  Was soundness-directional until the sub-side
       masking fix in [subrow_infer_sub_part_at] (a var that must be Req to mask a
       type-violating neighbour) closed the last over-reject — now a full iff. *)
    qc
      (QCheck.Test.make
         ~count:20000
         ~name:"row differential (1 var): check_row = exists-witness"
         arb_row_query
         (fun (sub, super) ->
           Bool.equal
             (accepts (Splat.tell_subrow ~sub ~super env_q))
             (exists_witness_row sub super)));
    (* INFERENCE + SOLVING round-trip: an erased example (a row var standing in for a
       concrete witness) should be accepted AND let us recover a solution.  We assert the
       solving half — when inference accepts a witness-bearing query, the row the solver
       recovers, grounded and substituted back, genuinely satisfies the original subrow.
       Soundness-directional: a reject is the known keep-both over-reject (nothing to
       recover), so it passes vacuously. *)
    qc
      (QCheck.Test.make
         ~count:20000
         ~name:"row solving: recovered solution is a witness"
         arb_row_query
         (fun (sub, super) ->
           QCheck.assume (exists_witness_row sub super);
           let (env, p) = Splat.tell_subrow ~sub ~super env_q in
           Prop.is_invalid p
           ||
           let sol = recover env in
           gsubrow (subst_row sol sub) (subst_row sol super)));
    (* TWO row vars: SOUND only (accept ⇒ witness).  This is the non-focused-var-to-
       empty axis — completeness is NOT expected (eliminating the non-focused var to
       [empty] over-rejects); the diagnostic below quantifies the gap. *)
    qc
      (QCheck.Test.make
         ~count:5000
         ~name:"row differential (2 vars): SOUND (accept ⇒ witness)"
         arb_row_query2
         (fun (sub, super) ->
           (not (accepts (Splat.tell_subrow ~sub ~super env_q)))
           || exists_witness2 sub super));
  ]

(* == component algebra: canon / meet / join / corners / merge ==============
   Test the building blocks directly against their lattice laws and the brute
   order oracles (gsub / gsub_field / gsubrow), not just the end-to-end decision. *)

let beq a b = gsub a b && gsub b a (* base denotation equivalence *)

let fdeq a b = gsub_field a b && gsub_field b a (* field equivalence *)

let roweq a b = gsubrow a b && gsubrow b a (* row equivalence *)

let all_req = [Field_req.Req; Field_req.Opt]

let arb_field = QCheck.make ~print:Field_desc.to_string (gen_field 3)

let arb_field_pair =
  QCheck.make
    ~print:(fun (a, b) ->
      Field_desc.to_string a ^ " , " ^ Field_desc.to_string b)
    QCheck.Gen.(pair (gen_field 3) (gen_field 3))

let arb_field_triple =
  QCheck.make
    ~print:(fun (a, b, c) ->
      Field_desc.(to_string a ^ " , " ^ to_string b ^ " , " ^ to_string c))
    QCheck.Gen.(triple (gen_field 3) (gen_field 3) (gen_field 3))

let arb_row = QCheck.make ~print:Row.to_string (gen_row 3)

let arb_row_pair =
  QCheck.make
    ~print:(fun (a, b) -> Row.to_string a ^ " , " ^ Row.to_string b)
    QCheck.Gen.(pair (gen_row 3) (gen_row 3))

let arb_row_triple =
  QCheck.make
    ~print:(fun (a, b, c) ->
      Row.(to_string a ^ " , " ^ to_string b ^ " , " ^ to_string c))
    QCheck.Gen.(triple (gen_row 3) (gen_row 3) (gen_row 3))

(* -- Field_req: a 2-point lattice, so test EXHAUSTIVELY -- *)
let field_req_tests =
  let open Field_req in
  let eq (x : t) y = x = y in
  let pairs =
    List.concat_map (fun a -> List.map (fun b -> (a, b)) all_req) all_req
  in
  let triples =
    List.concat_map
      (fun a ->
        List.concat_map (fun b -> List.map (fun c -> (a, b, c)) all_req) all_req)
      all_req
  in
  let all_p f = List.for_all (fun (a, b) -> f a b) pairs in
  let all_t f = List.for_all (fun (a, b, c) -> f a b c) triples in
  let yes name b = Alcotest.(check bool) name true b in
  [
    Alcotest.test_case "Field_req meet/join commutative" `Quick (fun () ->
        yes "meet" (all_p (fun a b -> eq (meet a b) (meet b a)));
        yes "join" (all_p (fun a b -> eq (join a b) (join b a))));
    Alcotest.test_case "Field_req meet/join idempotent" `Quick (fun () ->
        yes
          "i"
          (List.for_all (fun a -> eq (meet a a) a && eq (join a a) a) all_req));
    Alcotest.test_case "Field_req meet/join associative" `Quick (fun () ->
        yes
          "meet"
          (all_t (fun a b c -> eq (meet (meet a b) c) (meet a (meet b c))));
        yes
          "join"
          (all_t (fun a b c -> eq (join (join a b) c) (join a (join b c)))));
    Alcotest.test_case "Field_req absorption" `Quick (fun () ->
        yes "m" (all_p (fun a b -> eq (meet a (join a b)) a));
        yes "j" (all_p (fun a b -> eq (join a (meet a b)) a)));
    Alcotest.test_case "Field_req identities (⊤=Opt / ⊥=Req)" `Quick (fun () ->
        yes "meet⊤" (List.for_all (fun a -> eq (meet top a) a) all_req);
        yes "join⊥" (List.for_all (fun a -> eq (join bot a) a) all_req);
        yes "meet⊥" (List.for_all (fun a -> eq (meet bot a) bot) all_req);
        yes "join⊤" (List.for_all (fun a -> eq (join top a) top) all_req));
    Alcotest.test_case "Field_req lteq agrees with meet/join" `Quick (fun () ->
        yes
          "lteq=meet"
          (all_p (fun a b -> Bool.equal (lteq a b) (eq (meet a b) a)));
        yes
          "lteq=join"
          (all_p (fun a b -> Bool.equal (lteq a b) (eq (join a b) b))));
  ]

(* -- Field_desc.corners: the box extremes -- *)
let corners_tests =
  let s fds = String.concat "; " (List.map Field_desc.to_string fds) in
  let req = Field_desc.req and opt = Field_desc.opt in
  [
    Alcotest.test_case "corners (Req,Opt) → 4 corners" `Quick (fun () ->
        Alcotest.(check string)
          "4"
          (s [req Base.nat; opt Base.top; opt Base.nat; req Base.top])
          (s (Field_desc.corners ~lower:(req Base.nat) ~upper:(opt Base.top))));
    Alcotest.test_case "corners (Req,Req) → lower,upper" `Quick (fun () ->
        Alcotest.(check string)
          "2"
          (s [req Base.nat; req Base.top])
          (s (Field_desc.corners ~lower:(req Base.nat) ~upper:(req Base.top))));
    Alcotest.test_case "corners (Opt,Opt) → lower,upper" `Quick (fun () ->
        Alcotest.(check string)
          "2"
          (s [opt Base.nat; opt Base.top])
          (s (Field_desc.corners ~lower:(opt Base.nat) ~upper:(opt Base.top))));
    Alcotest.test_case "corners (Opt,Req) ill-formed → []" `Quick (fun () ->
        Alcotest.(check string)
          "[]"
          ""
          (s (Field_desc.corners ~lower:(opt Base.nat) ~upper:(req Base.top))));
  ]

(* -- Base / Field_desc / Row lattice laws against the brute order oracles -- *)
let algebra_props =
  [
    qc
      (QCheck.Test.make
         ~count:5000
         ~name:"Base.canon idempotent"
         arb_base
         (fun t ->
           String.equal
             (Base.to_string (Base.canon (Base.canon t)))
             (Base.to_string (Base.canon t))));
    qc
      (QCheck.Test.make
         ~count:5000
         ~name:"Base.canon preserves denotation"
         arb_base
         (fun t -> beq t (Base.canon t)));
    qc
      (QCheck.Test.make
         ~count:5000
         ~name:"Base.is_bot iff ≡ ⊥"
         arb_base
         (fun t -> Bool.equal (Base.is_bot (Base.canon t)) (beq t Base.bot)));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Base.meet commutative"
         arb_pair
         (fun (a, b) -> beq (Base.meet a b) (Base.meet b a)));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Base.meet idempotent"
         arb_base
         (fun a -> beq (Base.meet a a) a));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Base.meet associative"
         arb_triple
         (fun (a, b, c) ->
           beq (Base.meet (Base.meet a b) c) (Base.meet a (Base.meet b c))));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Base.join commutative"
         arb_pair
         (fun (a, b) -> beq (Base.join a b) (Base.join b a)));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Base.join idempotent"
         arb_base
         (fun a -> beq (Base.join a a) a));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Base.join associative"
         arb_triple
         (fun (a, b, c) ->
           beq (Base.join (Base.join a b) c) (Base.join a (Base.join b c))));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Base absorption"
         arb_pair
         (fun (a, b) ->
           beq (Base.meet a (Base.join a b)) a
           && beq (Base.join a (Base.meet a b)) a));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Base identities ⊤/⊥"
         arb_base
         (fun a ->
           beq (Base.meet Base.top a) a
           && beq (Base.join Base.bot a) a
           && beq (Base.meet Base.bot a) Base.bot
           && beq (Base.join Base.top a) Base.top));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Field_desc.meet is GLB"
         arb_field_triple
         (fun (a, b, c) ->
           let m = Field_desc.meet a b in
           gsub_field m a
           && gsub_field m b
           && ((not (gsub_field c a && gsub_field c b)) || gsub_field c m)));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Field_desc.join is LUB"
         arb_field_triple
         (fun (a, b, c) ->
           let j = Field_desc.join a b in
           gsub_field a j
           && gsub_field b j
           && ((not (gsub_field a c && gsub_field b c)) || gsub_field j c)));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Field_desc meet/join comm + absorb"
         arb_field_pair
         (fun (a, b) ->
           fdeq (Field_desc.meet a b) (Field_desc.meet b a)
           && fdeq (Field_desc.join a b) (Field_desc.join b a)
           && fdeq (Field_desc.meet a (Field_desc.join a b)) a
           && fdeq (Field_desc.join a (Field_desc.meet a b)) a));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Field_desc.merge identity (opt ⊥)"
         arb_field
         (fun fd ->
           let e = Field_desc.opt Base.bot in
           fdeq (Field_desc.merge ~left:fd ~right:e) fd
           && fdeq (Field_desc.merge ~left:e ~right:fd) fd));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Field_desc.corners lie in [meet,join] box"
         arb_field_pair
         (fun (a, b) ->
           let lower = Field_desc.meet a b and upper = Field_desc.join a b in
           List.for_all
             (fun c -> gsub_field lower c && gsub_field c upper)
             (Field_desc.corners ~lower ~upper)));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Row.canon idempotent"
         arb_row
         (fun r ->
           String.equal
             (Row.to_string (Row.canon (Row.canon r)))
             (Row.to_string (Row.canon r))));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Row.canon preserves denotation"
         arb_row
         (fun r -> roweq r (Row.canon r)));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Row.meet is GLB"
         arb_row_triple
         (fun (a, b, c) ->
           let m = Row.meet a b in
           gsubrow m a
           && gsubrow m b
           && ((not (gsubrow c a && gsubrow c b)) || gsubrow c m)));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Row.join is LUB"
         arb_row_triple
         (fun (a, b, c) ->
           let j = Row.join a b in
           gsubrow a j
           && gsubrow b j
           && ((not (gsubrow a c && gsubrow b c)) || gsubrow j c)));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Row meet/join comm + absorb"
         arb_row_pair
         (fun (a, b) ->
           roweq (Row.meet a b) (Row.meet b a)
           && roweq (Row.join a b) (Row.join b a)
           && roweq (Row.meet a (Row.join a b)) a
           && roweq (Row.join a (Row.meet a b)) a));
    qc
      (QCheck.Test.make
         ~count:3000
         ~name:"Simple.merge identity (empty row)"
         arb_row
         (fun r ->
           match r with
           | Row.Row_simple s ->
             let e = Row.Simple.empty in
             roweq (Row.Row_simple (Row.Simple.merge ~left:e ~right:s)) r
             && roweq (Row.Row_simple (Row.Simple.merge ~left:s ~right:e)) r
           | Row.Row_splat _ -> true));
  ]

(* == F-SUB rule (rigid params at base / field-type position) ================
   [t] ranges over [nat, ⊤]; [u] is exactly [nat].  The rule reduces a rigid
   param through its bound (reflexivity first), incl. distinct param-vs-param. *)
let fsub_env =
  let tp = Env.Ty_param_env.empty in
  let tp = Env.Ty_param_env.add tp "t" ~lower:Base.nat ~upper:Base.top in
  let tp = Env.Ty_param_env.add tp "u" ~lower:Base.nat ~upper:Base.nat in
  { Env.empty with Env.ty_param = tp }

let rigid_t = Base.rigid "t"

let rigid_u = Base.rigid "u"

let fsub_tests =
  let chk name e ~sub ~super =
    Alcotest.(check bool)
      name
      e
      (accepts (Splat.tell_subtype ~sub ~super fsub_env))
  in
  [
    Alcotest.test_case "F-sub rule" `Quick (fun () ->
        chk "t <: ⊤" true ~sub:rigid_t ~super:Base.top;
        chk "⊥ <: t" true ~sub:Base.bot ~super:rigid_t;
        chk "t <: t (refl)" true ~sub:rigid_t ~super:rigid_t;
        chk "nat <: t (via lower)" true ~sub:Base.nat ~super:rigid_t;
        chk "t <: nat (upper ⊤ </: nat)" false ~sub:rigid_t ~super:Base.nat;
        chk "bool <: t (bool </: lower nat)" false ~sub:Base.bool ~super:rigid_t;
        chk
          "u <: t (distinct, upper u nat <: lower t nat)"
          true
          ~sub:rigid_u
          ~super:rigid_t;
        chk
          "t <: u (distinct, upper t ⊤ </: lower u nat)"
          false
          ~sub:rigid_t
          ~super:rigid_u);
  ]

(* == RIGID-PARAMETER differential ===========================================
   Params [P], [Q] at spread position with shape bounds over label "a"; a TRUE
   denotational oracle quantifies the subrow UNIVERSALLY over every instantiation
   of [P], [Q] within their bound boxes.  This is the coverage the flex
   differentials never reach: corner enumeration, the masking shortcut, and
   bound-label discovery. *)
let bound_of fd = Base.shape (Row.closed [("a", fd)])

(* every chain field-desc in the box [lo, hi] (lo <: hi guaranteed by [gen_box]) *)
let box lo hi =
  List.filter (fun fd -> gsub_field lo fd && gsub_field fd hi) chain_fd

let param_env (loP, hiP) (loQ, hiQ) =
  let tp = Env.Ty_param_env.empty in
  let tp =
    Env.Ty_param_env.add tp "P" ~lower:(bound_of loP) ~upper:(bound_of hiP)
  in
  let tp =
    Env.Ty_param_env.add tp "Q" ~lower:(bound_of loQ) ~upper:(bound_of hiQ)
  in
  { Env.empty with Env.ty_param = tp }

let subst_params fdP fdQ (r : Row.t) : Row.t =
  let closed fd = Row.Splat_elem.simple (Row.Simple.closed [("a", fd)]) in
  match r with
  | Row.Row_simple _ -> r
  | Row.Row_splat { elems } ->
    Row.canon
      (Row.splat
         (List.map
            (function
              | Row.Splat_elem.Spread (Base.Rigid p) when String.equal p "P" ->
                closed fdP
              | Row.Splat_elem.Spread (Base.Rigid p) when String.equal p "Q" ->
                closed fdQ
              | e -> e)
            elems))

(* the subrow holds (rigid/universal reading) iff it holds at EVERY instantiation *)
let forall_inst (loP, hiP) (loQ, hiQ) sub super =
  List.for_all
    (fun fdP ->
      List.for_all
        (fun fdQ ->
          gsubrow (subst_params fdP fdQ sub) (subst_params fdP fdQ super))
        (box loQ hiQ))
    (box loP hiP)

(* a VALID box [meet a b, join a b] from two chain field-descs *)
let gen_box =
  QCheck.Gen.map2
    (fun a b -> (Field_desc.meet a b, Field_desc.join a b))
    (QCheck.Gen.oneofl chain_fd)
    (QCheck.Gen.oneofl chain_fd)

(* a side: three optional inlines around P and Q, where each param is independently
   present/absent AND their relative order is flipped at random.  Varying presence gives
   single-side params (the sub-only / super-only corner drops); varying order between the
   two sides lets a param be masked in one side but not the other (the masked-sub vs
   masked-super drops); absent inlines leave label "a" living only in the bounds. *)
let gen_param_side =
  QCheck.Gen.(
    let inline =
      map
        (fun fd -> Row.Splat_elem.simple (Row.Simple.closed [("a", fd)]))
        (oneofl chain_fd)
    in
    let oil =
      map
        (function
          | Some x -> [x]
          | None -> [])
        (opt inline)
    in
    let optp name =
      map
        (fun b ->
          if b then
            [Row.Splat_elem.spread (Base.rigid name)]
          else
            [])
        bool
    in
    map
      (fun (flip, (i0, (p, (i1, (q, i2))))) ->
        let params =
          if flip then
            q @ i1 @ p
          else
            p @ i1 @ q
        in
        Row.splat (i0 @ params @ i2))
      (pair bool (pair oil (pair (optp "P") (pair oil (pair (optp "Q") oil))))))

let gen_param_case =
  QCheck.Gen.(
    map
      (fun ((bP, bQ), (sub, super)) -> (bP, bQ, sub, super))
      (pair (pair gen_box gen_box) (pair gen_param_side gen_param_side)))

let arb_param_case =
  QCheck.make
    ~print:(fun (_bP, _bQ, sub, super) ->
      Row.to_string sub ^ " <: " ^ Row.to_string super)
    gen_param_case

(* == deterministic regressions for the three bugs the flex suite couldn't reach == *)
let param_tests =
  let shp fd = Base.shape (Row.closed [("x", fd)]) in
  let menv =
    let tp = Env.Ty_param_env.empty in
    let tp =
      Env.Ty_param_env.add
        tp
        "a"
        ~lower:(shp (Field_desc.req Base.nat))
        ~upper:(shp (Field_desc.req Base.top))
    in
    let tp =
      Env.Ty_param_env.add
        tp
        "b"
        ~lower:(shp (Field_desc.opt Base.nat))
        ~upper:(shp (Field_desc.opt Base.nat))
    in
    let tp =
      Env.Ty_param_env.add
        tp
        "g"
        ~lower:(shp (Field_desc.req Base.nat))
        ~upper:(shp (Field_desc.req Base.nat))
    in
    { Env.empty with Env.ty_param = tp }
  in
  let sp n = Row.Splat_elem.spread (Base.rigid n) in
  let inline =
    Row.Splat_elem.simple (Row.Simple.closed [("x", Field_desc.opt Base.bot)])
  in
  let rej name sub super =
    Alcotest.test_case name `Quick (fun () ->
        Alcotest.(check bool)
          name
          false
          (accepts (Splat.tell_subrow ~sub ~super menv)))
  in
  [
    (* (a)+(iii): masking must read the RIGHTWARD param's bounds: g (Req) masks a in
       super [corner drop (iii) → upper], b (Opt) does not in sub, so at a=⊤ we get
       sub.x=⊤ </: super.x=nat → reject. *)
    rej
      "masking: rightward param's bounds decide"
      (Row.splat [inline; sp "a"; sp "b"])
      (Row.splat [inline; sp "a"; sp "g"]);
    (* (ii): the mirror — g (Req) masks a in SUB [corner drop (ii) → lower], b (Opt) in
       super; a's only contribution is to super, and sub.x=req nat <: super.x=req(a∪nat)
       for all a∈[nat,⊤] → accept. *)
    Alcotest.test_case "masked-in-sub corner drop accepts" `Quick (fun () ->
        Alcotest.(check bool)
          "accept"
          true
          (accepts
             (Splat.tell_subrow
                ~sub:(Row.splat [inline; sp "a"; sp "g"])
                ~super:(Row.splat [inline; sp "a"; sp "b"])
                menv)));
    (* (b) the label x lives only in the param bounds — it must still be checked. *)
    rej
      "bound-only label is discovered"
      (Row.splat [sp "a"; sp "b"])
      (Row.splat [sp "a"; sp "g"]);
    (* (c) a covariant spread var with two INCOMPARABLE lower shapes now resolves to the
       PRECISE union of shapes (via [Base.join] + canon distribution), not the lossy
       single-row [Row.join].  Checked against the ground-truth denotation oracle. *)
    Alcotest.test_case
      "spread var with 2 distinct lowers resolves to the precise union"
      `Quick
      (fun () ->
        let (v, env) = Env.fresh_ty_var Env.empty in
        let (ty_var, _) =
          Env.Ty_var_env.add_lower
            env.Env.ty_var
            v
            (Base.shape (Row.closed [("x", Field_desc.req Base.nat)]))
        in
        let (ty_var, _) =
          Env.Ty_var_env.add_lower
            ty_var
            v
            (Base.shape (Row.closed [("y", Field_desc.req Base.bool)]))
        in
        let env = { env with Env.ty_var } in
        (* read the solution out as a base (wrap + canon), which distributes the union *)
        let solved =
          Base.canon
            (Base.shape
               (Env.resolve_row
                  env
                  (Row.splat [Row.Splat_elem.spread (Base.flex v)])))
        in
        let precise =
          Base.union
            (shape [("x", Field_desc.req Base.nat)])
            (shape [("y", Field_desc.req Base.bool)])
        in
        (* the old lossy [Row.join] result {x:opt nat, y:opt bool} — wrongly admits {} *)
        let lossy =
          shape
            [("x", Field_desc.opt Base.nat); ("y", Field_desc.opt Base.bool)]
        in
        Alcotest.(check bool)
          "distributes to a Union"
          true
          (match solved with
          | Base.Union _ -> true
          | _ -> false);
        Alcotest.(check bool)
          "= the precise union (denotation)"
          true
          (gsub solved precise && gsub precise solved);
        Alcotest.(check bool)
          "STRICTLY more precise than the old Row.join"
          false
          (gsub lossy solved));
  ]

let param_props =
  [
    qc
      (QCheck.Test.make
         ~count:20000
         ~name:"rigid-param differential: check = forall-instantiation"
         arb_param_case
         (fun (bP, bQ, sub, super) ->
           let env = param_env bP bQ in
           Bool.equal
             (accepts (Splat.tell_subrow ~sub ~super env))
             (forall_inst bP bQ sub super)));
  ]

(* == NESTED-PARAMETER ("the hard case") =====================================
   Q's bound SPREADS P, so P is reachable ONLY through Q's bound — this requires
   the transitive [closure] (and transitive bound-label discovery), and without
   them the engine RAISES.  Q is pinned to eval(Q's bound, P); the oracle
   quantifies universally over P's instantiations with Q determined by P. *)
let nested_env (loP, hiP) inline_q =
  let tp = Env.Ty_param_env.empty in
  let tp =
    Env.Ty_param_env.add tp "P" ~lower:(bound_of loP) ~upper:(bound_of hiP)
  in
  let q_inline =
    Option.to_list
      (Option.map
         (fun fd -> Row.Splat_elem.simple (Row.Simple.closed [("a", fd)]))
         inline_q)
  in
  let qbound =
    Base.shape (Row.splat (q_inline @ [Row.Splat_elem.spread (Base.rigid "P")]))
  in
  let tp = Env.Ty_param_env.add tp "Q" ~lower:qbound ~upper:qbound in
  { Env.empty with Env.ty_param = tp }

let subst_nested inline_q fd_p (r : Row.t) : Row.t =
  let rP = Row.Simple.closed [("a", fd_p)] in
  let rQ =
    match inline_q with
    | None -> rP
    | Some fd ->
      Row.Simple.merge ~left:(Row.Simple.closed [("a", fd)]) ~right:rP
  in
  match r with
  | Row.Row_simple _ -> r
  | Row.Row_splat { elems } ->
    Row.canon
      (Row.splat
         (List.map
            (function
              | Row.Splat_elem.Spread (Base.Rigid p) when String.equal p "P" ->
                Row.Splat_elem.simple rP
              | Row.Splat_elem.Spread (Base.Rigid p) when String.equal p "Q" ->
                Row.Splat_elem.simple rQ
              | e -> e)
            elems))

let forall_inst_nested (loP, hiP) inline_q sub super =
  List.for_all
    (fun fd_p ->
      gsubrow
        (subst_nested inline_q fd_p sub)
        (subst_nested inline_q fd_p super))
    (box loP hiP)

(* a side: optional inline, ..Q, optional inline — P appears ONLY inside Q's bound *)
let gen_nested_side =
  QCheck.Gen.(
    let inline =
      map
        (fun fd -> Row.Splat_elem.simple (Row.Simple.closed [("a", fd)]))
        (oneofl chain_fd)
    in
    let oi = opt inline in
    map2
      (fun a b ->
        Row.splat
          (Option.to_list a
          @ [Row.Splat_elem.spread (Base.rigid "Q")]
          @ Option.to_list b))
      oi
      oi)

let gen_nested_case =
  QCheck.Gen.(
    map
      (fun (bP, (iq, (sub, super))) -> (bP, iq, sub, super))
      (pair
         gen_box
         (pair (opt (oneofl chain_fd)) (pair gen_nested_side gen_nested_side))))

let arb_nested_case =
  QCheck.make
    ~print:(fun (_bP, _iq, sub, super) ->
      Row.to_string sub ^ " <: " ^ Row.to_string super)
    gen_nested_case

let nested_tests =
  let env =
    let tp = Env.Ty_param_env.empty in
    let tp =
      Env.Ty_param_env.add
        tp
        "i"
        ~lower:(bound_of (Field_desc.req Base.nat))
        ~upper:(bound_of (Field_desc.req Base.top))
    in
    let obound =
      Base.shape (Row.splat [Row.Splat_elem.spread (Base.rigid "i")])
    in
    let tp = Env.Ty_param_env.add tp "o" ~lower:obound ~upper:obound in
    { Env.empty with Env.ty_param = tp }
  in
  let o = Row.splat [Row.Splat_elem.spread (Base.rigid "o")] in
  let q fd = Row.closed [("a", fd)] in
  let chk name e super =
    Alcotest.test_case name `Quick (fun () ->
        Alcotest.(check bool)
          name
          e
          (accepts (Splat.tell_subrow ~sub:o ~super env)))
  in
  [
    (* o is nested in i (∈ [{a:nat},{a:top}]); both need the transitive closure or this RAISES *)
    chk
      "|..o| <: {a:req top}  (o ranges [nat,top])"
      true
      (q (Field_desc.req Base.top));
    chk
      "|..o| <: {a:req nat}  (o could be top)"
      false
      (q (Field_desc.req Base.nat));
  ]

let nested_props =
  [
    qc
      (QCheck.Test.make
         ~count:20000
         ~name:"nested-param differential: check = forall-instantiation"
         arb_nested_case
         (fun (bP, iq, sub, super) ->
           let env = nested_env bP iq in
           Bool.equal
             (accepts (Splat.tell_subrow ~sub ~super env))
             (forall_inst_nested bP iq sub super)));
  ]

(* == MIXED flex spread var + rigid param =====================================
   A flex spread var ?ρ0 AND a rigid param [al] in one row, [al] to ?ρ0's right,
   [al] ∈ [{a:opt nat},{a:req nat}] so its two corners mask ?ρ0 under one (req) but
   not the other (opt) — the only way to reach the per-assignment masked branches of
   the inference. *)
let mixed_env =
  let tp =
    Env.Ty_param_env.add
      Env.Ty_param_env.empty
      "al"
      ~lower:(Base.shape (Row.closed [("a", Field_desc.req Base.nat)]))
      ~upper:(Base.shape (Row.closed [("a", Field_desc.opt Base.nat)]))
  in
  { env_q with Env.ty_param = tp }

let mixed_row =
  Row.splat
    [
      Row.Splat_elem.spread (Base.flex rv0);
      Row.Splat_elem.spread (Base.rigid "al");
    ]

let mixed_tests =
  let q fd = Row.closed [("a", fd)] in
  let chk name e sub super =
    Alcotest.test_case name `Quick (fun () ->
        Alcotest.(check bool)
          name
          e
          (accepts (Splat.tell_subrow ~sub ~super mixed_env)))
  in
  [
    (* sub side: at al=req nat, ?ρ0 is masked → sub.a=req nat </: req bool *)
    chk
      "|..ρ,..al| <: {a:req bool} (masked-assg reject)"
      false
      mixed_row
      (q (Field_desc.req Base.bool));
    (* satisfiable with ρ = {a:req nat} *)
    chk
      "|..ρ,..al| <: {a:req nat}  (masked-assg accept)"
      true
      mixed_row
      (q (Field_desc.req Base.nat));
    (* super side: at al=req nat, ?ρ0 is masked → req bool </: super.a=req nat *)
    chk
      "{a:req bool} <: |..ρ,..al| (masked-assg reject)"
      false
      (q (Field_desc.req Base.bool))
      mixed_row;
  ]

(* == canon of spread rows (adjacent opaque spreads; spread of a shape-of-splat) ==
   gen_row only builds simple rows, so these new canon paths are otherwise untested. *)
let canon_spread_tests =
  let sp n = Row.Splat_elem.spread (Base.rigid n) in
  let inl =
    Row.Splat_elem.simple (Row.Simple.closed [("x", Field_desc.req Base.nat)])
  in
  [
    Alcotest.test_case
      "canon |..a,..b| keeps two opaque spreads (no merge)"
      `Quick
      (fun () ->
        let r = Row.canon (Row.splat [sp "a"; sp "b"]) in
        Alcotest.(check bool)
          "two elems + idempotent"
          true
          ((match r with
           | Row.Row_splat { elems } -> List.length elems = 2
           | _ -> false)
          && String.equal (Row.to_string r) (Row.to_string (Row.canon r))));
    Alcotest.test_case
      "canon flattens a spread of a shape-of-splat"
      `Quick
      (fun () ->
        let inner = Row.splat [inl; sp "a"] in
        let r =
          Row.canon (Row.splat [Row.Splat_elem.spread (Base.shape inner)])
        in
        Alcotest.(check string)
          "= inlining the inner splat"
          (Row.to_string (Row.canon inner))
          (Row.to_string r));
    (* A union spread floats OUT of the splat: [{a:nat} ⊕ ({b:bool} | {b:nat})]
       canonicalises to the precise top-level union [{a:nat,b:bool} | {a:nat,b:nat}] —
       rightmost-wins merge distributes over union.  Checked against the ground oracle. *)
    Alcotest.test_case
      "canon distributes a union spread out of a splat"
      `Quick
      (fun () ->
        let sa =
          Row.Splat_elem.spread (shape [("a", Field_desc.req Base.nat)])
        in
        let ubc =
          Base.union
            (shape [("b", Field_desc.req Base.bool)])
            (shape [("b", Field_desc.req Base.nat)])
        in
        let t = Base.shape (Row.splat [sa; Row.Splat_elem.spread ubc]) in
        let canon_t = Base.canon t in
        let is_union =
          match canon_t with
          | Base.Union _ -> true
          | _ -> false
        in
        (* the precise distributed form *)
        let precise =
          Base.union
            (shape
               [("a", Field_desc.req Base.nat); ("b", Field_desc.req Base.bool)])
            (shape
               [("a", Field_desc.req Base.nat); ("b", Field_desc.req Base.nat)])
        in
        (* the lossy field-join [{a:nat, b: opt(bool|nat)}] — what [Row.join] would give;
           it wrongly admits [{a:nat}] (b absent) *)
        let lossy =
          shape
            [
              ("a", Field_desc.req Base.nat);
              ("b", Field_desc.opt (Base.union Base.bool Base.nat));
            ]
        in
        Alcotest.(check bool) "distributes to a top-level Union" true is_union;
        Alcotest.(check bool)
          "= the precise distributed union (denotation)"
          true
          (gsub canon_t precise && gsub precise canon_t);
        Alcotest.(check bool)
          "precise ⊆ the lossy field-join"
          true
          (gsub canon_t lossy);
        Alcotest.(check bool)
          "distribution is STRICTLY more precise than the field-join"
          false
          (gsub lossy canon_t);
        Alcotest.(check bool)
          "canon idempotent on the union spread"
          true
          (String.equal
             (Base.to_string canon_t)
             (Base.to_string (Base.canon canon_t))));
    (* End-to-end soundness differential: random shapes AND union-spread bases fed to the real
       engine ([tell_subtype]) must never accept something the ground denotation rejects, and
       must never crash (union spreads flow through canon distribution + the base/row routing). *)
    Alcotest.test_case
      "union-spread subtyping: sound + crash-free over 4000 random queries"
      `Quick
      (fun () ->
        Random.init 1234;
        let labs = [| "a"; "b"; "c" |]
        and prims = [| Base.nat; Base.bool; Base.top |] in
        let rand_field () =
          ( labs.(Random.int 3),
            (if Random.bool () then
              Field_desc.req
            else
              Field_desc.opt)
              prims.(Random.int 3) )
        in
        let rec mkf n =
          if n = 0 then
            []
          else
            rand_field () :: mkf (n - 1)
        in
        let rand_shape () = shape (mkf (1 + Random.int 2)) in
        let rand_us () =
          Base.shape
            (Row.splat
               [
                 Row.Splat_elem.spread (rand_shape ());
                 Row.Splat_elem.spread
                   (Base.union (rand_shape ()) (rand_shape ()));
               ])
        in
        let rand_base () =
          if Random.int 3 = 0 then
            rand_us ()
          else
            rand_shape ()
        in
        let unsound = ref 0 and crashes = ref 0 in
        for _ = 1 to 4000 do
          let sub = rand_base () and super = rand_base () in
          match
            try `R (holds ~sub ~super) with
            | _ -> `C
          with
          | `C -> incr crashes
          | `R e ->
            if e && not (gsub (Base.canon sub) (Base.canon super)) then
              incr unsound
        done;
        Alcotest.(check int) "no crashes" 0 !crashes;
        Alcotest.(check int) "no unsound acceptances" 0 !unsound);
  ]

(* == tail coverage: reachable arms no generator hits ========================= *)
let coverage_tail_tests =
  [
    (* canon collapses a shape whose row is bottom (a splat carrying [Spread Bottom]) to
       Bottom — the only surviving Shape→Bottom arm now that a simple row is never bottom *)
    Alcotest.test_case
      "shape of a Spread-Bottom splat canonicalises to Bottom"
      `Quick
      (fun () ->
        Alcotest.(check bool)
          "is_bot"
          true
          (Base.is_bot
             (Base.canon
                (Base.Shape (Row.splat [Row.Splat_elem.spread Base.bot])))));
    (* projecting a [Spread Bottom] (the ⊥ row) yields the bottom field [req ⊥] *)
    Alcotest.test_case "proj of |..⊥| is req ⊥" `Quick (fun () ->
        Alcotest.(check bool)
          "is_bot field"
          true
          (Field_desc.is_bot
             (Row.proj
                (Row.splat [Row.Splat_elem.spread Base.bot])
                (Some "x")
                Ty_param.Map.empty)));
    (* a parameter bounded by ⊥: |..z| is the ⊥ row, a subrow of anything *)
    Alcotest.test_case
      "Bottom-bounded param |..z| <: open accepts"
      `Quick
      (fun () ->
        let env =
          {
            Env.empty with
            Env.ty_param =
              Env.Ty_param_env.add
                Env.Ty_param_env.empty
                "z"
                ~lower:Base.bot
                ~upper:Base.bot;
          }
        in
        Alcotest.(check bool)
          "accept"
          true
          (accepts
             (Splat.tell_subrow
                ~sub:(Row.splat [Row.Splat_elem.spread (Base.rigid "z")])
                ~super:(Row.open_ [])
                env)));
    (* the focused spread var appears NESTED in a field type — resolve ~except leaves it *)
    Alcotest.test_case
      "nested focused spread var resolves without crash"
      `Quick
      (fun () ->
        let nested =
          Base.shape (Row.splat [Row.Splat_elem.spread (Base.flex rv0)])
        in
        let _ =
          Splat.tell_subrow
            ~sub:
              (Row.splat
                 [
                   Row.Splat_elem.simple
                     (Row.Simple.closed [("x", Field_desc.req nested)]);
                   Row.Splat_elem.spread (Base.flex rv0);
                 ])
            ~super:(Row.closed [("x", Field_desc.opt Base.top)])
            env_q
        in
        Alcotest.(check bool) "ran" true true);
    (* conj of a pending disjunction with an invalid is invalid *)
    Alcotest.test_case "conj (pending disj) invalid = invalid" `Quick (fun () ->
        let pending =
          Prop.disj
            (Prop.is_subtype_base ~sub:Base.nat ~super:Base.nat)
            (Prop.is_subtype_base ~sub:Base.bool ~super:Base.bool)
        in
        let bad =
          Prop.invalid (Prop.is_subtype_base ~sub:Base.top ~super:Base.bot)
        in
        Alcotest.(check bool)
          "invalid"
          true
          (Prop.is_invalid (Prop.conj pending bad)));
  ]

let () =
  if Sys.getenv_opt "DIAG" <> None then (
    let rand = Random.State.make_self_init () in
    (* over-reject rate of witness-bearing queries: [wit1]/[wit2] use the 1-var / 2-var
       brute as the truth.  Comparing mixed (req+opt) vs opt-only ISOLATES the axes:
       opt-only has no requiredness decision ⇒ no keep-both/field-var gap, so its
       over-reject is the empty-floor (non-focused-var elimination) ALONE. *)
    let rate1 n gen =
      let wit = ref 0 and over = ref 0 in
      for _ = 1 to n do
        let (sub, super) = QCheck.Gen.generate1 ~rand gen in
        if exists_witness_row sub super then (
          incr wit;
          if not (accepts (Splat.tell_subrow ~sub ~super env_q)) then incr over
        )
      done;
      100. *. float_of_int !over /. float_of_int (max 1 !wit)
    in
    let rate2 n gen =
      let wit = ref 0 and over = ref 0 in
      for _ = 1 to n do
        let (sub, super) = QCheck.Gen.generate1 ~rand gen in
        if exists_witness2 sub super then (
          incr wit;
          if not (accepts (Splat.tell_subrow ~sub ~super env_q)) then incr over
        )
      done;
      100. *. float_of_int !over /. float_of_int (max 1 !wit)
    in
    Printf.eprintf
      "DIAG over-reject %%   |  1-var (no empty-floor)  |  2-var\n%!";
    Printf.eprintf
      "  mixed req+opt     |       %5.1f             |  %5.1f\n%!"
      (rate1 12000 gen_row_query)
      (rate2 12000 gen_row_query2);
    Printf.eprintf
      "  opt-only (no kb)  |       %5.1f             |  %5.1f\n%!"
      (rate1 12000 gen_row_query_opt)
      (rate2 12000 gen_row_query2_opt);
    (* print ONE concrete case the engine wrongly rejects, with a witness *)
    let rec find k =
      if k = 0 then
        ()
      else
        let (sub, super) = QCheck.Gen.generate1 ~rand gen_row_query2 in
        let witness =
          List.find_opt
            (fun (s0, s1) ->
              let a = [(rv0, s0); (rv1, s1)] in
              gsubrow (subst_vars a sub) (subst_vars a super))
            (List.concat_map
               (fun a -> List.map (fun b -> (a, b)) row_universe)
               row_universe)
        in
        match witness with
        | Some (s0, s1) when not (accepts (Splat.tell_subrow ~sub ~super env_q))
          ->
          Printf.eprintf
            "\nCONCRETE over-reject:\n  sub   = %s\n  super = %s\n"
            (Row.to_string sub)
            (Row.to_string super);
          Printf.eprintf
            "  but ?p0 = %s , ?p1 = %s  IS a solution (engine still rejects)\n%!"
            (Row.Simple.to_string s0)
            (Row.Simple.to_string s1)
        | _ -> find (k - 1)
    in
    find 100000
  );
  Alcotest.run
    "splat"
    [
      ("subtype-units", unit_tests);
      ("subtype-properties", property_tests);
      ("fsub", fsub_tests);
      ("rigid-param-units", param_tests);
      ("rigid-param-properties", param_props);
      ("nested-param-units", nested_tests);
      ("nested-param-properties", nested_props);
      ("mixed-flex-rigid", mixed_tests);
      ("canon-spread", canon_spread_tests);
      ("coverage-tail", coverage_tail_tests);
      ("field-req", field_req_tests);
      ("corners", corners_tests);
      ("algebra", algebra_props);
    ]
