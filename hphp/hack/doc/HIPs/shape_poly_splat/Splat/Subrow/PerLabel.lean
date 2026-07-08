import Splat.Subtyping

/-!
# The semantic subrow and its per-label form

The ground instantiation `GEnv := Ty_param → Base` (a universal instantiation of the type
parameters), the *per-label* field a row contributes under it (`rowFieldAt`, `gOf`, `evalAt`), the
`Compatible` predicate relating an instantiation to a bounds environment `Γ`, and the semantic
subrow `SemSubRow` — the ground truth the corner decision is proved against.

The model is **per-label throughout**: there is no collapse of a row to a single `SimpleRow`.  A
`SimpleRow`'s `unknown` is now opt-only (a bare `Base`), so a row that mixes a bottom contribution
with other fields — whose faithful projection is *required* at the unknown label — cannot be stored
as a `SimpleRow`.  Instead every row is evaluated *at each label* directly by `rowFieldAt`
(`FieldDesc`-valued, computing the merge via `mergeField`, with `.bot` contributing `req ⊥`), which
is exactly the implementation's `Splat.proj`.  A spread parameter's per-label field is read from the
instantiation by `gOf`; `SubBase` itself is left ground (rigids opaque), the F-sub rule recovered
later on the decision side.

The unknown slot is not named separately: it is the field at any label *fresh* to the rows involved
(`rowFieldAt_absent_eq`, in `Decide`), so quantifying over `∀ l : Label` already constrains it.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 12. Ground instantiation                                                                    -/
/- ---------------------------------------------------------------------------------------------- -/

/-- A ground instantiation: each rigid parameter to a closed base type. -/
abbrev GEnv := Ty_param → Base

/- Substitute every type parameter by its assignment in `ρ` (a closed base), structurally.  This is
   the tool the *rigid-free* reasoning uses (`RigidFreeField`, the ground fragment): a field is
   ground exactly when every substitution fixes it. -/
mutual
  def substBase (ρ : GEnv) : Base → Base
    | .top       => .top
    | .bot       => .bot
    | .prim p    => .prim p
    | .union a b => .union (substBase ρ a) (substBase ρ b)
    | .shape r   => .shape (substRow ρ r)
    | .rigid α   => ρ α

  def substRow (ρ : GEnv) : Row → Row
    | .simple fs u => .simple (substFields ρ fs) (substBase ρ u)
    | .splat es    => .splat (substElems ρ es)

  def substFields (ρ : GEnv) : List (Label × FieldDesc) → List (Label × FieldDesc)
    | []           => []
    | (l, d) :: fs => (l, substField ρ d) :: substFields ρ fs

  def substField (ρ : GEnv) : FieldDesc → FieldDesc
    | .req b => .req (substBase ρ b)
    | .opt b => .opt (substBase ρ b)

  def substElems (ρ : GEnv) : List SplatElem → List SplatElem
    | []      => []
    | e :: es => substElem ρ e :: substElems ρ es

  def substElem (ρ : GEnv) : SplatElem → SplatElem
    | .spread b => .spread (substBase ρ b)
end

/-- The row a bound denotes.  Bounds are *general* `Base` types; there is a row to read off only
    when the bound is a shape or bottom — which, for a parameter used at *spread* position, is
    exactly the well-formedness condition `ShapeOrBot` (`WfEnv`).  This is a **total** function: a
    non-shape, non-bottom bound returns a junk row (the empty row), so every lemma phrased with
    `rowOfBase` is well-defined for *any* bound, WF or not.  Bottom is the surface bottom row
    `..⊥` (a splat carrying `spread .bot`); its per-label field is `req ⊥` at every label, so the
    junk empty row (`opt ⊥`, the merge identity) and it are genuinely different.  `ShapeOrBot` is
    therefore *not* required by the intermediate lemmas here; it is supplied at the top level, where
    the junk arm must be ruled out to claim faithfulness to real subtyping. -/
def rowOfBase : Base → Row
  | .shape r => r
  | .bot     => .splat [.spread .bot]
  | _        => .simple [] .bot

