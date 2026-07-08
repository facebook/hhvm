import Splat.Subrow.Corner

/-!
# Soundness of the corner check (the box-corner reduction)

`semSubRow_of_boxCorners`: the semantic subrow follows from the per-label, per-parameter box-corner
check.  This is the `peel_all` packaging — at each compatible instantiation `ρ` and label, the real
field assignment `gOf ρ l` lies in its (coupled) box (`inBox_of_compatible`), and `peel_all` reduces
the per-label goal at that assignment to the goal at every box-corner-completion, which the four
`Field_desc.corners` per parameter cover.

The box here is `fieldBoundsAt Γ (gOf ρ l) l` — still evaluated under `ρ` (coupled).  Making the
check `ρ`-independent (so it is decidable) is the separate topological bridge.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 30. Soundness via the box-corner check                                                      -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Soundness (box-corner reduction).**  For `Ground` rows and `Ground` bounds, the semantic
    subrow `SemSubRow r p Γ` follows from the per-label box-corner check: under every compatible
    instantiation `ρ` and label `l₀`, every assignment `g'` that agrees with `gOf ρ (some l₀)`
    outside `Ts` and sets each `Ts`-parameter to one of the four corners of its coupled box
    `fieldBoundsAt Γ (gOf ρ (some l₀)) (some l₀)` makes `r`'s per-label field a subfield of `p`'s.

    The box is evaluated under `ρ` (coupled); the topological, `ρ`-independent enumeration is layered
    on separately. -/
theorem semSubRow_of_boxCorners (r p : Row) (Γ : TyParamEnv) (Ts : List Ty_param)
    (hnodup : Ts.Nodup) (_hr : GroundRow r) (_hp : GroundRow p)
    (hdecl : ∀ U ∈ Ts, ∃ b, Γ U = some b ∧
      GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper))
    (hcheck : ∀ ρ, Compatible ρ Γ → ∀ l₀ : Label, ∀ g' : Ty_param → FieldDesc,
      (∀ U, U ∉ Ts → g' U = gOf ρ (some l₀) U) →
      (∀ U ∈ Ts,
        g' U = .req (fieldBoundsAt Γ (gOf ρ (some l₀)) (some l₀) U).1.base ∨
        g' U = .opt (fieldBoundsAt Γ (gOf ρ (some l₀)) (some l₀) U).1.base ∨
        g' U = .req (fieldBoundsAt Γ (gOf ρ (some l₀)) (some l₀) U).2.base ∨
        g' U = .opt (fieldBoundsAt Γ (gOf ρ (some l₀)) (some l₀) U).2.base) →
      SubField (rowFieldAt g' (some l₀) r) (rowFieldAt g' (some l₀) p)) :
    SemSubRow r p Γ := by
  rw [semSubRow_iff_perLabel]
  intro ρ hc l₀
  -- the per-label goal is already the symbolic projection at `some l₀` — no bridge needed
  -- peel every `Ts`-parameter over its coupled box, evaluated at the real assignment
  refine peel_all (some l₀) r p (fieldBoundsAt Γ (gOf ρ (some l₀)) (some l₀)) Ts hnodup
    (gOf ρ (some l₀)) ?hbox ?hchk
  case hbox =>
    -- the real assignment lies in each coupled box (`inBox_of_compatible`)
    intro T hT
    obtain ⟨b, hb, _, _⟩ := hdecl T hT
    have hbox := inBox_of_compatible ρ Γ hc l₀ T b hb
    simp only [fieldBoundsAt, hb]
    exact hbox
  case hchk =>
    -- a box-corner-completion is one of the four `{req,opt} × {lo,hi}` corners
    intro g' hag hcor
    refine hcheck ρ hc l₀ g' hag ?_
    intro U hU
    rcases hcor U hU with h | h
    · rcases FieldDesc.setBase_cases (gOf ρ (some l₀) U)
        (fieldBoundsAt Γ (gOf ρ (some l₀)) (some l₀) U).1.base with hc1 | hc1
      · exact Or.inl (h.trans hc1)
      · exact Or.inr (Or.inl (h.trans hc1))
    · rcases FieldDesc.setBase_cases (gOf ρ (some l₀) U)
        (fieldBoundsAt Γ (gOf ρ (some l₀)) (some l₀) U).2.base with hc2 | hc2
      · exact Or.inr (Or.inr (Or.inl (h.trans hc2)))
      · exact Or.inr (Or.inr (Or.inr (h.trans hc2)))

end Splat
