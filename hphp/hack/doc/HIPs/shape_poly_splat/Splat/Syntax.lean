import Mathlib.Data.Fintype.EquivFin
import Mathlib.Data.String.Basic

/-!
# Row substrate: syntax and computation

`Base`/`Row`/`SplatElem`/`FieldDesc` (mutual, with a `Spread`-of-base splat element and rigid
type parameters), `SimpleRow` (nodup-carrying), and the parameter-free core operations:
`nubLabels`, `proj`, `mergeField`/`mergeSimple`, `normalize`.

A `SimpleRow`'s `unknown` is a *base*, not a full `FieldDesc`: the absent labels are always
*optional*.  A required unknown would be uninhabited regardless of its base (every one of the
infinitely many absent labels forced present), so its only content is "is this row bottom" — and
bottom is represented *solely* as `Base.bot` (a `SimpleRow` is never bottom; a bottom contribution
is a splat carrying `spread .bot`).  This mirrors the OCaml `repr.ml` (opt-only `row_simple.unknown`).

Accordingly `proj` returns the stored descriptor, or `.opt unknown` for an absent label, and
`mergeSimple` merges the two `unknown` *bases* (`mergeField` of two optionals = a union of bases).
-/

namespace Splat

abbrev Label    := String
abbrev Ty_param := String

/-- Primitive base types (OCaml `Prim.t`). -/
inductive Prim : Type where
  | bool : Prim
  | nat  : Prim
deriving DecidableEq, Repr

/- ---------------------------------------------------------------------------------------------- -/
/- ## 1. Syntax                                                                                   -/
/- ---------------------------------------------------------------------------------------------- -/

/-
  Base, FieldDesc, Row, SplatElem are mutually recursive:
    Base uses Row (via `shape`)
    Row uses FieldDesc and SplatElem
    FieldDesc uses Base
    SplatElem uses Base (via `spread`)
-/
mutual
  /-- Base types (the OCaml `Base.t`, minus the inference-only `Flex`).  `rigid` is a ∀-bound
      rigid type parameter (OCaml `Rigid of Ty_param.t`), used at base position directly and at
      row position via `SplatElem.spread (rigid α)`. -/
  inductive Base : Type where
    | top   : Base
    | bot   : Base
    | prim  : Prim → Base
    | union : Base → Base → Base
    | shape : Row → Base
    | rigid : Ty_param → Base

  /-- Field descriptor.  `req`/`opt` is the requiredness (lower bound, `Req < Opt`); the
      carried `Base` is the type (upper bound). -/
  inductive FieldDesc : Type where
    | req : Base → FieldDesc   -- field that must be present
    | opt : Base → FieldDesc   -- field that may be absent

  /-- A row is either a simple row (known fields + an unknown-field descriptor) or a splat.
      The known fields are a raw assoc-list: the `Nodup` invariant cannot live on a nested
      inductive occurrence (kernel rejection), so it is carried by `SimpleRow` instead and
      re-established by `normalize`. -/
  inductive Row : Type where
    | simple : List (Label × FieldDesc) → Base → Row
    | splat  : List SplatElem → Row

  /-- A splat element is a `Spread` of a base type (OCaml `row_splat_elem = Spread of base`).
      An inline simple row is `spread (.shape (.simple fs u))`; the bottom-row contribution is
      `spread .bot`; a rigid row parameter is `spread (.rigid α)`.  (No flex: inference is out
      of scope.) -/
  inductive SplatElem : Type where
    | spread : Base → SplatElem
end

/-- A **simple row**: known fields, an `unknown` field descriptor, and a proof that the known
    field labels contain no duplicates. -/
structure SimpleRow where
  known   : List (Label × FieldDesc)
  unknown : Base
  nodup   : (known.map Prod.fst).Nodup

/- ---------------------------------------------------------------------------------------------- -/
/- ## 2. Duplicate-free key lists                                                                 -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Keep the first occurrence of each label. -/
def nubLabels : List Label → List Label
  | []      => []
  | l :: ls => l :: (nubLabels ls).filter (· != l)