/-- `substFields` preserves the key list (it rewrites descriptors, never labels). -/
theorem substFields_keys (ρ : GEnv) :
    ∀ fs : List (Label × FieldDesc), (substFields ρ fs).map Prod.fst = fs.map Prod.fst
  | []           => rfl
  | (k, d) :: tl => by
    show ((k, substField ρ d) :: substFields ρ tl).map Prod.fst = ((k, d) :: tl).map Prod.fst
    rw [List.map_cons, List.map_cons, substFields_keys ρ tl]

/-- Substitute the field descriptors of a simple row.  Keys are unchanged, so `Nodup` carries. -/
def substSimple (ρ : GEnv) (s : SimpleRow) : SimpleRow where
  known   := substFields ρ s.known
  unknown := substBase ρ s.unknown
  nodup   := by rw [substFields_keys]; exact s.nodup

/- ---------------------------------------------------------------------------------------------- -/
/- ## 13. Per-label symbolic projection                                                           -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Project a simple row at an *optional* label: `none` is the unknown slot, `some l` an ordinary
    label.  Mirrors the implementation's `Label.t option` interface, where `None` stands for every
    unmentioned label.  The unknown is opt-only (a bare `Base`), so at `none` the field is
    `.opt r.unknown`. -/
def projOpt (r : SimpleRow) : Option Label → FieldDesc
  | none   => .opt r.unknown
  | some l => proj r l

/- The field a *spread base* contributes at an optional label `l`, given a *field assignment* `g`
   that supplies each type parameter's field there.  This is the symbolic, per-label counterpart of
   the implementation's `Splat.proj`: `splatFoldAt` folds `mergeField` left-to-right over the splat
   (rightmost wins).  A spread of a parameter consults `g`; of the bottom row gives `req ⊥`; of an
   inline simple row projects it; of a splat shape recurses; the remaining non-well-formed arms give
   the merge-identity field `opt ⊥`. -/
mutual
  def splatFieldAt (g : Ty_param → FieldDesc) (l : Option Label) : List SplatElem → FieldDesc
    | []      => .opt .bot
    | e :: es => splatFoldAt g l es (elemFieldAt g l e)

  def splatFoldAt (g : Ty_param → FieldDesc) (l : Option Label) :
      List SplatElem → FieldDesc → FieldDesc
    | [],      acc => acc
    | e :: es, acc => splatFoldAt g l es (mergeField acc (elemFieldAt g l e))

  def elemFieldAt (g : Ty_param → FieldDesc) (l : Option Label) : SplatElem → FieldDesc
    | .spread b => baseFieldAt g l b

  def baseFieldAt (g : Ty_param → FieldDesc) (l : Option Label) : Base → FieldDesc
    | .rigid α              => g α
    | .bot                  => .req .bot
    | .shape (.simple fs u) => projOpt (normalize fs u) l
    | .shape (.splat es)    => splatFieldAt g l es
    | .top                  => .opt .bot
    | .prim _               => .opt .bot
    | .union _ _            => .opt .bot
end

/-- The field a row contributes at an optional label `l`, under a field assignment `g`.  A thin
    dispatcher over the mutual block above (it is not itself recursive). -/
def rowFieldAt (g : Ty_param → FieldDesc) (l : Option Label) : Row → FieldDesc
  | .simple fs u => projOpt (normalize fs u) l
  | .splat es    => splatFieldAt g l es

/-- The field assignment a ground instantiation `ρ` induces at label `l`: each parameter is sent to
    the field its assigned base contributes there.  Since `ρ α` is a closed base (no rigids), the
    inner dummy assignment `fun _ => .opt .bot` is never consulted.  This is the faithful per-label
    replacement for the old "collapse `ρ α` to a simple row and project it". -/
def gOf (ρ : GEnv) (l : Option Label) : Ty_param → FieldDesc :=
  fun α => baseFieldAt (fun _ => .opt .bot) l (ρ α)

/-- Evaluate a row under a ground instantiation, at label `l`: the per-label field with the spread
    parameters resolved by `gOf ρ l`.  The faithful replacement for the old `proj (evalRow ρ r) l`;
    the two-step "collapse then project" is now a single per-label computation. -/
def evalAt (ρ : GEnv) (l : Option Label) (r : Row) : FieldDesc := rowFieldAt (gOf ρ l) l r

