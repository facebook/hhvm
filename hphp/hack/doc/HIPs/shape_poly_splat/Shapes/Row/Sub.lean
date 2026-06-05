import Shapes.BaseTy.Merge

/- ========================================================================== -/
/-! # Row subtyping (`<:ʳ`)

Structural subtyping on rows, decomposed into per-field conditions.

- `<:` (semantic): `∀ v, denote τ₁ v → denote τ₂ v` — on types
- `<:ᶠ` (field): per-field-descriptor subtyping
- `<:ʳ` (row): `Row.NoDupKeys` on both + unknown ordering (`<:`) + per-field (`<:ᶠ`)

`<:ʳ` on rows lifts to `<:` on shapes via `row_sub_sound`.
Transitivity is free because `<:` and `<:ᶠ` are both transitive by composition. -/
/- ========================================================================== -/


/-- Row subtyping. Decomposes into Row.NoDupKeys on both sides, unknown-type
ordering (`<:`), and per-field subtyping (`<:ᶠ`). -/
def row_sub (ρ₁ ρ₂ : Row) : Prop :=
  Row.NoDupKeys ρ₁ ∧ Row.NoDupKeys ρ₂ ∧
  ρ₁.unknown <:ᵇ ρ₂.unknown ∧
  ∀ l, Row.proj ρ₁ l <:ᶠ Row.proj ρ₂ l

infixl:50 " <:ʳ " => row_sub


/- ========================================================================== -/
/-! ## Soundness -/
/- ========================================================================== -/

/-- Row sub lifts to type sub: if `ρ₁ <:ʳ ρ₂` then `.shape ρ₁ <:ᵇ .shape ρ₂`.
Uses `shape_denote_iff` to convert both sides to per-key form, then applies
the per-field hypothesis pointwise. -/
theorem row_sub_sound {ρ₁ ρ₂ : Row} (h : ρ₁ <:ʳ ρ₂) : .shape ρ₁ <:ᵇ .shape ρ₂ := by
  obtain ⟨hnd₁, hnd₂, _, hflds⟩ := h
  intro v hv
  cases v with
  | nat => rw [denote_shape_nat] at hv; exact absurd hv id
  | bool => rw [denote_shape_bool] at hv; exact absurd hv id
  | record entries =>
    rw [shape_denote_iff ρ₁ _ hnd₁] at hv
    rw [shape_denote_iff ρ₂ _ hnd₂]
    exact ⟨hv.1, fun k => hflds k _ (hv.2 k)⟩

/- ========================================================================== -/
/-! ## Incompleteness

`row_sub` is sound but not complete w.r.t. semantic sub. When a shape is
uninhabited (e.g., has a required field of type `bot`), semantic sub holds
vacuously for any target, but `row_sub` still checks fields pointwise. -/
/- ========================================================================== -/

private def r_uninhab : Row := .mk [("x", .req .bot), ("y", .req .nat)] .bot
private def r_target  : Row := .mk [("y", .req .bool)] .bot

/-- Semantic sub holds vacuously: `shape('x' => bot, 'y' => nat)` is uninhabited. -/
private theorem uninhab_sem_sub : .shape r_uninhab <:ᵇ .shape r_target := by
  intro v hv
  cases v with
  | nat => simp at hv
  | bool => simp at hv
  | record entries => simp [r_uninhab] at hv

/-- Row sub fails: `.req .nat <:ᶠ .req .bool` doesn't hold. -/
private theorem uninhab_not_row_sub : ¬ (r_uninhab <:ʳ r_target) := by
  intro ⟨_, _, _, hf⟩
  have := hf "y" (some (.nat 0))
  simp [Row.proj, r_uninhab, r_target, Row.fields, Row.unknown, fieldCheck] at this

