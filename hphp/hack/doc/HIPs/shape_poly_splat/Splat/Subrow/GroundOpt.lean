import Splat.Subrow.Irrelevance
import Splat.Subrow.Decide

/-!
# Ground-bounds corner cuts

When a parameter's bound rows are ground (mention no parameters) its field box is fixed, so the
enumeration is the cartesian `cornerAssigns` (§37) and each corner cut reads off in one step — no
topological threading, and masking is unrestricted (a plain fixed-box `Req` check).  This is the
clean form of the optimization; the coupled layer (`CoupledOpt`) is the same cuts threaded through
the topological enumeration.
-/

namespace Splat

variable [∀ a b : Base, Decidable (SubBase a b)]

/- ---------------------------------------------------------------------------------------------- -/
/- ## 67. Ground (fixed-box) corner cuts, read directly                                            -/
/- ---------------------------------------------------------------------------------------------- -/


/-! When every parameter's bound rows are *ground* (mention no parameters) the field box
`fieldBoundsAt Γ g l α` does not depend on the assignment `g`, so the corner enumeration is the
*fixed-box* cartesian `cornerAssigns` (§37) rather than the threaded `coupledCornerAssigns`.  Over a
fixed box the optimization reads off in one step, with none of the topological threading the coupled
case needs — and masking is *unrestricted*: an always-required masker is just a parameter whose
(fixed) box upper is `Req`, no assignment-quantifier.  These per-parameter cut lemmas are that clean
form; iterating them over a parameter list is the same peel the coupled `goalcheck_sel_iff` performs in
general. -/

/-- **Upper-endpoint cut (fixed box).**  If the super-row is independent of `T` across the box corners
    (`hpindep` — supplied below by non-liveness or masking), then checking only the corners that pin
    `T` to its upper endpoint suffices: the sub side is monotone in `T`, so the upper endpoint is the
    worst case. -/
theorem cornerAssigns_pin_hi (l : Option Label) (r p : Row)
    (box : Ty_param → FieldDesc × FieldDesc) (dflt : Ty_param → FieldDesc)
    (Ts : List Ty_param) (hnodup : Ts.Nodup) (T : Ty_param) (hT : T ∈ Ts)
    (hboxcons : ∀ U, SubField (box U).1 (box U).2)
    (hpindep : ∀ c : Ty_param → FieldDesc, (∀ U ∈ Ts, c U ∈ rigidCorners (box U).1 (box U).2) →
       rowFieldAt c l p = rowFieldAt (Function.update c T (box T).2) l p)
    (Hhi : ∀ c ∈ cornerAssigns box dflt Ts, c T = (box T).2 → goalAt l r p c) :
    ∀ c ∈ cornerAssigns box dflt Ts, goalAt l r p c := by
  intro c hc
  obtain ⟨hcin, hcoff⟩ := (mem_cornerAssigns Ts hnodup c).mp hc
  have hhi_mem : (box T).2 ∈ rigidCorners (box T).1 (box T).2 := by
    have h := mem_rigidCorners_hi (hboxcons T) (subField_refl (box T).2); rwa [setBase_self] at h
  -- the upper-pinned corner is again a corner
  have hc'mem : Function.update c T (box T).2 ∈ cornerAssigns box dflt Ts :=
    (mem_cornerAssigns Ts hnodup _).mpr ⟨fun U hU => by
        by_cases hUT : U = T
        · subst hUT; rw [Function.update_self]; exact hhi_mem
        · rw [Function.update_of_ne hUT]; exact hcin U hU,
      fun U hU => by rw [Function.update_of_ne (by rintro rfl; exact hU hT)]; exact hcoff U hU⟩
  have hGc' : goalAt l r p (Function.update c T (box T).2) :=
    Hhi _ hc'mem (by rw [Function.update_self])
  -- the upper-pinned corner agrees with `c` off `T`, sits above it at `T`, and leaves the super side
  -- unchanged (`hpindep`) — the box-agnostic upper cut
  exact goalCut_hi (fun β hβ => by rw [Function.update_of_ne hβ])
    (by rw [Function.update_self]; exact (mem_rigidCorners (hcin T hT)).2)
    (hpindep c hcin) hGc'

/-- **Lower-endpoint cut (fixed box).**  Mirror of `cornerAssigns_pin_hi`: if the sub-row is
    independent of `T`, checking only the lower-endpoint corners suffices (the super side is monotone). -/
