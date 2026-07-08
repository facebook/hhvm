import Splat.Subrow.Sound

/-!
# Completeness of the corner check — the witness primitives

The completeness direction (`SemSubRow → corner check`) realizes each corner assignment by a
`Compatible` instantiation `ρ` whose parameters sit at the chosen corners at the failing label and at
their upper bounds elsewhere.  This file builds the override primitive `rowUpdate` (set one field of a
simple row, keeping the rest) and its projection / monotonicity lemmas — the building block for that
witness.

With the per-label anchor there is no collapse: a parameter's contribution at label `l` is read
directly by `gOf ρ (some l)`, and the ground bounds are represented by opt-only simple rows
`Lb`/`Ub` whose per-label field (via `proj`) equals the bound's evaluation `evalAt ρ' (some l)`.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 31. Overriding one field of a simple row                                                     -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Override a simple row's field at one label (keep-first via `normalize`, so the new `(l₀, d)`
    shadows any existing entry). -/
def rowUpdate (R : SimpleRow) (l₀ : Label) (d : FieldDesc) : SimpleRow :=
  normalize ((l₀, d) :: R.known) R.unknown

/-- Projecting an override: the overridden label gives the new field, every other label is
    unchanged. -/
theorem proj_rowUpdate (R : SimpleRow) (l₀ : Label) (d : FieldDesc) (l : Label) :
    proj (rowUpdate R l₀ d) l = if l = l₀ then d else proj R l := by
  rw [rowUpdate, proj_normalize]
  by_cases h : l = l₀
  · -- the override key matches: the cons supplies `d`
    have e : ((l₀, d) :: R.known).lookup l = some d := by
      subst h; simp
    rw [e, if_pos h]
  · -- the override key differs: skip the cons, falling back to `proj R l`
    have e : ((l₀, d) :: R.known).lookup l = R.known.lookup l := by
      simp [List.lookup_cons, beq_eq_false_iff_ne.mpr h]
    rw [e, if_neg h]
    rfl

/-- `rowUpdate R l₀ d ≤ R` (as `SubField_row`) when the new field `d` refines `R`'s field at `l₀`. -/
theorem rowUpdate_le {R : SimpleRow} {l₀ : Label} {d : FieldDesc}
    (hd : SubField d (proj R l₀)) : SubField_row (rowUpdate R l₀ d) R := by
  apply SubField_row.mk
  intro l
  rw [proj_rowUpdate]
  by_cases h : l = l₀
  · subst h; rw [if_pos rfl]; exact hd
  · rw [if_neg h]; exact subField_refl _

/-- `R ≤ rowUpdate R l₀ d` when `R`'s field at `l₀` refines the new field `d`. -/
theorem le_rowUpdate {R : SimpleRow} {l₀ : Label} {d : FieldDesc}
    (hd : SubField (proj R l₀) d) : SubField_row R (rowUpdate R l₀ d) := by
  apply SubField_row.mk
  intro l
  rw [proj_rowUpdate]
  by_cases h : l = l₀
  · subst h; rw [if_pos rfl]; exact hd
  · rw [if_neg h]; exact subField_refl _

/- ---------------------------------------------------------------------------------------------- -/
/- ## 32. Ground simple rows                                                                      -/
/- ---------------------------------------------------------------------------------------------- -/

/-- A list `(k ↦ g k)` is `Ground` when every value is parameter-free. -/
theorem groundFields_map {g : Label → FieldDesc} (hg : ∀ k, RigidFreeField (g k)) :
    ∀ ks : List Label, GroundFields (ks.map (fun k => (k, g k)))
  | []      => trivial
  | k :: ks => ⟨hg k, groundFields_map hg ks⟩

/-- A value looked up in a `Ground` field list is parameter-free. -/
theorem groundFields_lookup {fs : List (Label × FieldDesc)} (h : GroundFields fs) {k : Label}
    {v : FieldDesc} (hv : fs.lookup k = some v) : RigidFreeField v := by
  induction fs with
  | nil => simp at hv
  | cons hd tl ih =>
    obtain ⟨a, b⟩ := hd
    obtain ⟨hb, htl⟩ := h
    rw [List.lookup_cons] at hv
    cases hbeq : k == a with
    | true  => simp [hbeq] at hv; subst hv; exact hb
    | false => simp [hbeq] at hv; exact ih htl hv

/-- `normalize` of a `Ground` field list (with a parameter-free `unknown` base) is `Ground`: its
    known values are looked-up from `fs` or default to `.opt u`, all parameter-free. -/
theorem groundFields_normalize {fs : List (Label × FieldDesc)} {u : Base}
    (hk : GroundFields fs) (hu : RigidFreeBase u) :
    GroundFields (normalize fs u).known := by
  show GroundFields ((nubLabels (fs.map Prod.fst)).map (fun k => (k, (fs.lookup k).getD (.opt u))))
  apply groundFields_map
  intro k
  cases hl : fs.lookup k with
  | none   => exact rigidFreeField_opt hu
  | some v => exact groundFields_lookup hk hl

/- ---------------------------------------------------------------------------------------------- -/
/- ## 33. The completeness witness                                                                -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The completeness witness for a corner assignment `c` at label `l₀`.  Each *declared* parameter is
    sent to its upper-bound row (`Ub α`) with the field at `l₀` overridden to the corner `c α`; an
    undeclared parameter to a junk closed row.  Its per-label field is `rowUpdate (Ub α) l₀ (c α)`,
    which realizes `c` at `l₀` and stays within the bounds. -/
def completeWitness (Γ : TyParamEnv) (Ub : Ty_param → SimpleRow) (c : Ty_param → FieldDesc)
    (l₀ : Label) : GEnv :=
  fun α => match Γ α with
    | some _ => .shape (.simple ((l₀, c α) :: (Ub α).known) (Ub α).unknown)
    | none   => .shape (.simple [] .bot)

/-- At a declared parameter, the witness's field at any label `l` is the upper-bound row overridden
    at `l₀`, projected at `l`. -/
theorem gOf_completeWitness_some (Γ : TyParamEnv) (Ub : Ty_param → SimpleRow)
    (c : Ty_param → FieldDesc) (l₀ : Label) {α : Ty_param} {b : Bounds} (h : Γ α = some b)
    (l : Label) :
    gOf (completeWitness Γ Ub c l₀) (some l) α = proj (rowUpdate (Ub α) l₀ (c α)) l := by
  show baseFieldAt (fun _ => .opt .bot) (some l) (completeWitness Γ Ub c l₀ α)
      = proj (rowUpdate (Ub α) l₀ (c α)) l
  simp only [completeWitness, h]
  rfl

/-- The witness realizes the corner: its field at `l₀` for a declared parameter is exactly `c α`. -/
theorem gOf_completeWitness (Γ : TyParamEnv) (Ub : Ty_param → SimpleRow) (c : Ty_param → FieldDesc)
    (l₀ : Label) {α : Ty_param} {b : Bounds} (h : Γ α = some b) :
    gOf (completeWitness Γ Ub c l₀) (some l₀) α = c α := by
  rw [gOf_completeWitness_some Γ Ub c l₀ h l₀, proj_rowUpdate, if_pos rfl]

/-- The witness is `Compatible` with `Γ`: at every declared parameter and label its field lies
    between the (ground, ρ-independent) bound rows — the lower at the corner `l₀` by box-membership
    and elsewhere by bound consistency, the upper by `rowUpdate_le`. -/
theorem compatible_completeWitness (Γ : TyParamEnv) (Ub Lb : Ty_param → SimpleRow)
    (c : Ty_param → FieldDesc) (l₀ : Label)
    (hLb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.lower) = proj (Lb α) l)
    (hUb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.upper) = proj (Ub α) l)
    (hconsist : ∀ α, SubField_row (Lb α) (Ub α))
    (hcbox : ∀ α b, Γ α = some b →
      SubField (proj (Lb α) l₀) (c α) ∧ SubField (c α) (proj (Ub α) l₀)) :
    Compatible (completeWitness Γ Ub c l₀) Γ := by
  intro α b h l
  rw [hLb α b h (completeWitness Γ Ub c l₀) l, hUb α b h (completeWitness Γ Ub c l₀) l,
      gOf_completeWitness_some Γ Ub c l₀ h l, proj_rowUpdate]
  by_cases hl : l = l₀
  · subst hl; rw [if_pos rfl]; exact hcbox α b h
  · rw [if_neg hl]; exact ⟨(hconsist α).at l, subField_refl _⟩

/- ---------------------------------------------------------------------------------------------- -/
/- ## 34. Completeness on the ground-bounds fragment                                              -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Completeness (ground bounds).**  For rows mentioning only declared parameters, and ground
    (ρ-independent) bounds, the semantic subrow forces the per-label goal at every in-box corner
    assignment `c`: realize `c` at `l₀` by the `completeWitness`, which is `Compatible`, then read
    `SemSubRow` off at `l₀` and rewrite each per-label field to `c`'s via relevance
    (`rowFieldAt_congr`, since the witness agrees with `c` on the rows' mentioned parameters). -/
theorem subrow_corner_complete_ground (r p : Row) (Γ : TyParamEnv)
    (Ub Lb : Ty_param → SimpleRow) (c : Ty_param → FieldDesc) (l₀ : Label)
    (hLb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.lower) = proj (Lb α) l)
    (hUb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.upper) = proj (Ub α) l)
    (hconsist : ∀ α, SubField_row (Lb α) (Ub α))
    (hcbox : ∀ α b, Γ α = some b →
      SubField (proj (Lb α) l₀) (c α) ∧ SubField (c α) (proj (Ub α) l₀))
    (hrel : ∀ α, (rowMentions r α ∨ rowMentions p α) → ∃ b, Γ α = some b)
    (hsem : SemSubRow r p Γ) :
    SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p) := by
  have hcompat : Compatible (completeWitness Γ Ub c l₀) Γ :=
    compatible_completeWitness Γ Ub Lb c l₀ hLb hUb hconsist hcbox
  have key := hsem (completeWitness Γ Ub c l₀) hcompat l₀
  simp only [evalAt] at key
  -- rewrite each witness per-label field to `c`'s symbolic projection (relevance)
  have er : rowFieldAt (gOf (completeWitness Γ Ub c l₀) (some l₀)) (some l₀) r
          = rowFieldAt c (some l₀) r :=
    rowFieldAt_congr (some l₀) r (fun α ha =>
      gOf_completeWitness Γ Ub c l₀ (hrel α (Or.inl ha)).choose_spec)
  have ep : rowFieldAt (gOf (completeWitness Γ Ub c l₀) (some l₀)) (some l₀) p
          = rowFieldAt c (some l₀) p :=
    rowFieldAt_congr (some l₀) p (fun α ha =>
      gOf_completeWitness Γ Ub c l₀ (hrel α (Or.inr ha)).choose_spec)
  rw [er, ep] at key
  exact key

/- ---------------------------------------------------------------------------------------------- -/
/- ## 35. Soundness on the ground-bounds fragment                                                 -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Soundness (ground bounds), `∀`-in-box form.**  If the per-label goal holds at *every* in-box
    assignment, at every label, then `SemSubRow` holds.  No corner reduction is needed here: the real
    assignment `gOf ρ (some l₀)` is itself in-box (`Compatible`), with the box reconciled to the
    ρ-independent `Lb`/`Ub` endpoints via `hLb`/`hUb`.  (Reducing the `∀`-in-box check to the finite
    corners is the separate decidability step, via `peel_all`.) -/
theorem subrow_ground_sound (r p : Row) (Γ : TyParamEnv) (Lb Ub : Ty_param → SimpleRow)
    (hLb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.lower) = proj (Lb α) l)
    (hUb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.upper) = proj (Ub α) l)
    (hcheck : ∀ l₀ : Label, ∀ c : Ty_param → FieldDesc,
      (∀ α b, Γ α = some b → InBox (proj (Lb α) l₀) (proj (Ub α) l₀) (c α)) →
      SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p)) :
    SemSubRow r p Γ := by
  rw [semSubRow_iff_perLabel]
  intro ρ hcompat l₀
  -- the per-label goal at `gOf ρ (some l₀)`, which is in-box (`Compatible`)
  apply hcheck l₀ (gOf ρ (some l₀))
  intro α b hΓ
  obtain ⟨hlo, hhi⟩ := hcompat α b hΓ l₀
  -- reconcile the box endpoints to the ρ-independent `Lb`/`Ub` projections
  rw [hLb α b hΓ ρ l₀] at hlo
  rw [hUb α b hΓ ρ l₀] at hhi
  exact ⟨hlo, hhi⟩

end Splat
