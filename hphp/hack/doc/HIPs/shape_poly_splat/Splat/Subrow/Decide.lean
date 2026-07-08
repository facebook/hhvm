import Splat.Subrow.Complete

/-!
# Toward decidability: the in-box corner enumeration

Decidability of `SubField` (given `Decidable SubBase`, the deferred F-sub decider), and the per-label
corner machinery: `rigidCorners`, the ≤4 in-box `{req,opt}×{lo,hi}` corners of a parameter's field
box, with the membership lemmas that connect a `setBase`-corner to it.  These feed the finite corner
enumeration that makes the box-corner check decidable.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 36. Decidability of `SubField`, and the in-box corners of a box                             -/
/- ---------------------------------------------------------------------------------------------- -/

variable [∀ a b : Base, Decidable (SubBase a b)]

/-- `SubField` is decidable as soon as `SubBase` is: case on the two requirednesses (the `opt`/`req`
    pairing is uninhabited; the other three carry a `SubBase`). -/
instance instDecidableSubField : ∀ d e : FieldDesc, Decidable (SubField d e)
  | .req t, .req u =>
    decidable_of_iff (SubBase t u) ⟨SubField.req_req, fun h => by cases h; assumption⟩
  | .req t, .opt u =>
    decidable_of_iff (SubBase t u) ⟨SubField.req_opt, fun h => by cases h; assumption⟩
  | .opt t, .opt u =>
    decidable_of_iff (SubBase t u) ⟨SubField.opt_opt, fun h => by cases h; assumption⟩
  | .opt _, .req _ => isFalse (fun h => by cases h)

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- A field whose requiredness is `d`'s and whose base lies between `lo`'s and `hi`'s bases is in the
    box `[lo, hi]`. -/
theorem setBase_in_box {lo hi d : FieldDesc} (h1 : SubField lo d) (h2 : SubField d hi) {t : Base}
    (ht1 : SubBase lo.base t) (ht2 : SubBase t hi.base) :
    SubField lo (d.setBase t) ∧ SubField (d.setBase t) hi := by
  cases d with
  | req dt =>
    -- `d.setBase t = .req t`; `h1`/`h2` force `lo = .req _` and `hi = .req _`/`.opt _`
    cases h1 with
    | req_req _ =>
      cases h2 with
      | req_req _ => exact ⟨SubField.req_req ht1, SubField.req_req ht2⟩
      | req_opt _ => exact ⟨SubField.req_req ht1, SubField.req_opt ht2⟩
  | opt dt =>
    -- `d.setBase t = .opt t`; `h2` forces `hi = .opt _`, `h1` allows `lo = .opt _`/`.req _`
    cases h2 with
    | opt_opt _ =>
      cases h1 with
      | opt_opt _ => exact ⟨SubField.opt_opt ht1, SubField.opt_opt ht2⟩
      | req_opt _ => exact ⟨SubField.req_opt ht1, SubField.opt_opt ht2⟩

/-- The in-box corners of a field box `[lo, hi]`: the four `{req,opt} × {lo.base, hi.base}` candidates
    kept only when actually inside the box (degenerate boxes give fewer). -/
def rigidCorners (lo hi : FieldDesc) : List FieldDesc :=
  [FieldDesc.req lo.base, .opt lo.base, .req hi.base, .opt hi.base].filter
    (fun d => decide (SubField lo d ∧ SubField d hi))

/-- A corner is in the box. -/
theorem mem_rigidCorners {lo hi d : FieldDesc} (h : d ∈ rigidCorners lo hi) :
    SubField lo d ∧ SubField d hi := by
  rw [rigidCorners, List.mem_filter] at h
  exact of_decide_eq_true h.2

/-- The `lo`-base corner of an in-box field is one of the box's corners. -/
theorem mem_rigidCorners_lo {lo hi d : FieldDesc} (h1 : SubField lo d) (h2 : SubField d hi) :
    d.setBase lo.base ∈ rigidCorners lo hi := by
  rw [rigidCorners, List.mem_filter]
  refine ⟨?_, decide_eq_true (setBase_in_box h1 h2 SubBase.refl
    (subField_base (subField_trans h1 h2)))⟩
  rcases FieldDesc.setBase_cases d lo.base with h | h <;> rw [h] <;> simp

