import Shapes.Expr.Typing

/- ========================================================================== -/
/-! # Type soundness for the ground fragment

If a closed ground expression has type `t` and evaluates to value `v`,
then `v` inhabits `t`: `denote t v`. -/
/- ========================================================================== -/


/- ========================================================================== -/
/-! ## Helpers -/
/- ========================================================================== -/

private theorem HasTypeFields.map_fst {fields : List (String × Expr)}
    {fs : List (String × FieldDesc)}
    (h : HasTypeFields fields fs) :
    fs.map Prod.fst = fields.map Prod.fst := by
  cases h with
  | nil => rfl
  | cons _ htrest => simp [HasTypeFields.map_fst htrest]

/-! ## Merge soundness -/

private theorem fieldCheck_merge_lookup
    (fd₁ fd₂ : FieldDesc)
    (entries₁ entries₂ : List (String × Val))
    (k : String) :
    fieldCheck fd₁ (entries₁.lookup k) →
    fieldCheck fd₂ (entries₂.lookup k) →
    fieldCheck (mergeFieldDesc fd₁ fd₂) ((mergeEntries entries₁ entries₂).lookup k) := by
  intro fc₁ fc₂
  cases hlook₂ : entries₂.lookup k with
  | some v₂ =>
    rw [hlook₂] at fc₂; rw [mergeEntries_lookup_right hlook₂]
    cases fd₁ <;> cases fd₂ <;> simp_all [mergeFieldDesc, fieldCheck]
    all_goals exact Or.inr fc₂
  | none =>
    rw [hlook₂] at fc₂
    cases hlook₁ : entries₁.lookup k with
    | some v₁ =>
      rw [hlook₁] at fc₁; rw [mergeEntries_lookup_left hlook₁ hlook₂]
      cases fd₁ <;> cases fd₂ <;> simp_all [mergeFieldDesc, fieldCheck]
      all_goals exact Or.inl fc₁
    | none =>
      rw [hlook₁] at fc₁; rw [mergeEntries_lookup_none hlook₁ hlook₂]
      cases fd₁ <;> cases fd₂ <;> simp_all [mergeFieldDesc, fieldCheck]

theorem merge_entries_sound {r₁ r₂ : Row}
    {entries₁ entries₂ : List (String × Val)}
    (hnd₁ : Row.NoDupKeys r₁) (hnd₂ : Row.NoDupKeys r₂)
    (h₁ : denote (.shape r₁) (.record entries₁))
    (h₂ : denote (.shape r₂) (.record entries₂)) :
    denote (.shape (mergeRow r₁ r₂)) (.record (mergeEntries entries₁ entries₂)) := by
  cases r₁ with
  | var _ => exact absurd hnd₁ id
  | mk f₁ u₁ => cases r₂ with
  | var _ => exact absurd hnd₂ id
  | mk f₂ u₂ =>
  have hfc₁ := (shape_denote_iff (.mk f₁ u₁) entries₁ hnd₁).mp h₁
  have hfc₂ := (shape_denote_iff (.mk f₂ u₂) entries₂ hnd₂).mp h₂
  rw [shape_denote_iff _ _ (mergeRow_nodupKeys (.mk f₁ u₁) (.mk f₂ u₂))]
  exact ⟨mergeEntries_nodup entries₁ entries₂, fun k => by
    rw [proj_mergeRow]
    exact fieldCheck_merge_lookup _ _ _ _ k (hfc₁.2 k) (hfc₂.2 k)⟩

/- ========================================================================== -/
/-! ## Record literal helpers -/
/- ========================================================================== -/

/-- Adding an entry with a key not among the field labels doesn't affect
`knownFieldsDenote`. -/
private theorem knownFieldsDenote_cons_of_not_mem
    {fs : List (String × FieldDesc)} {k : String} {v : Val}
    {entries : List (String × Val)}
    (hk : k ∉ fs.map Prod.fst)
    (h : knownFieldsDenote fs entries) :
    knownFieldsDenote fs ((k, v) :: entries) := by
  rw [knownFieldsDenote_iff] at h ⊢
  intro p hmem
  have hne : p.1 ≠ k := fun heq => by
    subst heq; exact hk (List.mem_map.mpr ⟨p, hmem, rfl⟩)
  rw [List.lookup_cons, show (p.1 == k) = false from by simp [hne]]
  exact h p hmem

private theorem record_unknown
    {fields : List (String × Expr)} {fs : List (String × FieldDesc)}
    {entries : List (String × Val)}
    (htf : HasTypeFields fields fs)
    (hef : EvalFields fields entries) :
    unknownFieldsDenote fs .bot entries := by
  rw [unknownFieldsDenote_iff]
  intro k v hmem hflookup
  have hk : k ∈ fs.map Prod.fst := by
    rw [HasTypeFields.map_fst htf, ← EvalFields.map_fst hef]
    exact List.mem_map.mpr ⟨(k, v), hmem, rfl⟩
  exact absurd hflookup (fun h => not_mem_map_fst_of_lookup_none h hk)

