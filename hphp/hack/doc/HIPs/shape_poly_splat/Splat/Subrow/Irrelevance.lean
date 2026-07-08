import Splat.Subrow.Corner

/-!
# Liveness and masking irrelevance (the box-agnostic kernel)

These are the row-level facts the corner optimizations rest on: a parameter that cannot reach a
label's projection — because it is not live there, or is masked by an always-required field to its
right — does not change that field.  Nothing here mentions parameter bounds or the corner
enumeration, so it is shared unchanged by the ground (`GroundOpt`) and coupled (`CoupledOpt`)
optimization layers.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 64. Definite maskers and liveness                                                            -/
/- ---------------------------------------------------------------------------------------------- -/

/-- A spread element that forces a `Req` field at `l` *no matter the assignment* — so it masks every
    contribution to its left under the rightmost-wins fold.  Mirrors the maskers `Row.live_spread_at`
    stops at: the bottom row (always `Req ⊥`) and an inline simple row whose projection is `Req`.  A
    spread `rigid` is never a definite masker — its requiredness depends on the assignment. -/
def defMaskerElem (l : Option Label) : SplatElem → Bool
  | .spread .bot                    => true
  | .spread (.shape (.simple fs u)) => (projOpt (normalize fs u) l).isReq
  | _                               => false

/-- Whether a list of spread elements contains a definite masker at `l`.  The fold's running
    accumulator is discarded once a definite masker is reached, so the elements left of one cannot
    reach the projection. -/
def defMasks (l : Option Label) : List SplatElem → Bool
  | []      => false
  | e :: es => defMaskerElem l e || defMasks l es

/-- Parameter `α` is **live** at `l` among the spread elements: it occurs as a spread `rigid α` with
    no definite masker to its right (the elements right of `e` are exactly the tail, since the fold
    merges left-to-right and a rightward `Req` masks the left).  Mirrors `Row.live_spread_at`, which
    scans right-to-left collecting spread `rigid`s until the first definite masker. -/
def LiveAt (l : Option Label) (α : Ty_param) : List SplatElem → Prop
  | []      => False
  | e :: es => LiveAt l α es ∨ (e = .spread (.rigid α) ∧ defMasks l es = false)

/-- A row's live parameters at `l`: a simple row has none (it has no spread parameters); a splat
    row's are its elements'.  Mirrors the spread structure of `rowMentions` (§28). -/
def RowLiveAt (l : Option Label) (α : Ty_param) : Row → Prop
  | .simple _ _ => False
  | .splat es   => LiveAt l α es

/-- No spread element is a nested splat shape `spread (shape (splat …))`.  `canon` flattens such
    elements away (`collapseSpread`), so canonical input always satisfies this; it lets the liveness
    scan treat every element as either a parameter, a definite masker, or assignment-independent. -/
def NoNestedSplat : List SplatElem → Prop
  | []      => True
  | e :: es => (∀ es', e ≠ .spread (.shape (.splat es'))) ∧ NoNestedSplat es

/-- A row carries no nested splat shape at spread position. -/
def RowNoNested : Row → Prop
  | .simple _ _ => True
  | .splat es   => NoNestedSplat es


/- ---------------------------------------------------------------------------------------------- -/
/- ## 65. A non-live parameter cannot change the label's field                                     -/
/- ---------------------------------------------------------------------------------------------- -/


/-- A `Req` field on the right of a merge overrides the left accumulator entirely (the masking step). -/
theorem mergeField_req (acc : FieldDesc) (t : Base) : mergeField acc (.req t) = .req t := rfl

/-- The fold over `es` is unchanged when the assignment moves only at a non-live `α`, even starting
    from different accumulators — provided the two accumulators are either equal or about to be
    discarded by a definite masker (`defMasks l es`).  The accumulator-relation is what carries the
    masking through the induction: once a definite masker is reached the differing accumulators are
    overwritten, and beyond it the surviving elements do not depend on `α` (it is not live there). -/