/-- The `hi`-base corner of an in-box field is one of the box's corners. -/
theorem mem_rigidCorners_hi {lo hi d : FieldDesc} (h1 : SubField lo d) (h2 : SubField d hi) :
    d.setBase hi.base ∈ rigidCorners lo hi := by
  rw [rigidCorners, List.mem_filter]
  refine ⟨?_, decide_eq_true (setBase_in_box h1 h2
    (subField_base (subField_trans h1 h2)) SubBase.refl)⟩
  rcases FieldDesc.setBase_cases d hi.base with h | h <;> rw [h] <;> simp

/- ---------------------------------------------------------------------------------------------- -/
/- ## 37. The finite corner enumeration over a parameter list                                      -/
/- ---------------------------------------------------------------------------------------------- -/

/-- All assignments pinning each parameter in `Ts` to one of its box corners, every other parameter
    to the default `dflt`.  A cartesian product over the list. -/
def cornerAssigns (box : Ty_param → FieldDesc × FieldDesc) (dflt : Ty_param → FieldDesc) :
    List Ty_param → List (Ty_param → FieldDesc)
  | []      => [dflt]
  | U :: Us => (cornerAssigns box dflt Us).flatMap
      (fun a => (rigidCorners (box U).1 (box U).2).map (fun d => Function.update a U d))

/-- Membership in `cornerAssigns`: an assignment is enumerated iff it pins each `Ts`-parameter to one
    of its box corners and is the default elsewhere. -/
