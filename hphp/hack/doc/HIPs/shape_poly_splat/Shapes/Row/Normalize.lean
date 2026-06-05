import Shapes.Row.Sub

/- ========================================================================== -/
/-! # Normalization

Normalize a list of rows (splat operands) into a single row by
left-folding `mergeRow`. Since all elements are `Row`s by construction,
there is no need for a `ValidSplat` predicate. -/
/- ========================================================================== -/

def normalize : List Row → Row
  | [] => Row.empty
  | [r] => r
  | r :: rest => mergeRow r (normalize rest)

theorem normalize_nodupKeys (rs : List Row)
    (h : ∀ r ∈ rs, Row.NoDupKeys r) : Row.NoDupKeys (normalize rs) := by
  induction rs with
  | nil =>
    simp [normalize, Row.NoDupKeys, Row.empty]
  | cons r tl ih =>
    cases tl with
    | nil =>
      simp [normalize]
      exact h r (.head _)
    | cons t tl' =>
      simp only [normalize]
      exact mergeRow_nodupKeys _ _

/- ========================================================================== -/
/-! ## Helpers -/
/- ========================================================================== -/

/-- mergeRow Row.empty r is shape-sub-equivalent to r (forward). -/
private theorem mergeRow_empty_left_ss (r : Row) (hnd : Row.NoDupKeys r) :
    r <:ʳ mergeRow Row.empty r := by
  refine ⟨hnd, mergeRow_nodupKeys .., ?_, ?_⟩
  · show r.unknown <:ᵇ (mergeRow Row.empty r).unknown
    simp [mergeRow, Row.empty, Row.unknown]
    exact sub_union_r .bot r.unknown
  · intro k
    rw [proj_mergeRow]
    simp [Row.empty, Row.proj, Row.fields, Row.unknown]
    exact fun val h => (fieldCheck_merge_bot_left _ val).mpr h

/-- mergeRow Row.empty r is shape-sub-equivalent to r (backward). -/
private theorem mergeRow_empty_left_ss' (r : Row) (hnd : Row.NoDupKeys r) :
    mergeRow Row.empty r <:ʳ r := by
  refine ⟨mergeRow_nodupKeys .., hnd, ?_, ?_⟩
  · simp [Row.empty, Row.unknown, mergeRow]
    exact sub_union_bot_left r.unknown
  · intro k
    rw [proj_mergeRow]
    simp [Row.empty, Row.proj, Row.fields, Row.unknown]
    exact fun val h => (fieldCheck_merge_bot_left _ val).mp h

/-- mergeRow r Row.empty is shape-sub-equivalent to r (forward). -/
private theorem mergeRow_empty_right_ss (r : Row) (hnd : Row.NoDupKeys r) :
    r <:ʳ mergeRow r Row.empty := by
  refine ⟨hnd, mergeRow_nodupKeys .., ?_, ?_⟩
  · simp [Row.empty, Row.unknown, mergeRow]
    exact sub_union_l r.unknown .bot
  · intro k
    rw [proj_mergeRow]
    simp [Row.empty, Row.proj, Row.fields, Row.unknown]
    exact fun val h => (fieldCheck_merge_bot_right _ val).mpr h

/-- mergeRow r Row.empty is shape-sub-equivalent to r (backward). -/
private theorem mergeRow_empty_right_ss' (r : Row) (hnd : Row.NoDupKeys r) :
    mergeRow r Row.empty <:ʳ r := by
  refine ⟨mergeRow_nodupKeys .., hnd, ?_, ?_⟩
  · simp [Row.empty, Row.unknown, mergeRow]
    exact sub_union_bot_right r.unknown
  · intro k
    rw [proj_mergeRow]
    simp [Row.empty, Row.proj, Row.fields, Row.unknown]
    exact fun val h => (fieldCheck_merge_bot_right _ val).mp h

