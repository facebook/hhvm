import Splat.Subrow.Decide

/-!
# The coupled topological corner enumeration

For *coupled* bounds — a parameter's bound row mentions other parameters — its field box
`fieldBoundsAt Γ g l α` depends on the assignment `g` at those other parameters, so the corner
enumeration must proceed in *topological order*: each parameter's box is computed under the corners
already chosen for the parameters its bounds depend on.

This file defines that topological dependency structure (`DependsOn`/`TopoSorted`/`BoundClosed`) and
the enumeration `coupledCornerAssigns` (mirroring the implementation's `corner_assignments`) — the
coupled counterpart of `cornerAssigns` (§37), whose box was fixed.  Per the design, the topological
order is supplied as a top-level faithfulness hypothesis (`TopoSorted`) rather than computed:
acyclicity guarantees such an order exists, and is assumed at the top exactly as the well-formedness
condition is.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 41. The topological dependency structure and the coupled enumeration                        -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Parameter `T`'s *bounds* mention `α`: `α` occurs (at spread position) in `T`'s lower- or
    upper-bound row.  This is the dependency that couples `T`'s field box to `α`'s field. -/
def DependsOn (Γ : TyParamEnv) (T α : Ty_param) : Prop :=
  ∃ b, Γ T = some b ∧ (rowMentions (rowOfBase b.lower) α ∨ rowMentions (rowOfBase b.upper) α)

/-- A parameter list is **topologically sorted** for `Γ` when each parameter's bound-dependencies all
    lie strictly earlier: the head `T` depends on nothing in `T :: rest` — neither itself nor any
    later parameter — recursively.  Processing the list front-to-back, every parameter's dependencies
    are therefore already assigned by the time it is reached. -/
def TopoSorted (Γ : TyParamEnv) : List Ty_param → Prop
  | []        => True
  | T :: rest => (∀ α, DependsOn Γ T α → α ∉ T :: rest) ∧ TopoSorted Γ rest

/-- A parameter list is **bound-closed** for `Γ` when it contains every parameter any of its members'
    bounds depend on — so the enumeration assigns a ground corner to each dependency (rather than
    falling back to a default), keeping the chosen boxes ρ-independent. -/
def BoundClosed (Γ : TyParamEnv) (Ts : List Ty_param) : Prop :=
  ∀ T ∈ Ts, ∀ α, DependsOn Γ T α → α ∈ Ts

variable [∀ a b : Base, Decidable (SubBase a b)]

/-- The **coupled corner enumeration** (mirrors the implementation's `corner_assignments`).  Walks the
    topologically-sorted `Ts` front-to-back, threading a field assignment `fa`: at each `T`, compute
    its field box `fieldBoundsAt Γ fa l T` *under the corners already chosen* (`fa`), branch over that
    box's `rigidCorners`, update `fa` at `T`, and recurse.  Unlike `cornerAssigns` (§37, fixed box),
    the box here shifts as earlier corners are chosen — the coupling. -/
def coupledCornerAssigns (Γ : TyParamEnv) (l : Option Label) :
    List Ty_param → (Ty_param → FieldDesc) → List (Ty_param → FieldDesc)
  | [],      fa => [fa]
  | T :: Ts, fa =>
      (rigidCorners (fieldBoundsAt Γ fa l T).1 (fieldBoundsAt Γ fa l T).2).flatMap
        (fun d => coupledCornerAssigns Γ l Ts (Function.update fa T d))

/- ---------------------------------------------------------------------------------------------- -/
/- ## 42. Requiredness invariance under base-only changes                                         -/
/- ---------------------------------------------------------------------------------------------- -/

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- `mergeField`'s output requiredness is the `or` of its inputs' — the base is irrelevant. -/
theorem mergeField_isReq (a b : FieldDesc) : (mergeField a b).isReq = (b.isReq || a.isReq) := by
  cases b with
  | req _ => rfl
  | opt _ => cases a <;> rfl

/- A row's per-label field requiredness depends on the assignment only through each parameter's
   requiredness (never its base): `mergeField`'s requiredness is base-blind (`mergeField_isReq`),
   every leaf requiredness is assignment-independent, and the spread-parameter arm reads `g α` only
   at `.isReq`.  Same mutual shape as the relevance block (§28). -/
omit [∀ a b : Base, Decidable (SubBase a b)] in
mutual
  theorem baseFieldAt_isReq_congr {g g' : Ty_param → FieldDesc}
      (h : ∀ α, (g α).isReq = (g' α).isReq) (l : Option Label) :
      ∀ b : Base, (baseFieldAt g l b).isReq = (baseFieldAt g' l b).isReq
    | .rigid β              => h β
    | .bot                  => rfl
    | .top                  => rfl
    | .prim _               => rfl
    | .union _ _            => rfl
    | .shape (.simple _ _)  => rfl
    | .shape (.splat es)    => splatFieldAt_isReq_congr h l es

  theorem elemFieldAt_isReq_congr {g g' : Ty_param → FieldDesc}
      (h : ∀ α, (g α).isReq = (g' α).isReq) (l : Option Label) :
      ∀ e : SplatElem, (elemFieldAt g l e).isReq = (elemFieldAt g' l e).isReq
    | .spread b => baseFieldAt_isReq_congr h l b

  theorem splatFieldAt_isReq_congr {g g' : Ty_param → FieldDesc}
      (h : ∀ α, (g α).isReq = (g' α).isReq) (l : Option Label) :
      ∀ es : List SplatElem, (splatFieldAt g l es).isReq = (splatFieldAt g' l es).isReq
    | []      => rfl
    | e :: es =>
        splatFoldAt_isReq_congr h l es (elemFieldAt g l e) (elemFieldAt g' l e)
          (elemFieldAt_isReq_congr h l e)

  theorem splatFoldAt_isReq_congr {g g' : Ty_param → FieldDesc}
      (h : ∀ α, (g α).isReq = (g' α).isReq) (l : Option Label) :
      ∀ (es : List SplatElem) (acc acc' : FieldDesc), acc.isReq = acc'.isReq →
        (splatFoldAt g l es acc).isReq = (splatFoldAt g' l es acc').isReq
    | [],      _,   _,    hacc => hacc
    | e :: es, acc, acc', hacc =>
        splatFoldAt_isReq_congr h l es (mergeField acc (elemFieldAt g l e))
          (mergeField acc' (elemFieldAt g' l e))
          (by rw [mergeField_isReq, mergeField_isReq, hacc, elemFieldAt_isReq_congr h l e])
end

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- **Requiredness invariance.**  Two assignments with the same per-parameter requiredness give a row
    the same per-label requiredness — so moving each parameter to a base-corner (keeping requiredness)
    leaves every box's requiredness interval fixed, even as its bases shift. -/
theorem rowFieldAt_isReq_congr {g g' : Ty_param → FieldDesc}
    (h : ∀ α, (g α).isReq = (g' α).isReq) (l : Option Label) (r : Row) :
    (rowFieldAt g l r).isReq = (rowFieldAt g' l r).isReq := by
  cases r with
  | simple _ _ => rfl
  | splat es   => exact splatFieldAt_isReq_congr h l es

/- ---------------------------------------------------------------------------------------------- -/
/- ## 43. Shifted-box corner membership                                                           -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Shifted-box corner membership.**  A tracked leaf is an enumerated corner after the box shifts to
    an earlier parameter's chosen corner.  With `c` in a box `[lo, hi]` and a *base-shifted* box
    `[lo', hi']` of the **same** requirednesses (`hreqlo`/`hreqhi` — guaranteed by §42, since the shift
    moves only bases) that is consistent (`lo'.base <: hi'.base`), the leaf `c.setBase lo'.base`
    (resp. `c.setBase hi'.base`) — keeping `c`'s requiredness, landing on the shifted endpoint base —
    is a corner of `[lo', hi']`.  `c`'s requiredness still fits because the req-interval is unchanged. -/
theorem setBase_mem_rigidCorners_shift {lo hi lo' hi' c : FieldDesc}
    (hc1 : SubField lo c) (hc2 : SubField c hi)
    (hreqlo : lo.isReq = lo'.isReq) (hreqhi : hi.isReq = hi'.isReq)
    (hcons : SubBase lo'.base hi'.base) :
    c.setBase lo'.base ∈ rigidCorners lo' hi' ∧ c.setBase hi'.base ∈ rigidCorners lo' hi' := by
  -- the leaf at base `b` (between the shifted bases) sits in the shifted box
  have key : ∀ b : Base, SubBase lo'.base b → SubBase b hi'.base →
      SubField lo' (c.setBase b) ∧ SubField (c.setBase b) hi' := by
    intro b hb1 hb2
    cases c with
    | req ct =>
      -- `lo <: .req ct` forces `lo` req, hence `lo'` req (`hreqlo`); `c.setBase b = .req b`
      cases hc1
      cases lo' with
      | opt _ => simp [FieldDesc.isReq] at hreqlo
      | req _ =>
        refine ⟨SubField.req_req hb1, ?_⟩
        cases hi' with
        | req _ => exact SubField.req_req hb2
        | opt _ => exact SubField.req_opt hb2
    | opt ct =>
      -- `.opt ct <: hi` forces `hi` opt, hence `hi'` opt (`hreqhi`); `c.setBase b = .opt b`
      cases hc2
      cases hi' with
      | req _ => simp [FieldDesc.isReq] at hreqhi
      | opt _ =>
        refine ⟨?_, SubField.opt_opt hb2⟩
        cases lo' with
        | req _ => exact SubField.req_opt hb1
        | opt _ => exact SubField.opt_opt hb1
  -- a leaf is a filtered candidate of `rigidCorners`
  refine ⟨?_, ?_⟩
  · rw [rigidCorners, List.mem_filter]
    refine ⟨?_, decide_eq_true (key lo'.base SubBase.refl hcons)⟩
    rcases FieldDesc.setBase_cases c lo'.base with h | h <;> rw [h] <;> simp
  · rw [rigidCorners, List.mem_filter]
    refine ⟨?_, decide_eq_true (key hi'.base hcons SubBase.refl)⟩
    rcases FieldDesc.setBase_cases c hi'.base with h | h <;> rw [h] <;> simp

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- The coupled box's requirednesses depend on the assignment only through each parameter's
    requiredness (§42 lifted to the bound rows).  So a base-only shift of the assignment leaves the
    box's requiredness interval fixed — exactly the `hreqlo`/`hreqhi` hypotheses of
    `setBase_mem_rigidCorners_shift` (§43). -/
theorem fieldBoundsAt_isReq_congr {g g' : Ty_param → FieldDesc}
    (h : ∀ α, (g α).isReq = (g' α).isReq) (Γ : TyParamEnv) (l : Option Label) (T : Ty_param) :
    (fieldBoundsAt Γ g l T).1.isReq = (fieldBoundsAt Γ g' l T).1.isReq ∧
    (fieldBoundsAt Γ g l T).2.isReq = (fieldBoundsAt Γ g' l T).2.isReq := by
  unfold fieldBoundsAt
  cases Γ T with
  | none   => exact ⟨rfl, rfl⟩
  | some b => exact ⟨rowFieldAt_isReq_congr h l (rowOfBase b.lower),
                     rowFieldAt_isReq_congr h l (rowOfBase b.upper)⟩

/- ---------------------------------------------------------------------------------------------- -/
/- ## 45. `setBase` and `FEquiv`                                                                  -/
/- ---------------------------------------------------------------------------------------------- -/

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- `setBase` to mutually-related bases gives `FEquiv` fields (only the base moves, within `≈`). -/
theorem setBase_fequiv (X : FieldDesc) {B B' : Base} (h1 : SubBase B B') (h2 : SubBase B' B) :
    FEquiv (X.setBase B) (X.setBase B') := by
  cases X with
  | req _ => exact ⟨SubField.req_req h1, SubField.req_req h2⟩
  | opt _ => exact ⟨SubField.opt_opt h1, SubField.opt_opt h2⟩

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- `setBase` to the bases of `FEquiv` fields gives `FEquiv` fields — the tracker reproduces a corner
    only up to `FEquiv`, since the bound's projection is only `FEquiv` (not equal) to its combinator. -/
theorem setBase_base_fequiv (X : FieldDesc) {a b : FieldDesc} (h : FEquiv a b) :
    FEquiv (X.setBase a.base) (X.setBase b.base) :=
  setBase_fequiv X (subField_base h.1) (subField_base h.2)

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- `setBase` keeps requiredness. -/
@[simp] theorem setBase_isReq (X : FieldDesc) (B : Base) : (X.setBase B).isReq = X.isReq := by
  cases X <;> rfl

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- `.base` commutes with `cond`. -/
theorem cond_base (b : Bool) (X Y : FieldDesc) : cond b X.base Y.base = (cond b X Y).base := by
  cases b <;> rfl

/- ---------------------------------------------------------------------------------------------- -/
/- ## 46. The tagged corner builder                                                               -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The **tagged corner** the peel lands on: walking the topologically-sorted `Ts` front-to-back,
    each parameter `T` is set to `(acc T).setBase` of the `tag`-selected endpoint base of its box
    `fieldBoundsAt Γ acc l T` — keeping `acc T`'s requiredness (the peel only moves bases), the box
    taken under the corners already chosen.  This is `coupledCornerAssigns` restricted to the
    requiredness-preserving base-corner at each parameter, indexed by an explicit `tag : Ty_param →
    Bool` (`false`/`true` = lower/upper endpoint) — so the lo/hi choice the tracker must follow is
    carried, not recovered. -/
def cornerStep (Γ : TyParamEnv) (l : Option Label) (tag : Ty_param → Bool) :
    List Ty_param → (Ty_param → FieldDesc) → (Ty_param → FieldDesc)
  | [],      acc => acc
  | T :: Ts, acc =>
      cornerStep Γ l tag Ts (Function.update acc T
        ((acc T).setBase (cond (tag T) (fieldBoundsAt Γ acc l T).2.base
                                       (fieldBoundsAt Γ acc l T).1.base)))

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- The builder leaves parameters outside the list untouched. -/
theorem cornerStep_notMem (Γ : TyParamEnv) (l : Option Label) (tag : Ty_param → Bool) :
    ∀ (Ts : List Ty_param) (acc : Ty_param → FieldDesc) (U : Ty_param), U ∉ Ts →
      cornerStep Γ l tag Ts acc U = acc U
  | [],      _,   _, _  => rfl
  | T :: Ts, acc, U, hU => by
      rw [List.mem_cons, not_or] at hU
      show cornerStep Γ l tag Ts (Function.update acc T
        ((acc T).setBase (cond (tag T) (fieldBoundsAt Γ acc l T).2.base
                                       (fieldBoundsAt Γ acc l T).1.base))) U = acc U
      rw [cornerStep_notMem Γ l tag Ts _ U hU.2, Function.update_of_ne hU.1]

/- ---------------------------------------------------------------------------------------------- -/
/- ## 47. Tagged corners are enumerated corners                                                   -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **A tagged corner is an enumerated corner.**  `cornerStep` lands in `coupledCornerAssigns`: at each
    step the chosen base-corner (keeping `g`'s requiredness) of the shifted box `box@acc` is a member of
    `rigidCorners (box@acc)` by `setBase_mem_rigidCorners_shift` (§43) — `g T` sits in `box@g`, the box's
    requirednesses are unchanged from `box@g` (since `acc` is a base-shift of `g`, §44) and it is
    consistent.  So the finite `coupledCornerAssigns` check covers every tagged corner the peel needs. -/
theorem cornerStep_mem (Γ : TyParamEnv) (l : Option Label) (tag : Ty_param → Bool)
    (g : Ty_param → FieldDesc)
    (hcons : ∀ (U : Ty_param) (acc' : Ty_param → FieldDesc),
      SubBase (fieldBoundsAt Γ acc' l U).1.base (fieldBoundsAt Γ acc' l U).2.base)
    (hinbox : ∀ U, SubField (fieldBoundsAt Γ g l U).1 (g U) ∧ SubField (g U) (fieldBoundsAt Γ g l U).2) :
    ∀ (Ts : List Ty_param), Ts.Nodup → ∀ (acc : Ty_param → FieldDesc),
      (∀ U ∈ Ts, acc U = g U) → (∀ α, (acc α).isReq = (g α).isReq) →
      cornerStep Γ l tag Ts acc ∈ coupledCornerAssigns Γ l Ts acc
  | [],      _,      _,   _,       _        => by
      rw [coupledCornerAssigns]; exact List.mem_singleton.mpr rfl
  | T :: Ts, hnodup, acc, hacc_eq, hacc_req => by
    have hTnotin : T ∉ Ts := (List.nodup_cons.mp hnodup).1
    have hnodup' : Ts.Nodup := (List.nodup_cons.mp hnodup).2
    have haccT : acc T = g T := hacc_eq T List.mem_cons_self
    -- the box at `acc` has the same requirednesses as the box at `g` (base-only shift), and is consistent
    have hreq := fieldBoundsAt_isReq_congr (fun α => (hacc_req α).symm) Γ l T
    have h43 := setBase_mem_rigidCorners_shift (hinbox T).1 (hinbox T).2 hreq.1 hreq.2 (hcons T acc)
    -- the recursion: any base-corner keeping `g T`'s requiredness extends to an enumerated corner
    have hrec : ∀ dT : FieldDesc, dT.isReq = (g T).isReq →
        cornerStep Γ l tag Ts (Function.update acc T dT)
          ∈ coupledCornerAssigns Γ l Ts (Function.update acc T dT) := by
      intro dT hdTreq
      refine cornerStep_mem Γ l tag g hcons hinbox Ts hnodup' (Function.update acc T dT) ?_ ?_
      · intro U hU
        rw [Function.update_of_ne (by rintro rfl; exact hTnotin hU)]
        exact hacc_eq U (List.mem_cons_of_mem _ hU)
      · intro α
        by_cases hα : α = T
        · subst hα; rw [Function.update_self]; exact hdTreq
        · rw [Function.update_of_ne hα]; exact hacc_req α
    show cornerStep Γ l tag Ts (Function.update acc T ((acc T).setBase
      (cond (tag T) (fieldBoundsAt Γ acc l T).2.base (fieldBoundsAt Γ acc l T).1.base)))
      ∈ coupledCornerAssigns Γ l (T :: Ts) acc
    rw [coupledCornerAssigns, List.mem_flatMap, haccT]
    cases htag : tag T with
    | false =>
      -- lower endpoint base-corner
      refine ⟨(g T).setBase (fieldBoundsAt Γ acc l T).1.base, h43.1, ?_⟩
      exact hrec _ (by rw [setBase_isReq])
    | true =>
      -- upper endpoint base-corner
      refine ⟨(g T).setBase (fieldBoundsAt Γ acc l T).2.base, h43.2, ?_⟩
      exact hrec _ (by rw [setBase_isReq])

/- ---------------------------------------------------------------------------------------------- -/
/- ## 48. Self-consistency of the tagged corner                                                   -/
/- ---------------------------------------------------------------------------------------------- -/

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- A parameter's box depends on the assignment only at the parameters its bounds mention
    (`DependsOn`).  Immediate from relevance (`rowFieldAt_congr`, §28) on the two bound rows. -/
theorem fieldBoundsAt_congr (Γ : TyParamEnv) (l : Option Label) (T : Ty_param)
    {g g' : Ty_param → FieldDesc} (h : ∀ α, DependsOn Γ T α → g α = g' α) :
    fieldBoundsAt Γ g l T = fieldBoundsAt Γ g' l T := by
  cases hΓ : Γ T with
  | none   => simp only [fieldBoundsAt, hΓ]
  | some b =>
    simp only [fieldBoundsAt, hΓ]
    rw [rowFieldAt_congr l (rowOfBase b.lower) (fun α ha => h α ⟨b, hΓ, Or.inl ha⟩),
        rowFieldAt_congr l (rowOfBase b.upper) (fun α ha => h α ⟨b, hΓ, Or.inr ha⟩)]

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- One unfolding step of the corner builder (definitional). -/
theorem cornerStep_cons (Γ : TyParamEnv) (l : Option Label) (tag : Ty_param → Bool)
    (T : Ty_param) (rest : List Ty_param) (acc : Ty_param → FieldDesc) :
    cornerStep Γ l tag (T :: rest) acc
      = cornerStep Γ l tag rest (Function.update acc T ((acc T).setBase
          (cond (tag T) (fieldBoundsAt Γ acc l T).2.base (fieldBoundsAt Γ acc l T).1.base))) := rfl

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- **Self-consistency of the tagged corner** (the `buildEnv_self` analog).  Each parameter's value is
    the `tag`-selected base-corner of its box computed under the *final* assignment — not just the box
    at its own step.  Sound because the box reads only the parameter's dependencies, which are
    topologically earlier (`TopoSorted`): at the head they lie outside `Ts` (so the rest of the fold,
    which only touches `Ts`, leaves them — and the box — fixed); below the head the IH applies. -/
theorem cornerStep_self (Γ : TyParamEnv) (l : Option Label) (tag : Ty_param → Bool) :
    ∀ (Ts : List Ty_param), Ts.Nodup → TopoSorted Γ Ts →
      ∀ (acc : Ty_param → FieldDesc) (T : Ty_param), T ∈ Ts →
        cornerStep Γ l tag Ts acc T = (acc T).setBase
          (cond (tag T) (fieldBoundsAt Γ (cornerStep Γ l tag Ts acc) l T).2.base
                        (fieldBoundsAt Γ (cornerStep Γ l tag Ts acc) l T).1.base)
  | T' :: rest, hnodup, htopo, acc, T, hT => by
    have hT'notin : T' ∉ rest := (List.nodup_cons.mp hnodup).1
    have hnodup' : rest.Nodup := (List.nodup_cons.mp hnodup).2
    obtain ⟨htopoHd, htopoTl⟩ : (∀ α, DependsOn Γ T' α → α ∉ T' :: rest) ∧ TopoSorted Γ rest := htopo
    by_cases hTT' : T = T'
    · -- head: `T`'s value is set now; the rest of the fold leaves `T`'s box fixed
      subst hTT'
      have hFT : cornerStep Γ l tag (T :: rest) acc T
          = (acc T).setBase (cond (tag T) (fieldBoundsAt Γ acc l T).2.base
                                          (fieldBoundsAt Γ acc l T).1.base) := by
        rw [cornerStep_cons, cornerStep_notMem Γ l tag rest _ T hT'notin, Function.update_self]
      rw [hFT, fieldBoundsAt_congr Γ l T (fun α hdep =>
            (cornerStep_notMem Γ l tag (T :: rest) acc α (htopoHd α hdep)).symm)]
    · -- below the head: the IH on `rest` (with `T'` already chosen)
      have hTrest : T ∈ rest := (List.mem_cons.mp hT).resolve_left hTT'
      rw [cornerStep_cons, cornerStep_self Γ l tag rest hnodup' htopoTl _ T hTrest,
          Function.update_of_ne hTT']

/- ---------------------------------------------------------------------------------------------- -/
/- ## 49. The symbolic tracker assignment                                                         -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The bound row a sink tracks: its upper (`tag = true`) or lower (`false`) bound row.  An undeclared
    parameter gets a junk empty row (never reached — sinks are declared). -/
def boundRow (Γ : TyParamEnv) (b : Bool) (U : Ty_param) : Row :=
  match Γ U with
  | some bs => cond b (rowOfBase bs.upper) (rowOfBase bs.lower)
  | none    => .simple [] .bot

/-- The **symbolic tracker assignment** for peeling the source while the sinks track it (the
    soundness analogue of `buildEnv`).  Folding over the sinks `rest` in topological order, each `U`
    is sent to the composite `composeFieldContrib (c' U) (rowFieldContrib sa (boundRow …))`: a field of
    `c' U`'s requiredness whose base tracks the peeled source's field through `U`'s `tag'`-selected
    bound row — itself evaluated under the trackers built so far (which, by `TopoSorted`, resolve the
    earlier sinks `U`'s bound mentions).  The source and untouched parameters keep their `sa` value. -/
def buildTracker (Γ : TyParamEnv) (l : Option Label) (tag' : Ty_param → Bool)
    (c' : Ty_param → FieldDesc) :
    List Ty_param → (Ty_param → FieldContrib) → (Ty_param → FieldContrib)
  | [],        sa => sa
  | U :: rest, sa => buildTracker Γ l tag' c' rest (Function.update sa U
      (composeFieldContrib (c' U) (rowFieldContrib sa l (boundRow Γ (tag' U) U))))

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- One unfolding step of the tracker fold (definitional). -/
theorem buildTracker_cons (Γ : TyParamEnv) (l : Option Label) (tag' : Ty_param → Bool)
    (c' : Ty_param → FieldDesc) (U : Ty_param) (rest : List Ty_param)
    (sa : Ty_param → FieldContrib) :
    buildTracker Γ l tag' c' (U :: rest) sa
      = buildTracker Γ l tag' c' rest (Function.update sa U
          (composeFieldContrib (c' U) (rowFieldContrib sa l (boundRow Γ (tag' U) U)))) := rfl

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- The tracker fold leaves parameters outside the sink list untouched. -/
theorem buildTracker_notMem (Γ : TyParamEnv) (l : Option Label) (tag' : Ty_param → Bool)
    (c' : Ty_param → FieldDesc) :
    ∀ (rest : List Ty_param) (sa : Ty_param → FieldContrib) (V : Ty_param), V ∉ rest →
      buildTracker Γ l tag' c' rest sa V = sa V
  | [],        _,  _, _  => rfl
  | U :: rest, sa, V, hV => by
      rw [List.mem_cons, not_or] at hV
      rw [buildTracker_cons, buildTracker_notMem Γ l tag' c' rest _ V hV.2,
          Function.update_of_ne hV.1]

/- ---------------------------------------------------------------------------------------------- -/
/- ## 50. The tracker tracks the tagged corner                                                    -/
/- ---------------------------------------------------------------------------------------------- -/

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- A declared parameter's tracked bound row projects to the `tag`-selected box endpoint. -/
theorem rowFieldAt_boundRow_some (Γ : TyParamEnv) (l : Option Label) (b : Bool) (U : Ty_param)
    (base : Ty_param → FieldDesc) {bs : Bounds} (h : Γ U = some bs) :
    rowFieldAt base l (boundRow Γ b U)
      = cond b (fieldBoundsAt Γ base l U).2 (fieldBoundsAt Γ base l U).1 := by
  simp only [boundRow, fieldBoundsAt, h]
  cases b <;> rfl

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- **The tracker tracks the tagged corner.**  Peeling the source at field `d`, the tracker fold sends
    each parameter `α` to a field `FEquiv` to its `cornerStep`-value under the base assignment `base`
    (= `g` with the source pinned to `d`).  Proved by a *relating-hypothesis* induction over the sink
    list: at each sink `U` the composite's base tracks `U`'s bound projected under the trackers built
    so far, which by the relating hypothesis match `base` (`rowFieldAt_extract` §24 +
    `rowFieldAt_fequiv` §26 + `rowFieldAt_boundRow_some`); requiredness comes from `g U = base U`.  No
    `TopoSorted` is needed — the relating hypothesis quantifies over *all* parameters. -/
theorem tracker_apply_gen (Γ : TyParamEnv) (l : Option Label) (tag' : Ty_param → Bool)
    (g : Ty_param → FieldDesc) (d : FieldDesc) :
    ∀ (rest : List Ty_param), rest.Nodup → (∀ U ∈ rest, ∃ bs, Γ U = some bs) →
      ∀ (sa : Ty_param → FieldContrib) (base : Ty_param → FieldDesc),
      (∀ U ∈ rest, base U = g U) → (∀ α, FEquiv ((sa α).apply d) (base α)) →
      ∀ α, FEquiv ((buildTracker Γ l tag' g rest sa α).apply d) (cornerStep Γ l tag' rest base α)
  | [],        _,      _,     sa, base, _,        hrel, α => hrel α
  | U :: rest, hnodup, hdecl, sa, base, hbase_eq, hrel, α => by
    have hUnotin : U ∉ rest := (List.nodup_cons.mp hnodup).1
    have hnodup' : rest.Nodup := (List.nodup_cons.mp hnodup).2
    rw [buildTracker_cons, cornerStep_cons]
    refine tracker_apply_gen Γ l tag' g d rest hnodup'
      (fun U' hU' => hdecl U' (List.mem_cons_of_mem _ hU'))
      (Function.update sa U (composeFieldContrib (g U)
        (rowFieldContrib sa l (boundRow Γ (tag' U) U))))
      (Function.update base U ((base U).setBase
        (cond (tag' U) (fieldBoundsAt Γ base l U).2.base (fieldBoundsAt Γ base l U).1.base)))
      ?_ ?_ α
    · -- the base still agrees with `g` on the remaining sinks
      intro U' hU'
      rw [Function.update_of_ne (by rintro rfl; exact hUnotin hU')]
      exact hbase_eq U' (List.mem_cons_of_mem _ hU')
    · -- the relating hypothesis is maintained: the new tracker tracks the new base value at `U`
      intro β
      by_cases hβ : β = U
      · subst hβ
        rw [Function.update_self, Function.update_self, composeFieldContrib_apply,
            hbase_eq β List.mem_cons_self]
        -- the composite's base is the bound projection, which matches `base` by the relating hyp
        obtain ⟨bs, hbs⟩ := hdecl β List.mem_cons_self
        have hψ : FEquiv ((rowFieldContrib sa l (boundRow Γ (tag' β) β)).apply d)
            (cond (tag' β) (fieldBoundsAt Γ base l β).2 (fieldBoundsAt Γ base l β).1) := by
          rw [← rowFieldAt_boundRow_some Γ l (tag' β) β base hbs]
          exact FEquiv.trans (rowFieldAt_extract sa l d (boundRow Γ (tag' β) β)).symm
            (rowFieldAt_fequiv hrel l (boundRow Γ (tag' β) β))
        rw [cond_base]
        exact setBase_base_fequiv (g β) hψ
      · rw [Function.update_of_ne hβ, Function.update_of_ne hβ]
        exact hrel β

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- The tagged corner depends on `tag` only at the list's parameters. -/
theorem cornerStep_tag_congr (Γ : TyParamEnv) (l : Option Label) {tag tag' : Ty_param → Bool} :
    ∀ (Ts : List Ty_param) (acc : Ty_param → FieldDesc), (∀ U ∈ Ts, tag U = tag' U) →
      cornerStep Γ l tag Ts acc = cornerStep Γ l tag' Ts acc
  | [],      _,   _ => rfl
  | T :: Ts, acc, h => by
    rw [cornerStep_cons, cornerStep_cons, h T List.mem_cons_self]
    exact cornerStep_tag_congr Γ l Ts _ (fun U hU => h U (List.mem_cons_of_mem _ hU))

/- ---------------------------------------------------------------------------------------------- -/
/- ## 51. The coupled peel                                                                        -/
/- ---------------------------------------------------------------------------------------------- -/

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- **The coupled peel.**  The per-label goal at the real in-box assignment `g` follows once it holds
    at every tagged corner `cornerStep tag Ts g`.  Induction on `Ts`, peeling the head source `T`:
    apply the inductive reduction on the sinks `rest` (with `T` fixed at `g T`), then for each
    sink-tag `tag'` peel `T` via `peel_step_contrib` with the sinks tracking `T` (`tracker_apply_gen`)
    — the two base-corners of `T`'s box land on the tagged corners of `T :: rest` (`cornerStep_cons` +
    `cornerStep_tag_congr`), supplied by `H`; the conclusion transports back to `cornerStep tag' rest g`. -/
theorem peelSound (Γ : TyParamEnv) (l : Option Label) (r p : Row) (g : Ty_param → FieldDesc) :
    ∀ (Ts : List Ty_param), Ts.Nodup → (∀ U ∈ Ts, ∃ bs, Γ U = some bs) →
      (∀ T ∈ Ts, SubField (fieldBoundsAt Γ g l T).1 (g T) ∧ SubField (g T) (fieldBoundsAt Γ g l T).2) →
      (∀ tag, SubField (rowFieldAt (cornerStep Γ l tag Ts g) l r)
                       (rowFieldAt (cornerStep Γ l tag Ts g) l p)) →
      SubField (rowFieldAt g l r) (rowFieldAt g l p)
  | [],        _,      _,     _,    H => H (fun _ => false)
  | T :: rest, hnodup, hdecl, hbox, H => by
    have hTnotin : T ∉ rest := (List.nodup_cons.mp hnodup).1
    have hnodup' : rest.Nodup := (List.nodup_cons.mp hnodup).2
    -- reduce to the sinks (with `T` fixed at `g T`), then peel `T` per sink-tag
    refine peelSound Γ l r p g rest hnodup'
      (fun U hU => hdecl U (List.mem_cons_of_mem _ hU))
      (fun U hU => hbox U (List.mem_cons_of_mem _ hU)) ?_
    intro tag'
    obtain ⟨hloT, hhiT⟩ := hbox T List.mem_cons_self
    set sa0 : Ty_param → FieldContrib :=
      Function.update (fun β => constFieldContrib (g β)) T idFieldContrib with hsa0
    set sa : Ty_param → FieldContrib := buildTracker Γ l tag' g rest sa0 with hsa
    -- the tracker tracks `cornerStep tag' rest base` at any peel field `d` (with `base T = d`)
    have htrack : ∀ (d : FieldDesc) (base : Ty_param → FieldDesc),
        base T = d → (∀ α, α ≠ T → base α = g α) →
        ∀ α, FEquiv ((sa α).apply d) (cornerStep Γ l tag' rest base α) := by
      intro d base hbT hoff
      rw [hsa]
      refine tracker_apply_gen Γ l tag' g d rest hnodup'
        (fun U hU => hdecl U (List.mem_cons_of_mem _ hU)) sa0 base
        (fun U hU => hoff U (by rintro rfl; exact hTnotin hU)) ?_
      intro α
      by_cases hα : α = T
      · subst hα; rw [hsa0, Function.update_self, hbT]; exact idFieldContrib_apply d
      · rw [hsa0, Function.update_of_ne hα, constFieldContrib_apply, hoff α hα]
        exact FEquiv.refl _
    -- transport `H` at the two base-corners (the tagged corners of `T :: rest`) into the peel inputs
    have corner : ∀ (s : Bool),
        SubField (rowFieldAt (fun α => (sa α).apply ((g T).setBase
            (cond s (fieldBoundsAt Γ g l T).2.base (fieldBoundsAt Γ g l T).1.base))) l r)
          (rowFieldAt (fun α => (sa α).apply ((g T).setBase
            (cond s (fieldBoundsAt Γ g l T).2.base (fieldBoundsAt Γ g l T).1.base))) l p) := by
      intro s
      set dc : FieldDesc := (g T).setBase
        (cond s (fieldBoundsAt Γ g l T).2.base (fieldBoundsAt Γ g l T).1.base) with hdc
      have hcs : cornerStep Γ l (Function.update tag' T s) (T :: rest) g
               = cornerStep Γ l tag' rest (Function.update g T dc) := by
        rw [cornerStep_cons, Function.update_self]
        exact cornerStep_tag_congr Γ l rest _
          (fun U hU => by rw [Function.update_of_ne (ne_of_mem_of_not_mem hU hTnotin)])
      have hHs := H (Function.update tag' T s)
      rw [hcs] at hHs
      have ht := htrack dc (Function.update g T dc) (by rw [Function.update_self])
        (fun α hα => by rw [Function.update_of_ne hα])
      exact SubField.of_fequiv (rowFieldAt_fequiv ht l r) (rowFieldAt_fequiv ht l p) hHs
    -- peel `T`; transport the conclusion back to `cornerStep tag' rest g`
    have hpeel := peel_step_contrib sa l r p hloT hhiT (corner false) (corner true)
    have ht0 := htrack (g T) g rfl (fun _ _ => rfl)
    exact SubField.of_fequiv (rowFieldAt_fequiv ht0 l r).symm (rowFieldAt_fequiv ht0 l p).symm hpeel

/- ---------------------------------------------------------------------------------------------- -/
/- ## 52. Per-label soundness against the coupled enumeration                                     -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Per-label coupled soundness.**  At an in-box assignment `g`, the per-label goal holds once it
    holds at every assignment the coupled enumeration `coupledCornerAssigns Γ l Ts g` produces.
    Combines the peel (`peelSound`, §51 — reduces to the tagged corners) with `cornerStep_mem` (§47 —
    each tagged corner is one of those enumerated). -/
theorem subrow_coupled_sound (Γ : TyParamEnv) (l : Option Label) (r p : Row)
    (g : Ty_param → FieldDesc) (Ts : List Ty_param) (hnodup : Ts.Nodup)
    (hdecl : ∀ U ∈ Ts, ∃ bs, Γ U = some bs)
    (hcons : ∀ (U : Ty_param) (acc' : Ty_param → FieldDesc),
      SubBase (fieldBoundsAt Γ acc' l U).1.base (fieldBoundsAt Γ acc' l U).2.base)
    (hinbox : ∀ U, SubField (fieldBoundsAt Γ g l U).1 (g U) ∧ SubField (g U) (fieldBoundsAt Γ g l U).2)
    (H : ∀ c ∈ coupledCornerAssigns Γ l Ts g,
      SubField (rowFieldAt c l r) (rowFieldAt c l p)) :
    SubField (rowFieldAt g l r) (rowFieldAt g l p) := by
  refine peelSound Γ l r p g Ts hnodup hdecl (fun T _ => hinbox T) ?_
  intro tag
  exact H _ (cornerStep_mem Γ l tag g hcons hinbox Ts hnodup g (fun _ _ => rfl) (fun _ => rfl))

/- ---------------------------------------------------------------------------------------------- -/
/- ## 53. Coupled soundness for the semantic subrow                                               -/
/- ---------------------------------------------------------------------------------------------- -/

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- `req ⊥` is below every field. -/
theorem subField_botReq (X : FieldDesc) : SubField (.req .bot) X := by
  cases X with
  | req _ => exact SubField.req_req SubBase.bot
  | opt _ => exact SubField.req_opt SubBase.bot

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- `opt ⊤` is above every field. -/
theorem subField_optTop (X : FieldDesc) : SubField X (.opt .top) := by
  cases X with
  | req _ => exact SubField.req_opt SubBase.top
  | opt _ => exact SubField.opt_opt SubBase.top

/-- **Coupled soundness (per-`ρ` form).**  For `Ground` rows over `Ground` (but possibly coupled)
    bounds, `SemSubRow r p Γ` follows from the per-`ρ`, per-label coupled corner check: under every
    compatible instantiation `ρ` and label, the per-label goal holds at every assignment the
    enumeration produces from the real assignment `gOf ρ`.  The coupled analogue of §36
    `subrow_ground_sound`: bridge the evaluated projection to the symbolic one (`rowFieldAt_eval_ground`),
    note the real assignment is in its coupled box (`inBox_of_compatible`; the full lattice for
    undeclared parameters), then apply the coupled peel (`subrow_coupled_sound`, §52).  The box being
    `ρ`-evaluated, the `ρ`-independent (decidable) enumeration is reconciled separately. -/
theorem semSubRow_of_coupledCorners (r p : Row) (Γ : TyParamEnv) (Ts : List Ty_param)
    (hnodup : Ts.Nodup) (_hr : GroundRow r) (_hp : GroundRow p)
    (hdecl : ∀ U ∈ Ts, ∃ bs, Γ U = some bs)
    (hcons : ∀ (l : Option Label) (U : Ty_param) (acc' : Ty_param → FieldDesc),
      SubBase (fieldBoundsAt Γ acc' l U).1.base (fieldBoundsAt Γ acc' l U).2.base)
    (_hground : ∀ U b, Γ U = some b → GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper))
    (hcheck : ∀ ρ, Compatible ρ Γ → ∀ l₀ : Label,
      ∀ c ∈ coupledCornerAssigns Γ (some l₀) Ts (gOf ρ (some l₀)),
        SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p)) :
    SemSubRow r p Γ := by
  rw [semSubRow_iff_perLabel]
  intro ρ hc l₀
  -- the per-label goal is already the symbolic projection at `some l₀` — no bridge needed
  refine subrow_coupled_sound Γ (some l₀) r p (gOf ρ (some l₀)) Ts hnodup hdecl
    (hcons (some l₀)) ?_ (hcheck ρ hc l₀)
  -- the real assignment lies in each coupled box
  intro U
  cases hU : Γ U with
  | none   => simp only [fieldBoundsAt, hU]; exact ⟨subField_botReq _, subField_optTop _⟩
  | some b =>
    simp only [fieldBoundsAt, hU]
    exact inBox_of_compatible ρ Γ hc l₀ U b hU

/- ---------------------------------------------------------------------------------------------- -/
/- ## 54. ρ-independence of the goal-check                                                        -/
/- ---------------------------------------------------------------------------------------------- -/

/-- An enumerated corner carries the initial assignment at every parameter outside `Ts`. -/
theorem coupledCornerAssigns_off (Γ : TyParamEnv) (l : Option Label) :
    ∀ (Ts : List Ty_param) (fa c : Ty_param → FieldDesc),
      c ∈ coupledCornerAssigns Γ l Ts fa → ∀ α, α ∉ Ts → c α = fa α
  | [],        _,  c, hc, _, _  => by rw [coupledCornerAssigns, List.mem_singleton] at hc; rw [hc]
  | T :: rest, fa, c, hc, α, hα => by
      rw [coupledCornerAssigns, List.mem_flatMap] at hc
      obtain ⟨d, _, hcd⟩ := hc
      rw [List.mem_cons, not_or] at hα
      rw [coupledCornerAssigns_off Γ l rest _ c hcd α hα.2, Function.update_of_ne hα.1]

/-- **The enumeration's corners agree on `Ts` across initial assignments that agree on the external
    dependencies.**  Given a corner of `coupledCornerAssigns Γ l Ts fa`, there is one of
    `… Ts fa'` agreeing with it on every `Ts`-parameter — the corner *choices* (the `rigidCorners` at
    each step) coincide because the boxes read only in-`Ts` dependencies (at the same chosen corners)
    and the external ones, where `fa`, `fa'` agree (`fieldBoundsAt_congr`). -/
theorem coupledCornerAssigns_match (Γ : TyParamEnv) (l : Option Label) :
    ∀ (Ts : List Ty_param), Ts.Nodup → TopoSorted Γ Ts → ∀ (fa fa' : Ty_param → FieldDesc),
      (∀ α, (∃ T ∈ Ts, DependsOn Γ T α) → α ∉ Ts → fa α = fa' α) →
      ∀ (c : Ty_param → FieldDesc), c ∈ coupledCornerAssigns Γ l Ts fa →
        ∃ c₀ ∈ coupledCornerAssigns Γ l Ts fa', ∀ U ∈ Ts, c U = c₀ U
  | [],        _,      _,     _,  fa', _,      c, _  =>
      ⟨fa', by rw [coupledCornerAssigns]; exact List.mem_singleton.mpr rfl,
        fun U hU => absurd hU (by simp)⟩
  | T :: rest, hnodup, htopo, fa, fa', hagree, c, hc => by
      have hTnotin : T ∉ rest := (List.nodup_cons.mp hnodup).1
      have hnodup' : rest.Nodup := (List.nodup_cons.mp hnodup).2
      obtain ⟨htopoHd, htopoTl⟩ : (∀ α, DependsOn Γ T α → α ∉ T :: rest) ∧ TopoSorted Γ rest := htopo
      rw [coupledCornerAssigns, List.mem_flatMap] at hc
      obtain ⟨d, hd, hcd⟩ := hc
      have hboxeq : fieldBoundsAt Γ fa l T = fieldBoundsAt Γ fa' l T :=
        fieldBoundsAt_congr Γ l T
          (fun α hdep => hagree α ⟨T, List.mem_cons_self, hdep⟩ (htopoHd α hdep))
      have hagree' : ∀ α, (∃ U ∈ rest, DependsOn Γ U α) → α ∉ rest →
          (Function.update fa T d) α = (Function.update fa' T d) α := by
        intro α hαdep hαnotin
        by_cases hαT : α = T
        · subst hαT; rw [Function.update_self, Function.update_self]
        · rw [Function.update_of_ne hαT, Function.update_of_ne hαT]
          obtain ⟨U, hU, hUdep⟩ := hαdep
          exact hagree α ⟨U, List.mem_cons_of_mem _ hU, hUdep⟩
            (by rw [List.mem_cons, not_or]; exact ⟨hαT, hαnotin⟩)
      obtain ⟨c₀, hc₀mem, hc₀eq⟩ := coupledCornerAssigns_match Γ l rest hnodup' htopoTl
        (Function.update fa T d) (Function.update fa' T d) hagree' c hcd
      refine ⟨c₀, ?_, ?_⟩
      · rw [coupledCornerAssigns, List.mem_flatMap]
        exact ⟨d, by rwa [hboxeq] at hd, hc₀mem⟩
      · intro U hU
        rcases List.mem_cons.mp hU with rfl | hUrest
        · rw [coupledCornerAssigns_off Γ l rest _ c hcd U hTnotin, Function.update_self,
              coupledCornerAssigns_off Γ l rest _ c₀ hc₀mem U hTnotin, Function.update_self]
        · exact hc₀eq U hUrest

/-- **The goal-check is initial-assignment-independent** (for `TopoSorted`, `BoundClosed` `Ts` and
    rows mentioning only `Ts`): if the per-label goal holds at every corner of `… Ts fa'`, it holds at
    every corner of `… Ts fa`.  Each corner of `enum fa` matches one of `enum fa'` on `Ts`
    (`coupledCornerAssigns_match`; no external dependencies by `BoundClosed`), and the goal reads the
    rows only at `Ts` (`rowFieldAt_congr`). -/
theorem coupledCornerAssigns_goalcheck (Γ : TyParamEnv) (l : Option Label) (r p : Row)
    (Ts : List Ty_param) (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (hclosed : BoundClosed Γ Ts)
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (fa fa' : Ty_param → FieldDesc)
    (H : ∀ c₀ ∈ coupledCornerAssigns Γ l Ts fa', SubField (rowFieldAt c₀ l r) (rowFieldAt c₀ l p))
    (c : Ty_param → FieldDesc) (hc : c ∈ coupledCornerAssigns Γ l Ts fa) :
    SubField (rowFieldAt c l r) (rowFieldAt c l p) := by
  obtain ⟨c₀, hc₀mem, hc₀eq⟩ := coupledCornerAssigns_match Γ l Ts hnodup htopo fa fa'
    (fun α ⟨T, hT, hdep⟩ hαnotin => absurd (hclosed T hT α hdep) hαnotin) c hc
  rw [rowFieldAt_congr l r (fun α hα => hc₀eq α (hrelr α hα)),
      rowFieldAt_congr l p (fun α hα => hc₀eq α (hrelp α hα))]
  exact H c₀ hc₀mem

/- ---------------------------------------------------------------------------------------------- -/
/- ## 55. Coupled soundness from a ρ-independent check                                            -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Coupled soundness, ρ-independent form.**  `SemSubRow r p Γ` follows from a check that no longer
    mentions `ρ`: at each label, the per-label goal holds at every corner the enumeration produces from
    a *fixed* default assignment `dflt`.  Combines §53 (`semSubRow_of_coupledCorners`, the per-`ρ`
    reduction) with §54 (`coupledCornerAssigns_goalcheck`, transporting the `gOf ρ`-enumeration's goal
    to the `dflt`-enumeration's by relevance + `TopoSorted`/`BoundClosed`).  The remaining infinitude is
    over labels (collapsed to a finite list by label-finiteness, as in §40b — the next step). -/
theorem semSubRow_of_coupledCorners_indep (r p : Row) (Γ : TyParamEnv) (Ts : List Ty_param)
    (dflt : Ty_param → FieldDesc)
    (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (hclosed : BoundClosed Γ Ts)
    (hr : GroundRow r) (hp : GroundRow p)
    (hdecl : ∀ U ∈ Ts, ∃ bs, Γ U = some bs)
    (hcons : ∀ (l : Option Label) (U : Ty_param) (acc' : Ty_param → FieldDesc),
      SubBase (fieldBoundsAt Γ acc' l U).1.base (fieldBoundsAt Γ acc' l U).2.base)
    (hground : ∀ U b, Γ U = some b → GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper))
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (H : ∀ l₀ : Label, ∀ c ∈ coupledCornerAssigns Γ (some l₀) Ts dflt,
      SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p)) :
    SemSubRow r p Γ := by
  apply semSubRow_of_coupledCorners r p Γ Ts hnodup hr hp hdecl hcons hground
  intro ρ _ l₀ c hcmem
  exact coupledCornerAssigns_goalcheck Γ (some l₀) r p Ts hnodup htopo hclosed hrelr hrelp
    (gOf ρ (some l₀)) dflt (H l₀) c hcmem

/- ---------------------------------------------------------------------------------------------- -/
/- ## 56. Label-finiteness for the coupled check                                                   -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The explicit labels of `Ts`'s bound rows. -/
def coupledBoundLabels (Γ : TyParamEnv) : List Ty_param → List Label
  | []      => []
  | U :: Us =>
      (match Γ U with
        | some b => rowLabels (rowOfBase b.lower) ++ rowLabels (rowOfBase b.upper)
        | none   => []) ++ coupledBoundLabels Γ Us

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- A label outside the bound-labels of `Ts` is outside each declared member's bound-row labels. -/
theorem not_mem_coupledBoundLabels (Γ : TyParamEnv) {l : Label} :
    ∀ (Ts : List Ty_param), l ∉ coupledBoundLabels Γ Ts →
      ∀ U ∈ Ts, ∀ b, Γ U = some b →
        l ∉ rowLabels (rowOfBase b.lower) ∧ l ∉ rowLabels (rowOfBase b.upper)
  | U :: Us, hl, V, hV, b, hb => by
      rw [coupledBoundLabels, List.mem_append, not_or] at hl
      rcases List.mem_cons.mp hV with rfl | hVUs
      · rw [hb] at hl
        rw [List.mem_append, not_or] at hl
        exact hl.1
      · exact not_mem_coupledBoundLabels Γ Us hl.2 V hVUs b hb

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- `coupledBoundLabels` of a tail is a sublist (no membership added by dropping the head). -/
theorem not_mem_coupledBoundLabels_tail (Γ : TyParamEnv) {l : Label} (T : Ty_param)
    (Us : List Ty_param) (hl : l ∉ coupledBoundLabels Γ (T :: Us)) : l ∉ coupledBoundLabels Γ Us := by
  rw [coupledBoundLabels, List.mem_append, not_or] at hl; exact hl.2

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- The coupled box is constant across labels both absent from the parameter's bound-row labels. -/
theorem fieldBoundsAt_label_congr (Γ : TyParamEnv) (acc : Ty_param → FieldDesc) (U : Ty_param)
    {l₀ l₀' : Label}
    (h : ∀ b, Γ U = some b →
      (l₀ ∉ rowLabels (rowOfBase b.lower) ∧ l₀ ∉ rowLabels (rowOfBase b.upper)) ∧
      (l₀' ∉ rowLabels (rowOfBase b.lower) ∧ l₀' ∉ rowLabels (rowOfBase b.upper))) :
    fieldBoundsAt Γ acc (some l₀) U = fieldBoundsAt Γ acc (some l₀') U := by
  cases hU : Γ U with
  | none   => simp only [fieldBoundsAt, hU]
  | some b =>
    obtain ⟨⟨h1, h3⟩, ⟨h2, h4⟩⟩ := h b hU
    simp only [fieldBoundsAt, hU]
    rw [rowFieldAt_absent_eq acc (rowOfBase b.lower) h1 h2,
        rowFieldAt_absent_eq acc (rowOfBase b.upper) h3 h4]

/-- The coupled enumeration is constant across labels both absent from `Ts`'s bound-row labels. -/
theorem coupledCornerAssigns_label_congr (Γ : TyParamEnv) {l₀ l₀' : Label} :
    ∀ (Ts : List Ty_param) (fa : Ty_param → FieldDesc),
      l₀ ∉ coupledBoundLabels Γ Ts → l₀' ∉ coupledBoundLabels Γ Ts →
      coupledCornerAssigns Γ (some l₀) Ts fa = coupledCornerAssigns Γ (some l₀') Ts fa
  | [],        _,  _,   _    => rfl
  | T :: rest, fa, hl₀, hl₀' => by
    have hbox : fieldBoundsAt Γ fa (some l₀) T = fieldBoundsAt Γ fa (some l₀') T :=
      fieldBoundsAt_label_congr Γ fa T (fun b hb =>
        ⟨not_mem_coupledBoundLabels Γ (T :: rest) hl₀ T List.mem_cons_self b hb,
         not_mem_coupledBoundLabels Γ (T :: rest) hl₀' T List.mem_cons_self b hb⟩)
    rw [coupledCornerAssigns, coupledCornerAssigns, hbox]
    congr 1
    funext d
    exact coupledCornerAssigns_label_congr Γ rest (Function.update fa T d)
      (not_mem_coupledBoundLabels_tail Γ T rest hl₀)
      (not_mem_coupledBoundLabels_tail Γ T rest hl₀')

/-- **Coupled soundness, finite form.**  The label quantifier of §55 collapses to a finite list — the
    rows' and bounds' explicit labels plus one fresh `lf` — by label-finiteness: off those labels the
    enumeration (`coupledCornerAssigns_label_congr`) and the goal (`rowFieldAt_absent_eq`) are constant,
    so any unmentioned label reduces to `lf`.  This is the decidable `←` of the coupled characterization. -/
theorem semSubRow_of_coupledCorners_finite (r p : Row) (Γ : TyParamEnv) (Ts : List Ty_param)
    (dflt : Ty_param → FieldDesc) (lf : Label)
    (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (hclosed : BoundClosed Γ Ts)
    (hr : GroundRow r) (hp : GroundRow p)
    (hdecl : ∀ U ∈ Ts, ∃ bs, Γ U = some bs)
    (hcons : ∀ (l : Option Label) (U : Ty_param) (acc' : Ty_param → FieldDesc),
      SubBase (fieldBoundsAt Γ acc' l U).1.base (fieldBoundsAt Γ acc' l U).2.base)
    (hground : ∀ U b, Γ U = some b → GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper))
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (hlf : lf ∉ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts)
    (Hfin : ∀ l₀ ∈ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts ++ [lf],
      ∀ c ∈ coupledCornerAssigns Γ (some l₀) Ts dflt,
        SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p)) :
    SemSubRow r p Γ := by
  refine semSubRow_of_coupledCorners_indep r p Γ Ts dflt hnodup htopo hclosed hr hp hdecl hcons
    hground hrelr hrelp ?_
  intro l₀
  by_cases hl₀ : l₀ ∈ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts
  case pos => exact Hfin l₀ (List.mem_append.mpr (Or.inl hl₀))
  case neg =>
    -- transport the check from the fresh label `lf`
    simp only [List.mem_append, not_or] at hl₀ hlf
    obtain ⟨⟨hl₀r, hl₀p⟩, hl₀b⟩ := hl₀
    obtain ⟨⟨hlfr, hlfp⟩, hlfb⟩ := hlf
    have henum : coupledCornerAssigns Γ (some l₀) Ts dflt
               = coupledCornerAssigns Γ (some lf) Ts dflt :=
      coupledCornerAssigns_label_congr Γ Ts dflt hl₀b hlfb
    intro c hc
    rw [henum] at hc
    have hc_lf := Hfin lf (List.mem_append.mpr (Or.inr (List.mem_singleton.mpr rfl))) c hc
    rw [rowFieldAt_absent_eq c r hlfr hl₀r, rowFieldAt_absent_eq c p hlfp hl₀p] at hc_lf
    exact hc_lf

end Splat