theorem cornerAssigns_pin_lo (l : Option Label) (r p : Row)
    (box : Ty_param → FieldDesc × FieldDesc) (dflt : Ty_param → FieldDesc)
    (Ts : List Ty_param) (hnodup : Ts.Nodup) (T : Ty_param) (hT : T ∈ Ts)
    (hboxcons : ∀ U, SubField (box U).1 (box U).2)
    (hrindep : ∀ c : Ty_param → FieldDesc, (∀ U ∈ Ts, c U ∈ rigidCorners (box U).1 (box U).2) →
       rowFieldAt c l r = rowFieldAt (Function.update c T (box T).1) l r)
    (Hlo : ∀ c ∈ cornerAssigns box dflt Ts, c T = (box T).1 → goalAt l r p c) :
    ∀ c ∈ cornerAssigns box dflt Ts, goalAt l r p c := by
  intro c hc
  obtain ⟨hcin, hcoff⟩ := (mem_cornerAssigns Ts hnodup c).mp hc
  have hlo_mem : (box T).1 ∈ rigidCorners (box T).1 (box T).2 := by
    have h := mem_rigidCorners_lo (subField_refl (box T).1) (hboxcons T); rwa [setBase_self] at h
  have hc'mem : Function.update c T (box T).1 ∈ cornerAssigns box dflt Ts :=
    (mem_cornerAssigns Ts hnodup _).mpr ⟨fun U hU => by
        by_cases hUT : U = T
        · subst hUT; rw [Function.update_self]; exact hlo_mem
        · rw [Function.update_of_ne hUT]; exact hcin U hU,
      fun U hU => by rw [Function.update_of_ne (by rintro rfl; exact hU hT)]; exact hcoff U hU⟩
  have hGc' : goalAt l r p (Function.update c T (box T).1) :=
    Hlo _ hc'mem (by rw [Function.update_self])
  -- the lower-pinned corner agrees with `c` off `T`, sits below it at `T`, and leaves the sub side
  -- unchanged (`hrindep`) — the box-agnostic lower cut
  exact goalCut_lo (fun β hβ => by rw [Function.update_of_ne hβ])
    (by rw [Function.update_self]; exact (mem_rigidCorners (hcin T hT)).1)
    (hrindep c hcin) hGc'

/-- Non-liveness discharges the super-side independence: `T` not live in the super-row leaves its field
    unchanged by any move at `T` (`rowFieldAt_liveAt_irrel`).  This is the free-single-side `[upper]`
    cut, fixed-box form. -/
theorem cornerAssigns_pin_hi_notLive (l : Option Label) (r p : Row)
    (box : Ty_param → FieldDesc × FieldDesc) (dflt : Ty_param → FieldDesc)
    (Ts : List Ty_param) (hnodup : Ts.Nodup) (T : Ty_param) (hT : T ∈ Ts)
    (hboxcons : ∀ U, SubField (box U).1 (box U).2)
    (hflatp : RowNoNested p) (hlivep : ¬ RowLiveAt l T p)
    (Hhi : ∀ c ∈ cornerAssigns box dflt Ts, c T = (box T).2 → goalAt l r p c) :
    ∀ c ∈ cornerAssigns box dflt Ts, goalAt l r p c :=
  cornerAssigns_pin_hi l r p box dflt Ts hnodup T hT hboxcons
    (fun c _ => rowFieldAt_liveAt_irrel (fun β hβ => by rw [Function.update_of_ne hβ]) p hflatp hlivep)
    Hhi

/-- Masking discharges the super-side independence **unrestricted**: `T` masked in the super-row by a
    parameter whose *fixed* box upper is `Req` (an always-required masker — no assignment-quantifier,
    since the box is fixed) leaves the super field unchanged (`rowFieldAt_liveAtR_irrel`).  The masker's
    required value comes from its box corner (`subField_isReq`).  This is the masking `[upper]` cut, in
    full — the definite/assignment-dependent split of the coupled case does not arise here. -/
theorem cornerAssigns_pin_hi_masked (l : Option Label) (r p : Row)
    (box : Ty_param → FieldDesc × FieldDesc) (dflt : Ty_param → FieldDesc)
    (Ts : List Ty_param) (hnodup : Ts.Nodup) (T : Ty_param) (hT : T ∈ Ts)
    (hboxcons : ∀ U, SubField (box U).1 (box U).2)
    (hflatp : RowNoNested p) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (hmaskp : ¬ RowLiveAtR (fun u => rowMentions p u ∧ u ≠ T ∧ (box u).2.isReq = true) l T p)
    (Hhi : ∀ c ∈ cornerAssigns box dflt Ts, c T = (box T).2 → goalAt l r p c) :
    ∀ c ∈ cornerAssigns box dflt Ts, goalAt l r p c :=
  cornerAssigns_pin_hi l r p box dflt Ts hnodup T hT hboxcons
    (fun c hcin => rowFieldAt_liveAtR_irrel _ (fun β hβ => by rw [Function.update_of_ne hβ])
      (fun u hu => subField_isReq (mem_rigidCorners (hcin u (hrelp u hu.1))).2 hu.2.2)
      (fun u hu => hu.2.1) p hflatp hmaskp)
    Hhi


end Splat