theorem mem_nubLabels {l : Label} : ∀ {xs}, l ∈ nubLabels xs ↔ l ∈ xs := by
  intro xs
  induction xs with
  | nil =>
    -- `nubLabels [] = []`, so both sides are `l ∈ []`.
    exact Iff.rfl
  | cons a as ih =>
    -- `nubLabels (a :: as) = a :: (nubLabels as).filter (· != a)` by definition.
    show l ∈ a :: (nubLabels as).filter (· != a) ↔ l ∈ a :: as
    -- Expand membership in both conses and in the filter, then fold the IH.
    rw [List.mem_cons, List.mem_cons, List.mem_filter, ih]
    -- Goal: `l = a ∨ (l ∈ as ∧ (l != a) = true)  ↔  l = a ∨ l ∈ as`.
    constructor
    case mp =>
      rintro (rfl | ⟨h, _⟩)
      case inl => exact Or.inl rfl
      case inr => exact Or.inr h
    case mpr =>
      rintro (rfl | h)
      case inl => exact Or.inl rfl
      case inr =>
        by_cases he : l = a
        case pos => exact Or.inl he
        case neg =>
          -- `l ≠ a` discharges the boolean side-condition `(l != a) = true`.
          exact Or.inr ⟨h, bne_iff_ne.mpr he⟩

theorem nubLabels_nodup : ∀ xs, (nubLabels xs).Nodup := by
  intro xs
  induction xs with
  | nil =>
    -- `nubLabels [] = []` is vacuously duplicate-free.
    exact List.nodup_nil
  | cons a as ih =>
    -- `nubLabels (a :: as) = a :: (nubLabels as).filter (· != a)`.
    show (a :: (nubLabels as).filter (· != a)).Nodup
    -- A cons is `Nodup` iff its head is absent from the tail and the tail is `Nodup`.
    rw [List.nodup_cons]
    refine ⟨fun hmem => ?_, ih.filter _⟩
    -- Every element the filter keeps is `!= a`, so the head `a` cannot be among them.
    have hbne : (a != a) = true := (List.mem_filter.mp hmem).2
    rw [bne_self_eq_false] at hbne
    exact Bool.noConfusion hbne

/-- A key-preserving map leaves the key list unchanged. -/
theorem map_fst_eq (f : Label → Label × FieldDesc) (hf : ∀ l, (f l).1 = l) :
    ∀ keys : List Label, (keys.map f).map Prod.fst = keys := by
  intro keys
  induction keys with
  | nil => rfl
  | cons a as ih =>
    -- `(a :: as).map f = f a :: as.map f`; map `.1` over it; rewrite head by `hf` and tail by IH.
    rw [List.map_cons, List.map_cons, hf a, ih]

/- ---------------------------------------------------------------------------------------------- -/
/- ## 3. Projection and its characterisation lemmas                                               -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Project a simple row at a label.
    Returns the stored descriptor, or the `unknown` descriptor if the label is absent. -/
def proj (r : SimpleRow) (l : Label) : FieldDesc :=
  match r.known.lookup l with
  | some d => d
  | none   => .opt r.unknown

-- Characterisation lemmas for the association-list representation.  With these, every
-- per-field projection proof reduces to membership / lookup reasoning.

/-- A label absent from the keys is absent from the association list. -/
theorem lookup_eq_none_keys {β} (xs : List (Label × β)) (l : Label)
    (h : l ∉ xs.map Prod.fst) : xs.lookup l = none := by
  induction xs with
  | nil => rfl
  | cons hd tl ih =>
    obtain ⟨a, b⟩ := hd
    -- Split the hypothesis: `l` is neither the head key `a` nor in the tail keys.
    -- (`simp only` here also reduces `Prod.fst (a, b)` to `a`, which `rw` would leave stuck.)
    simp only [List.map_cons, List.mem_cons, not_or] at h
    -- `lookup` skips the head because `l == a` is `false`, then recurses on the tail.
    rw [List.lookup_cons, beq_eq_false_iff_ne.mpr h.1]
    exact ih h.2