/-- `row_sub` is strictly stronger than semantic sub on shapes: there exist
rows where `.shape r₁ <:ᵇ .shape r₂` but `¬ (r₁ <:ʳ r₂)`. -/
theorem row_sub_not_complete :
    ∃ r₁ r₂ : Row, Row.NoDupKeys r₁ ∧ Row.NoDupKeys r₂ ∧
      (.shape r₁ <:ᵇ .shape r₂) ∧ ¬ (r₁ <:ʳ r₂) :=
  ⟨r_uninhab, r_target,
   by simp [Row.NoDupKeys, r_uninhab],
   by simp [Row.NoDupKeys, r_target],
   uninhab_sem_sub,
   uninhab_not_row_sub⟩

/- ========================================================================== -/
/-! ## Reflexivity and transitivity -/
/- ========================================================================== -/

theorem row_sub_refl {ρ : Row} (hnd : Row.NoDupKeys ρ) : ρ <:ʳ ρ :=
  ⟨hnd, hnd, sub_refl _, fun _ _ h => h⟩

theorem row_sub_trans {ρ₁ ρ₂ ρ₃ : Row}
    (h₁₂ : ρ₁ <:ʳ  ρ₂) (h₂₃ : ρ₂ <:ʳ ρ₃) : ρ₁ <:ʳ ρ₃ := by
  obtain ⟨hnd₁, _, hu₁₂, hf₁₂⟩ := h₁₂
  obtain ⟨_, hnd₃, hu₂₃, hf₂₃⟩ := h₂₃
  exact ⟨hnd₁, hnd₃, sub_trans hu₁₂ hu₂₃, fun k val h => hf₂₃ k val (hf₁₂ k val h)⟩

/- ========================================================================== -/
/-! ## Merge congruence -/
/- ========================================================================== -/

/-- Merge is congruent in the right argument under `<:ʳ`. -/
theorem merge_congr_right {left right₁ right₂ : Row}
    (h : right₁ <:ʳ right₂) : mergeRow left right₁ <:ʳ mergeRow left right₂ := by
  obtain ⟨hnd₁, hnd₂, hru, hrf⟩ := h
  cases right₁ with
  | var _ => exact absurd hnd₁ id
  | mk rf₁ ru₁ => cases right₂ with
  | var _ => exact absurd hnd₂ id
  | mk rf₂ ru₂ =>
  refine ⟨mergeRow_nodupKeys _ _, mergeRow_nodupKeys _ _, ?_, ?_⟩
  · show .union left.unknown ru₁ <:ᵇ .union left.unknown ru₂
    exact union_sub (sub_union_l left.unknown ru₂) (sub_trans hru (sub_union_r left.unknown ru₂))
  · intro k; rw [proj_mergeRow, proj_mergeRow]
    exact mergeFieldDesc_mono (fun v h => h) (hrf k)

/-- Merge is congruent in the left argument under `<:ʳ`. -/
theorem merge_congr_left {left₁ left₂ right : Row}
    (h : left₁ <:ʳ left₂) : mergeRow left₁ right <:ʳ mergeRow left₂ right := by
  obtain ⟨hnd₁, hnd₂, hlu, hlf⟩ := h
  cases left₁ with
  | var _ => exact absurd hnd₁ id
  | mk lf₁ lu₁ => cases left₂ with
  | var _ => exact absurd hnd₂ id
  | mk lf₂ lu₂ =>
  refine ⟨mergeRow_nodupKeys _ _, mergeRow_nodupKeys _ _, ?_, ?_⟩
  · show .union lu₁ right.unknown <:ᵇ .union lu₂ right.unknown
    exact union_sub (sub_trans hlu (sub_union_l lu₂ right.unknown)) (sub_union_r lu₂ right.unknown)
  · intro k; rw [proj_mergeRow, proj_mergeRow]
    exact mergeFieldDesc_mono (hlf k) (fun v h => h)

/- ========================================================================== -/
/-! ## Completeness boundary

`row_sub` is sound but incomplete w.r.t. semantic sub. The gap is exactly
uninhabitedness: when `∃ v, denote (.shape ρ₁) v`, semantic sub implies
structural row sub. -/
/- ========================================================================== -/