theorem mem_cornerAssigns {box : Ty_param → FieldDesc × FieldDesc} {dflt : Ty_param → FieldDesc} :
    ∀ (Ts : List Ty_param), Ts.Nodup → ∀ (a : Ty_param → FieldDesc),
      a ∈ cornerAssigns box dflt Ts ↔
        ((∀ U ∈ Ts, a U ∈ rigidCorners (box U).1 (box U).2) ∧ (∀ U, U ∉ Ts → a U = dflt U))
  | [], _, a => by
    simp only [cornerAssigns, List.mem_singleton]
    constructor
    · rintro rfl; exact ⟨by simp, fun _ _ => rfl⟩
    · rintro ⟨_, h2⟩; funext U; exact h2 U (by simp)
  | U :: Us, hnodup, a => by
    have hUnotin : U ∉ Us := (List.nodup_cons.mp hnodup).1
    have hnodup' : Us.Nodup := (List.nodup_cons.mp hnodup).2
    simp only [cornerAssigns, List.mem_flatMap, List.mem_map]
    constructor
    · -- decompose an enumerated assignment: a prefix-assignment `a'` updated at `U` to a corner `d`
      rintro ⟨a', ha', d, hd, rfl⟩
      rw [mem_cornerAssigns Us hnodup' a'] at ha'
      refine ⟨fun V hV => ?_, fun V hV => ?_⟩
      · rcases List.mem_cons.mp hV with rfl | hV
        · rw [Function.update_self]; exact hd
        · have hVneU : V ≠ U := fun h => hUnotin (h ▸ hV)
          rw [Function.update_of_ne hVneU]; exact ha'.1 V hV
      · rw [List.mem_cons, not_or] at hV
        rw [Function.update_of_ne hV.1]; exact ha'.2 V hV.2
    · -- conversely, build the prefix-assignment from `a` (reset at `U` to the default)
      rintro ⟨hcor, hdflt⟩
      refine ⟨Function.update a U (dflt U), ?_, a U, hcor U List.mem_cons_self, ?_⟩
      · rw [mem_cornerAssigns Us hnodup']
        refine ⟨fun V hV => ?_, fun V hV => ?_⟩
        · have hVneU : V ≠ U := fun h => hUnotin (h ▸ hV)
          rw [Function.update_of_ne hVneU]; exact hcor V (List.mem_cons_of_mem _ hV)
        · by_cases hVU : V = U
          · subst hVU; rw [Function.update_self]
          · rw [Function.update_of_ne hVU]
            exact hdflt V (by rw [List.mem_cons, not_or]; exact ⟨hVU, hV⟩)
      · funext V
        by_cases hVU : V = U
        · subst hVU; rw [Function.update_self]
        · rw [Function.update_of_ne hVU, Function.update_of_ne hVU]

/- ---------------------------------------------------------------------------------------------- -/
/- ## 38. Per-label finite reduction                                                               -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **Per-label finite reduction.**  At a label, the finite corner check (over `cornerAssigns`) is
    equivalent to the `∀`-in-box check.  Forward (`→`): an in-box `c`'s goal follows from its
    corner-completions (`peel_all`), each of which — restricted to `Ts`, defaulted elsewhere — is an
    enumerated corner assignment, agreeing with the completion on the rows' mentioned parameters
    (`rowFieldAt_congr`).  Backward (`←`): each corner assignment is in-box (`mem_rigidCorners`). -/
theorem perLabel_iff (r p : Row) (Ts : List Ty_param) (hnodup : Ts.Nodup)
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (box : Ty_param → FieldDesc × FieldDesc) (l : Option Label) :
    (∀ a ∈ cornerAssigns box (fun _ => .opt .bot) Ts,
        SubField (rowFieldAt a l r) (rowFieldAt a l p)) ↔
      (∀ c : Ty_param → FieldDesc,
        (∀ U ∈ Ts, SubField (box U).1 (c U) ∧ SubField (c U) (box U).2) →
        SubField (rowFieldAt c l r) (rowFieldAt c l p)) := by
  constructor
  · intro hfin c hcbox
    refine peel_all l r p box Ts hnodup c hcbox ?_
    intro g' _ hcor
    -- restrict the corner-completion to `Ts`; outside, the enumeration's default `opt ⊥`
    set a : Ty_param → FieldDesc := fun V => if V ∈ Ts then g' V else FieldDesc.opt .bot with ha
    have hamem : a ∈ cornerAssigns box (fun _ => .opt .bot) Ts := by
      rw [mem_cornerAssigns Ts hnodup]
      refine ⟨fun U hU => ?_, fun U hU => ?_⟩
      · simp only [ha]; rw [if_pos hU]
        rcases hcor U hU with h | h
        · rw [h]; exact mem_rigidCorners_lo (hcbox U hU).1 (hcbox U hU).2
        · rw [h]; exact mem_rigidCorners_hi (hcbox U hU).1 (hcbox U hU).2
      · simp only [ha]; rw [if_neg hU]
    have heqr : rowFieldAt g' l r = rowFieldAt a l r :=
      rowFieldAt_congr l r (fun α hα => by simp only [ha]; rw [if_pos (hrelr α hα)])
    have heqp : rowFieldAt g' l p = rowFieldAt a l p :=
      rowFieldAt_congr l p (fun α hα => by simp only [ha]; rw [if_pos (hrelp α hα)])
    rw [heqr, heqp]
    exact hfin a hamem
  · intro hRHS a hamem
    rw [mem_cornerAssigns Ts hnodup] at hamem
    exact hRHS a (fun U hU => mem_rigidCorners (hamem.1 U hU))

/- ---------------------------------------------------------------------------------------------- -/
/- ## 39. Label finiteness                                                                         -/
/- ---------------------------------------------------------------------------------------------- -/

/- The labels a row mentions *explicitly* (inline field keys, recursively through nested splat
   shapes).  A superset of the labels the row's projection can distinguish — every other label
   projects to an `unknown`. -/
mutual
  def baseLabels : Base → List Label
    | .shape (.simple fs _) => fs.map Prod.fst
    | .shape (.splat es)    => splatLabels es
    | .rigid _              => []
    | .bot                  => []
    | .top                  => []
    | .prim _               => []
    | .union _ _            => []
  def splatLabels : List SplatElem → List Label
    | []      => []
    | e :: es => elemLabels e ++ splatLabels es
  def elemLabels : SplatElem → List Label
    | .spread b => baseLabels b
end

/-- The explicit labels of a row. -/
def rowLabels : Row → List Label
  | .simple fs _ => fs.map Prod.fst
  | .splat es    => splatLabels es

/- The per-label field is the same at any two labels both absent from the explicit labels — every
   inline row projects to its `unknown` there, and the parameter / bottom contributions are
   label-independent.  Same mutual induction as the frame's §14 block. -/
omit [∀ a b : Base, Decidable (SubBase a b)] in
mutual
  theorem baseFieldAt_absent_eq (g : Ty_param → FieldDesc) {l l' : Label} :
      ∀ b : Base, l ∉ baseLabels b → l' ∉ baseLabels b →
        baseFieldAt g (some l) b = baseFieldAt g (some l') b
    | .rigid _,              _,  _   => rfl
    | .bot,                  _,  _   => rfl
    | .top,                  _,  _   => rfl
    | .prim _,               _,  _   => rfl
    | .union _ _,            _,  _   => rfl
    | .shape (.simple fs u), hl, hl' => by
        show proj (normalize fs u) l = proj (normalize fs u) l'
        rw [proj_normalize, proj_normalize, lookup_eq_none_keys fs l hl,
            lookup_eq_none_keys fs l' hl']
    | .shape (.splat es),    hl, hl' => splatFieldAt_absent_eq g es hl hl'

  theorem splatFieldAt_absent_eq (g : Ty_param → FieldDesc) {l l' : Label} :
      ∀ es : List SplatElem, l ∉ splatLabels es → l' ∉ splatLabels es →
        splatFieldAt g (some l) es = splatFieldAt g (some l') es
    | [],      _,  _   => rfl
    | e :: es, hl, hl' => by
        show splatFoldAt g (some l) es (elemFieldAt g (some l) e)
           = splatFoldAt g (some l') es (elemFieldAt g (some l') e)
        rw [elemFieldAt_absent_eq g e (fun hc => hl (List.mem_append_left _ hc))
              (fun hc => hl' (List.mem_append_left _ hc))]
        exact splatFoldAt_absent_eq g es (fun hc => hl (List.mem_append_right _ hc))
          (fun hc => hl' (List.mem_append_right _ hc)) (elemFieldAt g (some l') e)

  theorem splatFoldAt_absent_eq (g : Ty_param → FieldDesc) {l l' : Label} :
      ∀ es : List SplatElem, l ∉ splatLabels es → l' ∉ splatLabels es → ∀ acc : FieldDesc,
        splatFoldAt g (some l) es acc = splatFoldAt g (some l') es acc
    | [],      _,  _,   _   => rfl
    | e :: es, hl, hl', acc => by
        show splatFoldAt g (some l) es (mergeField acc (elemFieldAt g (some l) e))
           = splatFoldAt g (some l') es (mergeField acc (elemFieldAt g (some l') e))
        rw [elemFieldAt_absent_eq g e (fun hc => hl (List.mem_append_left _ hc))
              (fun hc => hl' (List.mem_append_left _ hc))]
        exact splatFoldAt_absent_eq g es (fun hc => hl (List.mem_append_right _ hc))
          (fun hc => hl' (List.mem_append_right _ hc)) (mergeField acc (elemFieldAt g (some l') e))

  theorem elemFieldAt_absent_eq (g : Ty_param → FieldDesc) {l l' : Label} :
      ∀ e : SplatElem, l ∉ elemLabels e → l' ∉ elemLabels e →
        elemFieldAt g (some l) e = elemFieldAt g (some l') e
    | .spread b, hl, hl' => baseFieldAt_absent_eq g b hl hl'
end

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- **Label finiteness.**  A row's per-label field is the same at any two labels both absent from its
    explicit labels — so checking one fresh label covers all unmentioned labels. -/
theorem rowFieldAt_absent_eq (g : Ty_param → FieldDesc) {l l' : Label} :
    ∀ r : Row, l ∉ rowLabels r → l' ∉ rowLabels r →
      rowFieldAt g (some l) r = rowFieldAt g (some l') r
  | .simple fs u, hl, hl' => by
      show proj (normalize fs u) l = proj (normalize fs u) l'
      rw [proj_normalize, proj_normalize, lookup_eq_none_keys fs l hl,
          lookup_eq_none_keys fs l' hl']
  | .splat es,    hl, hl' => splatFieldAt_absent_eq g es hl hl'

/- ---------------------------------------------------------------------------------------------- -/
/- ## 40. The ground-bounds decision procedure                                                    -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The corner enumeration depends only on the box at the parameters of `Ts`. -/
theorem cornerAssigns_congr {box box' : Ty_param → FieldDesc × FieldDesc}
    {dflt : Ty_param → FieldDesc} :
    ∀ Ts : List Ty_param, (∀ U ∈ Ts, box U = box' U) →
      cornerAssigns box dflt Ts = cornerAssigns box' dflt Ts
  | [],      _ => rfl
  | U :: Us, h => by
    simp only [cornerAssigns]
    rw [cornerAssigns_congr Us (fun V hV => h V (List.mem_cons_of_mem _ hV)),
        h U List.mem_cons_self]

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- The ground box `(proj (Lb U) ·, proj (Ub U) ·)` is constant across labels absent from the bound
    rows' explicit labels. -/
theorem box_absent (Lb Ub : Ty_param → SimpleRow) (U : Ty_param) {l l' : Label}
    (hLl : l ∉ (Lb U).known.map Prod.fst) (hUl : l ∉ (Ub U).known.map Prod.fst)
    (hLl' : l' ∉ (Lb U).known.map Prod.fst) (hUl' : l' ∉ (Ub U).known.map Prod.fst) :
    (proj (Lb U) l, proj (Ub U) l) = (proj (Lb U) l', proj (Ub U) l') := by
  rw [proj_not_mem (Lb U) l hLl, proj_not_mem (Ub U) l hUl,
      proj_not_mem (Lb U) l' hLl', proj_not_mem (Ub U) l' hUl']

/- Ground-corner helpers: the in-box corners of a *ground* box are themselves rigid-free.  This is
   what lets the completeness lemma (§34) fire at each enumerated corner — its `hcg` needs the corner
   field to be parameter-free, which holds because the box endpoints come from ground bound rows. -/

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- A rigid-free field's base is fixed by any substitution (its requiredness aside). -/
theorem rigidFree_base {fd : FieldDesc} (h : RigidFreeField fd) (ρ' : GEnv) :
    substBase ρ' fd.base = fd.base := by
  cases fd with
  | req t => exact FieldDesc.req.inj (h ρ')   -- `substField ρ' (.req t) = .req t` ⇒ `substBase ρ' t = t`
  | opt t => exact FieldDesc.opt.inj (h ρ')

omit [∀ a b : Base, Decidable (SubBase a b)] in
/-- Projecting a *ground* simple row (rigid-free known fields and `unknown`) gives a rigid-free
    field: the projection is either a looked-up known field or the `unknown`. -/
theorem rigidFreeField_proj {R : SimpleRow} (hk : GroundFields R.known)
    (hu : RigidFreeBase R.unknown) (l : Label) : RigidFreeField (proj R l) := by
  show RigidFreeField (match R.known.lookup l with | some d => d | none => .opt R.unknown)
  cases hl : R.known.lookup l with
  | none   => exact rigidFreeField_opt hu       -- absent label projects to `.opt unknown`
  | some d => exact groundFields_lookup hk hl   -- present label projects to a known (ground) field

/-- An in-box corner of a box with rigid-free endpoints is rigid-free: every corner candidate is a
    `req`/`opt` of `lo.base` or `hi.base`, each fixed by `rigidFree_base`. -/
theorem rigidFreeField_of_mem_rigidCorners {lo hi d : FieldDesc}
    (hlo : RigidFreeField lo) (hhi : RigidFreeField hi) (hd : d ∈ rigidCorners lo hi) :
    RigidFreeField d := by
  -- a corner passes the filter, so it is one of the four `{req,opt} × {lo.base, hi.base}` candidates
  rw [rigidCorners, List.mem_filter] at hd
  obtain ⟨hmem, _⟩ := hd
  simp only [List.mem_cons, List.not_mem_nil, or_false] at hmem
  intro ρ'
  obtain rfl | rfl | rfl | rfl := hmem
  · -- `d = .req lo.base`
    show FieldDesc.req (substBase ρ' lo.base) = FieldDesc.req lo.base
    rw [rigidFree_base hlo ρ']
  · -- `d = .opt lo.base`
    show FieldDesc.opt (substBase ρ' lo.base) = FieldDesc.opt lo.base
    rw [rigidFree_base hlo ρ']
  · -- `d = .req hi.base`
    show FieldDesc.req (substBase ρ' hi.base) = FieldDesc.req hi.base
    rw [rigidFree_base hhi ρ']
  · -- `d = .opt hi.base`
    show FieldDesc.opt (substBase ρ' hi.base) = FieldDesc.opt hi.base
    rw [rigidFree_base hhi ρ']

/-- **The ground-bounds characterization.**  For `Ground` rows over `Ground` (parameter-free) bounds,
    `SemSubRow` is equivalent to a *finite* check: at each label in a finite list — the rows' and
    bounds' explicit labels, plus one fresh `lf` standing for every other (unmentioned) label — the
    per-label goal holds at every enumerated corner assignment.

    Forward (completeness): `SemSubRow` forces the goal at *every* in-box corner at *every* label, via
    §34 `subrow_corner_complete_ground` per corner — each enumerated corner is ground in-box, by
    `mem_rigidCorners` (membership) and `rigidFreeField_of_mem_rigidCorners` (groundness, since the box
    endpoints come from the ground bound rows) — so in particular at the finite ones.  Backward
    (soundness): label-finiteness extends the finite check to *all* labels — `box_absent` +
    `cornerAssigns_congr` carry the corner enumeration, and `rowFieldAt_absent_eq` the goal, from any
    unmentioned label to `lf` — then `perLabel_iff` lifts the corner check to the `∀`-in-box check
    feeding §35 `subrow_ground_sound`. -/
theorem semSubRow_iff_finite (r p : Row) (Γ : TyParamEnv) (Lb Ub : Ty_param → SimpleRow)
    (Ts : List Ty_param) (lf : Label)
    (hLb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.lower) = proj (Lb α) l)
    (hUb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.upper) = proj (Ub α) l)
    (hconsist : ∀ α, SubField_row (Lb α) (Ub α))
    (hnodup : Ts.Nodup) (hdecl : ∀ U, U ∈ Ts ↔ ∃ b, Γ U = some b)
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (hlf : lf ∉ rowLabels r ++ rowLabels p ++
        Ts.flatMap (fun U => (Lb U).known.map Prod.fst ++ (Ub U).known.map Prod.fst)) :
    SemSubRow r p Γ ↔
      ∀ l₀ ∈ rowLabels r ++ rowLabels p ++
          Ts.flatMap (fun U => (Lb U).known.map Prod.fst ++ (Ub U).known.map Prod.fst) ++ [lf],
        ∀ a ∈ cornerAssigns (fun α => (proj (Lb α) l₀, proj (Ub α) l₀)) (fun _ => .opt .bot) Ts,
          SubField (rowFieldAt a (some l₀) r) (rowFieldAt a (some l₀) p) := by
  constructor
  case mp =>
    -- completeness: `SemSubRow` forces the goal at each enumerated corner, at any label
    intro hsem l₀ _hl₀ a ha
    rw [mem_cornerAssigns Ts hnodup a] at ha
    apply subrow_corner_complete_ground r p Γ Ub Lb a l₀ hLb hUb hconsist
    · -- hcbox: each enumerated corner lies in its box
      intro α b hΓ
      exact mem_rigidCorners (ha.1 α ((hdecl α).mpr ⟨b, hΓ⟩))
    · -- hrel: a mentioned parameter is declared
      intro α hmen
      rcases hmen with hm | hm
      · exact (hdecl α).mp (hrelr α hm)
      · exact (hdecl α).mp (hrelp α hm)
    · exact hsem
  case mpr =>
    intro hfin
    -- the corner check holds at *every* label, transported from the finite list by label-finiteness
    have hcornerAll : ∀ l₀ : Label,
        ∀ a ∈ cornerAssigns (fun α => (proj (Lb α) l₀, proj (Ub α) l₀)) (fun _ => .opt .bot) Ts,
          SubField (rowFieldAt a (some l₀) r) (rowFieldAt a (some l₀) p) := by
      intro l₀ a ha
      by_cases hl₀ : l₀ ∈ rowLabels r ++ rowLabels p ++
          Ts.flatMap (fun U => (Lb U).known.map Prod.fst ++ (Ub U).known.map Prod.fst)
      case pos =>
        -- l₀ is one of the listed labels, so the finite check covers it directly
        exact hfin l₀ (List.mem_append.mpr (Or.inl hl₀)) a ha
      case neg =>
        -- l₀ absent from the rows and bounds: transport from the fresh label `lf`
        simp only [List.mem_append, not_or] at hl₀ hlf
        obtain ⟨⟨hl₀r, hl₀p⟩, hl₀b⟩ := hl₀
        obtain ⟨⟨hlfr, hlfp⟩, hlfb⟩ := hlf
        -- the corner enumerations at `l₀` and at `lf` coincide (box constant off the bound labels)
        have hcongr :
            cornerAssigns (fun α => (proj (Lb α) l₀, proj (Ub α) l₀)) (fun _ => .opt .bot) Ts
              = cornerAssigns (fun α => (proj (Lb α) lf, proj (Ub α) lf)) (fun _ => .opt .bot) Ts :=
          cornerAssigns_congr Ts (fun U hU =>
            box_absent Lb Ub U
              (fun hm => hl₀b (List.mem_flatMap.mpr ⟨U, hU, List.mem_append_left _ hm⟩))
              (fun hm => hl₀b (List.mem_flatMap.mpr ⟨U, hU, List.mem_append_right _ hm⟩))
              (fun hm => hlfb (List.mem_flatMap.mpr ⟨U, hU, List.mem_append_left _ hm⟩))
              (fun hm => hlfb (List.mem_flatMap.mpr ⟨U, hU, List.mem_append_right _ hm⟩)))
        rw [hcongr] at ha
        -- the finite check at `lf`, then carry the goal back to `l₀` (both labels unmentioned)
        have hatlf := hfin lf (List.mem_append.mpr (Or.inr (List.mem_singleton.mpr rfl))) a ha
        rw [rowFieldAt_absent_eq a r hlfr hl₀r, rowFieldAt_absent_eq a p hlfp hl₀p] at hatlf
        exact hatlf
    -- soundness: lift the corner check to the `∀`-in-box check (§35) via `perLabel_iff`
    apply subrow_ground_sound r p Γ Lb Ub hLb hUb
    intro l₀ c hcboxc
    refine (perLabel_iff r p Ts hnodup hrelr hrelp
        (fun α => (proj (Lb α) l₀, proj (Ub α) l₀)) (some l₀)).mp (hcornerAll l₀) c ?_
    -- the in-box condition for `c`, in the box-pair form `perLabel_iff` expects
    intro U hU
    obtain ⟨b, hΓ⟩ := (hdecl U).mp hU
    exact hcboxc U b hΓ

/-- **Ground-bounds decidability.**  Given the F-sub leaf decider `[∀ a b, Decidable (SubBase a b)]`,
    `SemSubRow` is decidable for `Ground` rows over `Ground` bounds: `semSubRow_iff_finite` reduces it
    to a finite conjunction of per-label, per-corner `SubField` checks (each decidable by
    `instDecidableSubField`).  Noncomputable only because the fresh label is chosen classically
    (`exists_fresh_label`); the reduced check is itself a concrete decidable list quantification. -/
noncomputable def decidableSemSubRow (r p : Row) (Γ : TyParamEnv) (Lb Ub : Ty_param → SimpleRow)
    (Ts : List Ty_param)
    (hLb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.lower) = proj (Lb α) l)
    (hUb : ∀ α b, Γ α = some b → ∀ ρ' l, evalAt ρ' (some l) (rowOfBase b.upper) = proj (Ub α) l)
    (hconsist : ∀ α, SubField_row (Lb α) (Ub α))
    (hnodup : Ts.Nodup) (hdecl : ∀ U, U ∈ Ts ↔ ∃ b, Γ U = some b)
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts) :
    Decidable (SemSubRow r p Γ) :=
  -- choose one fresh label to stand for every unmentioned label
  let h := exists_fresh_label (rowLabels r ++ rowLabels p ++
      Ts.flatMap (fun U => (Lb U).known.map Prod.fst ++ (Ub U).known.map Prod.fst))
  decidable_of_iff _
    (semSubRow_iff_finite r p Γ Lb Ub Ts h.choose hLb hUb
      hconsist hnodup hdecl hrelr hrelp h.choose_spec).symm

end Splat