/-- Looking up in a key-indexed map. -/
theorem lookup_map_pair (g : Label → FieldDesc) (l : Label) :
    ∀ keys : List Label,
      (keys.map (fun k => (k, g k))).lookup l = if l ∈ keys then some (g l) else none := by
  intro keys
  induction keys with
  | nil =>
    -- empty map: `lookup` is `none`, and `l ∈ []` is false so the `if` is `none` too
    rfl
  | cons a as ih =>
    -- map over the cons, then `lookup` on the resulting cons
    rw [List.map, List.lookup_cons]
    by_cases h : l = a
    case pos =>
      -- head matches: `l == a` is `true` and `l ∈ a :: as` holds, both sides give `some (g l)`
      subst h
      simp only [beq_self_eq_true, List.mem_cons, true_or, if_true]
    case neg =>
      -- head differs: `l == a` is `false` (skip head); `l ∈ a :: as` collapses to `l ∈ as`
      rw [beq_eq_false_iff_ne.mpr h]
      simp only [List.mem_cons, h, false_or, ih]

/-- If a label is absent from a row's keys, it projects to `.opt unknown`. -/
theorem proj_not_mem (r : SimpleRow) (l : Label) (h : l ∉ r.known.map Prod.fst) :
    proj r l = .opt r.unknown := by
  -- the label is absent, so `lookup` returns `none` and `proj` falls back to `.opt unknown`
  unfold proj
  rw [lookup_eq_none_keys r.known l h]

/- ---------------------------------------------------------------------------------------------- -/
/- ## 4. Field and row merge                                                                      -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Rightmost-wins merge of two field descriptors (OCaml `merge_field_desc`).
    • Right `Req t`               → `Req t`            (right Req overrides everything)
    • Right `Opt t`, Left `Req u` → `Req (u ∪ t)`      (preserve required, union types)
    • Right `Opt t`, Left `Opt u` → `Opt (u ∪ t)`      (both optional, union types)    -/
def mergeField (left right : FieldDesc) : FieldDesc :=
  match right with
  | .req t => .req t
  | .opt t => match left with
    | .req u => .req (.union u t)
    | .opt u => .opt (.union u t)

/-- Rightmost-wins merge of two simple rows (OCaml `merge_row_simple`).

    The result's key list is the deduplicated union of both key lists, so the `Nodup`
    obligation is discharged directly from `nubLabels_nodup`.  Each key's descriptor is
    obtained by `mergeField`ing the two projections; absent labels project to `.opt unknown`,
    so this single formula is correct for every label.  Both unknowns are optional, so their
    merge is the union of their bases (matching `mergeField` on two `.opt`s). -/
def mergeSimple (left right : SimpleRow) : SimpleRow :=
  let keys := nubLabels (left.known.map Prod.fst ++ right.known.map Prod.fst)
  { known   := keys.map (fun l => (l, mergeField (proj left l) (proj right l)))
    unknown := .union left.unknown right.unknown
    nodup   := by rw [map_fst_eq _ (fun l => rfl)]; exact nubLabels_nodup _ }

/-- The key structural theorem: `proj` distributes over `mergeSimple`. Given two rows `left` and
    `right` and a label `l`, the field resulting from merging `left` and `right` then projecting
    is the same as projecting each then merging the fields -/
theorem proj_mergeSimple (left right : SimpleRow) (l : Label) :
    proj (mergeSimple left right) l = mergeField (proj left l) (proj right l) := by
  -- unfold `proj` and `mergeSimple`, then look `l` up in the merged key-map
  unfold mergeSimple
  unfold proj
  rw [lookup_map_pair]
  by_cases h : l ∈ nubLabels (left.known.map Prod.fst ++ right.known.map Prod.fst)
  case pos =>
    -- `l` is a known key: its stored descriptor is exactly the merged field
    rw [if_pos h]
  case neg =>
    -- `l` is absent from both sides: each projection falls back to `.opt unknown`, and the merged
    -- unknown is `.union`; `mergeField (.opt _) (.opt _) = .opt (.union _ _)` closes it definitionally
    rw [if_neg h]
    rw [mem_nubLabels, List.mem_append, not_or] at h
    rw [lookup_eq_none_keys left.known l h.1, lookup_eq_none_keys right.known l h.2]
    rfl