/- ========================================================================== -/
/-! ## Main soundness theorem -/
/- ========================================================================== -/

/- ========================================================================== -/
/-! ## Soundness: well-typed evaluation preserves denotation -/
/- ========================================================================== -/

mutual
  /-- If `HasType e t` and `Eval e v`, then `denote t v`. -/
  theorem type_sound : ∀ {e : Expr} {t : BaseTy} {v : Val},
      HasType e t → Eval e v → denote t v
    | _, _, _, .nat, .nat => by simp
    | _, _, _, .bool, .bool => by simp
    | _, _, _, .record htf hnd, .record hef => by
      rw [denote_shape]
      exact ⟨by rw [EvalFields.map_fst hef, ← HasTypeFields.map_fst htf]; exact hnd,
             type_sound_fields htf hef hnd,
             record_unknown htf hef⟩
    | _, _, _, .field (k := k) hte hnd hmem, .field hev hlook => by
      have hden := type_sound hte hev
      have hfc := ((shape_denote_iff _ _ hnd).mp hden).2 k
      rw [proj_of_mem hnd hmem, hlook] at hfc
      simp [fieldCheck] at hfc
      exact hfc
    | _, _, _, .merge ht₁ ht₂ hnd₁ hnd₂, .merge he₁ he₂ =>
      merge_entries_sound hnd₁ hnd₂ (type_sound ht₁ he₁) (type_sound ht₂ he₂)
    | _, _, _, .sub hte hsub, he =>
      hsub _ (type_sound hte he)

  /-- Soundness for field lists: if each field expression has its declared type
  and evaluates to a value, then the known-fields denotation holds. -/
  theorem type_sound_fields : ∀ {fields : List (String × Expr)}
      {fs : List (String × FieldDesc)} {entries : List (String × Val)},
      HasTypeFields fields fs →
      EvalFields fields entries →
      (fs.map Prod.fst).Nodup →
      knownFieldsDenote fs entries
    | _, _, _, .nil, .nil, _ => by simp
    | _, _, _, @HasTypeFields.cons k _ t _ rest_fs hte htrest,
      .cons hev hefrest, hnd => by
      simp only [knownFields_req]
      have hnd_simp := List.nodup_cons.mp hnd
      exact ⟨⟨_, List.lookup_cons_self, type_sound hte hev⟩,
             knownFieldsDenote_cons_of_not_mem hnd_simp.1
               (type_sound_fields htrest hefrest hnd_simp.2)⟩
end

/- ========================================================================== -/
/-! ## Progress: well-typed expressions evaluate -/
/- ========================================================================== -/

mutual
  /-- Every well-typed closed expression evaluates to some value. -/
  theorem progress : ∀ {e : Expr} {t : BaseTy},
      HasType e t → ∃ v, Eval e v
    | _, _, .nat => ⟨_, .nat⟩
    | _, _, .bool => ⟨_, .bool⟩
    | _, _, .record htf hnd =>
      let ⟨entries, hef⟩ := progress_fields htf
      ⟨.record entries, .record hef⟩
    | _, _, .field hte hnd hmem => by
      obtain ⟨v, hev⟩ := progress hte
      have hden := type_sound hte hev
      cases v with
      | nat => simp at hden
      | bool => simp at hden
      | record entries =>
        have hfc := ((shape_denote_iff _ _ hnd).mp hden).2 _
        rw [proj_of_mem hnd hmem] at hfc
        cases hlook : entries.lookup _ with
        | none => rw [hlook] at hfc; simp [fieldCheck] at hfc
        | some val => exact ⟨val, .field hev hlook⟩
    | _, _, .merge ht₁ ht₂ hnd₁ hnd₂ => by
      obtain ⟨v₁, hev₁⟩ := progress ht₁
      obtain ⟨v₂, hev₂⟩ := progress ht₂
      have hden₁ := type_sound ht₁ hev₁
      have hden₂ := type_sound ht₂ hev₂
      cases v₁ with
      | nat => simp at hden₁
      | bool => simp at hden₁
      | record e₁ => cases v₂ with
        | nat => simp at hden₂
        | bool => simp at hden₂
        | record e₂ => exact ⟨_, .merge hev₁ hev₂⟩
    | _, _, .sub hte _ =>
      progress hte

  private theorem progress_fields : ∀ {fields : List (String × Expr)}
      {fs : List (String × FieldDesc)},
      HasTypeFields fields fs → ∃ entries, EvalFields fields entries
    | _, _, .nil => ⟨[], .nil⟩
    | _, _, .cons hte htrest =>
      let ⟨v, hev⟩ := progress hte
      let ⟨rest, hefrest⟩ := progress_fields htrest
      ⟨(_, v) :: rest, .cons hev hefrest⟩
end

/-- Well-typed closed expressions evaluate to values inhabiting their type. -/
theorem type_safety {e : Expr} {t : BaseTy} (ht : HasType e t) :
    ∃ v, Eval e v ∧ denote t v :=
  let ⟨v, hev⟩ := progress ht
  ⟨v, hev, type_sound ht hev⟩