private theorem merge_assoc_ss (a b c : Row) :
    mergeRow (mergeRow a b) c <:ʳ
    mergeRow a (mergeRow b c) := by
  refine ⟨mergeRow_nodupKeys .., mergeRow_nodupKeys .., ?_, ?_⟩
  · simp only [mergeRow, Row.unknown]
    intro v hv; simp at hv ⊢
    rcases hv with (ha | hb) | hc
    · exact Or.inl ha
    · exact Or.inr (Or.inl hb)
    · exact Or.inr (Or.inr hc)
  · intro k; rw [proj_mergeRow, proj_mergeRow, proj_mergeRow, proj_mergeRow]
    exact mergeFieldDesc_assoc _ _ _

private theorem merge_assoc_ss' (a b c : Row) :
    mergeRow a (mergeRow b c) <:ʳ
    mergeRow (mergeRow a b) c := by
  refine ⟨mergeRow_nodupKeys .., mergeRow_nodupKeys .., ?_, ?_⟩
  · simp only [mergeRow, Row.unknown]
    intro v hv; simp at hv ⊢
    rcases hv with ha | hb | hc
    · exact Or.inl (Or.inl ha)
    · exact Or.inl (Or.inr hb)
    · exact Or.inr hc
  · intro k; rw [proj_mergeRow, proj_mergeRow, proj_mergeRow, proj_mergeRow]
    exact mergeFieldDesc_assoc' _ _ _

/- ========================================================================== -/
/-! ## Core theorem: normalize_append

Both sides yield rows with mutual `<:ʳ`. -/
/- ========================================================================== -/