/- ========================================================================== -/
/-! ### List manipulation helpers -/
/- ========================================================================== -/

private def replaceVal (k : String) (v : Val) : List (String × Val) → List (String × Val)
  | [] => [(k, v)]
  | (k', v') :: rest =>
    if k' == k then (k, v) :: rest else (k', v') :: replaceVal k v rest

private def removeKey (k : String) : List (String × Val) → List (String × Val)
  | [] => []
  | (k', v') :: rest =>
    if k' == k then rest else (k', v') :: removeKey k rest

private theorem mem_map_fst {l : List (String × Val)} {k : String} {v : Val}
    (h : (k, v) ∈ l) : k ∈ l.map Prod.fst :=
  List.mem_map.mpr ⟨(k, v), h, rfl⟩

/- ========================================================================== -/
/-! #### replaceVal -/
/- ========================================================================== -/

private theorem replaceVal_lookup_self (k : String) (v : Val) (l : List (String × Val)) :
    (replaceVal k v l).lookup k = some v := by
  induction l with
  | nil => simp [replaceVal]
  | cons hd tl ih =>
    obtain ⟨hk, hv⟩ := hd
    simp only [replaceVal]
    by_cases heq : hk = k
    · subst heq; rw [if_pos (by simp)]; exact List.lookup_cons_self
    · rw [if_neg (by simp [beq_iff_eq, heq])]
      rw [lookup_cons_ne' hv (replaceVal k v tl) (Ne.symm heq)]
      exact ih

private theorem replaceVal_lookup_ne (k₁ k₂ : String) (v : Val) (l : List (String × Val))
    (h : k₁ ≠ k₂) :
    (replaceVal k₂ v l).lookup k₁ = l.lookup k₁ := by
  induction l with
  | nil =>
    simp only [replaceVal]
    rw [lookup_cons_ne' v [] h]
  | cons hd tl ih =>
    obtain ⟨hk, hv⟩ := hd
    simp only [replaceVal]
    by_cases heq : hk = k₂
    · subst heq
      rw [if_pos (by simp)]
      rw [lookup_cons_ne' v tl h, lookup_cons_ne' hv tl h]
    · rw [if_neg (by simp [beq_iff_eq, heq])]
      by_cases heq2 : k₁ = hk
      · subst heq2; simp [List.lookup_cons_self]
      · rw [lookup_cons_ne' hv (replaceVal k₂ v tl) heq2,
            lookup_cons_ne' hv tl heq2, ih]

private theorem replaceVal_keys (k : String) (v : Val) (l : List (String × Val))
    {k' : String} (h : k' ∈ (replaceVal k v l).map Prod.fst) :
    k' = k ∨ k' ∈ l.map Prod.fst := by
  rw [List.mem_map] at h
  obtain ⟨⟨pk, pv⟩, hmem, hpk⟩ := h
  simp at hpk; subst hpk
  induction l with
  | nil =>
    simp [replaceVal] at hmem
    exact Or.inl hmem.1
  | cons hd tl ih =>
    obtain ⟨hk, hv⟩ := hd
    simp only [replaceVal] at hmem
    split at hmem
    · next heq =>
      rw [beq_iff_eq] at heq
      rcases List.mem_cons.mp hmem with ⟨rfl, rfl⟩ | htl
      · exact Or.inl rfl
      · exact Or.inr (mem_map_fst (List.mem_cons.mpr (Or.inr htl)))
    · rcases List.mem_cons.mp hmem with ⟨rfl, rfl⟩ | htl
      · exact Or.inr (mem_map_fst (List.mem_cons.mpr (Or.inl rfl)))
      · rcases ih htl with rfl | hmem'
        · exact Or.inl rfl
        · exact Or.inr (by rw [List.map_cons]; exact List.mem_cons.mpr (Or.inr hmem'))

private theorem replaceVal_nodup (k : String) (v : Val) (l : List (String × Val))
    (hnd : (l.map Prod.fst).Nodup) :
    ((replaceVal k v l).map Prod.fst).Nodup := by
  induction l with
  | nil => simp [replaceVal]
  | cons hd tl ih =>
    obtain ⟨hk, hv⟩ := hd
    simp only [replaceVal]
    split
    · next heq => rw [beq_iff_eq] at heq; subst heq; simpa using hnd
    · next hne =>
      rw [List.map_cons, List.nodup_cons] at hnd ⊢
      exact ⟨fun hmem => by
        rcases replaceVal_keys k v tl hmem with rfl | hmem'
        · simp at hne
        · exact hnd.1 hmem', ih hnd.2⟩

/- ========================================================================== -/
/-! #### removeKey -/
/- ========================================================================== -/

private theorem removeKey_lookup_self (k : String) (l : List (String × Val))
    (hnd : (l.map Prod.fst).Nodup) :
    (removeKey k l).lookup k = none := by
  induction l with
  | nil => rfl
  | cons hd tl ih =>
    obtain ⟨hk, hv⟩ := hd
    simp only [removeKey]
    rw [List.map_cons, List.nodup_cons] at hnd
    by_cases heq : hk = k
    · subst heq; rw [if_pos (by simp)]
      exact lookup_none_of_not_mem_keys hnd.1
    · rw [if_neg (by simp [beq_iff_eq, heq])]
      rw [lookup_cons_ne' hv (removeKey k tl) (Ne.symm heq)]
      exact ih hnd.2

private theorem removeKey_lookup_ne (k₁ k₂ : String) (l : List (String × Val))
    (h : k₁ ≠ k₂) :
    (removeKey k₂ l).lookup k₁ = l.lookup k₁ := by
  induction l with
  | nil => rfl
  | cons hd tl ih =>
    obtain ⟨hk, hv⟩ := hd
    simp only [removeKey]
    by_cases heq : hk = k₂
    · subst heq
      rw [if_pos (by simp)]
      exact (lookup_cons_ne' hv tl h).symm
    · rw [if_neg (by simp [beq_iff_eq, heq])]
      by_cases heq2 : k₁ = hk
      · subst heq2; simp [List.lookup_cons_self]
      · rw [lookup_cons_ne' hv (removeKey k₂ tl) heq2,
            lookup_cons_ne' hv tl heq2, ih]

private theorem removeKey_keys (k : String) (l : List (String × Val))
    {k' : String} (h : k' ∈ (removeKey k l).map Prod.fst) :
    k' ∈ l.map Prod.fst := by
  rw [List.mem_map] at h
  obtain ⟨⟨pk, pv⟩, hmem, hpk⟩ := h
  simp at hpk; subst hpk
  induction l with
  | nil => simp [removeKey] at hmem
  | cons hd tl ih =>
    obtain ⟨hk, hv⟩ := hd
    simp only [removeKey] at hmem
    split at hmem
    · exact mem_map_fst (List.mem_cons.mpr (Or.inr hmem))
    · rcases List.mem_cons.mp hmem with ⟨rfl, rfl⟩ | htl
      · exact mem_map_fst (List.mem_cons.mpr (Or.inl rfl))
      · rw [List.map_cons]; exact List.mem_cons.mpr (Or.inr (ih htl))

private theorem removeKey_nodup (k : String) (l : List (String × Val))
    (hnd : (l.map Prod.fst).Nodup) :
    ((removeKey k l).map Prod.fst).Nodup := by
  induction l with
  | nil => exact hnd
  | cons hd tl ih =>
    obtain ⟨hk, hv⟩ := hd
    simp only [removeKey]
    split
    · rw [List.map_cons, List.nodup_cons] at hnd; exact hnd.2
    · rw [List.map_cons, List.nodup_cons] at hnd ⊢
      exact ⟨fun hmem => hnd.1 (removeKey_keys k tl hmem), ih hnd.2⟩

/- ========================================================================== -/
/-! #### Fresh string -/
/- ========================================================================== -/

private theorem foldl_append_length_ge (acc : String) (l : List String) :
    acc.length ≤ (l.foldl (fun r s => r ++ s) acc).length := by
  induction l generalizing acc with
  | nil => simp [List.foldl]
  | cons hd tl ih =>
    simp only [List.foldl]
    calc acc.length
        ≤ (acc ++ hd).length := by rw [String.length_append]; omega
      _ ≤ _ := ih _

private theorem foldl_append_length_ge_mem
    {l : List String} {s : String} (h : s ∈ l) (acc : String) :
    s.length ≤ (l.foldl (fun r s => r ++ s) acc).length := by
  induction l generalizing acc with
  | nil => simp at h
  | cons hd tl ih =>
    simp only [List.foldl]
    rcases List.mem_cons.mp h with rfl | hmem
    · calc s.length
          ≤ (acc ++ s).length := by rw [String.length_append]; omega
        _ ≤ _ := foldl_append_length_ge _ _
    · exact ih hmem _

private theorem exists_not_mem (l : List String) : ∃ s, s ∉ l := by
  refine ⟨l.foldl (fun r s => r ++ s) "" ++ "a", fun hmem => ?_⟩
  have h1 := String.length_append (l.foldl (fun r s => r ++ s) "") "a"
  have h2 := foldl_append_length_ge_mem hmem ""
  have h3 : "a".length ≥ 1 := by native_decide
  omega

/- ========================================================================== -/
/-! ### Incompleteness -/
/- ========================================================================== -/

private def ρ_bot_open : Row := .mk [("x", .req .bot)] .top
private def ρ_closed   : Row := .mk [] .bot

private theorem sem_sub_bot_open : .shape ρ_bot_open <:ᵇ .shape ρ_closed := by
  intro v hv
  cases v with
  | nat => exact absurd hv (by simp)
  | bool => exact absurd hv (by simp)
  | record entries =>
    simp only [ρ_bot_open] at hv
    rw [denote_shape] at hv
    obtain ⟨_, hknown, _⟩ := hv
    rw [knownFields_req] at hknown
    obtain ⟨⟨_, _, hbot⟩, _⟩ := hknown
    exact absurd hbot (by simp)

private theorem not_row_sub_bot_open : ¬ (ρ_bot_open <:ʳ ρ_closed) := by
  intro ⟨_, _, hunk, _⟩
  have h := hunk (.nat 0) (by simp [ρ_bot_open, Row.unknown])
  simp [ρ_closed, Row.unknown] at h

/-- Semantic sub does not imply structural row sub: a shape with a required
field of type `⊥` and unknown type `⊤` is semantically below everything
(vacuously), but `row_sub` rejects the unknown ordering `⊤ <:ᵇ ⊥`. -/
theorem row_sub_incomplete :
    ∃ ρ₁ ρ₂ : Row, Row.NoDupKeys ρ₁ ∧ Row.NoDupKeys ρ₂ ∧
      (.shape ρ₁ <:ᵇ .shape ρ₂) ∧ ¬ (ρ₁ <:ʳ ρ₂) :=
  ⟨ρ_bot_open, ρ_closed,
   by simp [Row.NoDupKeys, ρ_bot_open],
   by simp [Row.NoDupKeys, ρ_closed],
   sem_sub_bot_open,
   not_row_sub_bot_open⟩

/- ========================================================================== -/
/-! ### Completeness under inhabitedness -/
/- ========================================================================== -/

/-- When `ρ₁` is inhabited, semantic subtyping implies structural row
subtyping. The inhabitedness hypothesis rules out vacuous-truth cases.

The proof constructs modified records to probe each field independently:
`replaceVal k v` forces `lookup k = some v`; `removeKey k` forces
`lookup k = none`. Each modified record inherits inhabitedness from the
witness, so semantic sub transfers the field check to `ρ₂`. -/
theorem row_sub_complete_inhabited
    {ρ₁ ρ₂ : Row}
    (hnd₁ : Row.NoDupKeys ρ₁) (hnd₂ : Row.NoDupKeys ρ₂)
    (hsem : .shape ρ₁ <:ᵇ .shape ρ₂)
    (hinhab : ∃ v, denote (.shape ρ₁) v) :
    ρ₁ <:ʳ ρ₂ := by
  obtain ⟨v₀, hv₀⟩ := hinhab
  cases v₀ with
  | nat => exact absurd hv₀ (by simp)
  | bool => exact absurd hv₀ (by simp)
  | record entries₀ =>
  have hρ₁ := (shape_denote_iff ρ₁ entries₀ hnd₁).mp hv₀
  obtain ⟨hnd_e₀, hfc₁⟩ := hρ₁
  have hfld : ∀ k, Row.proj ρ₁ k <:ᶠ Row.proj ρ₂ k := by
    intro k val hval
    cases val with
    | some v =>
      have hnd_e₁ := replaceVal_nodup k v entries₀ hnd_e₀
      have hfc₁' : ∀ k', fieldCheck (Row.proj ρ₁ k') ((replaceVal k v entries₀).lookup k') := by
        intro k'
        by_cases hk : k' = k
        · rw [hk, replaceVal_lookup_self]; exact hval
        · rw [replaceVal_lookup_ne k' k v entries₀ hk]; exact hfc₁ k'
      have hv₁ := (shape_denote_iff ρ₁ _ hnd₁).mpr ⟨hnd_e₁, hfc₁'⟩
      have hv₂ := (shape_denote_iff ρ₂ _ hnd₂).mp (hsem _ hv₁)
      have := hv₂.2 k
      rw [replaceVal_lookup_self] at this
      exact this
    | none =>
      have hnd_e₁ := removeKey_nodup k entries₀ hnd_e₀
      have hfc₁' : ∀ k', fieldCheck (Row.proj ρ₁ k') ((removeKey k entries₀).lookup k') := by
        intro k'
        by_cases hk : k' = k
        · rw [hk, removeKey_lookup_self k entries₀ hnd_e₀]; exact hval
        · rw [removeKey_lookup_ne k' k entries₀ hk]; exact hfc₁ k'
      have hv₁ := (shape_denote_iff ρ₁ _ hnd₁).mpr ⟨hnd_e₁, hfc₁'⟩
      have hv₂ := (shape_denote_iff ρ₂ _ hnd₂).mp (hsem _ hv₁)
      have := hv₂.2 k
      rw [removeKey_lookup_self k entries₀ hnd_e₀] at this
      exact this
  have hunk : ρ₁.unknown <:ᵇ ρ₂.unknown := by
    cases ρ₁ with
    | var _ => exact absurd hnd₁ id
    | mk f₁ u₁ => cases ρ₂ with
    | var _ => exact absurd hnd₂ id
    | mk f₂ u₂ =>
    intro v hv
    obtain ⟨fresh, hfresh⟩ := exists_not_mem (f₁.map Prod.fst ++ f₂.map Prod.fst)
    have hf₁ : fresh ∉ f₁.map Prod.fst := fun h => hfresh (List.mem_append_left _ h)
    have hf₂ : fresh ∉ f₂.map Prod.fst := fun h => hfresh (List.mem_append_right _ h)
    have hp₁ : Row.proj (.mk f₁ u₁) fresh = .opt u₁ := by
      simp [Row.proj, Row.fields, Row.unknown, lookup_none_of_not_mem_keys hf₁]
    have hp₂ : Row.proj (.mk f₂ u₂) fresh = .opt u₂ := by
      simp [Row.proj, Row.fields, Row.unknown, lookup_none_of_not_mem_keys hf₂]
    have := hfld fresh (some v) (by rw [hp₁]; simp [fieldCheck]; exact hv)
    rw [hp₂] at this; simp [fieldCheck] at this
    exact this
  exact ⟨hnd₁, hnd₂, hunk, hfld⟩