/- ---------------------------------------------------------------------------------------------- -/
/- ## 5. Normalisation                                                                            -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Turn a raw (possibly duplicate-keyed) field list into a duplicate-free `SimpleRow`,
    keeping the first occurrence of each label.  `proj` is preserved (see `proj_normalize`). -/
def normalize (fs : List (Label × FieldDesc)) (u : Base) : SimpleRow :=
  { known   := (nubLabels (fs.map Prod.fst)).map (fun l => (l, (fs.lookup l).getD (.opt u)))
    unknown := u
    nodup   := by rw [map_fst_eq _ (fun l => rfl)]; exact nubLabels_nodup _ }

theorem proj_normalize (fs : List (Label × FieldDesc)) (u : Base) (l : Label) :
    proj (normalize fs u) l = match fs.lookup l with | some d => d | none => .opt u := by
  -- unfold `proj` on the normalised row (whose keys are the deduped originals)
  show (match ((nubLabels (fs.map Prod.fst)).map
          (fun l => (l, (fs.lookup l).getD (.opt u)))).lookup l with
        | some d => d | none => .opt u) = _
  rw [lookup_map_pair]
  by_cases h : l ∈ fs.map Prod.fst
  case pos =>
    -- present key: dedup keeps it, and `(fs.lookup l).getD u` is the original `some` descriptor
    rw [if_pos (mem_nubLabels.mpr h)]; cases fs.lookup l <;> rfl
  case neg =>
    -- absent key: both the deduped lookup and the original lookup are `none`
    rw [if_neg (fun hc => h (mem_nubLabels.mp hc)), lookup_eq_none_keys fs l h]

/-- A `SimpleRow` round-trips (projection-wise) through its own syntax under `normalize`. -/
theorem proj_normalize_simplerow (r : SimpleRow) (l : Label) :
    proj (normalize r.known r.unknown) l = proj r l := by
  rw [proj_normalize]; rfl

/- ---------------------------------------------------------------------------------------------- -/
/- ## 6. Distinguished rows                                                                       -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The empty / merge-identity row: every absent field is optional at ⊥ (OCaml `Simple.empty`).
    There is no bottom *row*: bottom is solely `Base.bot` (a `SimpleRow` is never bottom). -/
def emptyRow : SimpleRow := ⟨[], .bot, List.nodup_nil⟩

/-- The top row: every absent field is optional at ⊤ (OCaml `Simple.top`). -/
def topRow : SimpleRow := ⟨[], .top, List.nodup_nil⟩

/- ---------------------------------------------------------------------------------------------- -/
/- ## 7. Canonicalisation                                                                         -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Smart union (OCaml `union_base`): `⊤` absorbs, `⊥` is the identity, and a right-nested union
    is flattened so the result only ever carries a union as a *left* child — which is what makes
    `canonBase` idempotent.  We do **not** distribute a union of two shapes into a single shape:
    that would be sound only when the rows differ in at most one field (see `repr.ml`). -/
def unionBase : Base → Base → Base
  | .top, _          => .top
  | _, .top          => .top
  | .bot, t          => t
  | t, .bot          => t
  | t1, .union t2 t3 => unionBase (unionBase t1 t2) t3
  | t1, t2           => .union t1 t2
termination_by _ t2 => sizeOf t2

/- N.B. these `isBot*` predicates are only meaningful *after* canonicalisation. -/

/-- A base is the bottom type exactly when it is `.bot` (post-canon). -/
def isBotBase : Base → Bool
  | .bot => true
  | _    => false

/-- A field descriptor is bottom (uninhabited) when it is *required* at a bottom type; an
    optional field is always inhabited (by absence). -/
def isBotField : FieldDesc → Bool
  | .req b => isBotBase b
  | .opt _ => false

/-- A row is bottom exactly when it is a splat carrying a `spread .bot` (the bottom-row
    contribution, which `canon` collapses to `Base.bot` when shape-wrapped).  A *simple* row is
    never bottom — its `unknown` is opt-only, so it cannot force the absent labels.  (Per the
    per-field interpretation a known field at `req ⊥` is deliberately *not* collapsed.) -/
