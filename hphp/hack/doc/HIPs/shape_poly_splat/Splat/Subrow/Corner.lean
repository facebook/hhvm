import Splat.Subrow.PerLabel

/-!
# The combinator corner reduction

The heart of the per-label corner reduction: as a function of *one* type parameter's field, a
row's per-label projection has a restricted shape — a pair of independent input-requiredness
branches, each a *linear* base function `b ↦ (req/opt) (pre ∪ b)` (or a constant `pre`).  This is
the `FieldContrib` combinator.  For such a shape the per-label goal `SubField (φ.apply d) (ψ.apply d)`
over `d`'s whole field box `[lo, hi]` is determined by the two *base* endpoints `d.setBase lo.base`
and `d.setBase hi.base` (`FieldContrib.corner_reduction`) — keeping `d`'s requiredness fixed and moving the
base from `lo.base` to `hi.base`.  Quantifying the requiredness corners on top (in the eventual
enumeration) yields the ≤4 `Field_desc.corners`.

`FieldContrib` keeps the two requiredness branches *independent* (unlike a single affine field function,
whose `req`-input branch would force a `req` output): the corner reduction is then purely
base-level and collapses, per branch, to plain monotonicity plus a `∪`-cancellation
(`BaseContrib.baseAt_corner`).  This file proves the combinator and its corner reduction; connecting an
actual `rowFieldAt` to a `FieldContrib` (the *extraction*) is the next section.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 21. Field/base helpers                                                                      -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The base (type / upper bound) carried by a field descriptor. -/
def FieldDesc.base : FieldDesc → Base
  | .req b => b
  | .opt b => b

/-- Whether a field descriptor is required. -/
def FieldDesc.isReq : FieldDesc → Bool
  | .req _ => true
  | .opt _ => false

/-- Replace a field's base, keeping its requiredness (used to name the box's base-corner fields
    `d.setBase lo.base` / `d.setBase hi.base`). -/
def FieldDesc.setBase : FieldDesc → Base → FieldDesc
  | .req _, b => .req b
  | .opt _, b => .opt b

/-- Setting a field's base yields a `req`/`opt` of that base (the requiredness survives). -/
theorem FieldDesc.setBase_cases (d : FieldDesc) (b : Base) :
    d.setBase b = .req b ∨ d.setBase b = .opt b := by
  cases d with
  | req _ => exact Or.inl rfl
  | opt _ => exact Or.inr rfl

/-- A subfield forces a subtype on the underlying bases. -/
theorem subField_base {d e : FieldDesc} (h : SubField d e) : SubBase d.base e.base := by
  cases h with
  | req_req hb => exact hb
  | opt_opt hb => exact hb
  | req_opt hb => exact hb

/- ---------------------------------------------------------------------------------------------- -/
/- ## 22. The `FieldContrib` combinator and its corner reduction                                  -/
/- ---------------------------------------------------------------------------------------------- -/

/-- A **base contribution**: one input-requiredness branch of a `FieldContrib`.  Sends an input base
    `b` to a field of FIXED requiredness `req` whose base is `pre ∪ b` (when `usesInput`) or the
    constant `pre`. -/
structure BaseContrib where
  req : Bool       -- output requiredness (true = req)
  pre : Base     -- prefix base
  usesInput : Bool   -- whether the output base unions in the input base

/-- The output base of a `BaseContrib` at input base `b`. -/
def BaseContrib.baseAt (f : BaseContrib) (b : Base) : Base :=
  match f.usesInput with
  | true  => .union f.pre b
  | false => f.pre

/-- Evaluate a `BaseContrib` at an input base. -/
def BaseContrib.apply (f : BaseContrib) (b : Base) : FieldDesc :=
  match f.req with
  | true  => .req (f.baseAt b)
  | false => .opt (f.baseAt b)

/-- The base read off a `BaseContrib`'s output is its `baseAt`. -/
theorem BaseContrib.apply_base (f : BaseContrib) (b : Base) : (f.apply b).base = f.baseAt b := by
  obtain ⟨q, pre, ut⟩ := f
  cases q <;> rfl