theorem splatFoldAt_irrel {g g' : Ty_param → FieldDesc} {α : Ty_param} {l : Option Label}
    (hag : ∀ β, β ≠ α → g β = g' β) :
    ∀ (es : List SplatElem), NoNestedSplat es → ¬ LiveAt l α es →
      ∀ acc acc' : FieldDesc, (acc = acc' ∨ defMasks l es = true) →
        splatFoldAt g l es acc = splatFoldAt g' l es acc' := by
  intro es
  induction es with
  | nil =>
    -- empty fold returns the accumulator; `defMasks l [] = false`, so the two accumulators are equal
    intro _ _ acc acc' hcc
    rcases hcc with rfl | hdm
    case inl => rfl
    case inr => exact Bool.noConfusion hdm
  | cons e es ih =>
    intro hflat hlive acc acc' hcc
    -- one fold step merges `e`'s field into each accumulator, then recurses on the tail
    show splatFoldAt g l es (mergeField acc (elemFieldAt g l e))
       = splatFoldAt g' l es (mergeField acc' (elemFieldAt g' l e))
    -- the new accumulators are again equal-or-about-to-be-masked; the case analysis on `e` supplies it
    have hdisj : mergeField acc (elemFieldAt g l e) = mergeField acc' (elemFieldAt g' l e)
        ∨ defMasks l es = true := by
      cases e with
      | spread b =>
        cases b with
        | rigid β =>
          by_cases hβ : β = α
          case pos =>
            -- `e = spread (rigid α)`: not being live forces a definite masker in the tail
            subst hβ
            right
            by_contra hne
            have hfalse : defMasks l es = false := by
              cases hd : defMasks l es with
              | false => rfl
              | true  => exact absurd hd hne
            exact hlive (Or.inr ⟨rfl, hfalse⟩)
          case neg =>
            -- `β ≠ α`: the element's field is `g β = g' β`, so equality survives a masker-free tail
            have hgb : g β = g' β := hag β hβ
            rcases hcc with hac | hdm
            case inl =>
              left
              show mergeField acc (g β) = mergeField acc' (g' β)
              rw [hac, hgb]
            case inr =>
              -- `e` is not a masker, so a masker in `e :: es` lies in `es`
              right; exact hdm
        | bot =>
          -- definite masker: `Req ⊥` overrides both accumulators identically
          left
          show mergeField acc (FieldDesc.req .bot) = mergeField acc' (FieldDesc.req .bot)
          rw [mergeField_req, mergeField_req]
        | top =>
          -- assignment-independent, not a masker
          rcases hcc with hac | hdm
          case inl => left; show mergeField acc (FieldDesc.opt .bot) = mergeField acc' (FieldDesc.opt .bot); rw [hac]
          case inr => right; exact hdm
        | prim p =>
          rcases hcc with hac | hdm
          case inl => left; show mergeField acc (FieldDesc.opt .bot) = mergeField acc' (FieldDesc.opt .bot); rw [hac]
          case inr => right; exact hdm
        | union a b =>
          rcases hcc with hac | hdm
          case inl => left; show mergeField acc (FieldDesc.opt .bot) = mergeField acc' (FieldDesc.opt .bot); rw [hac]
          case inr => right; exact hdm
        | shape row =>
          cases row with
          | simple fs u =>
            -- assignment-independent; whether it masks depends on its projection's requiredness
            cases hf : projOpt (normalize fs u) l with
            | req t =>
              -- definite masker: `Req t` overrides both accumulators identically
              left
              show mergeField acc (projOpt (normalize fs u) l) = mergeField acc' (projOpt (normalize fs u) l)
              rw [hf, mergeField_req, mergeField_req]
            | opt t =>
              -- not a masker
              rcases hcc with hac | hdm
              case inl =>
                left
                show mergeField acc (projOpt (normalize fs u) l) = mergeField acc' (projOpt (normalize fs u) l)
                rw [hac]
              case inr =>
                right
                -- `defMaskerElem` here is `(opt t).isReq = false`, so the masker is in `es`
                simp only [defMasks, defMaskerElem, hf, FieldDesc.isReq, Bool.false_or] at hdm
                exact hdm
          | splat es' =>
            -- nested splat shape: ruled out by flatness
            exact absurd rfl (hflat.1 es')
    exact ih hflat.2 (fun h => hlive (Or.inl h)) _ _ hdisj

/-- A parameter not live at `l` among the spread elements does not change the field they contribute
    there, under any assignment move confined to it. -/
theorem splatFieldAt_irrel {g g' : Ty_param → FieldDesc} {α : Ty_param} {l : Option Label}
    (hag : ∀ β, β ≠ α → g β = g' β) (es : List SplatElem)
    (hflat : NoNestedSplat es) (hlive : ¬ LiveAt l α es) :
    splatFieldAt g l es = splatFieldAt g' l es := by
  cases es with
  | nil => rfl
  | cons e es' =>
    -- the head seeds the fold; the seed is either assignment-independent or about to be masked
    show splatFoldAt g l es' (elemFieldAt g l e) = splatFoldAt g' l es' (elemFieldAt g' l e)
    have hhead : elemFieldAt g l e = elemFieldAt g' l e ∨ defMasks l es' = true := by
      cases e with
      | spread b =>
        cases b with
        | rigid β =>
          by_cases hβ : β = α
          case pos =>
            -- `e = spread (rigid α)`: not being live forces a definite masker in the tail
            subst hβ
            right
            by_contra hne
            have hfalse : defMasks l es' = false := by
              cases hd : defMasks l es' with
              | false => rfl
              | true  => exact absurd hd hne
            exact hlive (Or.inr ⟨rfl, hfalse⟩)
          case neg => left; show g β = g' β; exact hag β hβ
        | bot => left; rfl
        | top => left; rfl
        | prim p => left; rfl
        | union a b => left; rfl
        | shape row =>
          cases row with
          | simple fs u => left; rfl
          | splat es'' => exact absurd rfl (hflat.1 es'')
    exact splatFoldAt_irrel hag es' hflat.2 (fun h => hlive (Or.inl h)) _ _ hhead

/-- **Liveness irrelevance.**  A parameter not live at `l` in a row does not change the row's field
    there.  The masking refinement of relevance (§28 `rowFieldAt_congr`): there an *unmentioned*
    parameter is irrelevant, here a *masked* one is too. -/
theorem rowFieldAt_liveAt_irrel {g g' : Ty_param → FieldDesc} {α : Ty_param} {l : Option Label}
    (hag : ∀ β, β ≠ α → g β = g' β) (r : Row)
    (hflat : RowNoNested r) (hlive : ¬ RowLiveAt l α r) :
    rowFieldAt g l r = rowFieldAt g' l r := by
  cases r with
  | simple fs u => rfl
  | splat es    => exact splatFieldAt_irrel hag es hflat hlive

/-- Resetting a field's base to its own base is the identity. -/
theorem setBase_self (d : FieldDesc) : d.setBase d.base = d := by cases d <;> rfl

/-- The per-label goal at an assignment: the sub-row's field is a subfield of the super-row's. -/
abbrev goalAt (l : Option Label) (r p : Row) (c : Ty_param → FieldDesc) : Prop :=
  SubField (rowFieldAt c l r) (rowFieldAt c l p)

/-- **Upper-endpoint cut (the box-agnostic core of every `[hi]` shortcut).**  Move the assignment from
    `c₀` down to `c` at one parameter `T` (`c T ≤ c₀ T`, agreeing elsewhere) while leaving the super-row's
    field at `l` unchanged (`hp`).  Then the per-label goal carries from `c₀` to `c`: the sub side is
    monotone in the assignment, so the larger `c₀ T` is the worst case for it.  This is why checking only
    `T`'s upper endpoint suffices when the super side does not see `T` — the conceptual content the
    free-single-side and masking `[hi]` shortcuts share, with no mention of boxes or the enumeration. -/
theorem goalCut_hi {l : Option Label} {r p : Row} {T : Ty_param} {c c₀ : Ty_param → FieldDesc}
    (hag : ∀ β, β ≠ T → c β = c₀ β) (hcT : SubField (c T) (c₀ T))
    (hp : rowFieldAt c l p = rowFieldAt c₀ l p) (hg : goalAt l r p c₀) : goalAt l r p c := by
  -- `c ≤ c₀` everywhere: at `T` by `hcT`, off `T` by the agreement `hag`
  have hmono : ∀ β, SubField (c β) (c₀ β) := by
    intro β; by_cases h : β = T
    case pos => rw [h]; exact hcT
    case neg => rw [hag β h]; exact subField_refl _
  -- the sub side grows monotonically to `c₀`; the super side is unchanged (`hp`)
  show SubField (rowFieldAt c l r) (rowFieldAt c l p)
  rw [hp]
  exact subField_trans (rowFieldAt_mono hmono l r) hg

/-- **Lower-endpoint cut (the box-agnostic core of every `[lo]` shortcut).**  Mirror of `goalCut_hi`:
    move the assignment from `c₀` up to `c` at one parameter `T` (`c₀ T ≤ c T`, agreeing elsewhere) while
    leaving the sub-row's field at `l` unchanged (`hr`).  The per-label goal carries from `c₀` to `c`
    because the super side is monotone, so the smaller `c₀ T` is the worst case for it — why checking
    only `T`'s lower endpoint suffices when the sub side does not see `T`. -/
theorem goalCut_lo {l : Option Label} {r p : Row} {T : Ty_param} {c c₀ : Ty_param → FieldDesc}
    (hag : ∀ β, β ≠ T → c β = c₀ β) (hcT : SubField (c₀ T) (c T))
    (hr : rowFieldAt c l r = rowFieldAt c₀ l r) (hg : goalAt l r p c₀) : goalAt l r p c := by
  -- `c₀ ≤ c` everywhere: at `T` by `hcT`, off `T` by the agreement `hag`
  have hmono : ∀ β, SubField (c₀ β) (c β) := by
    intro β; by_cases h : β = T
    case pos => rw [h]; exact hcT
    case neg => rw [hag β h]; exact subField_refl _
  -- the super side grows monotonically from `c₀`; the sub side is unchanged (`hr`)
  show SubField (rowFieldAt c l r) (rowFieldAt c l p)
  rw [hr]
  exact subField_trans hg (rowFieldAt_mono hmono l p)

/-- A subfield of a required field is required. -/
theorem subField_isReq {x y : FieldDesc} (h : SubField x y) (hy : y.isReq = true) :
    x.isReq = true := by
  cases h with
  | req_req _ => rfl
  | req_opt _ => rfl
  | opt_opt _ => exact Bool.noConfusion hy

/- ---------------------------------------------------------------------------------------------- -/
/- ## 66. Masking irrelevance (R-relative liveness)                                               -/
/- ---------------------------------------------------------------------------------------------- -/

/-- An element that masks at `l` *relative to a set `R` of parameters known to be `Req`*: a definite
    masker (§64), or a spread of a parameter in `R` (whose field is `Req` under the assignment, so it
    overrides everything to its left under the rightmost-wins fold).  Generalises §64's `defMaskerElem`
    by also counting the `R`-parameters — the always-required rigids the masking analysis identifies. -/
def MaskerElemR (R : Ty_param → Prop) (l : Option Label) (e : SplatElem) : Prop :=
  defMaskerElem l e = true ∨ ∃ u, e = .spread (.rigid u) ∧ R u

/-- A list of elements contains an `R`-masker. -/
def MasksR (R : Ty_param → Prop) (l : Option Label) : List SplatElem → Prop
  | []      => False
  | e :: es => MaskerElemR R l e ∨ MasksR R l es

/-- `α` is **live relative to `R`**: it occurs as a spread `rigid α` with no `R`-masker to its right. -/
def LiveAtR (R : Ty_param → Prop) (l : Option Label) (α : Ty_param) : List SplatElem → Prop
  | []      => False
  | e :: es => LiveAtR R l α es ∨ (e = .spread (.rigid α) ∧ ¬ MasksR R l es)

/-- A row's `R`-relative liveness at `l` (none for a simple row). -/
def RowLiveAtR (R : Ty_param → Prop) (l : Option Label) (α : Ty_param) : Row → Prop
  | .simple _ _ => False
  | .splat es   => LiveAtR R l α es

/-- An element that is neither a definite masker nor a spread parameter is not an `R`-masker. -/
theorem not_maskerElemR_of {R : Ty_param → Prop} {l : Option Label} {e : SplatElem}
    (hdm : defMaskerElem l e = false) (hnr : ∀ u, e ≠ .spread (.rigid u)) :
    ¬ MaskerElemR R l e := by
  rintro (h | ⟨u, hu, _⟩)
  · rw [hdm] at h; exact Bool.noConfusion h
  · exact hnr u hu

/-- The `R`-relative analogue of `splatFoldAt_irrel` (§65): the fold is unchanged when the assignment
    moves only at `T`, provided `T` is not live relative to `R` and `R`'s parameters are required under
    the assignment (and distinct from `T`).  The `R`-masker case is the only addition over §65 — a
    spread `rigid u` with `u ∈ R` carries a `Req` field (`hRreq`) equal across the two assignments
    (`u ≠ T`), so it overrides both accumulators identically. -/
theorem splatFoldAt_irrelR {c c' : Ty_param → FieldDesc} {T : Ty_param} {l : Option Label}
    (R : Ty_param → Prop) (hag : ∀ β, β ≠ T → c β = c' β)
    (hRreq : ∀ u, R u → (c u).isReq = true) (hRneT : ∀ u, R u → u ≠ T) :
    ∀ (es : List SplatElem), NoNestedSplat es → ¬ LiveAtR R l T es →
      ∀ acc acc' : FieldDesc, (acc = acc' ∨ MasksR R l es) →
        splatFoldAt c l es acc = splatFoldAt c' l es acc' := by
  intro es
  induction es with
  | nil =>
    intro _ _ acc acc' hcc
    rcases hcc with rfl | hms
    case inl => rfl
    case inr => exact hms.elim
  | cons e es ih =>
    intro hflat hlive acc acc' hcc
    show splatFoldAt c l es (mergeField acc (elemFieldAt c l e))
       = splatFoldAt c' l es (mergeField acc' (elemFieldAt c' l e))
    have hdisj : mergeField acc (elemFieldAt c l e) = mergeField acc' (elemFieldAt c' l e)
        ∨ MasksR R l es := by
      cases e with
      | spread b =>
        cases b with
        | rigid β =>
          by_cases hβ : β = T
          case pos =>
            -- `e = spread (rigid T)`: not being `R`-live forces an `R`-masker in the tail
            subst hβ
            right
            by_contra hnm
            exact hlive (Or.inr ⟨rfl, hnm⟩)
          case neg =>
            by_cases hRβ : R β
            case pos =>
              -- `β ∈ R`: its field is `Req` under both assignments and overrides each accumulator
              left
              have hreq : (c β).isReq = true := hRreq β hRβ
              have hcc' : c β = c' β := hag β hβ
              show mergeField acc (c β) = mergeField acc' (c' β)
              rw [← hcc']
              cases hcβ : c β with
              | req b' => rw [mergeField_req, mergeField_req]
              | opt b' => rw [hcβ] at hreq; exact Bool.noConfusion hreq
            case neg =>
              -- `β ∉ R`, `β ≠ T`: not a masker; its field agrees across the two assignments
              have hcc' : c β = c' β := hag β hβ
              rcases hcc with hac | hms
              case inl => left; show mergeField acc (c β) = mergeField acc' (c' β); rw [hac, hcc']
              case inr =>
                right
                rcases hms with hme | hms'
                · exfalso
                  rcases hme with hdm | ⟨u, hu, hRu⟩
                  · exact Bool.noConfusion hdm
                  · simp only [SplatElem.spread.injEq, Base.rigid.injEq] at hu
                    subst hu; exact hRβ hRu
                · exact hms'
        | bot =>
          left
          show mergeField acc (FieldDesc.req .bot) = mergeField acc' (FieldDesc.req .bot)
          rw [mergeField_req, mergeField_req]
        | top =>
          rcases hcc with hac | hms
          case inl => left; show mergeField acc (FieldDesc.opt .bot) = mergeField acc' (FieldDesc.opt .bot); rw [hac]
          case inr =>
            right; rcases hms with hme | hms'
            · exact absurd hme (not_maskerElemR_of rfl (fun u hu => Base.noConfusion (SplatElem.spread.inj hu)))
            · exact hms'
        | prim p =>
          rcases hcc with hac | hms
          case inl => left; show mergeField acc (FieldDesc.opt .bot) = mergeField acc' (FieldDesc.opt .bot); rw [hac]
          case inr =>
            right; rcases hms with hme | hms'
            · exact absurd hme (not_maskerElemR_of rfl (fun u hu => Base.noConfusion (SplatElem.spread.inj hu)))
            · exact hms'
        | union a b =>
          rcases hcc with hac | hms
          case inl => left; show mergeField acc (FieldDesc.opt .bot) = mergeField acc' (FieldDesc.opt .bot); rw [hac]
          case inr =>
            right; rcases hms with hme | hms'
            · exact absurd hme (not_maskerElemR_of rfl (fun u hu => Base.noConfusion (SplatElem.spread.inj hu)))
            · exact hms'
        | shape row =>
          cases row with
          | simple fs u =>
            cases hf : projOpt (normalize fs u) l with
            | req t =>
              left
              show mergeField acc (projOpt (normalize fs u) l) = mergeField acc' (projOpt (normalize fs u) l)
              rw [hf, mergeField_req, mergeField_req]
            | opt t =>
              rcases hcc with hac | hms
              case inl =>
                left
                show mergeField acc (projOpt (normalize fs u) l) = mergeField acc' (projOpt (normalize fs u) l)
                rw [hac]
              case inr =>
                right; rcases hms with hme | hms'
                · refine absurd hme (not_maskerElemR_of ?_ (fun u hu => Base.noConfusion (SplatElem.spread.inj hu)))
                  simp only [defMaskerElem, hf, FieldDesc.isReq]
                · exact hms'
          | splat es' => exact absurd rfl (hflat.1 es')
    exact ih hflat.2 (fun h => hlive (Or.inl h)) _ _ hdisj

/-- A parameter not live relative to `R` does not change the field a spread list contributes. -/
theorem splatFieldAt_irrelR {c c' : Ty_param → FieldDesc} {T : Ty_param} {l : Option Label}
    (R : Ty_param → Prop) (hag : ∀ β, β ≠ T → c β = c' β)
    (hRreq : ∀ u, R u → (c u).isReq = true) (hRneT : ∀ u, R u → u ≠ T)
    (es : List SplatElem) (hflat : NoNestedSplat es) (hlive : ¬ LiveAtR R l T es) :
    splatFieldAt c l es = splatFieldAt c' l es := by
  cases es with
  | nil => rfl
  | cons e es' =>
    show splatFoldAt c l es' (elemFieldAt c l e) = splatFoldAt c' l es' (elemFieldAt c' l e)
    have hhead : elemFieldAt c l e = elemFieldAt c' l e ∨ MasksR R l es' := by
      cases e with
      | spread b =>
        cases b with
        | rigid β =>
          by_cases hβ : β = T
          case pos => subst hβ; right; by_contra hnm; exact hlive (Or.inr ⟨rfl, hnm⟩)
          case neg => left; show c β = c' β; exact hag β hβ
        | bot => left; rfl
        | top => left; rfl
        | prim p => left; rfl
        | union a b => left; rfl
        | shape row =>
          cases row with
          | simple fs u => left; rfl
          | splat es'' => exact absurd rfl (hflat.1 es'')
    exact splatFoldAt_irrelR R hag hRreq hRneT es' hflat.2 (fun h => hlive (Or.inl h)) _ _ hhead

/-- **Masking irrelevance.**  A parameter not live relative to `R` in a row does not change the row's
    field there.  When `R` is the set of always-required parameters, this is the masking shortcut's
    independence fact: a parameter overwritten by an always-required field to its right cannot affect
    the projection. -/
theorem rowFieldAt_liveAtR_irrel {c c' : Ty_param → FieldDesc} {T : Ty_param} {l : Option Label}
    (R : Ty_param → Prop) (hag : ∀ β, β ≠ T → c β = c' β)
    (hRreq : ∀ u, R u → (c u).isReq = true) (hRneT : ∀ u, R u → u ≠ T)
    (r : Row) (hflat : RowNoNested r) (hlive : ¬ RowLiveAtR R l T r) :
    rowFieldAt c l r = rowFieldAt c' l r := by
  cases r with
  | simple fs u => rfl
  | splat es    => exact splatFieldAt_irrelR R hag hRreq hRneT es hflat hlive

end Splat