def isBotRow : Row → Bool
  | .simple _ _ => false
  | .splat es   => es.any (fun e => match e with | .spread b => isBotBase b)

/-- The simple-row view of a base that is a *concrete* row contribution: a closed shape of a
    simple row (deduplicated via `normalize`).  `.bot` (the bottom row) has *no* single-row view —
    it is handled at the splat level by `isBotRow` / the `shapeBase` collapse.  Opaque spreads
    (`.rigid`), non-shapes, and a shape of a splat have none either (OCaml
    `base_as_row_simple_opt`). -/
def baseAsSimple : Base → Option SimpleRow
  | .shape (.simple fs u) => some (normalize fs u)
  | _                     => none

/-- Merge two adjacent splat elements, but only when both are concrete row contributions (never a
    `spread .bot`, which has no single-row view); the merged simple row — never bottom — is
    re-wrapped as a shape (OCaml `merge_splat_elem_opt`). -/
def mergeSplatElem : SplatElem → SplatElem → Option SplatElem
  | .spread bl, .spread br =>
    match baseAsSimple bl, baseAsSimple br with
    | some sl, some sr =>
      let m := mergeSimple sl sr
      some (.spread (.shape (.simple m.known m.unknown)))
    | _, _ => none

/-- Coalesce adjacent concrete spreads in a single right-to-left pass (OCaml
    `canon_row_splat_help`): process the tail, then try to merge the head into the head of the
    processed tail.  A successful merge cascades, since the merged element becomes the new head. -/
def mergeAdjacent : List SplatElem → List SplatElem
  | []           => []
  | left :: rest =>
    match mergeAdjacent rest with
    | right :: rest' =>
      match mergeSplatElem left right with
      | some merged => merged :: rest'
      | none        => left :: right :: rest'
    | [] => [left]

/-- Smart shape constructor (OCaml `Base.shape`): a shape of the bottom row *is* `.bot`. -/
def shapeBase (row : Row) : Base := if isBotRow row then .bot else .shape row

/-- Smart splat constructor (OCaml `Types.row_splat`): a lone concrete spread collapses to its
    simple row; otherwise the elements stay a splat. -/
def rowSplat : List SplatElem → Row
  | [.spread b] =>
    match baseAsSimple b with
    | some s => .simple s.known s.unknown
    | none   => .splat [.spread b]
  | elems => .splat elems

/- Canonicalisation (OCaml `canon_*`).  `canonBase` normalises unions and collapses an
   uninhabited shape to `.bot`; `canonRow` canonicalises each field and, on a splat, inlines
   `spread (shape r)` / flattens `spread (shape (splat …))` (via `collapseSpread`), then merges
   adjacent concrete spreads (`mergeAdjacent`) and collapses a lone concrete spread to a simple
   row.  Opaque `spread (rigid α)` stays put.  A simple row with a `req ⊥` field is *not*
   collapsed (per-field interpretation).  The explicit list recursions keep the block
   structurally terminating. -/
mutual
  def canonBase : Base → Base
    | .top         => .top
    | .bot         => .bot
    | .prim p      => .prim p
    | .rigid α     => .rigid α
    | .union b1 b2 => unionBase (canonBase b1) (canonBase b2)
    | .shape row   =>
      let row := canonRow row
      if isBotRow row then .bot else .shape row

  def canonRow : Row → Row
    | .simple fs u => .simple (canonFields fs) (canonBase u)
    | .splat elems =>
      let elems := mergeAdjacent (canonSplatElems elems)
      match elems with
      | [.spread b] =>
        match baseAsSimple b with
        | some s => .simple s.known s.unknown   -- a lone concrete spread collapses
        | none   => .splat elems
      | _ => .splat elems

  def canonField : FieldDesc → FieldDesc
    | .req b => .req (canonBase b)
    | .opt b => .opt (canonBase b)

  def canonFields : List (Label × FieldDesc) → List (Label × FieldDesc)
    | []           => []
    | (l, d) :: fs => (l, canonField d) :: canonFields fs

  /-- `concat_map collapseSpread`: canonicalise each element, splicing a flattened splat shape. -/
  def canonSplatElems : List SplatElem → List SplatElem
    | []      => []
    | e :: es => collapseSpread e ++ canonSplatElems es

  /-- Canonicalise one spread element: a spread of a (canonicalised) splat shape is *flattened*
      into its elements; everything else stays a single element. -/
  def collapseSpread : SplatElem → List SplatElem
    | .spread base =>
      match canonBase base with
      | .shape (.splat elems) => elems
      | b                     => [.spread b]
