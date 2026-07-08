import Splat.Subrow.Irrelevance
import Splat.Subrow.CoupledComplete

/-!
# The coupled optimized check (faithful to the implementation's `corners_for`)

The selected-corner enumeration with a per-parameter corner selector, the adequacy conditions a
selector must meet (free-single-side, masking, liveness), the proof that an adequate selector decides
`SemSubRow` exactly as the full enumeration, and the concrete `cornersFor` selector mirroring the
implementation.  This is the general (interdependent-bounds) layer; it threads the box-shift the
ground layer (`GroundOpt`) avoids.
-/

namespace Splat

variable [∀ a b : Base, Decidable (SubBase a b)]

/- ---------------------------------------------------------------------------------------------- -/
/- ## 68. The selected-corner enumeration                                                         -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The coupled enumeration with a per-parameter **corner selector** `sel` in place of the full
    `rigidCorners`.  Identical in shape to `coupledCornerAssigns` (§41) — walk `Ts` front-to-back,
    branch at each `T` over its chosen corners, thread the assignment — but at `T` it branches over
    `sel T (box).1 (box).2` rather than every box corner.  The implementation's `corners_for`
    (`splat.ml`) is one such selector. -/
def coupledCornerAssignsSel (Γ : TyParamEnv) (l : Option Label)
    (sel : Ty_param → FieldDesc → FieldDesc → List FieldDesc) :
    List Ty_param → (Ty_param → FieldDesc) → List (Ty_param → FieldDesc)
  | [],      fa => [fa]
  | T :: Ts, fa =>
      (sel T (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2).flatMap
        (fun d => coupledCornerAssignsSel Γ l sel Ts (Function.update fa T d))

/-- A parameter is **always required** at `l`: its upper-bound row projects to a required field there
    under every assignment.  Such a parameter takes only required corners (`coupledCornerAssigns_isReq`
    below), so it acts as a definite masker of everything to its left — the masking analysis's
    always-required rightward rigid, restricted to the case where the requiredness is forced by the
    bound itself (independent of the assignment). -/
def AlwaysReqAt (Γ : TyParamEnv) (l : Option Label) (u : Ty_param) : Prop :=
  ∀ g, (fieldBoundsAt Γ g l u).2.isReq = true

/-- **Always-required parameters take required corners.**  If a parameter's upper bound projects to a
    required field at `l` under *every* assignment, then every corner the enumeration assigns it is
    required — its box upper is required, and a corner lies below it.  This is what lets the masking
    shortcut treat such a parameter as a definite masker of everything to its left. -/
theorem coupledCornerAssigns_isReq (Γ : TyParamEnv) (l : Option Label) (u : Ty_param)
    (hu : ∀ g, (fieldBoundsAt Γ g l u).2.isReq = true) :
    ∀ (Ts : List Ty_param), Ts.Nodup → u ∈ Ts → ∀ (fa : Ty_param → FieldDesc),
      ∀ c ∈ coupledCornerAssigns Γ l Ts fa, (c u).isReq = true := by
  intro Ts
  induction Ts with
  | nil => intro _ huin; exact absurd huin (by simp)
  | cons U rest ih =>
    intro hnodup huin fa c hc
    have hUnotin : U ∉ rest := (List.nodup_cons.mp hnodup).1
    have hnodup' : rest.Nodup := (List.nodup_cons.mp hnodup).2
    rw [coupledCornerAssigns, List.mem_flatMap] at hc
    obtain ⟨d, hd, hcd⟩ := hc
    rcases List.mem_cons.mp huin with rfl | hurest
    · -- `u = U`: its corner `d` lies in the box whose upper is required, so `d` is required
      have hcu : c u = d := by
        rw [coupledCornerAssigns_off Γ l rest _ c hcd u hUnotin, Function.update_self]
      rw [hcu]
      exact subField_isReq (mem_rigidCorners hd).2 (hu fa)
    · -- `u` lies in the tail: the induction hypothesis
      exact ih hnodup' hurest (Function.update fa U d) c hcd

/- ---------------------------------------------------------------------------------------------- -/
/- ## 69. The selector adequacy condition                                                         -/
/- ---------------------------------------------------------------------------------------------- -/

/-- A selector is **adequate** at `T` when its choice for `T` is one the full check can be cut down to
    without changing the answer.  The disjuncts mirror `corners_for`'s branches: keep the full
    `rigidCorners`; or keep only the upper endpoint `[hi]` when the super side is independent of `T` —
    either because `T` is not live there (free-single-side) or because `T` is masked there by an
    always-required parameter to its right (masking); or only the lower endpoint `[lo]`, the mirrors for
    the sub side.  Each cut needs `T` *free* (no parameter in `Ts` depends on it, so its corner does not
    move another box).  The masking masker set keys on `rowMentions`, so it is independent of the
    enumeration list. -/
def SelAdequate (Γ : TyParamEnv) (l : Option Label) (r p : Row)
    (sel : Ty_param → FieldDesc → FieldDesc → List FieldDesc)
    (Ts : List Ty_param) (T : Ty_param) : Prop :=
  (∀ lo hi, sel T lo hi = rigidCorners lo hi)
  ∨ ((∀ lo hi, sel T lo hi = [hi]) ∧ (∀ U ∈ Ts, ¬ DependsOn Γ U T)
      ∧ ¬ RowLiveAt l T p ∧ RowNoNested p)
  ∨ ((∀ lo hi, sel T lo hi = [lo]) ∧ (∀ U ∈ Ts, ¬ DependsOn Γ U T)
      ∧ ¬ RowLiveAt l T r ∧ RowNoNested r)
  ∨ ((∀ lo hi, sel T lo hi = [hi]) ∧ (∀ U ∈ Ts, ¬ DependsOn Γ U T)
      ∧ ¬ RowLiveAtR (fun u => rowMentions p u ∧ u ≠ T ∧ AlwaysReqAt Γ l u) l T p ∧ RowNoNested p)
  ∨ ((∀ lo hi, sel T lo hi = [lo]) ∧ (∀ U ∈ Ts, ¬ DependsOn Γ U T)
      ∧ ¬ RowLiveAtR (fun u => rowMentions r u ∧ u ≠ T ∧ AlwaysReqAt Γ l u) l T r ∧ RowNoNested r)


/- ---------------------------------------------------------------------------------------------- -/
/- ## 70. The selected check decides exactly what the full check does                             -/
/- ---------------------------------------------------------------------------------------------- -/


/-- **Cross-seed corner correspondence for a free parameter.**  When `T` is free for `Ts` (no `Ts`
    parameter depends on it) and outside `Ts`, every corner `c` of the enumeration seeded at
    `update fa T d` has a corner `c₀` of the enumeration seeded at `update fa T e` that agrees with it
    *everywhere except `T`* — `c T = d`, `c₀ T = e`.  Because no `Ts` box reads `T`, the corner choices
    coincide (`coupledCornerAssigns_match`), and both carry their seed off `Ts` (`coupledCornerAssigns_off`). -/
theorem match_off_T (Γ : TyParamEnv) (l : Option Label) (Ts : List Ty_param)
    (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (T : Ty_param) (hTnotin : T ∉ Ts)
    (hfree : ∀ U ∈ Ts, ¬ DependsOn Γ U T) (fa : Ty_param → FieldDesc) (d e : FieldDesc)
    {c : Ty_param → FieldDesc} (hc : c ∈ coupledCornerAssigns Γ l Ts (Function.update fa T d)) :
    ∃ c₀, c₀ ∈ coupledCornerAssigns Γ l Ts (Function.update fa T e)
      ∧ (∀ β, β ≠ T → c β = c₀ β) ∧ c T = d ∧ c₀ T = e := by
  -- the corner choices coincide because no `Ts` box reads `T`, where the two seeds differ
  obtain ⟨c₀, hc₀mem, hc₀eq⟩ := coupledCornerAssigns_match Γ l Ts hnodup htopo
    (Function.update fa T d) (Function.update fa T e)
    (fun α ⟨U, hU, hdep⟩ _ => by
      by_cases hαT : α = T
      case pos => exact absurd (hαT ▸ hdep) (hfree U hU)
      case neg => rw [Function.update_of_ne hαT, Function.update_of_ne hαT])
    c hc
  refine ⟨c₀, hc₀mem, ?_, ?_, ?_⟩
  · -- agreement off `T`: inside `Ts` by the match, outside by both carrying the (off-`T`-equal) seeds
    intro β hβ
    by_cases hβin : β ∈ Ts
    case pos => exact hc₀eq β hβin
    case neg =>
      rw [coupledCornerAssigns_off Γ l Ts _ c hc β hβin,
          coupledCornerAssigns_off Γ l Ts _ c₀ hc₀mem β hβin,
          Function.update_of_ne hβ, Function.update_of_ne hβ]
  · rw [coupledCornerAssigns_off Γ l Ts _ c hc T hTnotin, Function.update_self]
  · rw [coupledCornerAssigns_off Γ l Ts _ c₀ hc₀mem T hTnotin, Function.update_self]

/-- **The selected check is faithful.**  For an adequate selector over `TopoSorted` `Ts` with
    consistent boxes, the per-label goal holds at every selected-enumeration corner iff it holds at
    every full-enumeration corner.  The cut corners are redundant: a parameter whose side is
    independent of it (not live there, or masked there by an always-required parameter to its right)
    fixes the worst case at one endpoint — monotonicity (`rowFieldAt_mono`) on the side it affects,
    irrelevance (`rowFieldAt_liveAt_irrel` for non-liveness, `rowFieldAt_liveAtR_irrel` for masking) on
    the side it does not — and being free its corner does not perturb the rest of the enumeration
    (`match_off_T`).  The seed-required invariant `hseedReq` carries the fact that an already-assigned
    always-required masker holds a required value, so masking stays valid as the list is peeled. -/
theorem goalcheck_sel_iff (Γ : TyParamEnv) (l : Option Label) (r p : Row)
    (sel : Ty_param → FieldDesc → FieldDesc → List FieldDesc)
    (hboxcons : ∀ (fa : Ty_param → FieldDesc) (T : Ty_param),
      SubField (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2) :
    ∀ (Ts : List Ty_param), Ts.Nodup → TopoSorted Γ Ts →
      (∀ T ∈ Ts, SelAdequate Γ l r p sel Ts T) →
      ∀ (fa : Ty_param → FieldDesc),
        (∀ u, (rowMentions r u ∨ rowMentions p u) → AlwaysReqAt Γ l u → u ∉ Ts →
          (fa u).isReq = true) →
        ((∀ c ∈ coupledCornerAssignsSel Γ l sel Ts fa, goalAt l r p c) ↔
         (∀ c ∈ coupledCornerAssigns Γ l Ts fa, goalAt l r p c)) := by
  intro Ts
  induction Ts with
  | nil =>
    -- both enumerations are the singleton `[fa]`
    intro _ _ _ fa _; exact Iff.rfl
  | cons T Ts' IH =>
    intro hnodup htopo hadeq fa hseedReq
    have hTnotin : T ∉ Ts' := (List.nodup_cons.mp hnodup).1
    have hnodup' : Ts'.Nodup := (List.nodup_cons.mp hnodup).2
    obtain ⟨htopoHd, htopoTl⟩ :
        (∀ α, DependsOn Γ T α → α ∉ T :: Ts') ∧ TopoSorted Γ Ts' := htopo
    -- adequacy weakens to the tail (only the free condition shrinks; masker sets key on `rowMentions`)
    have hadeq' : ∀ T'' ∈ Ts', SelAdequate Γ l r p sel Ts' T'' := by
      intro T'' hT''
      rcases hadeq T'' (List.mem_cons_of_mem T hT'') with
        hfb | ⟨hso, hfree, hrest⟩ | ⟨hso, hfree, hrest⟩ | ⟨hso, hfree, hrest⟩ | ⟨hso, hfree, hrest⟩
      · exact Or.inl hfb
      · exact Or.inr (Or.inl ⟨hso, fun U hU => hfree U (List.mem_cons_of_mem T hU), hrest⟩)
      · exact Or.inr (Or.inr (Or.inl ⟨hso, fun U hU => hfree U (List.mem_cons_of_mem T hU), hrest⟩))
      · exact Or.inr (Or.inr (Or.inr (Or.inl
          ⟨hso, fun U hU => hfree U (List.mem_cons_of_mem T hU), hrest⟩)))
      · exact Or.inr (Or.inr (Or.inr (Or.inr
          ⟨hso, fun U hU => hfree U (List.mem_cons_of_mem T hU), hrest⟩)))
    -- the head box and its endpoints, which are corners by consistency
    have hboxconsT : SubField (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2 := hboxcons fa T
    have hlo_mem : (fieldBoundsAt Γ fa l T).1 ∈
        rigidCorners (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2 := by
      have h := mem_rigidCorners_lo (subField_refl (fieldBoundsAt Γ fa l T).1) hboxconsT
      rwa [setBase_self] at h
    have hhi_mem : (fieldBoundsAt Γ fa l T).2 ∈
        rigidCorners (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2 := by
      have h := mem_rigidCorners_hi hboxconsT (subField_refl (fieldBoundsAt Γ fa l T).2)
      rwa [setBase_self] at h
    -- the seed-required invariant is maintained when a box corner `d` is added at `T`
    have hseed_step : ∀ d, d ∈ rigidCorners (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2 →
        ∀ u, (rowMentions r u ∨ rowMentions p u) → AlwaysReqAt Γ l u → u ∉ Ts' →
          (Function.update fa T d u).isReq = true := by
      intro d hd u hmen hAR huts'
      by_cases huT : u = T
      · subst huT; rw [Function.update_self]; exact subField_isReq (mem_rigidCorners hd).2 (hAR fa)
      · rw [Function.update_of_ne huT]
        exact hseedReq u hmen hAR (by rw [List.mem_cons, not_or]; exact ⟨huT, huts'⟩)
    rcases hadeq T List.mem_cons_self with
      hfb | ⟨hso, hfree, hlivep, hflatp⟩ | ⟨hso, hfree, hliver, hflatr⟩
        | ⟨hso, hfree, hmaskp, hflatp⟩ | ⟨hso, hfree, hmaskr, hflatr⟩
    case inl =>
      -- fallback: `sel` keeps the full corners here, so the enumerations agree branch-by-branch
      have hsel : sel T (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2
          = rigidCorners (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2 := hfb _ _
      constructor
      · intro Hsel c hc
        rw [coupledCornerAssigns, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        have hSeld : ∀ c' ∈ coupledCornerAssignsSel Γ l sel Ts' (Function.update fa T d),
            goalAt l r p c' := fun c' hc' => Hsel c' (by
          rw [coupledCornerAssignsSel, List.mem_flatMap]; exact ⟨d, by rw [hsel]; exact hd, hc'⟩)
        exact (IH hnodup' htopoTl hadeq' (Function.update fa T d) (hseed_step d hd)).mp hSeld c hcd
      · intro Hfull c hc
        rw [coupledCornerAssignsSel, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        rw [hsel] at hd
        have hFulld : ∀ c' ∈ coupledCornerAssigns Γ l Ts' (Function.update fa T d),
            goalAt l r p c' := fun c' hc' => Hfull c' (by
          rw [coupledCornerAssigns, List.mem_flatMap]; exact ⟨d, hd, hc'⟩)
        exact (IH hnodup' htopoTl hadeq' (Function.update fa T d) (hseed_step d hd)).mpr hFulld c hcd
    case inr.inl =>
      -- free-single-side, sub only: `sel T = [hi]`; super side independent (not live), sub monotone
      have hfree' : ∀ U ∈ Ts', ¬ DependsOn Γ U T := fun U hU => hfree U (List.mem_cons_of_mem T hU)
      have hso' : sel T (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2
          = [(fieldBoundsAt Γ fa l T).2] := hso _ _
      constructor
      · intro Hsel c hc
        rw [coupledCornerAssigns, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        have hSel_hi : ∀ c' ∈ coupledCornerAssignsSel Γ l sel Ts'
            (Function.update fa T (fieldBoundsAt Γ fa l T).2), goalAt l r p c' := fun c' hc' => Hsel c' (by
          rw [coupledCornerAssignsSel, List.mem_flatMap]
          exact ⟨(fieldBoundsAt Γ fa l T).2, by rw [hso']; exact List.mem_singleton.mpr rfl, hc'⟩)
        have Hfull_hi := (IH hnodup' htopoTl hadeq' (Function.update fa T (fieldBoundsAt Γ fa l T).2)
          (hseed_step _ hhi_mem)).mp hSel_hi
        obtain ⟨c₀, hc₀mem, hag, hcT, hc₀T⟩ :=
          match_off_T Γ l Ts' hnodup' htopoTl T hTnotin hfree' fa d (fieldBoundsAt Γ fa l T).2 hcd
        have hGc₀ : goalAt l r p c₀ := Hfull_hi c₀ hc₀mem
        have hpeq : rowFieldAt c l p = rowFieldAt c₀ l p := rowFieldAt_liveAt_irrel hag p hflatp hlivep
        -- super side independent (`T` not live there), sub side monotone — the upper cut
        exact goalCut_hi hag (by rw [hcT, hc₀T]; exact (mem_rigidCorners hd).2) hpeq hGc₀
      · intro Hfull c hc
        rw [coupledCornerAssignsSel, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        rw [hso', List.mem_singleton] at hd; subst hd
        have hFull_hi : ∀ c' ∈ coupledCornerAssigns Γ l Ts'
            (Function.update fa T (fieldBoundsAt Γ fa l T).2), goalAt l r p c' := fun c' hc' => Hfull c' (by
          rw [coupledCornerAssigns, List.mem_flatMap]; exact ⟨(fieldBoundsAt Γ fa l T).2, hhi_mem, hc'⟩)
        exact (IH hnodup' htopoTl hadeq' (Function.update fa T (fieldBoundsAt Γ fa l T).2)
          (hseed_step _ hhi_mem)).mpr hFull_hi c hcd
    case inr.inr.inl =>
      -- free-single-side, super only: `sel T = [lo]`; sub side independent (not live), super monotone
      have hfree' : ∀ U ∈ Ts', ¬ DependsOn Γ U T := fun U hU => hfree U (List.mem_cons_of_mem T hU)
      have hso' : sel T (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2
          = [(fieldBoundsAt Γ fa l T).1] := hso _ _
      constructor
      · intro Hsel c hc
        rw [coupledCornerAssigns, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        have hSel_lo : ∀ c' ∈ coupledCornerAssignsSel Γ l sel Ts'
            (Function.update fa T (fieldBoundsAt Γ fa l T).1), goalAt l r p c' := fun c' hc' => Hsel c' (by
          rw [coupledCornerAssignsSel, List.mem_flatMap]
          exact ⟨(fieldBoundsAt Γ fa l T).1, by rw [hso']; exact List.mem_singleton.mpr rfl, hc'⟩)
        have Hfull_lo := (IH hnodup' htopoTl hadeq' (Function.update fa T (fieldBoundsAt Γ fa l T).1)
          (hseed_step _ hlo_mem)).mp hSel_lo
        obtain ⟨c₀, hc₀mem, hag, hcT, hc₀T⟩ :=
          match_off_T Γ l Ts' hnodup' htopoTl T hTnotin hfree' fa d (fieldBoundsAt Γ fa l T).1 hcd
        have hGc₀ : goalAt l r p c₀ := Hfull_lo c₀ hc₀mem
        have hreq : rowFieldAt c l r = rowFieldAt c₀ l r := rowFieldAt_liveAt_irrel hag r hflatr hliver
        -- sub side independent (`T` not live there), super side monotone — the lower cut
        exact goalCut_lo hag (by rw [hcT, hc₀T]; exact (mem_rigidCorners hd).1) hreq hGc₀
      · intro Hfull c hc
        rw [coupledCornerAssignsSel, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        rw [hso', List.mem_singleton] at hd; subst hd
        have hFull_lo : ∀ c' ∈ coupledCornerAssigns Γ l Ts'
            (Function.update fa T (fieldBoundsAt Γ fa l T).1), goalAt l r p c' := fun c' hc' => Hfull c' (by
          rw [coupledCornerAssigns, List.mem_flatMap]; exact ⟨(fieldBoundsAt Γ fa l T).1, hlo_mem, hc'⟩)
        exact (IH hnodup' htopoTl hadeq' (Function.update fa T (fieldBoundsAt Γ fa l T).1)
          (hseed_step _ hlo_mem)).mpr hFull_lo c hcd
    case inr.inr.inr.inl =>
      -- masking, super masked: `sel T = [hi]`; super side independent (masked), sub monotone
      have hfree' : ∀ U ∈ Ts', ¬ DependsOn Γ U T := fun U hU => hfree U (List.mem_cons_of_mem T hU)
      have hso' : sel T (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2
          = [(fieldBoundsAt Γ fa l T).2] := hso _ _
      constructor
      · intro Hsel c hc
        rw [coupledCornerAssigns, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        have hSel_hi : ∀ c' ∈ coupledCornerAssignsSel Γ l sel Ts'
            (Function.update fa T (fieldBoundsAt Γ fa l T).2), goalAt l r p c' := fun c' hc' => Hsel c' (by
          rw [coupledCornerAssignsSel, List.mem_flatMap]
          exact ⟨(fieldBoundsAt Γ fa l T).2, by rw [hso']; exact List.mem_singleton.mpr rfl, hc'⟩)
        have Hfull_hi := (IH hnodup' htopoTl hadeq' (Function.update fa T (fieldBoundsAt Γ fa l T).2)
          (hseed_step _ hhi_mem)).mp hSel_hi
        obtain ⟨c₀, hc₀mem, hag, hcT, hc₀T⟩ :=
          match_off_T Γ l Ts' hnodup' htopoTl T hTnotin hfree' fa d (fieldBoundsAt Γ fa l T).2 hcd
        have hGc₀ : goalAt l r p c₀ := Hfull_hi c₀ hc₀mem
        -- the always-required maskers hold required values: in the tail by `coupledCornerAssigns_isReq`,
        -- in the seed by `hseedReq`
        have hRreq : ∀ u, (rowMentions p u ∧ u ≠ T ∧ AlwaysReqAt Γ l u) → (c u).isReq = true := by
          rintro u ⟨hmen, huT, hAR⟩
          by_cases huTs : u ∈ Ts'
          · exact coupledCornerAssigns_isReq Γ l u hAR Ts' hnodup' huTs (Function.update fa T d) c hcd
          · rw [coupledCornerAssigns_off Γ l Ts' _ c hcd u huTs, Function.update_of_ne huT]
            exact hseedReq u (Or.inr hmen) hAR (by rw [List.mem_cons, not_or]; exact ⟨huT, huTs⟩)
        have hpeq : rowFieldAt c l p = rowFieldAt c₀ l p :=
          rowFieldAt_liveAtR_irrel (fun u => rowMentions p u ∧ u ≠ T ∧ AlwaysReqAt Γ l u)
            hag hRreq (fun u hu => hu.2.1) p hflatp hmaskp
        -- super side independent (`T` masked there), sub side monotone — the upper cut
        exact goalCut_hi hag (by rw [hcT, hc₀T]; exact (mem_rigidCorners hd).2) hpeq hGc₀
      · intro Hfull c hc
        rw [coupledCornerAssignsSel, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        rw [hso', List.mem_singleton] at hd; subst hd
        have hFull_hi : ∀ c' ∈ coupledCornerAssigns Γ l Ts'
            (Function.update fa T (fieldBoundsAt Γ fa l T).2), goalAt l r p c' := fun c' hc' => Hfull c' (by
          rw [coupledCornerAssigns, List.mem_flatMap]; exact ⟨(fieldBoundsAt Γ fa l T).2, hhi_mem, hc'⟩)
        exact (IH hnodup' htopoTl hadeq' (Function.update fa T (fieldBoundsAt Γ fa l T).2)
          (hseed_step _ hhi_mem)).mpr hFull_hi c hcd
    case inr.inr.inr.inr =>
      -- masking, sub masked: `sel T = [lo]`; sub side independent (masked), super monotone
      have hfree' : ∀ U ∈ Ts', ¬ DependsOn Γ U T := fun U hU => hfree U (List.mem_cons_of_mem T hU)
      have hso' : sel T (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2
          = [(fieldBoundsAt Γ fa l T).1] := hso _ _
      constructor
      · intro Hsel c hc
        rw [coupledCornerAssigns, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        have hSel_lo : ∀ c' ∈ coupledCornerAssignsSel Γ l sel Ts'
            (Function.update fa T (fieldBoundsAt Γ fa l T).1), goalAt l r p c' := fun c' hc' => Hsel c' (by
          rw [coupledCornerAssignsSel, List.mem_flatMap]
          exact ⟨(fieldBoundsAt Γ fa l T).1, by rw [hso']; exact List.mem_singleton.mpr rfl, hc'⟩)
        have Hfull_lo := (IH hnodup' htopoTl hadeq' (Function.update fa T (fieldBoundsAt Γ fa l T).1)
          (hseed_step _ hlo_mem)).mp hSel_lo
        obtain ⟨c₀, hc₀mem, hag, hcT, hc₀T⟩ :=
          match_off_T Γ l Ts' hnodup' htopoTl T hTnotin hfree' fa d (fieldBoundsAt Γ fa l T).1 hcd
        have hGc₀ : goalAt l r p c₀ := Hfull_lo c₀ hc₀mem
        have hRreq : ∀ u, (rowMentions r u ∧ u ≠ T ∧ AlwaysReqAt Γ l u) → (c u).isReq = true := by
          rintro u ⟨hmen, huT, hAR⟩
          by_cases huTs : u ∈ Ts'
          · exact coupledCornerAssigns_isReq Γ l u hAR Ts' hnodup' huTs (Function.update fa T d) c hcd
          · rw [coupledCornerAssigns_off Γ l Ts' _ c hcd u huTs, Function.update_of_ne huT]
            exact hseedReq u (Or.inl hmen) hAR (by rw [List.mem_cons, not_or]; exact ⟨huT, huTs⟩)
        have hreq : rowFieldAt c l r = rowFieldAt c₀ l r :=
          rowFieldAt_liveAtR_irrel (fun u => rowMentions r u ∧ u ≠ T ∧ AlwaysReqAt Γ l u)
            hag hRreq (fun u hu => hu.2.1) r hflatr hmaskr
        -- sub side independent (`T` masked there), super side monotone — the lower cut
        exact goalCut_lo hag (by rw [hcT, hc₀T]; exact (mem_rigidCorners hd).1) hreq hGc₀
      · intro Hfull c hc
        rw [coupledCornerAssignsSel, List.mem_flatMap] at hc
        obtain ⟨d, hd, hcd⟩ := hc
        rw [hso', List.mem_singleton] at hd; subst hd
        have hFull_lo : ∀ c' ∈ coupledCornerAssigns Γ l Ts'
            (Function.update fa T (fieldBoundsAt Γ fa l T).1), goalAt l r p c' := fun c' hc' => Hfull c' (by
          rw [coupledCornerAssigns, List.mem_flatMap]; exact ⟨(fieldBoundsAt Γ fa l T).1, hlo_mem, hc'⟩)
        exact (IH hnodup' htopoTl hadeq' (Function.update fa T (fieldBoundsAt Γ fa l T).1)
          (hseed_step _ hlo_mem)).mpr hFull_lo c hcd



/- ---------------------------------------------------------------------------------------------- -/
/- ## 71. The coupled characterization against the selected enumeration                           -/
/- ---------------------------------------------------------------------------------------------- -/


/-- **Coupled characterization, selected enumeration.**  The §63 characterization with the cheaper
    selected enumeration in place of the full one: for a *per-label* adequate selector `selAt` with
    consistent boxes, `SemSubRow` is equivalent to the per-label goal holding at every *selected*
    corner.  Obtained by composing the full-enumeration characterization
    (`semSubRow_iff_coupledCorners`) with the selected-vs-full equivalence (`goalcheck_sel_iff`) at
    each checked label.  The selector is indexed by the label because the implementation recomputes its
    live sets per label (`live_spread_at sub label`). -/
theorem semSubRow_iff_coupledCornersSel (r p : Row) (Γ : TyParamEnv) (Ts : List Ty_param)
    (selAt : Option Label → Ty_param → FieldDesc → FieldDesc → List FieldDesc)
    (dflt : Ty_param → FieldDesc) (lf : Label)
    (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (hclosed : BoundClosed Γ Ts)
    (hr : GroundRow r) (hp : GroundRow p)
    (hdecl : ∀ U ∈ Ts, ∃ bs, Γ U = some bs) (hTs_all : ∀ α b, Γ α = some b → α ∈ Ts)
    (hground : ∀ T b, Γ T = some b → GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper))
    (hnb : ∀ T b, Γ T = some b → NotBotRow (rowOfBase b.lower) ∧ NotBotRow (rowOfBase b.upper))
    (hcons : ∀ (l : Option Label) (U : Ty_param) (acc' : Ty_param → FieldDesc),
      SubBase (fieldBoundsAt Γ acc' l U).1.base (fieldBoundsAt Γ acc' l U).2.base)
    (hconsist : ∀ T b, Γ T = some b → ∀ ρ,
      SubField_row (evalRow ρ (rowOfBase b.lower)) (evalRow ρ (rowOfBase b.upper)))
    (hdflt : ∀ α, RigidFreeField (dflt α))
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (hlf : lf ∉ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts)
    (hboxcons : ∀ (l : Option Label) (fa : Ty_param → FieldDesc) (T : Ty_param),
      SubField (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2)
    (hadeq : ∀ l₀ ∈ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts ++ [lf],
      ∀ T ∈ Ts, SelAdequate Γ (some l₀) r p (selAt (some l₀)) Ts T) :
    SemSubRow r p Γ ↔
      ∀ l₀ ∈ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts ++ [lf],
        ∀ c ∈ coupledCornerAssignsSel Γ (some l₀) (selAt (some l₀)) Ts dflt,
          SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p) := by
  -- full-enumeration characterization, then swap the full check for the selected one at each label
  refine (semSubRow_iff_coupledCorners r p Γ Ts dflt lf hnodup htopo hclosed hr hp hdecl hTs_all
    hground hnb hcons hconsist hdflt hrelr hrelp hlf).trans ?_
  refine forall_congr' (fun l₀ => imp_congr_right (fun hl₀ => ?_))
  -- the seed-required invariant is vacuous at the full list: every mentioned parameter is in `Ts`
  exact (goalcheck_sel_iff Γ (some l₀) r p (selAt (some l₀)) (hboxcons (some l₀)) Ts hnodup htopo
    (hadeq l₀ hl₀) dflt
    (fun u hmen _ huts => absurd (Or.elim hmen (hrelr u) (hrelp u)) huts)).symm


/- ---------------------------------------------------------------------------------------------- -/
/- ## 72. The `corners_for` selector                                                              -/
/- ---------------------------------------------------------------------------------------------- -/


open Classical in
/-- The implementation's `corners_for` (`splat.ml`), free-single-side and masking cuts.  A *free*
    parameter (no parameter in `Ts` depends on it) live in only the sub-row keeps just its upper
    endpoint; live in only the super-row keeps just its lower endpoint; masked on the super-row (by an
    always-required parameter to its right) keeps the upper endpoint; masked on the sub-row keeps the
    lower endpoint; everything else keeps the full corners.  The first two are `corners_for`'s
    `is_free && in_sub && ¬in_super → [upper]` / `is_free && ¬in_sub && in_super → [lower]`; the next
    two are its both-sides masking branch (`masking_super = Masked → [upper]`,
    `masking_sub = Masked → [lower]`), restricted to *definite* always-required maskers (`AlwaysReqAt`).
    Noncomputable because the classification branches on the propositional liveness/dependency facts
    directly (as does the surrounding `decidableSemSubRow_coupled`). -/
noncomputable def cornersFor (Γ : TyParamEnv) (r p : Row) (Ts : List Ty_param)
    (l : Option Label) (T : Ty_param) (lo hi : FieldDesc) : List FieldDesc :=
  if (∀ U ∈ Ts, ¬ DependsOn Γ U T) ∧ RowLiveAt l T r ∧ ¬ RowLiveAt l T p then [hi]
  else if (∀ U ∈ Ts, ¬ DependsOn Γ U T) ∧ ¬ RowLiveAt l T r ∧ RowLiveAt l T p then [lo]
  else if (∀ U ∈ Ts, ¬ DependsOn Γ U T)
      ∧ ¬ RowLiveAtR (fun u => rowMentions p u ∧ u ≠ T ∧ AlwaysReqAt Γ l u) l T p then [hi]
  else if (∀ U ∈ Ts, ¬ DependsOn Γ U T)
      ∧ ¬ RowLiveAtR (fun u => rowMentions r u ∧ u ≠ T ∧ AlwaysReqAt Γ l u) l T r then [lo]
  else rigidCorners lo hi

/-- `cornersFor` is an adequate selector at every parameter (for flat rows): each of its five branches
    is one of `SelAdequate`'s disjuncts, with the branch condition supplying exactly the freeness and
    the one-sided independence (non-liveness or masking) that disjunct requires. -/
theorem cornersFor_adequate (Γ : TyParamEnv) (r p : Row) (Ts : List Ty_param) (l : Option Label)
    (hflatr : RowNoNested r) (hflatp : RowNoNested p) (T : Ty_param) :
    SelAdequate Γ l r p (cornersFor Γ r p Ts l) Ts T := by
  by_cases h1 : (∀ U ∈ Ts, ¬ DependsOn Γ U T) ∧ RowLiveAt l T r ∧ ¬ RowLiveAt l T p
  case pos =>
    -- free, live in sub only ⇒ upper endpoint
    exact Or.inr (Or.inl ⟨fun lo hi => by unfold cornersFor; rw [if_pos h1], h1.1, h1.2.2, hflatp⟩)
  case neg =>
    by_cases h2 : (∀ U ∈ Ts, ¬ DependsOn Γ U T) ∧ ¬ RowLiveAt l T r ∧ RowLiveAt l T p
    case pos =>
      -- free, live in super only ⇒ lower endpoint
      exact Or.inr (Or.inr (Or.inl
        ⟨fun lo hi => by unfold cornersFor; rw [if_neg h1, if_pos h2], h2.1, h2.2.1, hflatr⟩))
    case neg =>
      by_cases h3 : (∀ U ∈ Ts, ¬ DependsOn Γ U T)
          ∧ ¬ RowLiveAtR (fun u => rowMentions p u ∧ u ≠ T ∧ AlwaysReqAt Γ l u) l T p
      case pos =>
        -- free, masked on super ⇒ upper endpoint
        exact Or.inr (Or.inr (Or.inr (Or.inl
          ⟨fun lo hi => by unfold cornersFor; rw [if_neg h1, if_neg h2, if_pos h3], h3.1, h3.2, hflatp⟩)))
      case neg =>
        by_cases h4 : (∀ U ∈ Ts, ¬ DependsOn Γ U T)
            ∧ ¬ RowLiveAtR (fun u => rowMentions r u ∧ u ≠ T ∧ AlwaysReqAt Γ l u) l T r
        case pos =>
          -- free, masked on sub ⇒ lower endpoint
          exact Or.inr (Or.inr (Or.inr (Or.inr
            ⟨fun lo hi => by unfold cornersFor; rw [if_neg h1, if_neg h2, if_neg h3, if_pos h4],
              h4.1, h4.2, hflatr⟩)))
        case neg =>
          -- otherwise the full corners
          exact Or.inl (fun lo hi => by unfold cornersFor; rw [if_neg h1, if_neg h2, if_neg h3, if_neg h4])

/-- **The `corners_for` check decides `SemSubRow`.**  Specialises `semSubRow_iff_coupledCornersSel`
    to the `cornersFor` selector, discharging adequacy by `cornersFor_adequate` (needing only that the
    rows are flat).  So the implementation's free-single-side and (definite) masking corner cuts are
    faithful: the enumeration they produce decides `SemSubRow` exactly as the full enumeration does. -/
theorem semSubRow_iff_cornersFor (r p : Row) (Γ : TyParamEnv) (Ts : List Ty_param)
    (dflt : Ty_param → FieldDesc) (lf : Label)
    (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (hclosed : BoundClosed Γ Ts)
    (hr : GroundRow r) (hp : GroundRow p)
    (hdecl : ∀ U ∈ Ts, ∃ bs, Γ U = some bs) (hTs_all : ∀ α b, Γ α = some b → α ∈ Ts)
    (hground : ∀ T b, Γ T = some b → GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper))
    (hnb : ∀ T b, Γ T = some b → NotBotRow (rowOfBase b.lower) ∧ NotBotRow (rowOfBase b.upper))
    (hcons : ∀ (l : Option Label) (U : Ty_param) (acc' : Ty_param → FieldDesc),
      SubBase (fieldBoundsAt Γ acc' l U).1.base (fieldBoundsAt Γ acc' l U).2.base)
    (hconsist : ∀ T b, Γ T = some b → ∀ ρ,
      SubField_row (evalRow ρ (rowOfBase b.lower)) (evalRow ρ (rowOfBase b.upper)))
    (hdflt : ∀ α, RigidFreeField (dflt α))
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (hlf : lf ∉ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts)
    (hboxcons : ∀ (l : Option Label) (fa : Ty_param → FieldDesc) (T : Ty_param),
      SubField (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2)
    (hflatr : RowNoNested r) (hflatp : RowNoNested p) :
    SemSubRow r p Γ ↔
      ∀ l₀ ∈ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts ++ [lf],
        ∀ c ∈ coupledCornerAssignsSel Γ (some l₀) (cornersFor Γ r p Ts (some l₀)) Ts dflt,
          SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p) :=
  semSubRow_iff_coupledCornersSel r p Γ Ts (fun l => cornersFor Γ r p Ts l) dflt lf
    hnodup htopo hclosed hr hp hdecl hTs_all hground hnb hcons hconsist hdflt hrelr hrelp hlf hboxcons
    (fun l₀ _ T _ => cornersFor_adequate Γ r p Ts (some l₀) hflatr hflatp T)


/- ---------------------------------------------------------------------------------------------- -/
/- ## 73. Liveness pruning: a non-live parameter can be dropped from the enumeration              -/
/- ---------------------------------------------------------------------------------------------- -/


/-- **Liveness pruning.**  A *free* parameter (no parameter in `Ts'` depends on it) that is live in
    neither row can be removed from the enumeration list altogether without changing the per-label
    check.  The implementation prunes exactly these — its `ty_params_topo` is the live closure, omitting
    such parameters.  The check is unchanged because the parameter's field never reaches either
    projection (`rowFieldAt_liveAt_irrel` on both sides), so every `(T :: Ts')`-corner agrees with an
    `Ts'`-corner off `T` (`match_off_T`) at an equal goal value.

    (The `cornersFor` selector already cuts such a parameter to a single corner — `¬ RowLiveAt`
    implies `¬ RowLiveAtR`, so the masking branch fires — which has the same cost; this lemma is the
    list-level form, matching the implementation's literal omission.) -/
theorem goalcheck_drop_live (Γ : TyParamEnv) (l : Option Label) (r p : Row)
    (T : Ty_param) (Ts' : List Ty_param)
    (hnodup' : Ts'.Nodup) (htopo' : TopoSorted Γ Ts') (hTnotin : T ∉ Ts')
    (hfree : ∀ U ∈ Ts', ¬ DependsOn Γ U T)
    (hliver : ¬ RowLiveAt l T r) (hlivep : ¬ RowLiveAt l T p)
    (hflatr : RowNoNested r) (hflatp : RowNoNested p)
    (fa : Ty_param → FieldDesc)
    (hboxcons : SubField (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2) :
    (∀ c ∈ coupledCornerAssigns Γ l (T :: Ts') fa, goalAt l r p c) ↔
    (∀ c ∈ coupledCornerAssigns Γ l Ts' fa, goalAt l r p c) := by
  -- the goal is the same at two assignments differing only at the non-live `T`
  have hindep : ∀ {c c₀ : Ty_param → FieldDesc}, (∀ β, β ≠ T → c β = c₀ β) →
      (goalAt l r p c ↔ goalAt l r p c₀) := by
    intro c c₀ hag
    show SubField (rowFieldAt c l r) (rowFieldAt c l p)
       ↔ SubField (rowFieldAt c₀ l r) (rowFieldAt c₀ l p)
    rw [rowFieldAt_liveAt_irrel hag r hflatr hliver, rowFieldAt_liveAt_irrel hag p hflatp hlivep]
  have hlo_mem : (fieldBoundsAt Γ fa l T).1 ∈
      rigidCorners (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2 := by
    have h := mem_rigidCorners_lo (subField_refl (fieldBoundsAt Γ fa l T).1) hboxcons
    rwa [setBase_self] at h
  constructor
  · -- forward: a dropped-list corner matches the `box.1`-branch corner off `T`
    intro Hfull ct hct
    have hct' : ct ∈ coupledCornerAssigns Γ l Ts' (Function.update fa T (fa T)) := by
      rw [Function.update_eq_self]; exact hct
    obtain ⟨c₀, hc₀mem, hag, _, _⟩ :=
      match_off_T Γ l Ts' hnodup' htopo' T hTnotin hfree fa (fa T) (fieldBoundsAt Γ fa l T).1 hct'
    exact (hindep hag).mpr (Hfull c₀ (by
      rw [coupledCornerAssigns, List.mem_flatMap]; exact ⟨_, hlo_mem, hc₀mem⟩))
  · -- backward: a `(T :: Ts')`-corner matches the dropped-list corner off `T`
    intro Hdrop c hc
    rw [coupledCornerAssigns, List.mem_flatMap] at hc
    obtain ⟨d, _, hcd⟩ := hc
    obtain ⟨c₀, hc₀mem, hag, _, _⟩ :=
      match_off_T Γ l Ts' hnodup' htopo' T hTnotin hfree fa d (fa T) hcd
    rw [Function.update_eq_self] at hc₀mem
    exact (hindep hag).mpr (Hdrop c₀ hc₀mem)


end Splat