/- ---------------------------------------------------------------------------------------------- -/
/- ## 14. Compatibility and the semantic subrow                                                   -/
/- ---------------------------------------------------------------------------------------------- -/

/-- An instantiation is compatible with a bounds environment when it sends every declared parameter
    *between its bounds, at every label*: the field the parameter contributes at `l` (`gOf ρ l α`)
    lies between the lower- and upper-bound rows' fields there, each evaluated under the same `ρ`
    (so coupling — bounds mentioning other parameters — and base-position rigids are already ground).

    This is stated per-label (over `∀ l : Label`) rather than as a whole-row `SubField_row`, because
    a *splat*-shape (coupled) bound has no single-row view to compare; the per-label field is the
    faithful object.  The unknown slot is captured by the labels fresh to the rows.

    NB the bounds are general `Base` types.  Reading a row out of a bound (via `rowOfBase`) is only
    faithful when the bound is a shape or bottom, i.e. under the WF condition `ShapeOrBot` for
    spread-position parameters.  That condition is *not* a hypothesis here: `rowOfBase` is total, so
    for a non-shape bound this clause is well-typed but junk.  WF is supplied at the top level. -/
def Compatible (ρ : GEnv) (Γ : TyParamEnv) : Prop :=
  ∀ α b, Γ α = some b → ∀ l : Label,
    SubField (evalAt ρ (some l) (rowOfBase b.lower)) (gOf ρ (some l) α) ∧
    SubField (gOf ρ (some l) α) (evalAt ρ (some l) (rowOfBase b.upper))

/-- The semantic subrow: `r` is a subrow of `p` under `Γ` when, for every compatible instantiation
    and every label, `r`'s per-label field is a subfield of `p`'s.  This is the ground truth the
    corner decision is later proved to decide. -/
def SemSubRow (r p : Row) (Γ : TyParamEnv) : Prop :=
  ∀ ρ, Compatible ρ Γ → ∀ l : Label, SubField (evalAt ρ (some l) r) (evalAt ρ (some l) p)

/-- `SemSubRow` is already the per-label form — the object the corner reduction consumes.  Stated as
    a lemma (definitionally `Iff.rfl`) so downstream reads `rowFieldAt (gOf ρ (some l))` directly. -/
theorem semSubRow_iff_perLabel (r p : Row) (Γ : TyParamEnv) :
    SemSubRow r p Γ ↔
      ∀ ρ, Compatible ρ Γ → ∀ l : Label,
        SubField (rowFieldAt (gOf ρ (some l)) (some l) r) (rowFieldAt (gOf ρ (some l)) (some l) p) :=
  Iff.rfl

/- ---------------------------------------------------------------------------------------------- -/
/- ## 15. Naturality of substitution over merge                                                   -/
/- ---------------------------------------------------------------------------------------------- -/

/-- `substField` is a homomorphism for `mergeField`.  `mergeField` only ever wraps its bases with
    the raw `union` constructor, through which `substBase` pushes definitionally, so every case is
    `rfl` once the right/left requiredness is split. -/
theorem substField_mergeField (ρ : GEnv) (x y : FieldDesc) :
    substField ρ (mergeField x y) = mergeField (substField ρ x) (substField ρ y) := by
  cases y with
  | req t => rfl
  | opt t =>
    cases x with
    | req u => rfl
    | opt u => rfl

/- ---------------------------------------------------------------------------------------------- -/
/- ## 16. The coupled box                                                                         -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The field interval a parameter ranges over at label `l`, under a field assignment `g` that
    resolves the *other* parameters mentioned in its bounds: the field its lower- and upper-bound
    rows contribute there.  The interval is *coupled* — it depends on `g` — which is why the corner
    enumeration must proceed in topological order.  Mirrors the implementation's `field_bounds`. -/
def fieldBoundsAt (Γ : TyParamEnv) (g : Ty_param → FieldDesc) (l : Option Label)
    (α : Ty_param) : FieldDesc × FieldDesc :=
  match Γ α with
  | some b => (rowFieldAt g l (rowOfBase b.lower), rowFieldAt g l (rowOfBase b.upper))
  | none   => (.req .bot, .opt .top)