end

/- ---------------------------------------------------------------------------------------------- -/
/- ## 8. Well-formedness                                                                          -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The `[lower, upper]` bounds of a rigid type parameter (OCaml `Ty_param_env.bounds`). -/
structure Bounds where
  lower : Base
  upper : Base

/-- The parameter-bounds environment Γ: a finite map from each declared parameter to its bounds,
    modelled as a partial function (`none` = not in the domain).  This is the minimal slice of
    the eventual full `Env` that the well-formedness predicate needs (OCaml `Ty_param_env.t`). -/
abbrev TyParamEnv := Ty_param → Option Bounds

/-- A base is *shape-or-bottom* — the requirement on a parameter's bounds for that parameter to
    be usable at *spread* position, where its bound's row must be projectable (OCaml
    `row_of_base` is total exactly here).  This is a *shallow* check; the bound row's own
    well-formedness is discharged once, for every parameter, by `WfEnv`. -/
def ShapeOrBot (b : Base) : Prop := b = .bot ∨ ∃ r, b = .shape r

/- Structural well-formedness of a term against Γ.  The only parameter-dependent conditions are
   at the leaves: a *base-position* `rigid α` must be declared (`α ∈ dom Γ`); a *spread-position*
   `spread (rigid α)` must additionally have shape/bottom bounds (so its row is projectable).
   `spread`s of `top`/`prim`/`union` are ill-formed.  No recursion descends into a parameter's
   bounds here — that is `WfEnv`'s single pass over the domain — so this stays structural (and,
   deliberately, does not by itself rule out cyclic bounds; acyclicity is a separate hypothesis
   added where the topological machinery needs it). -/
mutual
  def WfBase (Γ : TyParamEnv) : Base → Prop
    | .top         => True
    | .bot         => True
    | .prim _      => True
    | .union a b   => WfBase Γ a ∧ WfBase Γ b
    | .shape r     => WfRow Γ r
    | .rigid α     => (Γ α).isSome = true        -- declared; any bounds (resolved by F-sub)

  def WfRow (Γ : TyParamEnv) : Row → Prop
    | .simple fs u => WfFields Γ fs ∧ WfBase Γ u
    | .splat es    => WfElems Γ es

  def WfField (Γ : TyParamEnv) : FieldDesc → Prop
    | .req b => WfBase Γ b
    | .opt b => WfBase Γ b

  def WfFields (Γ : TyParamEnv) : List (Label × FieldDesc) → Prop
    | []           => True
    | (_, d) :: fs => WfField Γ d ∧ WfFields Γ fs

  def WfElems (Γ : TyParamEnv) : List SplatElem → Prop
    | []      => True
    | e :: es => WfElem Γ e ∧ WfElems Γ es

  def WfElem (Γ : TyParamEnv) : SplatElem → Prop
    | .spread .bot         => True                -- the bottom row
    | .spread (.shape r)   => WfRow Γ r           -- an inline row (simple or splat)
    | .spread (.rigid α)   =>                     -- a rigid row parameter: bounds shape/bottom
      match Γ α with
      | some b => ShapeOrBot b.lower ∧ ShapeOrBot b.upper
      | none   => False
    | .spread .top         => False
    | .spread (.prim _)    => False
    | .spread (.union _ _) => False
end

/-- The bounds environment is well-formed: every declared parameter's `lower` and `upper` bound
    is itself a well-formed base.  A single quantification over the domain — together with the
    shallow `rigid` arms above — is what makes the per-term predicates structural while still
    transitively constraining a parameter reachable only through another's bound. -/
def WfEnv (Γ : TyParamEnv) : Prop :=
  ∀ α b, Γ α = some b → WfBase Γ b.lower ∧ WfBase Γ b.upper

end Splat
