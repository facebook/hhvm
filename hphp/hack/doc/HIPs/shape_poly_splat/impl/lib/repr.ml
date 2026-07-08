open StdLabels

(* -- Primitive types ------------------------------------------------------- *)
module Prim = struct
  type t =
    | Bool
    | Nat

  let pp ppf t =
    match t with
    | Bool -> Fmt.any "bool" ppf ()
    | Nat -> Fmt.any "nat" ppf ()

  let to_string = Fmt.to_to_string pp

  let print = pp Format.std_formatter

  let equal t1 t2 =
    match (t1, t2) with
    | (Bool, Bool)
    | (Nat, Nat) ->
      true
    | (Bool, Nat)
    | (Nat, Bool) ->
      false
end

module Types = struct
  (* -- Base types (no quantified types of functions) ----------------------- *)
  type base =
    | Top
    | Bottom
    | Prim of Prim.t
    | Union of base * base
    | Shape of row
    | Flex of Ty_var.t
    | Rigid of Ty_param.t

  (* -- Rows over base types ------------------------------------------------ *)
  and row =
    | Row_simple of row_simple
    | Row_splat of row_splat

  (* -- Simple rows: known and unknown fields -------------------------------
     [unknown] is the base of the (always optional) absent labels, NOT a full
     field_desc: a REQUIRED unknown is uninhabited regardless of its base (it
     demands every absent label be present), so its only real content is "is this
     row bottom" — and bottom is represented solely as [Base.Bottom].  An absent
     label thus projects to [Field_desc.opt unknown]. *)
  and row_simple = {
    known: field_desc Label.Map.t;
    unknown: base;
  }

  (* -- Splats: ordered list of splat elements ------------------------------ *)
  and row_splat = { elems: row_splat_elem list }

  (* -- Splat elements: a spread of a base type.  An inline simple row is
     [Spread (Shape (Row_simple s))]; the bottom-row contribution is [Spread Bottom]. *)
  and row_splat_elem = Spread of base

  (* -- Field descriptors --------------------------------------------------- *)

  (* Field descriptors are a pair of bounds;
     - requiredness is a lower bound
     - type is an upper bound *)
  and field_desc = {
    req: field_req;
    base: base;
  }

  and field_req =
    | Req
    | Opt

  (* -- Meet , join & merge ------------------------------------------------- *)

  (* Meet / greatest lower bound for field requiredness: this is aa 2-point
     lattice with [Req] as bottom [Opt] as top.*)
  let meet_field_req req1 req2 =
    match (req1, req2) with
    | (Req, _)
    | (_, Req) ->
      Req
    | (Opt, Opt) -> Opt

  (* Join (least upper bound) for field requiredness: [Opt] unless both [Req]. *)
  let join_field_req req1 req2 =
    match (req1, req2) with
    | (Opt, _)
    | (_, Opt) ->
      Opt
    | (Req, Req) -> Req

  (* Left-associate fully: the result only ever has a union as a LEFT child (the final
     arm builds [Union (t1, t2)] only when [t2] is not itself a union), so [canon_base]
     is idempotent.  Recursing — rather than the one-level rotate [Union (Union (t1, t2),
     t3)] — is what flattens a right-nested argument all the way down.

     We do NOT distribute a union of two shapes into a single shape.  Collapsing
     [Shape r1 | Shape r2] to [Shape (r1 ⊔ r2)] (the pointwise field join) would
     close a completeness gap — e.g. [{x: Bool|Nat}] is then recognised as a
     supertype of [{x: Bool} | {x: Nat}] — but it is only DENOTATION-PRESERVING,
     and hence sound, when the two rows differ in at most one field.  Once they
     differ in two or more fields the join over-approximates via a cross term:
       {x: Bool, y: Bool} | {x: Nat, y: Nat}  ≠  {x: Bool|Nat, y: Bool|Nat}
     the RHS wrongly admits {x: Bool, y: Nat}, which is in neither operand.  So an
     unconditional distribution would trade incompleteness for UNsoundness; a
     guarded one (only when <= 1 field differs) would be sound but is left for later. *)
  let rec union_base t1 t2 =
    match (t1, t2) with
    | (Top, _)
    | (_, Top) ->
      Top
    | (Bottom, t)
    | (t, Bottom) ->
      t
    | (t1, Union (t2, t3)) -> union_base (union_base t1 t2) t3
    | _ -> Union (t1, t2)

  let rec meet_base t1 t2 =
    match (t1, t2) with
    | (Bottom, _)
    | (_, Bottom) ->
      Bottom
    | (Top, t)
    | (t, Top) ->
      t
    | (Shape r1, Shape r2) -> Shape (meet_row r1 r2)
    | (Union (a, b), t)
    | (t, Union (a, b)) ->
      (* distributive lattice: (a ∪ b) ⊓ t = (a ⊓ t) ∪ (b ⊓ t) *)
      union_base (meet_base a t) (meet_base b t)
    | (Prim p1, Prim p2) ->
      if Prim.equal p1 p2 then
        t1
      else
        Bottom
    | ((Flex _ | Rigid _), _)
    | (_, (Flex _ | Rigid _)) ->
      (* meet with an unsolved type variable or a rigid type parameter is an
         unrepresentable intersection; solve / instantiate it before meeting *)
      failwith "meet_base: unrepresentable meet with a flex or rigid leaf"
    | _ -> Bottom (* disjoint: a Prim and a Shape share no values *)

  and meet_field_desc fd1 fd2 =
    let req = meet_field_req fd1.req fd2.req
    and base = meet_base fd1.base fd2.base in
    { req; base }

  (* MEET of rows.  A splat still carrying a row variable or rigid denotes a FAMILY
     of rows, which has no single-row meet; substitute solved variables / canonicalize
     first so only simple rows (points in labels → field) reach here. *)
  and meet_row r1 r2 =
    match (r1, r2) with
    | (Row_simple s1, Row_simple s2) -> Row_simple (meet_row_simple s1 s2)
    | (Row_splat _, _)
    | (_, Row_splat _) ->
      failwith
        "meet_row: splat row — substitute row variables / canonicalize first"

  and meet_row_simple (r1 : row_simple) (r2 : row_simple) =
    let known =
      Label.Map.merge
        (fun _lbl fd_sub_opt fd_sup_opt ->
          let fd_sub =
            Option.value fd_sub_opt ~default:{ req = Opt; base = r1.unknown }
          and fd_super =
            Option.value fd_sup_opt ~default:{ req = Opt; base = r2.unknown }
          in
          Some (meet_field_desc fd_sub fd_super))
        r1.known
        r2.known
    and unknown = meet_base r1.unknown r2.unknown in
    { known; unknown }

  (* JOIN (lub).  For base types the lub is just [union_base] — union types are
     first-class, so join never needs to distribute into shapes.  For rows it is the
     pointwise field join, and (like meet) only total on simple rows. *)
  let join_field_desc fd1 fd2 =
    let req = join_field_req fd1.req fd2.req
    and base = union_base fd1.base fd2.base in
    { req; base }

  let join_row_simple (r1 : row_simple) (r2 : row_simple) =
    let known =
      Label.Map.merge
        (fun _lbl fd1_opt fd2_opt ->
          let fd1 =
            Option.value fd1_opt ~default:{ req = Opt; base = r1.unknown }
          and fd2 =
            Option.value fd2_opt ~default:{ req = Opt; base = r2.unknown }
          in
          Some (join_field_desc fd1 fd2))
        r1.known
        r2.known
    and unknown = union_base r1.unknown r2.unknown in
    { known; unknown }

  let join_row r1 r2 =
    match (r1, r2) with
    | (Row_simple s1, Row_simple s2) -> Row_simple (join_row_simple s1 s2)
    | (Row_splat _, _)
    | (_, Row_splat _) ->
      failwith
        "join_row: splat row — substitute row variables / canonicalize first"

  let merge_field_desc ~left ~right =
    match right.req with
    | Req -> right
    | Opt ->
      let req = meet_field_req left.req Opt
      and base = union_base left.base right.base in
      { req; base }

  let merge_row_simple
      ~left:{ known = kl; unknown = ul } ~right:{ known = kr; unknown = ur } =
    (* Both unknowns are optional, so their merge is just the union of their bases. *)
    let unknown = union_base ul ur
    and known =
      Label.Map.merge
        (fun _lbl fdl_opt fdr_opt ->
          let left = Option.value fdl_opt ~default:{ req = Opt; base = ul }
          and right = Option.value fdr_opt ~default:{ req = Opt; base = ur } in
          Some (merge_field_desc ~left ~right))
        kl
        kr
    in
    { known; unknown }

  (* -- Canonicalization / inhabitedness ------------------------------------ *)

  (* -- N.B. These predicates are only meaningful after canonicalization! --- *)
  let is_bot_base = function
    | Bottom -> true
    | Top
    | Prim _
    | Union _
    | Shape _
    | Flex _
    | Rigid _ ->
      false

  (* The known-field bottom test: a [Req ⊥] known field is uninhabited. *)
  let is_bot_field_desc fd =
    match fd.req with
    | Req -> is_bot_base fd.base
    | Opt -> false

  (* A simple row is NEVER bottom: bottom is solely [Base.Bottom], and the [unknown] is
     opt-only so it cannot make the row uninhabited.  A splat is bottom exactly when it
     carries a [Spread Bottom] (which canon then collapses to [Base.Bottom] when wrapped). *)
  let is_bot_row = function
    | Row_simple _ -> false
    | Row_splat { elems } ->
      List.exists elems ~f:(fun (Spread base) -> is_bot_base base)

  let bot_field_desc = { req = Req; base = Bottom }

  (* The simple-row view of a base that is a CONCRETE row contribution: a closed shape
     of a simple row.  [Bottom] (the bottom row) has NO single-row view — it is handled
     at the splat level by [is_bot_row] / the [shape] collapse.  Opaque spreads
     ([Rigid]/[Flex]), non-shapes, and a shape of a splat (which is flattened during
     canon, not merged) have none either. *)
  let base_as_row_simple_opt (b : base) : row_simple option =
    match b with
    | Shape (Row_simple s) -> Some s
    | Bottom
    | Top
    | Prim _
    | Union _
    | Flex _
    | Rigid _
    | Shape (Row_splat _) ->
      None

  (* Two adjacent spreads merge only when both are concrete row contributions (never a
     [Spread Bottom], which has no single-row view); the merged simple row — never bottom
     — is re-wrapped as a shape. *)
  let merge_splat_elem_opt ~left ~right =
    match (left, right) with
    | (Spread bl, Spread br) ->
      (match (base_as_row_simple_opt bl, base_as_row_simple_opt br) with
      | (Some sl, Some sr) ->
        let merged = merge_row_simple ~left:sl ~right:sr in
        Some (Spread (Shape (Row_simple merged)))
      | (_, _) -> None)

  let row_splat elems =
    match elems with
    | [Spread b] ->
      (match base_as_row_simple_opt b with
      | Some s -> Row_simple s
      | None -> Row_splat { elems })
    | _ -> Row_splat { elems }

  (* Labels a row contributes, looking THROUGH a [Spread (Shape r)] (an inline row) to
     [r]'s labels.  Opaque spreads ([Rigid]/[Flex]) and [Spread Bottom] contribute none. *)
  let rec row_label_set (r : row) : Label.Set.t =
    match r with
    | Row_simple { known; _ } -> Label.Map.keyset known
    | Row_splat { elems } ->
      List.fold_left elems ~init:Label.Set.empty ~f:(fun acc e ->
          Label.Set.union acc (splat_elem_label_set e))

  and splat_elem_label_set (e : row_splat_elem) : Label.Set.t =
    match e with
    | Spread (Shape r) -> row_label_set r
    | Spread (Top | Bottom | Prim _ | Union _ | Flex _ | Rigid _) ->
      Label.Set.empty

  (* Find the first [Spread (Union …)] in a splat's elements, returning the elements
     before it, the two union branches, and the elements after — or [None] if the splat
     has no union spread. *)
  let rec split_first_union_spread elems =
    match elems with
    | [] -> None
    | Spread (Union (p, q)) :: rest -> Some ([], p, q, rest)
    | e :: rest ->
      (match split_first_union_spread rest with
      | Some (prefix, p, q, suffix) -> Some (e :: prefix, p, q, suffix)
      | None -> None)

  let rec canon_base base =
    match base with
    | Top
    | Bottom
    | Prim _
    | Flex _
    | Rigid _ ->
      base
    | Union (b1, b2) -> union_base (canon_base b1) (canon_base b2)
    | Shape row ->
      let row = canon_row row in
      if is_bot_row row then
        Bottom
      else (
        (* A union spread floats OUT of the splat: rightmost-wins merge distributes over
           union, so [shape(.. ⊕ ..(a|b) ⊕ ..)] = [shape(.. ⊕ ..a ⊕ ..) | shape(.. ⊕ ..b ⊕ ..)].
           Distributing (recursively, one union spread at a time) keeps every canonical
           [Shape] free of union spreads — the union lives at the base level instead, where
           [subtype_base] already handles it (soundly; super-side unions stay conservative). *)
        match row with
        | Row_splat { elems } ->
          (match split_first_union_spread elems with
          | Some (prefix, p, q, suffix) ->
            let branch b =
              canon_base
                (Shape (Row_splat { elems = prefix @ (Spread b :: suffix) }))
            in
            union_base (branch p) (branch q)
          | None -> Shape row)
        | Row_simple _ -> Shape row
      )

  and canon_row row =
    match row with
    | Row_simple row_simple ->
      let row_simple = canon_row_simple row_simple in
      Row_simple row_simple
    | Row_splat splat ->
      let ({ elems } as splat) = canon_row_splat splat in
      (match elems with
      | [Spread b] ->
        (* a sole concrete spread collapses to its simple row *)
        (match base_as_row_simple_opt b with
        | Some simple -> Row_simple simple
        | None -> Row_splat splat)
      | _ -> Row_splat splat)

  (* Canonicalise a [Spread] element.  A spread of a nested splat shape is FLATTENED
     (its already-canonicalised elements are spliced into the parent splat); everything
     else — an inline [Spread (Shape (Row_simple s))], [Spread Bottom], and opaque
     [Spread (Rigid/Flex …)] — stays a single element.  Returns a list to allow the
     flattening. *)
  and collapse_spread (Spread base) =
    match canon_base base with
    | Shape (Row_splat { elems }) -> elems
    | b -> [Spread b]

  (* The 1-to-1 element canonicaliser exposed as [Splat_elem.canon]. *)
  and canon_row_splat_elem (Spread base) = Spread (canon_base base)

  and canon_field_desc fd =
    let base = canon_base fd.base in
    { fd with base }

  and canon_row_simple { known; unknown } =
    (* NB we do not canonicalize away a [Req ⊥] KNOWN field since we have a per-field
       interpretation for rows; the unknown is just a base. *)
    let known = Label.Map.map canon_field_desc known in
    let unknown = canon_base unknown in
    { known; unknown }

  and canon_row_splat { elems } =
    (* Inline closed spreads (shape / bottom) first so adjacent simples can merge. *)
    let elems = List.concat_map elems ~f:collapse_spread in
    canon_row_splat_help elems ~k:(fun elems -> { elems })

  and canon_row_splat_help elems ~k =
    match elems with
    | [] -> k []
    | left :: elems ->
      canon_row_splat_help elems ~k:(fun elems ->
          match elems with
          | right :: rest ->
            (match merge_splat_elem_opt ~left ~right with
            | Some merged -> k (merged :: rest)
            | _ -> k (left :: elems))
          | _ -> k [left])

  (* -- Pretty printers ----------------------------------------------------- *)
  let pp_field_req ppf field_req =
    match field_req with
    | Req -> Fmt.any "req" ppf ()
    | Opt -> Fmt.any "opt" ppf ()

  let rec pp_base ppf base =
    match base with
    | Top -> Fmt.(any "⊤") ppf ()
    | Bottom -> Fmt.(any "⊥") ppf ()
    | Prim prim -> Prim.pp ppf prim
    | Union (ty1, ty2) ->
      Fmt.(parens @@ hovbox @@ pair ~sep:(any "|") pp_base pp_base)
        ppf
        (ty1, ty2)
    | Shape row -> Fmt.(hovbox @@ (any "shape" ++ parens pp_row)) ppf row
    | Flex ty_var -> Ty_var.pp ppf ty_var
    | Rigid ty_param -> Ty_param.pp ppf ty_param

  and pp_row ppf row =
    match row with
    | Row_simple row_simple -> pp_row_simple ppf row_simple
    | Row_splat row_splat -> pp_row_splat ppf row_splat

  and pp_row_simple ppf { known; unknown } =
    Fmt.(
      braces
      @@ pair ~sep:comma (Label.Map.pp pp_field_desc) (pp_base ++ any "..."))
      ppf
      (known, unknown)

  and pp_field_desc ppf field_desc =
    Fmt.(hovbox @@ pair ~sep:sp pp_field_req pp_base)
      ppf
      (field_desc.req, field_desc.base)

  and pp_row_splat ppf { elems } =
    Fmt.(hovbox @@ list ~sep:(any " ⊕ ") pp_row_splat_elem) ppf elems

  and pp_row_splat_elem ppf (Spread base) =
    match base with
    | Shape row -> pp_row ppf row (* an inline row prints as the row itself *)
    | Top
    | Bottom
    | Prim _
    | Union _
    | Flex _
    | Rigid _ ->
      Fmt.(any "..." ++ pp_base) ppf base
end

(* -- Modules for export ---------------------------------------------------- *)
module Base = struct
  type t = Types.base =
    | Top
    | Bottom
    | Prim of Prim.t
    | Union of t * t
    | Shape of Types.row
    | Flex of Ty_var.t
    | Rigid of Ty_param.t

  (* -- Distinguished elements -- *)
  let top = Top

  let bot = Bottom

  let join t1 t2 = Types.union_base (Types.canon_base t1) (Types.canon_base t2)

  (* No intersection types: meet distributes structurally into shapes and bottoms out
     at [Bottom] for disjoint types (see [Types.meet_base]).  Canon first; and canon the
     RESULT too — meeting two shapes can yield an uninhabited [Shape] (a field meet hits
     ⊥), which [meet_base] builds with the raw constructor rather than collapsing to
     [Bottom].  Canonicalising the output keeps the contract "meet returns a canonical
     type" so [is_bot]/equality stay correct. *)
  let meet t1 t2 =
    Types.canon_base
      (Types.meet_base (Types.canon_base t1) (Types.canon_base t2))

  (* -- Canonicalization / inhabitedness -- *)
  let canon t = Types.canon_base t

  (* After canonicalization the bottom type is always represented by [Bottom] *)
  let is_bot t = Types.is_bot_base t

  (* -- Ctor / dtors --- *)
  let nat = Prim Prim.Nat

  let bool = Prim Prim.Bool

  let union t1 t2 = Types.union_base t1 t2

  let shape row =
    if Types.is_bot_row row then
      bot
    else
      Shape row

  let flex tv = Flex tv

  let rigid tp = Rigid tp

  (* -- Pretty printing -- *)
  let pp ppf t = Types.pp_base ppf t

  let to_string = Fmt.to_to_string pp

  let print = pp Format.std_formatter
end

module Field_req = struct
  type t = Types.field_req =
    | Req
    | Opt

  (* -- Distinguished elements -- *)
  let top = Opt

  let bot = Req

  (* -- Ops -- *)

  let lteq t1 t2 =
    match (t1, t2) with
    | ((Opt | Req), Opt) -> true
    | (Req, Req) -> true
    | (Opt, Req) -> false

  let meet t1 t2 = Types.meet_field_req t1 t2

  let join t1 t2 = Types.join_field_req t1 t2

  (* -- Pretty printing -- *)
  let pp ppf t = Types.pp_field_req ppf t

  let to_string = Fmt.to_to_string pp

  let print = pp Format.std_formatter
end

module Field_desc = struct
  type t = Types.field_desc = {
    req: Field_req.t;
    base: Base.t;
  }

  (* -- Distinguished elements -- *)
  let top = { req = Field_req.top; base = Base.top }

  let bot = { req = Field_req.bot; base = Base.bot }

  let meet { req = r1; base = b1 } { req = r2; base = b2 } =
    let req = Field_req.meet r1 r2 and base = Base.meet b1 b2 in
    { req; base }

  let join { req = r1; base = b1 } { req = r2; base = b2 } =
    let req = Field_req.join r1 r2 and base = Base.join b1 b2 in
    { req; base }

  (* -- Canonicalization / inhabiteness -- *)

  let canon t = Types.canon_field_desc t

  (* After canonicalization the bottom type is always represented by [Bottom] *)
  let is_bot t = Types.is_bot_field_desc t

  (* -- Ctor / dtor --- *)
  let req base = { req = Req; base }

  let opt base = { req = Opt; base }

  let is_req t =
    match t.req with
    | Req -> true
    | Opt -> false

  let is_opt t =
    match t.req with
    | Opt -> true
    | Req -> false

  (* -- Ops -- *)
  let merge ~left ~right = Types.merge_field_desc ~left ~right

  (** Field descriptors are a product with a requiredness lower bound (the
      minimum requiredness) and a base type upper bound (the maximum type).
      Given an interval between two field descriptors [lower] and [upper] this
      means we can have up to four 'corners', the possible extremes the
      field descriptor can take within the interval. Assuming a non-degenerate
      interval ( lower <= upper) this happens exactly when the lower bound
      has requiredness [Req] (the bottom element) and the upper bound has
      requiredness [Opt] (the top element).

      The corners are used when we have subrow propositions involving
      row parameters; consider

      ```
      |{x: Req Bool}, T|  <:  |T|   with   T ∈ [ {x: Req Nat}, {x: Opt Top} ]
      ```

      For this to be true for all posible `T` it must be true at all of `T`s
      extremes. Within `T`s interval, `x` can take on all of the following
      field descriptors:
      1) `Req Nat` (lower bound)
      2) `Opt Nat` (moving along the requiredness axis)
      3) `Req Top` (moving along the type axis)
      4) `Opt Top` (moving along both axes, upper bound)

      If we only considered the lower and upper bounds, we would reach the
      wrong conclusion:

      1) Lower bound
      ```
      T = {x: Req Nat}

      | {x: Req Bool}, {x: Req Nat} | <: | {x: Req Nat} |
      | {x: Req Nat} | <: | {x: Req Nat} |
      true
      ```

      4) Upper bound
      ```
      T = {x: Opt Top}

      | {x: Req Bool}, {x: Opt Top} | <: | {x: Opt Top} |
      | {x: Req Top} | <: | {x: Opt Top} |
      true
      ```

      It's only when we consider case (2) that we see the proposition is false:

      2) Upper requiredness bound, lower type bound
      ```
      T = {x: Opt Nat}
      | {x: Req Bool}, {x: Opt Nat} | <: | {x: Opt Nat} |
      | {x: Req (Bool|Nat)} | <: | {x: Opt Nat} |
      false
      ```

      *)
  let corners ~lower ~upper =
    match (lower.req, upper.req) with
    | (Req, Opt) ->
      let lower_opt = { lower with req = Opt }
      and upper_req = { upper with req = Req } in
      [lower; upper; lower_opt; upper_req]
    | (Req, Req)
    | (Opt, Opt) ->
      [lower; upper]
    | (Opt, Req) -> (* ill-formed! *) []

  (* -- Pretty printing -- *)

  let pp ppf t = Types.pp_field_desc ppf t

  let to_string = Fmt.to_to_string pp

  let print = pp Format.std_formatter