/-- A field descriptor lies in the box `[lo, hi]` when `lo` refines to it and it refines to `hi`. -/
def InBox (lo hi fd : FieldDesc) : Prop := SubField lo fd ∧ SubField fd hi

/-- A label outside a given finite list — used to read off the `unknown` sub-field of a row, which
    is what every unmentioned label projects to.  Labels are an infinite type, so a finite key list
    always leaves one free. -/
theorem exists_fresh_label (ks : List Label) : ∃ l : Label, l ∉ ks := by
  obtain ⟨l, hl⟩ := Infinite.exists_notMem_finset ks.toFinset
  exact ⟨l, fun h => hl (List.mem_toFinset.mpr h)⟩

/-- **Compatible instantiations land in the box.**  Under a compatible instantiation, the field a
    parameter contributes at `l` (`gOf ρ (some l) α`) lies between the bound rows' fields there.
    This is exactly `Compatible` read at `l` — the per-label model makes it immediate (no bridge,
    no closedness, no groundness hypothesis). -/
theorem inBox_of_compatible (ρ : GEnv) (Γ : TyParamEnv) (hc : Compatible ρ Γ)
    (l : Label) (α : Ty_param) (b : Bounds) (hb : Γ α = some b) :
    InBox (rowFieldAt (gOf ρ (some l)) (some l) (rowOfBase b.lower))
          (rowFieldAt (gOf ρ (some l)) (some l) (rowOfBase b.upper)) (gOf ρ (some l) α) :=
  hc α b hb l

/- ---------------------------------------------------------------------------------------------- -/
/- ## 17. Phase 1: base-rigid-free rows                                                           -/
/- ---------------------------------------------------------------------------------------------- -/

/-- A field is rigid-free when no substitution touches it (its base mentions no parameter). -/
def RigidFreeField (fd : FieldDesc) : Prop := ∀ ρ', substField ρ' fd = fd

/-- A base is rigid-free when no substitution touches it.  A simple row's opt-only `unknown` is a
    base, so a `Ground` inline row now needs its `unknown` rigid-free at this level. -/
def RigidFreeBase (b : Base) : Prop := ∀ ρ', substBase ρ' b = b

/-- `.opt` of a rigid-free base is a rigid-free field. -/
theorem rigidFreeField_opt {b : Base} (h : RigidFreeBase b) : RigidFreeField (.opt b) :=
  fun ρ' => by show FieldDesc.opt (substBase ρ' b) = .opt b; rw [h ρ']

/-- `.req` of a rigid-free base is a rigid-free field. -/
theorem rigidFreeField_req {b : Base} (h : RigidFreeBase b) : RigidFreeField (.req b) :=
  fun ρ' => by show FieldDesc.req (substBase ρ' b) = .req b; rw [h ρ']

/- The phase-1 restriction: rigids occur only at *spread* position, never inside a field's base.
   A row is `Ground` when its inline field descriptors (and opt-only `unknown` base) are rigid-free;
   a spread element may be a spread parameter (`.rigid`), the bottom row, or a shape of a further
   `Ground` row. -/
mutual
  def GroundRow : Row → Prop
    | .simple fs u => GroundFields fs ∧ RigidFreeBase u
    | .splat es    => GroundElems es
  def GroundFields : List (Label × FieldDesc) → Prop
    | []           => True
    | (_, d) :: fs => RigidFreeField d ∧ GroundFields fs
  def GroundElems : List SplatElem → Prop
    | []      => True
    | e :: es => GroundElem e ∧ GroundElems es
  def GroundElem : SplatElem → Prop
    | .spread b => GroundSpread b
  def GroundSpread : Base → Prop
    | .shape r => GroundRow r
    | _        => True   -- a spread parameter / bottom (junk top/prim/union excluded by WF)
end

/-- Substitution fixes a `Ground` field list. -/
theorem substFields_fixed (ρ : GEnv) : ∀ {fs}, GroundFields fs → substFields ρ fs = fs
  | [],          _ => rfl
  | (l, d) :: fs, h => by
    show (l, substField ρ d) :: substFields ρ fs = (l, d) :: fs
    rw [substFields_fixed ρ h.2, h.1 ρ]

end Splat