private theorem normalize_append_aux (xs ys : List Row)
    (hxs : ∀ r ∈ xs, Row.NoDupKeys r) (hys : ∀ r ∈ ys, Row.NoDupKeys r) :
    normalize (xs ++ ys) <:ʳ normalize [normalize xs, normalize ys] ∧
    normalize [normalize xs, normalize ys] <:ʳ normalize (xs ++ ys) := by
  induction xs with
  | nil =>
    -- normalize ([] ++ ys) = normalize ys
    -- normalize [normalize [], normalize ys] = normalize [Row.empty, normalize ys]
    --   = mergeRow Row.empty (normalize ys)
    simp only [List.nil_append, normalize]
    exact ⟨mergeRow_empty_left_ss _ (normalize_nodupKeys ys hys),
           mergeRow_empty_left_ss' _ (normalize_nodupKeys ys hys)⟩
  | cons r tl ih =>
    have htl : ∀ r ∈ tl, Row.NoDupKeys r := fun r' hr' => hxs r' (.tail _ hr')
    have hr : Row.NoDupKeys r := hxs r (.head _)
    cases tl with
    | nil =>
      -- xs = [r]
      cases ys with
      | nil =>
        -- ys = []: LHS = r, RHS = mergeRow r Row.empty
        simp only [normalize, List.append_nil]
        exact ⟨mergeRow_empty_right_ss _ hr,
               mergeRow_empty_right_ss' _ hr⟩
      | cons y ys' =>
        -- xs = [r], ys = y :: ys'
        -- LHS = normalize (r :: y :: ys') = mergeRow r (normalize (y :: ys'))
        -- RHS = normalize [r, normalize (y :: ys')] = mergeRow r (normalize (y :: ys'))
        simp only [List.cons_append, List.nil_append, normalize]
        exact ⟨row_sub_refl (mergeRow_nodupKeys ..),
               row_sub_refl (mergeRow_nodupKeys ..)⟩
    | cons t tl' =>
      -- xs = r :: t :: tl' (nonempty tail)
      obtain ⟨ih_fwd, ih_bwd⟩ := ih htl
      simp only [List.cons_append, normalize]
      constructor
      · exact row_sub_trans
          (merge_congr_right ih_fwd)
          (merge_assoc_ss' r (normalize (t :: tl')) (normalize ys))
      · exact row_sub_trans
          (merge_assoc_ss r (normalize (t :: tl')) (normalize ys))
          (merge_congr_right ih_bwd)


/- ========================================================================== -/
/-! ## Main theorem -/
/- ========================================================================== -/

theorem normalize_append (xs ys : List Row)
    (hxs : ∀ r ∈ xs, Row.NoDupKeys r) (hys : ∀ r ∈ ys, Row.NoDupKeys r) :
    normalize (xs ++ ys) <:ʳ normalize [normalize xs, normalize ys] ∧
    normalize [normalize xs, normalize ys] <:ʳ normalize (xs ++ ys) :=
  normalize_append_aux xs ys hxs hys


/- ========================================================================== -/
/-! ## Monotonicity

Pointwise `<:ʳ` on inputs implies `<:ʳ` on `normalize` outputs.
Follows from `merge_congr_left` and `merge_congr_right` through the fold. -/
/- ========================================================================== -/

theorem normalize_mono {rs₁ rs₂ : List Row}
    (hlen : rs₁.length = rs₂.length)
    (hnd₁ : ∀ r ∈ rs₁, Row.NoDupKeys r)
    (hnd₂ : ∀ r ∈ rs₂, Row.NoDupKeys r)
    (hpw : ∀ (i : Nat) (hi : i < rs₁.length),
       rs₁[i] <:ʳ rs₂[i]'(hlen ▸ hi)) :
    normalize rs₁ <:ʳ normalize rs₂ := by
  induction rs₁ generalizing rs₂ with
  | nil =>
    cases rs₂ with
    | nil => exact row_sub_refl (show Row.NoDupKeys Row.empty from List.nodup_nil)
    | cons _ _ => simp at hlen
  | cons r₁ tl₁ ih =>
    cases rs₂ with
    | nil => simp at hlen
    | cons r₂ tl₂ =>
      have hlen' : tl₁.length = tl₂.length := by simp at hlen; exact hlen
      have hpw₀ : r₁ <:ʳ r₂ := by
        have := hpw 0 (by simp)
        simpa using this
      have hpw_tl : ∀ (i : Nat) (hi : i < tl₁.length),
          tl₁[i] <:ʳ tl₂[i]'(hlen' ▸ hi) := by
        intro i hi
        have := hpw (i + 1) (by simp; omega)
        simpa using this
      cases tl₁ with
      | nil =>
        cases tl₂ with
        | nil => simpa [normalize] using hpw₀
        | cons _ _ => simp at hlen'
      | cons t₁ rest₁ =>
        cases tl₂ with
        | nil => simp at hlen'
        | cons t₂ rest₂ =>
          simp only [normalize]
          have ih_tl := ih hlen'
            (fun r hr => hnd₁ r (.tail _ hr))
            (fun r hr => hnd₂ r (.tail _ hr))
            hpw_tl
          exact row_sub_trans
            (merge_congr_right ih_tl)
            (merge_congr_left hpw₀)

/- ========================================================================== -/
/-! ## Substitution monotonicity

Replacing one element with a larger row gives a larger normalized result.
This is the core lemma for the HIP's [Splat-Rigid-Sub/Super] rules. -/
/- ========================================================================== -/

theorem normalize_substAt_mono {rs : List Row} {i : Nat} {r₁ r₂ : Row}
    (_hi : i < rs.length)
    (hnd : ∀ r ∈ rs, Row.NoDupKeys r)
    (hsub : r₁ <:ʳ r₂) :
    normalize (rs.set i r₁) <:ʳ normalize (rs.set i r₂) := by
  apply normalize_mono (by simp [List.length_set])
  · intro r hr
    rcases (List.mem_or_eq_of_mem_set hr).symm with rfl | hmem
    · exact hsub.1
    · exact hnd r hmem
  · intro r hr
    rcases (List.mem_or_eq_of_mem_set hr).symm with rfl | hmem
    · exact hsub.2.1
    · exact hnd r hmem
  · intro j hj
    simp [List.length_set] at hj
    by_cases hji : i = j
    · simp [hji]; exact hsub
    · simp [hji]
      exact row_sub_refl (hnd _ (List.getElem_mem ..))