end

module Row = struct
  module Simple = struct
    type t = Types.row_simple = {
      known: Field_desc.t Label.Map.t;
      unknown: Base.t;
    }

    (* -- Distinguished elements -- *)

    (* The top simple row has all fields optional at type [Top] *)
    let top = { known = Label.Map.empty; unknown = Base.top }

    (* The empty simple row has no required fields and all optional fields at
       type [Bottom]. This is the left and right unit of merge.
       NB the empty simple row is _not_ bottom since `Opt _ </: Req _` *)
    let empty = { known = Label.Map.empty; unknown = Base.bot }

    (* -- Canonicalization / inhabitedness -- *)
    let canon t = Types.canon_row_simple t

    (* -- Ctor /dtor -- *)

    (* [unknown] is the base of the (optional) absent labels. *)
    let of_fields fields unknown =
      let known = Label.Map.of_list fields in
      { known; unknown }

    let closed fields = of_fields fields Base.bot

    let open_ fields = of_fields fields Base.top

    let labels { known; _ } = Label.Map.keys known

    let label_set { known; _ } = Label.Map.keyset known

    (* -- Merge -- *)
    let merge ~left ~right = Types.merge_row_simple ~left ~right

    (* -- Projection -- *)

    (* NB projection is on optional labels where [None] corresponds to an unknown
       label; an absent label projects to its (optional) unknown base. *)
    let proj { known; unknown } lbl_opt =
      match Option.bind lbl_opt (fun lbl -> Label.Map.find_opt lbl known) with
      | Some fd -> fd
      | None -> Field_desc.opt unknown

    (* -- Pretty printing -- *)

    let pp ppf t = Types.pp_row_simple ppf t

    let to_string = Fmt.to_to_string pp

    let print = pp Format.std_formatter
  end

  module Splat_elem = struct
    type t = Types.row_splat_elem = Spread of Base.t

    (* -- Distinguished elements -- *)

    (* The empty row spread — the merge identity, contributing nothing. *)
    let empty = Spread (Base.shape (Types.Row_simple Simple.empty))

    (* -- Ops -- *)
    let merge_opt ~left ~right = Types.merge_splat_elem_opt ~left ~right

    (* -- Canonicalization / inhabitedness -- *)

    let canon t = Types.canon_row_splat_elem t

    (* Bottom exactly when the spread is the bottom row, i.e. [Spread Bottom] after
       canonicalisation (an inline [Spread (Shape r)] with [r] uninhabited canonicalises
       to that). *)
    let is_bot (Spread base) = Types.is_bot_base base

    (* -- Ctor / dtor -- *)
    let spread base = Spread base

    let simple simple = Spread (Base.shape (Types.Row_simple simple))

    (* Looks THROUGH a [Spread (Shape r)] (an inline row) to [r]'s labels; opaque
       spreads ([Rigid]/[Flex]) and [Spread Bottom] contribute none. *)
    let label_set t = Types.splat_elem_label_set t

    let labels t = Label.Set.elements (Types.splat_elem_label_set t)

    let spread_param_opt (Spread base) =
      match base with
      | Base.Rigid ty_param -> Some ty_param
      | Base.Top
      | Base.Bottom
      | Base.Prim _
      | Base.Union _
      | Base.Shape _
      | Base.Flex _ ->
        None

    let spread_var_opt (Spread base) =
      match base with
      | Base.Flex ty_var -> Some ty_var
      | Base.Top
      | Base.Bottom
      | Base.Prim _
      | Base.Union _
      | Base.Shape _
      | Base.Rigid _ ->
        None

    let proj (Spread base) lbl fld_assignment =
      match base with
      | Base.Rigid ty_param -> Ty_param.Map.find ty_param fld_assignment
      | Base.Bottom -> Types.bot_field_desc
      | Base.Shape (Types.Row_simple simple) -> Simple.proj simple lbl
      | Base.Shape (Types.Row_splat _)
      | Base.Top
      | Base.Prim _
      | Base.Union _
      | Base.Flex _ ->
        failwith "Row.Splat_elem.proj: unexpected spread element"

    (* -- Pretty printing -- *)

    let pp ppf t = Types.pp_row_splat_elem ppf t

    let to_string = Fmt.to_to_string pp

    let print = pp Format.std_formatter
  end

  module Splat = struct
    type t = Types.row_splat = { elems: Splat_elem.t list }

    (* -- Canonicalization / inhabitedness -- *)

    let canon (t : t) = Types.canon_row_splat t

    (* -- Ctor / dtors -- *)

    (* NB: this doesn't guard on non-empty lists *)
    let of_list elems = { elems }

    let labels { elems } = List.concat_map elems ~f:Splat_elem.labels

    let label_set { elems } =
      List.fold_left elems ~init:Label.Set.empty ~f:(fun acc elem ->
          Label.Set.union acc (Splat_elem.label_set elem))

    let spread_params { elems } : Ty_param.t list =
      List.filter_map elems ~f:Splat_elem.spread_param_opt

    let spread_param_set { elems } : Ty_param.Set.t =
      List.fold_left elems ~init:Ty_param.Set.empty ~f:(fun acc splat_elem ->
          Option.fold
            (Splat_elem.spread_param_opt splat_elem)
            ~none:acc
            ~some:(fun param -> Ty_param.Set.add param acc))

    let has_spread_var { elems } : bool =
      List.exists elems ~f:(fun elem ->
          Option.is_some (Splat_elem.spread_var_opt elem))

    let spread_vars { elems } : Ty_var.t list =
      List.filter_map elems ~f:Splat_elem.spread_var_opt

    (** Try and find a topological ordering of the spread (flex) variables; if there
       is a cycle just skip the element which introduces it *)
    let topo_spread_vars t : Ty_var.t list =
      let vars = spread_vars t in
      let rec loop vars seen acc =
        match vars with
        | next :: (head :: _ as rest) when Ty_var.equal next head ->
          loop rest seen acc
        | next :: rest ->
          let acc =
            if Ty_var.Set.mem next seen then
              acc
            else
              next :: acc
          in
          loop rest (Ty_var.Set.add next seen) acc
        | [] -> List.rev acc
      in
      loop vars Ty_var.Set.empty []

    let spread_var_set { elems } : Ty_var.Set.t =
      List.fold_left elems ~init:Ty_var.Set.empty ~f:(fun acc splat_elem ->
          Option.fold
            (Splat_elem.spread_var_opt splat_elem)
            ~none:acc
            ~some:(fun var -> Ty_var.Set.add var acc))

    (* For inference, we only want rows with at most one spread variable so this
       predicate tells us when we need to eliminate and which spread variable
       to retain *)
    let rec rightmost_spread_var_help elems (n, rightmost_opt) =
      match elems with
      | [] -> Option.map (fun var -> (n, var)) rightmost_opt
      | elem :: elems ->
        (match Splat_elem.spread_var_opt elem with
        | Some v -> rightmost_spread_var_help elems (n + 1, Some v)
        | None -> rightmost_spread_var_help elems (n, rightmost_opt))

    let rightmost_spread_var { elems } =
      rightmost_spread_var_help elems (0, None)

    (* Find all rigid spread parameters which are to the right of a `Req` for a
       given label. Only these can contribute to the label's merged field
       descriptor *)
    let live_spread_at { elems } lbl =
      let rec aux rev_elems acc =
        match rev_elems with
        | [] -> acc
        | Splat_elem.Spread (Base.Rigid ty_param) :: left ->
          aux left (ty_param :: acc)
        | Splat_elem.Spread Base.Bottom :: _left ->
          (* the bottom row forces the field [Req ⊥]; everything to its left is masked *)
          acc
        | Splat_elem.Spread (Base.Shape (Types.Row_simple simple)) :: left ->
          (match (Simple.proj simple lbl).req with
          | Req -> acc
          | Opt -> aux left acc)
        | Splat_elem.Spread _ :: left ->
          (* a flex spread var / opaque element: doesn't definitively mask, keep going *)
          aux left acc
      in
      aux (List.rev elems) []

    let rec proj_help
        (elems : Splat_elem.t list)
        (lbl : Label.t option)
        (fd_right : Field_desc.t)
        (fld_assignment : Field_desc.t Ty_param.Map.t) =
      match (fd_right.req, elems) with
      | (Req, _)
      | (_, []) ->
        fd_right
      | (Opt, rightmost :: left) ->
        let fd_left = Splat_elem.proj rightmost lbl fld_assignment in
        let fd = Field_desc.merge ~left:fd_left ~right:fd_right in
        proj_help left lbl fd fld_assignment

    let proj
        { elems }
        (lbl : Label.t option)
        (fld_assignment : Field_desc.t Ty_param.Map.t) =
      match List.rev elems with
      | [] -> failwith "Row.Splat.proj: empty splat"
      | rightmost :: left ->
        let fd = Splat_elem.proj rightmost lbl fld_assignment in
        proj_help left lbl fd fld_assignment

    let partition_at_var { elems } ty_var =
      let rec loop elems left =
        match elems with
        | [] -> None
        | elem :: elems ->
          (match Splat_elem.spread_var_opt elem with
          | Some v when Ty_var.equal ty_var v -> Some (List.rev left, elems)
          | _ -> loop elems (elem :: left))
      in
      loop elems []

    (* -- Pretty printing -- *)

    let pp ppf t = Types.pp_row_splat ppf t

    let to_string = Fmt.to_to_string pp

    let print = pp Format.std_formatter
  end

  type t = Types.row =
    | Row_simple of Simple.t
    | Row_splat of Splat.t

  (* -- Distinguished elements -- *)
  let top = Row_simple Simple.top

  (* The bottom row is the [Spread Bottom] contribution; it shape-wraps to [Base.Bottom]. *)
  let bot = Row_splat (Splat.of_list [Splat_elem.spread Base.bot])

  let empty = Row_simple Simple.empty

  (* -- Canonicalization -- *)

  let canon t = Types.canon_row t

  let is_bot t = Types.is_bot_row t

  (* -- Meet / join -- *)

  (* Canon first so any splat whose holes are resolved collapses to a simple row;
     a splat that survives (unresolved variable / rigid) has no single-row meet/join
     and raises (substitute the variable's current solution before combining).  The
     bottom row is now a splat ([Spread Bottom]), so handle it up front: it is the meet
     absorber / join identity (preserving the old behaviour when [bot] was a simple row). *)
  let meet t1 t2 =
    if is_bot t1 || is_bot t2 then
      bot
    else
      Types.canon_row (Types.meet_row (Types.canon_row t1) (Types.canon_row t2))

  let join t1 t2 =
    if is_bot t1 then
      Types.canon_row t2
    else if is_bot t2 then
      Types.canon_row t1
    else
      Types.join_row (Types.canon_row t1) (Types.canon_row t2)

  (* -- Ctors / dtors -- *)

  let simple fields unknown = Row_simple (Simple.of_fields fields unknown)

  let closed fields = simple fields Base.bot

  let open_ fields = simple fields Base.top

  let splat elems =
    match elems with
    | [] -> empty
    | _ -> Row_splat (Splat.of_list elems)

  (* For inference, we only want rows with at most one spread variable so this
     predicate tells us when we need to eliminate and which spread variable
     to retain *)
  let rightmost_var t =
    match t with
    | Row_simple _ -> None
    | Row_splat splat -> Splat.rightmost_spread_var splat

  let labels t =
    match t with
    | Row_simple simple -> Simple.labels simple
    | Row_splat splat -> Splat.labels splat

  let label_set t =
    match t with
    | Row_simple simple -> Simple.label_set simple
    | Row_splat splat -> Splat.label_set splat

  let spread_params t =
    match t with
    | Row_simple _ -> []
    | Row_splat splat -> Splat.spread_params splat

  let spread_param_set t =
    match t with
    | Row_simple _ -> Ty_param.Set.empty
    | Row_splat splat -> Splat.spread_param_set splat

  let has_spread_var t =
    match t with
    | Row_simple _ -> false
    | Row_splat splat -> Splat.has_spread_var splat

  let spread_vars t =
    match t with
    | Row_simple _ -> []
    | Row_splat splat -> Splat.spread_vars splat

  let topo_spread_vars t =
    match t with
    | Row_simple _ -> []
    | Row_splat splat -> Splat.topo_spread_vars splat

  let spread_var_set t =
    match t with
    | Row_simple _ -> Ty_var.Set.empty
    | Row_splat splat -> Splat.spread_var_set splat

  (* A bare spread variable [|...?v|] — the shape [subrow_infer_*] emits for a flex
     spread variable.
     TODO: should this be another ctor?
  *)
  let as_spread_var (t : t) : Ty_var.t option =
    match t with
    | Row_splat { elems = [elem] } -> Splat_elem.spread_var_opt elem
    | _ -> None

  let live_spread_at t lbl =
    match t with
    | Row_simple _ -> []
    | Row_splat splat -> Splat.live_spread_at splat lbl

  let proj
      (t : t)
      (lbl : Label.t option)
      (fld_assignment : Field_desc.t Ty_param.Map.t) =
    match t with
    | Row_simple simple -> Simple.proj simple lbl
    | Row_splat splat -> Splat.proj splat lbl fld_assignment

  let partition_at_var t ty_var =
    match t with
    | Row_simple _ -> None
    | Row_splat splat -> Splat.partition_at_var splat ty_var

  (* -- Pretty printing -- *)
  let pp ppf t = Types.pp_row ppf t

  let to_string = Fmt.to_to_string pp

  let print = pp Format.std_formatter
end

(* -- Types ----------------------------------------------------------------- *)
module Ty = struct
  type t =
    | Base of Base.t
    | Forall of {
        quant: Ty_param.t;
        scope: t;
      }
    | Fn of t * t

  (* -- Pretty printing -- *)

  let rec pp ppf ty =
    match ty with
    | Base base -> Base.pp ppf base
    | Forall { quant; scope } ->
      Fmt.(hovbox @@ pair ~sep:(any ". ") (any "forall " ++ Ty_param.pp) pp)
        ppf
        (quant, scope)
    | Fn (dom, codom) ->
      Fmt.(hovbox @@ pair ~sep:(any " -> ") pp pp) ppf (dom, codom)

  let to_string = Fmt.to_to_string pp

  let print = pp Format.std_formatter
end