/-- An `BaseContrib` is monotone in its input base. -/
theorem BaseContrib.mono (f : BaseContrib) {b b' : Base} (h : SubBase b b') :
    SubField (f.apply b) (f.apply b') := by
  obtain ⟨q, pre, ut⟩ := f
  -- the output bases are related: constant (`usesInput = false`) or monotone in the input (`true`)
  have hb : SubBase (BaseContrib.baseAt ⟨q, pre, ut⟩ b) (BaseContrib.baseAt ⟨q, pre, ut⟩ b') := by
    cases ut with
    | false => exact SubBase.refl                                  -- both bases are `pre`
    | true  =>                                                     -- `pre ∪ b <: pre ∪ b'`
      exact SubBase.union_sub SubBase.union_left (SubBase.trans h SubBase.union_right)
  -- lift the base relation to a subfield, at the branch's fixed requiredness
  cases q with
  | false => exact SubField.opt_opt hb
  | true  => exact SubField.req_req hb

/-- **Base-level corner core.**  The affine-or-constant base function `baseAt` over an interval is
    determined by its endpoints: when both sides union in the input `b`, the `b` cancels, leaving
    plain monotonicity (no genuine cartesian corner is needed). -/
theorem BaseContrib.baseAt_corner (f h : BaseContrib) {lo b hi : Base}
    (hlo : SubBase lo b) (hhi : SubBase b hi)
    (Hlo : SubBase (f.baseAt lo) (h.baseAt lo)) (Hhi : SubBase (f.baseAt hi) (h.baseAt hi)) :
    SubBase (f.baseAt b) (h.baseAt b) := by
  obtain ⟨qf, pf, uf⟩ := f
  obtain ⟨qh, ph, uh⟩ := h
  cases uf <;> cases uh <;> simp only [BaseContrib.baseAt] at Hlo Hhi ⊢
  · -- both constant: the goal is `pf <: ph`, exactly `Hlo`
    exact Hlo
  · -- `f` constant, `h` affine: `pf <: ph ∪ lo` weakens to `pf <: ph ∪ b` (since `lo <: b`)
    exact SubBase.trans Hlo
      (SubBase.union_sub SubBase.union_left (SubBase.trans hlo SubBase.union_right))
  · -- `f` affine, `h` constant: `pf ∪ b <: pf ∪ hi <: ph` (since `b <: hi`)
    exact SubBase.trans
      (SubBase.union_sub SubBase.union_left (SubBase.trans hhi SubBase.union_right)) Hhi
  · -- both affine: `pf ∪ b <: ph ∪ b`, via `pf <: pf ∪ lo <: ph ∪ lo <: ph ∪ b` and `b <: ph ∪ b`
    refine SubBase.union_sub ?_ SubBase.union_right
    -- `pf <: ph ∪ b`, threading `Hlo : pf ∪ lo <: ph ∪ lo`
    have h1 : SubBase pf (Base.union pf lo) := SubBase.union_left
    have h2 : SubBase (Base.union ph lo) (Base.union ph b) :=
      SubBase.union_sub SubBase.union_left (SubBase.trans hlo SubBase.union_right)
    exact SubBase.trans h1 (SubBase.trans Hlo h2)

/-- **`BaseContrib` corner reduction.**  Within one requiredness branch, the per-label goal over an
    input-base interval `[lo, hi]` follows from the two endpoints. -/
theorem BaseContrib.corner_reduction (f h : BaseContrib) {lo b hi : Base}
    (hlo : SubBase lo b) (hhi : SubBase b hi)
    (Hlo : SubField (f.apply lo) (h.apply lo))
    (Hhi : SubField (f.apply hi) (h.apply hi)) :
    SubField (f.apply b) (h.apply b) := by
  -- reduce the subfield goal to a base goal, then corner-reduce the bases
  have hbase : SubBase (f.baseAt b) (h.baseAt b) :=
    BaseContrib.baseAt_corner f h hlo hhi
      (by rw [← f.apply_base lo, ← h.apply_base lo]; exact subField_base Hlo)
      (by rw [← f.apply_base hi, ← h.apply_base hi]; exact subField_base Hhi)
  obtain ⟨qf, pf, uf⟩ := f
  obtain ⟨qh, ph, uh⟩ := h
  cases qf <;> cases qh <;> simp only [BaseContrib.apply] at Hlo ⊢
  · exact SubField.opt_opt hbase            -- opt/opt
  · cases Hlo                               -- opt/req: `Hlo` is an impossible `opt <: req`
  · exact SubField.req_opt hbase            -- req/opt
  · exact SubField.req_req hbase            -- req/req

/-- A **field contribution**: the field a parameter contributes at a label, as a function of its
    field — independent `req`- and `opt`-input branches.  In particular it can
    output a FIXED requiredness regardless of input requiredness — which the coupled peel produces
    when a parameter is pinned inside another parameter's box. -/
structure FieldContrib where
  onReq : BaseContrib   -- applied when the input field is `req b`
  onOpt : BaseContrib   -- applied when the input field is `opt b`

/-- Apply a `FieldContrib` to an input field, dispatching on its requiredness. -/
def FieldContrib.apply (g : FieldContrib) : FieldDesc → FieldDesc
  | .req b => g.onReq.apply b
  | .opt b => g.onOpt.apply b

/-- **`FieldContrib` corner reduction.**  With the other parameters fixed, a parameter's field `d` confined
    to `[lo, hi]` satisfies the per-label goal as soon as the two base-corners
    `d.setBase lo.base` and `d.setBase hi.base` do.  Reduces, per requiredness branch, to
    `BaseContrib.corner_reduction`. -/
theorem FieldContrib.corner_reduction {φ ψ : FieldContrib} {lo hi d : FieldDesc}
    (hlo : SubField lo d) (hhi : SubField d hi)
    (Hlo : SubField (φ.apply (d.setBase lo.base)) (ψ.apply (d.setBase lo.base)))
    (Hhi : SubField (φ.apply (d.setBase hi.base)) (ψ.apply (d.setBase hi.base))) :
    SubField (φ.apply d) (ψ.apply d) := by
  cases d with
  | req dt =>
    -- `d.setBase _ = .req _`, so both corners dispatch to the `onReq` branch
    simp only [FieldDesc.setBase, FieldContrib.apply] at Hlo Hhi ⊢
    exact BaseContrib.corner_reduction φ.onReq ψ.onReq (subField_base hlo) (subField_base hhi) Hlo Hhi
  | opt dt =>
    -- `d.setBase _ = .opt _`, so both corners dispatch to the `onOpt` branch
    simp only [FieldDesc.setBase, FieldContrib.apply] at Hlo Hhi ⊢
    exact BaseContrib.corner_reduction φ.onOpt ψ.onOpt (subField_base hlo) (subField_base hhi) Hlo Hhi

/- ---------------------------------------------------------------------------------------------- -/
/- ## 23. Field equivalence and the `FieldContrib` merge algebra                                  -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Field equivalence:** mutual subfielding — the same field up to `∪`-reassociation.  The fold
    that builds a `FieldContrib` reassociates the unions `mergeField` introduces, so the extraction is
    stated up to `FEquiv` rather than on the nose. -/
def FEquiv (a b : FieldDesc) : Prop := SubField a b ∧ SubField b a

theorem FEquiv.refl (a : FieldDesc) : FEquiv a a := ⟨subField_refl a, subField_refl a⟩

theorem FEquiv.symm {a b : FieldDesc} (h : FEquiv a b) : FEquiv b a := ⟨h.2, h.1⟩

theorem FEquiv.trans {a b c : FieldDesc} (h1 : FEquiv a b) (h2 : FEquiv b c) : FEquiv a c :=
  ⟨subField_trans h1.1 h2.1, subField_trans h2.2 h1.2⟩

/-- Transport a subfield across `FEquiv` on both ends. -/
theorem SubField.of_fequiv {a a' b b' : FieldDesc} (ha : FEquiv a' a) (hb : FEquiv b' b)
    (h : SubField a b) : SubField a' b' :=
  subField_trans ha.1 (subField_trans h hb.2)

/-- `mergeField` respects `FEquiv` (it is monotone both ways). -/
theorem FEquiv.mergeField {a a' b b' : FieldDesc} (ha : FEquiv a a') (hb : FEquiv b b') :
    FEquiv (mergeField a b) (mergeField a' b') :=
  ⟨mergeField_mono ha.1 hb.1, mergeField_mono ha.2 hb.2⟩

/-- Left-associated `∪` refines into right-associated. -/
theorem subBase_union_assoc_le {a b c : Base} :
    SubBase (.union (.union a b) c) (.union a (.union b c)) :=
  -- `(a∪b)∪c <: a∪(b∪c)`: send `a∪b` left and `c` into the right component
  SubBase.union_sub
    (SubBase.union_sub SubBase.union_left (SubBase.trans SubBase.union_left SubBase.union_right))
    (SubBase.trans SubBase.union_right SubBase.union_right)

/-- Right-associated `∪` refines into left-associated. -/
theorem subBase_union_assoc_ge {a b c : Base} :
    SubBase (.union a (.union b c)) (.union (.union a b) c) :=
  -- `a∪(b∪c) <: (a∪b)∪c`: send `a` and `b` into the left component, `c` to the right
  SubBase.union_sub
    (SubBase.trans SubBase.union_left SubBase.union_left)
    (SubBase.union_sub (SubBase.trans SubBase.union_right SubBase.union_left) SubBase.union_right)

/-- A **base contribution**'s output base under a `mergeField`-style merge: the prefix
    `pre₁ ∪ pre₂` unioned with the input (whenever either side uses it), refining *into* the
    `∪` of the two branch outputs.  (`→` direction.) -/
theorem BaseContrib.baseAt_merge_le (g1 g2 : BaseContrib) (b : Base) :
    SubBase (BaseContrib.baseAt ⟨true, .union g1.pre g2.pre, g1.usesInput || g2.usesInput⟩ b)
            (.union (g1.baseAt b) (g2.baseAt b)) := by
  obtain ⟨_, p1, u1⟩ := g1
  obtain ⟨_, p2, u2⟩ := g2
  cases u1 <;> cases u2 <;>
    simp only [BaseContrib.baseAt, Bool.or_false, Bool.or_true]
  · -- neither uses the input: `p1∪p2 <: p1∪p2`
    exact SubBase.refl
  · -- only `g2`: `(p1∪p2)∪b <: p1∪(p2∪b)`
    exact subBase_union_assoc_le
  · -- only `g1`: `(p1∪p2)∪b <: (p1∪b)∪p2`
    exact SubBase.union_sub
      (subBase_union_mono SubBase.union_left SubBase.refl)
      (SubBase.trans SubBase.union_right SubBase.union_left)
  · -- both: `(p1∪p2)∪b <: (p1∪b)∪(p2∪b)` (the input `b` duplicates)
    exact SubBase.union_sub
      (subBase_union_mono SubBase.union_left SubBase.union_left)
      (SubBase.trans SubBase.union_right SubBase.union_left)

/-- The `←` direction of `BaseContrib.baseAt_merge_le`. -/
theorem BaseContrib.baseAt_merge_ge (g1 g2 : BaseContrib) (b : Base) :
    SubBase (.union (g1.baseAt b) (g2.baseAt b))
            (BaseContrib.baseAt ⟨true, .union g1.pre g2.pre, g1.usesInput || g2.usesInput⟩ b) := by
  obtain ⟨_, p1, u1⟩ := g1
  obtain ⟨_, p2, u2⟩ := g2
  cases u1 <;> cases u2 <;>
    simp only [BaseContrib.baseAt, Bool.or_false, Bool.or_true]
  · -- neither uses the input
    exact SubBase.refl
  · -- only `g2`: `p1∪(p2∪b) <: (p1∪p2)∪b`
    exact subBase_union_assoc_ge
  · -- only `g1`: `(p1∪b)∪p2 <: (p1∪p2)∪b`
    exact SubBase.union_sub
      (subBase_union_mono SubBase.union_left SubBase.refl)
      (SubBase.trans SubBase.union_right SubBase.union_left)
  · -- both: `(p1∪b)∪(p2∪b) <: (p1∪p2)∪b`
    exact SubBase.union_sub
      (subBase_union_mono SubBase.union_left SubBase.refl)
      (subBase_union_mono SubBase.union_right SubBase.refl)

/-- Merge two `BaseContrib` branches (rightmost-wins, mirroring `mergeField`): a `req` right output
    overrides; otherwise the left requiredness is kept and the bases union. -/
def BaseContrib.merge (f1 f2 : BaseContrib) : BaseContrib :=
  match f2.req with
  | true  => ⟨true, f2.pre, f2.usesInput⟩
  | false => ⟨f1.req, .union f1.pre f2.pre, f1.usesInput || f2.usesInput⟩

/-- `BaseContrib.merge` computes `mergeField` of the two branch evaluations, up to `FEquiv`. -/
theorem BaseContrib.merge_apply (f1 f2 : BaseContrib) (b : Base) :
    FEquiv ((f1.merge f2).apply b) (mergeField (f1.apply b) (f2.apply b)) := by
  obtain ⟨q1, p1, u1⟩ := f1
  obtain ⟨q2, p2, u2⟩ := f2
  cases q2 with
  | true =>
    -- right branch is `req`: `mergeField _ (.req _) = .req _`, exactly the merge's output
    exact FEquiv.refl _
  | false =>
    cases q1 with
    | false =>
      -- both `opt`: the merged base equals the `∪` of the two branch bases (both ways)
      exact ⟨SubField.opt_opt (BaseContrib.baseAt_merge_le ⟨false, p1, u1⟩ ⟨false, p2, u2⟩ b),
             SubField.opt_opt (BaseContrib.baseAt_merge_ge ⟨false, p1, u1⟩ ⟨false, p2, u2⟩ b)⟩
    | true =>
      -- left `req`, right `opt`: merged is `req`, same base-`∪` reasoning
      exact ⟨SubField.req_req (BaseContrib.baseAt_merge_le ⟨true, p1, u1⟩ ⟨false, p2, u2⟩ b),
             SubField.req_req (BaseContrib.baseAt_merge_ge ⟨true, p1, u1⟩ ⟨false, p2, u2⟩ b)⟩

/-- Merge two `FieldContrib`s per requiredness branch. -/
def FieldContrib.merge (g1 g2 : FieldContrib) : FieldContrib :=
  ⟨g1.onReq.merge g2.onReq, g1.onOpt.merge g2.onOpt⟩

/-- `FieldContrib.merge` computes `mergeField` of the two applications, up to `FEquiv`. -/
theorem FieldContrib.merge_apply (g1 g2 : FieldContrib) (d : FieldDesc) :
    FEquiv ((g1.merge g2).apply d) (mergeField (g1.apply d) (g2.apply d)) := by
  cases d with
  | req t => exact BaseContrib.merge_apply g1.onReq g2.onReq t
  | opt t => exact BaseContrib.merge_apply g1.onOpt g2.onOpt t

/-- The constant `FieldContrib` (ignores its input field). -/
def constFieldContrib (c : FieldDesc) : FieldContrib := ⟨⟨c.isReq, c.base, false⟩, ⟨c.isReq, c.base, false⟩⟩

@[simp] theorem constFieldContrib_apply (c d : FieldDesc) : (constFieldContrib c).apply d = c := by
  cases c <;> cases d <;> rfl

/- ---------------------------------------------------------------------------------------------- -/
/- ## 24. Extraction: a row's per-label field is a `FieldContrib` of one parameter's field          -/
/- ---------------------------------------------------------------------------------------------- -/

/- The `FieldContrib`-valued counterpart of the frame's `baseFieldAt`/`splatFieldAt` block (§14): given a
   *symbolic assignment* `sa : Ty_param → FieldContrib` of a `FieldContrib` to each parameter, the per-label field
   of a row — viewed as a function of the *peeled* parameter's field `d`, with every parameter `α`
   resolved to `(sa α).apply d` — is captured by a single `FieldContrib`.  Mirrors the frame's recursion
   exactly: a spread of a parameter consults `sa`; of the bottom row gives the constant `req ⊥`; of
   an inline simple row a constant projection; of a *nested* splat shape recurses; the remaining
   arms are constant merge-identity fields. -/
mutual
  def splatFieldContrib (sa : Ty_param → FieldContrib) (l : Option Label) : List SplatElem → FieldContrib
    | []      => constFieldContrib (.opt .bot)
    | e :: es => splatFoldFieldContrib sa l es (elemFieldContrib sa l e)

  def splatFoldFieldContrib (sa : Ty_param → FieldContrib) (l : Option Label) : List SplatElem → FieldContrib → FieldContrib
    | [],      acc => acc
    | e :: es, acc => splatFoldFieldContrib sa l es (acc.merge (elemFieldContrib sa l e))

  def elemFieldContrib (sa : Ty_param → FieldContrib) (l : Option Label) : SplatElem → FieldContrib
    | .spread b => baseFieldContrib sa l b

  def baseFieldContrib (sa : Ty_param → FieldContrib) (l : Option Label) : Base → FieldContrib
    | .rigid α              => sa α
    | .bot                  => constFieldContrib (.req .bot)
    | .shape (.simple fs u) => constFieldContrib (projOpt (normalize fs u) l)
    | .shape (.splat es)    => splatFieldContrib sa l es
    | .top                  => constFieldContrib (.opt .bot)
    | .prim _               => constFieldContrib (.opt .bot)
    | .union _ _            => constFieldContrib (.opt .bot)
end

/-- The `FieldContrib` of a whole row under a symbolic assignment. -/
def rowFieldContrib (sa : Ty_param → FieldContrib) (l : Option Label) : Row → FieldContrib
  | .simple fs u => constFieldContrib (projOpt (normalize fs u) l)
  | .splat es    => splatFieldContrib sa l es

/- The extraction, by mutual induction over the splat structure — the same shape as the bridge
   (§17), carrying `FEquiv` (rather than equality) because the fold reassociates the `∪`s
   `mergeField` introduces.  Each leaf is `constFieldContrib` (a constant field, `constFieldContrib_apply`), the
   parameter arm is `sa α` (refl), and the fold step is `FieldContrib.merge_apply` + `FEquiv.mergeField`. -/
mutual
  theorem baseFieldContrib_apply (sa : Ty_param → FieldContrib) (l : Option Label) (d : FieldDesc) :
      ∀ b : Base, FEquiv (baseFieldAt (fun α => (sa α).apply d) l b) ((baseFieldContrib sa l b).apply d)
    | .rigid _              => FEquiv.refl _    -- both sides are `(sa α).apply d`
    | .bot                  => by simp only [baseFieldAt, baseFieldContrib, constFieldContrib_apply]; exact FEquiv.refl _
    | .top                  => by simp only [baseFieldAt, baseFieldContrib, constFieldContrib_apply]; exact FEquiv.refl _
    | .prim _               => by simp only [baseFieldAt, baseFieldContrib, constFieldContrib_apply]; exact FEquiv.refl _
    | .shape (.simple _ _)  => by simp only [baseFieldAt, baseFieldContrib, constFieldContrib_apply]; exact FEquiv.refl _
    | .union _ _            => by simp only [baseFieldAt, baseFieldContrib, constFieldContrib_apply]; exact FEquiv.refl _
    | .shape (.splat es)    => splatFieldContrib_apply sa l d es

  theorem elemFieldContrib_apply (sa : Ty_param → FieldContrib) (l : Option Label) (d : FieldDesc) :
      ∀ e : SplatElem, FEquiv (elemFieldAt (fun α => (sa α).apply d) l e) ((elemFieldContrib sa l e).apply d)
    | .spread b => baseFieldContrib_apply sa l d b

  theorem splatFieldContrib_apply (sa : Ty_param → FieldContrib) (l : Option Label) (d : FieldDesc) :
      ∀ es : List SplatElem,
        FEquiv (splatFieldAt (fun α => (sa α).apply d) l es) ((splatFieldContrib sa l es).apply d)
    | []      => by simp only [splatFieldContrib, constFieldContrib_apply]; exact FEquiv.refl _
    | e :: es =>
        splatFoldFieldContrib_apply sa l d es (elemFieldContrib sa l e)
          (elemFieldAt (fun α => (sa α).apply d) l e) (elemFieldContrib_apply sa l d e)

  theorem splatFoldFieldContrib_apply (sa : Ty_param → FieldContrib) (l : Option Label) (d : FieldDesc) :
      ∀ (es : List SplatElem) (acc : FieldContrib) (accF : FieldDesc),
        FEquiv accF (acc.apply d) →
        FEquiv (splatFoldAt (fun α => (sa α).apply d) l es accF) ((splatFoldFieldContrib sa l es acc).apply d)
    | [],      _,   _,    hinv => hinv
    | e :: es, acc, accF, hinv =>
        splatFoldFieldContrib_apply sa l d es (acc.merge (elemFieldContrib sa l e))
          (mergeField accF (elemFieldAt (fun α => (sa α).apply d) l e))
          (FEquiv.trans (FEquiv.mergeField hinv (elemFieldContrib_apply sa l d e))
            (FieldContrib.merge_apply acc (elemFieldContrib sa l e) d).symm)
end

/-- **Extraction (rows).**  `rowFieldAt`, as a function of one parameter's field `d` (every
    parameter `α` resolved to `(sa α).apply d`), is a single `FieldContrib` applied to `d`, up to `FEquiv`. -/
theorem rowFieldAt_extract (sa : Ty_param → FieldContrib) (l : Option Label) (d : FieldDesc) :
    ∀ r : Row, FEquiv (rowFieldAt (fun α => (sa α).apply d) l r) ((rowFieldContrib sa l r).apply d)
  | .simple _ _ => by simp only [rowFieldAt, rowFieldContrib, constFieldContrib_apply]; exact FEquiv.refl _
  | .splat es   => splatFieldContrib_apply sa l d es

/- ---------------------------------------------------------------------------------------------- -/
/- ## 25. The coupled single-parameter peel                                                       -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Coupled single-parameter peel.**  With the peeled parameter's field `d` flowing through a
    symbolic assignment `sa` (so `d` may occur both in the rows *and* inside other parameters'
    assigned values), the per-label goal over `d`'s box `[lo, hi]` follows from the two
    base-corners.  The extraction turns each side into a single `FieldContrib`; `FieldContrib.corner_reduction`
    (fixed-requiredness branches) finishes. -/
theorem peel_step_contrib (sa : Ty_param → FieldContrib) (l : Option Label) (r p : Row)
    {lo hi d : FieldDesc} (hlo : SubField lo d) (hhi : SubField d hi)
    (Hlo : SubField (rowFieldAt (fun α => (sa α).apply (d.setBase lo.base)) l r)
                    (rowFieldAt (fun α => (sa α).apply (d.setBase lo.base)) l p))
    (Hhi : SubField (rowFieldAt (fun α => (sa α).apply (d.setBase hi.base)) l r)
                    (rowFieldAt (fun α => (sa α).apply (d.setBase hi.base)) l p)) :
    SubField (rowFieldAt (fun α => (sa α).apply d) l r)
             (rowFieldAt (fun α => (sa α).apply d) l p) := by
  -- restate both corner subfield goals on the extracted `FieldContrib`s
  have HloP := SubField.of_fequiv (rowFieldAt_extract sa l (d.setBase lo.base) r).symm
    (rowFieldAt_extract sa l (d.setBase lo.base) p).symm Hlo
  have HhiP := SubField.of_fequiv (rowFieldAt_extract sa l (d.setBase hi.base) r).symm
    (rowFieldAt_extract sa l (d.setBase hi.base) p).symm Hhi
  -- corner-reduce the combinators, then transport back to `rowFieldAt`
  exact SubField.of_fequiv (rowFieldAt_extract sa l d r) (rowFieldAt_extract sa l d p)
    (FieldContrib.corner_reduction hlo hhi HloP HhiP)

/- ---------------------------------------------------------------------------------------------- -/
/- ## 26. Monotonicity of symbolic projection, and the identity combinator                        -/
/- ---------------------------------------------------------------------------------------------- -/

/- The per-label projection is monotone in the field assignment — the engine that makes the whole
   corner machinery monotone in each parameter's field.  By mutual induction over the splat
   structure (the frame's §14 block): a parameter contributes `g α` (hypothesis `h`); every other
   leaf is `g`-independent (reflexivity); the fold is monotone by `mergeField_mono`. -/
mutual
  theorem baseFieldAt_mono {g g' : Ty_param → FieldDesc} (h : ∀ α, SubField (g α) (g' α))
      (l : Option Label) :
      ∀ b : Base, SubField (baseFieldAt g l b) (baseFieldAt g' l b)
    | .rigid α              => h α
    | .bot                  => subField_refl _
    | .top                  => subField_refl _
    | .prim _               => subField_refl _
    | .union _ _            => subField_refl _
    | .shape (.simple _ _)  => subField_refl _
    | .shape (.splat es)    => splatFieldAt_mono h l es

  theorem elemFieldAt_mono {g g' : Ty_param → FieldDesc} (h : ∀ α, SubField (g α) (g' α))
      (l : Option Label) :
      ∀ e : SplatElem, SubField (elemFieldAt g l e) (elemFieldAt g' l e)
    | .spread b => baseFieldAt_mono h l b

  theorem splatFieldAt_mono {g g' : Ty_param → FieldDesc} (h : ∀ α, SubField (g α) (g' α))
      (l : Option Label) :
      ∀ es : List SplatElem, SubField (splatFieldAt g l es) (splatFieldAt g' l es)
    | []      => subField_refl _
    | e :: es =>
        splatFoldAt_mono h l es (elemFieldAt g l e) (elemFieldAt g' l e) (elemFieldAt_mono h l e)

  theorem splatFoldAt_mono {g g' : Ty_param → FieldDesc} (h : ∀ α, SubField (g α) (g' α))
      (l : Option Label) :
      ∀ (es : List SplatElem) (acc acc' : FieldDesc), SubField acc acc' →
        SubField (splatFoldAt g l es acc) (splatFoldAt g' l es acc')
    | [],      _,   _,    hacc => hacc
    | e :: es, acc, acc', hacc =>
        splatFoldAt_mono h l es (mergeField acc (elemFieldAt g l e))
          (mergeField acc' (elemFieldAt g' l e)) (mergeField_mono hacc (elemFieldAt_mono h l e))
end

/-- **Symbolic projection is monotone in the assignment.**  A row's per-label field grows
    monotonically as each parameter's field grows. -/
theorem rowFieldAt_mono {g g' : Ty_param → FieldDesc} (h : ∀ α, SubField (g α) (g' α))
    (l : Option Label) : ∀ r : Row, SubField (rowFieldAt g l r) (rowFieldAt g' l r)
  | .simple _ _ => subField_refl _
  | .splat es   => splatFieldAt_mono h l es

/-- **Symbolic projection respects `FEquiv` of the assignment** — both monotonicity directions. -/
theorem rowFieldAt_fequiv {g g' : Ty_param → FieldDesc} (h : ∀ α, FEquiv (g α) (g' α))
    (l : Option Label) (r : Row) : FEquiv (rowFieldAt g l r) (rowFieldAt g' l r) :=
  ⟨rowFieldAt_mono (fun α => (h α).1) l r, rowFieldAt_mono (fun α => (h α).2) l r⟩

/-- The **identity combinator**: `idFieldContrib.apply d ~ d`.  Sends a parameter to its own field (up to the
    `bot ∪ ·` the affine branch introduces), so a symbolic peel of one parameter collapses to a
    plain assignment update at that parameter. -/
def idFieldContrib : FieldContrib := ⟨⟨true, .bot, true⟩, ⟨false, .bot, true⟩⟩

theorem idFieldContrib_apply (d : FieldDesc) : FEquiv (idFieldContrib.apply d) d := by
  cases d with
  | req t =>
    -- `idFieldContrib.apply (.req t) = .req (⊥ ∪ t) ~ .req t`
    exact ⟨SubField.req_req (SubBase.union_sub SubBase.bot SubBase.refl),
           SubField.req_req SubBase.union_right⟩
  | opt t =>
    -- `idFieldContrib.apply (.opt t) = .opt (⊥ ∪ t) ~ .opt t`
    exact ⟨SubField.opt_opt (SubBase.union_sub SubBase.bot SubBase.refl),
           SubField.opt_opt SubBase.union_right⟩

/- ---------------------------------------------------------------------------------------------- -/
/- ## 27. Peeling a plain assignment                                                              -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Single-parameter peel (plain assignment).**  At label `l`, the per-label goal under an
    assignment `g` follows from the goal at `g` with parameter `T`'s field's base set to the interval
    endpoints `lo.base` / `hi.base` (keeping `T`'s requiredness).  Derived from the coupled
    `peel_step_contrib` by taking the symbolic assignment that sends `T` to the identity combinator and
    every other parameter to its constant `g`-value. -/
theorem peel_step (g : Ty_param → FieldDesc) (l : Option Label) (r p : Row) (T : Ty_param)
    {lo hi : FieldDesc} (hlo : SubField lo (g T)) (hhi : SubField (g T) hi)
    (Hlo : SubField (rowFieldAt (Function.update g T ((g T).setBase lo.base)) l r)
                    (rowFieldAt (Function.update g T ((g T).setBase lo.base)) l p))
    (Hhi : SubField (rowFieldAt (Function.update g T ((g T).setBase hi.base)) l r)
                    (rowFieldAt (Function.update g T ((g T).setBase hi.base)) l p)) :
    SubField (rowFieldAt g l r) (rowFieldAt g l p) := by
  -- `T ↦ idFieldContrib`, every other parameter to its constant `g`-value
  set sa : Ty_param → FieldContrib := Function.update (fun α => constFieldContrib (g α)) T idFieldContrib with hsa
  -- evaluated at any field `v`, the symbolic assignment is `FEquiv`-pointwise to `update g T v`
  have key : ∀ (v : FieldDesc) (α : Ty_param), FEquiv ((sa α).apply v) (Function.update g T v α) := by
    intro v α
    by_cases hα : α = T
    · subst hα
      simp only [hsa, Function.update_self]
      exact idFieldContrib_apply v
    · simp only [hsa, Function.update_of_ne hα, constFieldContrib_apply]
      exact FEquiv.refl _
  -- restate the two corner subfield goals on the symbolic assignment
  have HloP : SubField (rowFieldAt (fun α => (sa α).apply ((g T).setBase lo.base)) l r)
                       (rowFieldAt (fun α => (sa α).apply ((g T).setBase lo.base)) l p) :=
    SubField.of_fequiv (rowFieldAt_fequiv (key ((g T).setBase lo.base)) l r)
      (rowFieldAt_fequiv (key ((g T).setBase lo.base)) l p) Hlo
  have HhiP : SubField (rowFieldAt (fun α => (sa α).apply ((g T).setBase hi.base)) l r)
                       (rowFieldAt (fun α => (sa α).apply ((g T).setBase hi.base)) l p) :=
    SubField.of_fequiv (rowFieldAt_fequiv (key ((g T).setBase hi.base)) l r)
      (rowFieldAt_fequiv (key ((g T).setBase hi.base)) l p) Hhi
  -- peel symbolically, then transport back: `(sa α).apply (g T) ~ g α`
  have concl := peel_step_contrib sa l r p hlo hhi HloP HhiP
  have keyg : ∀ α, FEquiv ((sa α).apply (g T)) (g α) := by
    intro α
    have h := key (g T) α
    rwa [Function.update_eq_self] at h
  exact SubField.of_fequiv (rowFieldAt_fequiv keyg l r).symm (rowFieldAt_fequiv keyg l p).symm concl

/-- **Cartesian peel.**  At label `l`, the per-label goal under an in-box `g` follows once it holds at
    every corner-completion of `g` over the (nodup) parameter list `Ts` — peeling one parameter at a
    time with `peel_step`, each branching into the `lo.base` / `hi.base` corners.  The `box` is a
    *fixed* (already-evaluated) interval per parameter; the topological coupling — choosing each
    box under the outer parameters' corners — is layered on separately. -/
theorem peel_all (l : Option Label) (r p : Row) (box : Ty_param → FieldDesc × FieldDesc) :
    ∀ (Ts : List Ty_param), Ts.Nodup → ∀ (g : Ty_param → FieldDesc),
      (∀ T ∈ Ts, SubField (box T).1 (g T) ∧ SubField (g T) (box T).2) →
      (∀ g' : Ty_param → FieldDesc, (∀ U, U ∉ Ts → g' U = g U) →
        (∀ U ∈ Ts, g' U = (g U).setBase (box U).1.base ∨ g' U = (g U).setBase (box U).2.base) →
        SubField (rowFieldAt g' l r) (rowFieldAt g' l p)) →
      SubField (rowFieldAt g l r) (rowFieldAt g l p)
  | [], _, g, _, hchk => hchk g (fun _ _ => rfl) (fun U hU => by simp at hU)
  | T :: Ts', hnodup, g, hbox, hchk => by
    obtain ⟨hlo, hhi⟩ := hbox T List.mem_cons_self
    have hTnotin : T ∉ Ts' := (List.nodup_cons.mp hnodup).1
    have hnodup' : Ts'.Nodup := (List.nodup_cons.mp hnodup).2
    -- peel `T` to its two base-corners; recurse on `Ts'` for each
    refine peel_step g l r p T hlo hhi ?_ ?_
    · refine peel_all l r p box Ts' hnodup'
        (Function.update g T ((g T).setBase (box T).1.base)) ?_ ?_
      · -- the updated `g` is still in box on `Ts'` (untouched there)
        intro U hU
        have hUneT : U ≠ T := fun h => hTnotin (h ▸ hU)
        rw [Function.update_of_ne hUneT]; exact hbox U (List.mem_cons_of_mem _ hU)
      · -- a corner-completion over `Ts'` of the updated `g` is one over `T :: Ts'` of `g`
        intro g' hag hcor
        refine hchk g' ?_ ?_
        · intro U hU
          rw [List.mem_cons, not_or] at hU
          rw [hag U hU.2, Function.update_of_ne hU.1]
        · intro U hU
          rw [List.mem_cons] at hU
          rcases hU with rfl | hU
          · rw [hag U hTnotin, Function.update_self]; exact Or.inl rfl
          · have hUneT : U ≠ T := fun h => hTnotin (h ▸ hU)
            have hc := hcor U hU; rw [Function.update_of_ne hUneT] at hc; exact hc
    · refine peel_all l r p box Ts' hnodup'
        (Function.update g T ((g T).setBase (box T).2.base)) ?_ ?_
      · intro U hU
        have hUneT : U ≠ T := fun h => hTnotin (h ▸ hU)
        rw [Function.update_of_ne hUneT]; exact hbox U (List.mem_cons_of_mem _ hU)
      · intro g' hag hcor
        refine hchk g' ?_ ?_
        · intro U hU
          rw [List.mem_cons, not_or] at hU
          rw [hag U hU.2, Function.update_of_ne hU.1]
        · intro U hU
          rw [List.mem_cons] at hU
          rcases hU with rfl | hU
          · rw [hag U hTnotin, Function.update_self]; exact Or.inr rfl
          · have hUneT : U ≠ T := fun h => hTnotin (h ▸ hU)
            have hc := hcor U hU; rw [Function.update_of_ne hUneT] at hc; exact hc

/- ---------------------------------------------------------------------------------------------- -/
/- ## 28. Relevance: a row's per-label field depends only on the parameters it mentions            -/
/- ---------------------------------------------------------------------------------------------- -/

/- The parameters a row *mentions* — those occurring at spread position, recursively through nested
   splat shapes.  Mirrors the frame's §14 recursion; only the spread `rigid` arm and the nested
   splat shape carry parameters. -/
mutual
  def baseMentions : Base → Ty_param → Prop
    | .rigid β,              α => β = α
    | .shape (.splat es),    α => splatMentions es α
    | .top,                  _ => False
    | .bot,                  _ => False
    | .prim _,               _ => False
    | .union _ _,            _ => False
    | .shape (.simple _ _),  _ => False

  def splatMentions : List SplatElem → Ty_param → Prop
    | [],      _ => False
    | e :: es, α => elemMentions e α ∨ splatMentions es α

  def elemMentions : SplatElem → Ty_param → Prop
    | .spread b, α => baseMentions b α
end

/-- A parameter is mentioned by a row when it occurs at spread position (recursively). -/
def rowMentions : Row → Ty_param → Prop
  | .simple _ _, _ => False
  | .splat es,   α => splatMentions es α

/- Two assignments agreeing on a row's mentioned parameters give the same per-label field.  By the
   same mutual induction as the frame's §14 block. -/
mutual
  theorem baseFieldAt_congr {g g' : Ty_param → FieldDesc} (l : Option Label) :
      ∀ b : Base, (∀ α, baseMentions b α → g α = g' α) →
        baseFieldAt g l b = baseFieldAt g' l b
    | .rigid β,              h => h β rfl
    | .bot,                  _ => rfl
    | .top,                  _ => rfl
    | .prim _,               _ => rfl
    | .union _ _,            _ => rfl
    | .shape (.simple _ _),  _ => rfl
    | .shape (.splat es),    h => splatFieldAt_congr l es h

  theorem elemFieldAt_congr {g g' : Ty_param → FieldDesc} (l : Option Label) :
      ∀ e : SplatElem, (∀ α, elemMentions e α → g α = g' α) →
        elemFieldAt g l e = elemFieldAt g' l e
    | .spread b, h => baseFieldAt_congr l b h

  theorem splatFieldAt_congr {g g' : Ty_param → FieldDesc} (l : Option Label) :
      ∀ es : List SplatElem, (∀ α, splatMentions es α → g α = g' α) →
        splatFieldAt g l es = splatFieldAt g' l es
    | [],      _ => rfl
    | e :: es, h =>
        splatFoldAt_congr l es (elemFieldAt g l e) (elemFieldAt g' l e)
          (elemFieldAt_congr l e (fun α ha => h α (Or.inl ha)))
          (fun α ha => h α (Or.inr ha))

  theorem splatFoldAt_congr {g g' : Ty_param → FieldDesc} (l : Option Label) :
      ∀ (es : List SplatElem) (acc acc' : FieldDesc), acc = acc' →
        (∀ α, splatMentions es α → g α = g' α) →
        splatFoldAt g l es acc = splatFoldAt g' l es acc'
    | [],      _,   _,    hacc, _ => hacc
    | e :: es, acc, acc', hacc, h =>
        splatFoldAt_congr l es (mergeField acc (elemFieldAt g l e))
          (mergeField acc' (elemFieldAt g' l e))
          (by rw [hacc, elemFieldAt_congr l e (fun α ha => h α (Or.inl ha))])
          (fun α ha => h α (Or.inr ha))
end

/-- **Relevance.**  A row's per-label field depends only on the assignment at the parameters the row
    mentions — what lets the corner check range over only the relevant parameters. -/
theorem rowFieldAt_congr {g g' : Ty_param → FieldDesc} (l : Option Label) (r : Row)
    (h : ∀ α, rowMentions r α → g α = g' α) :
    rowFieldAt g l r = rowFieldAt g' l r := by
  cases r with
  | simple _ _ => rfl
  | splat es   => exact splatFieldAt_congr l es h

/- ---------------------------------------------------------------------------------------------- -/
/- ## 29. The composite combinator: a parameter pinned to a field-dependent box endpoint           -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Force a `FieldContrib`'s output requiredness to `q` on both branches (keeping each branch's base), so
    the application's requiredness becomes fixed while its base still tracks the input.  This is the
    fixed-requiredness slot a plain `BaseContrib`/affine combinator cannot express. -/
def FieldContrib.setReq (g : FieldContrib) (q : Bool) : FieldContrib :=
  ⟨⟨q, g.onReq.pre, g.onReq.usesInput⟩, ⟨q, g.onOpt.pre, g.onOpt.usesInput⟩⟩

theorem FieldContrib.setReq_apply (g : FieldContrib) (c d : FieldDesc) :
    (g.setReq c.isReq).apply d = c.setBase ((g.apply d).base) := by
  -- case the branch requirednesses too: `(g.apply d).base` is otherwise stuck on `g`'s `BaseContrib` `q`s
  obtain ⟨⟨qr, pr, ur⟩, ⟨qo, po, uo⟩⟩ := g
  cases c <;> cases d <;> cases qr <;> cases qo <;> rfl

/-- The **composite combinator** for a parameter pinned to a box endpoint that itself depends on the
    peeled parameter's field: requiredness fixed by `c` (the kept corner requiredness), base tracking
    the peeled field through `ψ` (the box-endpoint combinator, e.g. `rowFieldContrib` of a bound row).  So
    `(composeFieldContrib c ψ).apply d` is the corner field `c.setBase` of the box endpoint `(ψ.apply d).base`. -/
def composeFieldContrib (c : FieldDesc) (ψ : FieldContrib) : FieldContrib := ψ.setReq c.isReq

theorem composeFieldContrib_apply (c : FieldDesc) (ψ : FieldContrib) (d : FieldDesc) :
    (composeFieldContrib c ψ).apply d = c.setBase ((ψ.apply d).base) :=
  FieldContrib.setReq_apply ψ c d

end Splat
