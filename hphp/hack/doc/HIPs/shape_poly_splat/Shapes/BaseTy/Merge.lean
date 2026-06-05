import Shapes.BaseTy.Sub

/- ========================================================================== -/
/-! # Merge operations and algebra

Rightmost-wins merge on field descriptors and rows (§4.1.3 of the HIP),
plus the monoid properties: identity, monotonicity, associativity.

These hold up to semantic subtyping (`sub`), not syntactic equality, because
`mergeFieldDesc` introduces `BaseTy.union` constructors. -/
/- ========================================================================== -/


/- ========================================================================== -/
/-! ## Definitions -/
/- ========================================================================== -/

/-- Merge two field descriptors with rightmost-wins semantics. -/
def mergeFieldDesc (left right : FieldDesc) : FieldDesc :=
  match left, right with
  | _,       .req r => .req r
  | .req l,  .opt r => .req (.union l r)
  | .opt l,  .opt r => .opt (.union l r)

/-- Merge two rows. The result's fields are the merged field descriptors over the
union of both key sets. The result's unknown is the union of both unknowns. -/
def mergeRow (left right : Row) : Row :=
  let allKeys := (left.fields.map Prod.fst ++ right.fields.map Prod.fst).eraseDups
  let merged := allKeys.map fun k => (k, mergeFieldDesc (Row.proj left k) (Row.proj right k))
  .mk merged (.union left.unknown right.unknown)

/-- mergeRow produces a row with unique keys. -/
theorem mergeRow_nodupKeys (left right : Row) : Row.NoDupKeys (mergeRow left right) := by
  simp [Row.NoDupKeys, mergeRow, Row.fields, List.map_map]
  show ((left.fields.map Prod.fst ++ right.fields.map Prod.fst).eraseDups.map fun k => k).Nodup
  simp; exact eraseDups_nodup _

/-- Merge two shapes = wrap mergeRow in BaseTy.shape. -/
def mergeShape (left right : Row) : BaseTy :=
  .shape (mergeRow left right)

/- ========================================================================== -/
/-! ## Simp lemmas for merge primitives -/
/- ========================================================================== -/

/-- `proj` reduces to the found descriptor when lookup succeeds. -/
@[simp] theorem proj_found {fields : List (String × FieldDesc)} {k : String} {fd : FieldDesc}
    {unk : BaseTy}
    (h : fields.lookup k = some fd) : Row.proj (Row.mk fields unk) k = fd := by
  simp [Row.proj, Row.fields, h]

/-- `proj` falls back to `.opt unk` when lookup fails. -/
@[simp] theorem proj_not_found {fields : List (String × FieldDesc)} {k : String}
    {unk : BaseTy}
    (h : fields.lookup k = none) : Row.proj (Row.mk fields unk) k = .opt unk := by
  simp [Row.proj, Row.fields, Row.unknown, h]

/- ========================================================================== -/
/-! ## fieldCheck for mergeFieldDesc with (.opt .bot)

These show that merging with the "absent" descriptor `.opt .bot` is a no-op
under `fieldCheck`. This is the field-level version of "the empty shape is
the identity for merge." -/
/- ========================================================================== -/

/-- Merging `.opt .bot` on the LEFT with any descriptor `fd` is denotationally
the identity. The cases:
- `fd = .req t`: merge gives `.req t` (rightmost wins), trivially equal.
- `fd = .opt t`: merge gives `.opt (.union .bot t)`. Since `denote .bot v = False`,
  `False ∨ denote t v ↔ denote t v`, so `fieldCheck` is preserved.
`cases val` handles `some v` vs `none` for the optional case. -/
theorem fieldCheck_merge_bot_left (fd : FieldDesc) (val : Option Val) :
    fieldCheck (mergeFieldDesc (.opt .bot) fd) val ↔ fieldCheck fd val := by
  cases fd with
  | req t => simp [mergeFieldDesc, fieldCheck]
  | opt t => cases val <;> simp [mergeFieldDesc, fieldCheck]

/-- Merging `.opt .bot` on the RIGHT with any descriptor `fd`.
- `fd = .req t`: merge gives `.req (.union t .bot)`. `denote t v ∨ False ↔ denote t v`.
- `fd = .opt t`: merge gives `.opt (.union t .bot)`. Same idea.
`or_comm` is needed because the union puts `t` on the left and `.bot` on the right. -/
theorem fieldCheck_merge_bot_right (fd : FieldDesc) (val : Option Val) :
    fieldCheck (mergeFieldDesc fd (.opt .bot)) val ↔ fieldCheck fd val := by
  cases fd with
  | req t => cases val <;> simp [mergeFieldDesc, fieldCheck, or_comm]
  | opt t => cases val <;> simp [mergeFieldDesc, fieldCheck, or_comm]

/- ========================================================================== -/
/-! ## Merge identity

The empty row `Row.empty` (= `Row.mk [] .bot`) is the identity element for merge.
`mergeShape Row.empty r` and `mergeShape r Row.empty` are both
sub-equivalent to `.shape r`. -/
/- ========================================================================== -/

/-- **Left identity**: merging the empty row on the left doesn't change the type.
`mergeShape Row.empty right ≡ .shape right` (up to `sub`-equivalence).

Requires `NoDupKeys right` because the bridge lemma `knownFieldsDenote_proj` needs
unique keys to equate the pair-based and lookup-based characterizations. -/
theorem merge_id_left (right : Row)
    (hnd : Row.NoDupKeys right) :
    sub (mergeShape Row.empty right) (.shape right) ∧
    sub (.shape right) (mergeShape Row.empty right) := by
  cases right with
  | var _ => exact absurd hnd id
  | mk rf ru =>
  -- Unfold mergeShape/mergeRow to expose the List.map structure.
  simp only [mergeShape, mergeRow, Row.empty, Row.fields, Row.unknown]
  constructor
  · -- FORWARD: denote(merged) → denote(original)
    intro v hv
    cases v with
    | nat => simp at hv              -- shapes can't be nat values
    | bool => simp [denote_shape_bool] at hv
    | record entries =>
      -- Unfold shape denotation on both sides to get the triple:
      -- NoDup ∧ knownFieldsDenote ∧ unknownFieldsDenote
      rw [denote_shape] at hv ⊢
      obtain ⟨hnodup, hknown_m, hunknown_m⟩ := hv
      -- Convert knownFieldsDenote to fieldCheck form:
      -- merged side uses knownFieldsDenote_map (fields built by List.map)
      rw [knownFieldsDenote_map] at hknown_m
      -- Convert unknownFieldsDenote to membership-based form on both sides
      rw [unknownFieldsDenote_iff] at hunknown_m ⊢
      -- NoDup passes through unchanged
      refine ⟨hnodup, ?_, ?_⟩
      · -- Known fields: for each k ∈ fieldLabels, show fieldCheck (proj right k) (lookup k entries)
        -- Use knownFieldsDenote_proj to convert the goal
        exact (knownFieldsDenote_proj (Row.mk rf ru) entries hnd).mpr fun k hk => by
          -- k ∈ fieldLabels rf → k ∈ (rf.map Prod.fst).eraseDups
          have hk_ed : k ∈ (rf.map Prod.fst).eraseDups :=
            List.mem_eraseDups.mpr hk
          -- From hknown_m: fieldCheck (mergeFieldDesc (.opt .bot) (proj (Row.mk rf ru) k)) (lookup k entries)
          have := hknown_m k hk_ed
          -- proj (Row.mk [] .bot) k = .opt .bot (empty field list always misses)
          simp [Row.proj, Row.fields, Row.unknown] at this
          -- fieldCheck_merge_bot_left: merging with .opt .bot is identity
          rw [fieldCheck_merge_bot_left] at this
          exact this
      · -- Unknown fields: for each (k, v) ∈ entries with rf.lookup k = none, show denote ru v
        intro k v hmem hlookup
        have hunknown_kv := hunknown_m k v hmem
        -- Need: the merged field list also has lookup = none at k.
        -- hlookup says rf.lookup k = none, so k ∉ rf's keys, hence k ∉ eraseDups,
        -- hence lookup on the mapped list is none.
        have hnotmem_rf : k ∉ rf.map Prod.fst :=
          fun h => by
            rw [List.mem_map] at h
            obtain ⟨⟨_, fd⟩, hmem_rf, rfl⟩ := h
            -- lookup_some_of_mem: (k, fd) ∈ rf with NoDup → lookup = some fd
            -- But hlookup says lookup = none — contradiction.
            exact absurd (lookup_some_of_mem hnd hmem_rf ▸ hlookup) (by simp)
        have hnotmem_ed : k ∉ (rf.map Prod.fst).eraseDups :=
          fun h => hnotmem_rf (List.mem_eraseDups.mp h)
        -- lookup_map_none: k ∉ keys → mapped list lookup = none
        have hlookup_m := lookup_map_none
          (f := fun k => mergeFieldDesc (Row.proj (Row.mk [] .bot) k) (Row.proj (Row.mk rf ru) k)) hnotmem_ed
        -- Apply hunknown_kv with the merged lookup = none
        have hdu := hunknown_kv hlookup_m
        -- hdu : denote (.union .bot ru) v. Since denote .bot v = False,
        -- this simplifies to denote ru v.
        simp at hdu; exact hdu
  · -- BACKWARD: denote(original) → denote(merged)
    intro v hv
    cases v with
    | nat => simp [denote_shape_nat] at hv
    | bool => simp [denote_shape_bool] at hv
    | record entries =>
      rw [denote_shape] at hv ⊢
      obtain ⟨hnodup, hknown, hunknown⟩ := hv
      -- Convert the original side from structural to fieldCheck form
      have hknown' := (knownFieldsDenote_proj (Row.mk rf ru) entries hnd).mp hknown
      rw [knownFieldsDenote_map]
      rw [unknownFieldsDenote_iff] at hunknown ⊢
      refine ⟨hnodup, ?_, ?_⟩
      · -- Known fields: for each k in eraseDups, show merged fieldCheck
        intro k hk
        have hk_rf : k ∈ Row.fieldLabels (Row.mk rf ru) :=
          List.mem_eraseDups.mp hk
        have := hknown' k hk_rf
        -- Rewrite proj (Row.mk [] .bot) k = .opt .bot, then apply fieldCheck_merge_bot_left
        simp [Row.proj, Row.fields, Row.unknown] at this ⊢
        rw [fieldCheck_merge_bot_left]
        exact this
      · -- Unknown fields: show denote (.union .bot ru) v, which simplifies to denote ru v
        intro k v hmem hlookup
        -- simp reduces .union .bot ru to just needing denote ru v
        simp
        -- Need rf.lookup k = none. Extract from hlookup (merged list lookup = none)
        -- using List.lookup_eq_none_iff: every pair in the mapped list has k != its key.
        have hrf_none : rf.lookup k = none := by
          rw [List.lookup_eq_none_iff] at hlookup ⊢
          intro p hp
          -- p ∈ rf → (p.1, f(p.1)) ∈ mapped list → hlookup says k != p.1
          exact hlookup ⟨p.1, (mergeFieldDesc (Row.proj (Row.mk [] .bot) p.1) (Row.proj (Row.mk rf ru) p.1))⟩
            (List.mem_map.mpr ⟨p.1, List.mem_eraseDups.mpr (List.mem_map.mpr ⟨p, hp, rfl⟩), rfl⟩)
        exact hunknown k v hmem hrf_none

/-- **Right identity**: merging the empty row on the right doesn't change the type.
`mergeShape left Row.empty ≡ .shape left` (up to `sub`-equivalence).

Symmetric to `merge_id_left` but uses `fieldCheck_merge_bot_right` instead of
`fieldCheck_merge_bot_left`, and the unknown type is `.union lu .bot` instead
of `.union .bot ru` (needs `or_comm` to simplify). -/
theorem merge_id_right (left : Row)
    (hnd : Row.NoDupKeys left) :
    sub (mergeShape left Row.empty) (.shape left) ∧
    sub (.shape left) (mergeShape left Row.empty) := by
  cases left with
  | var _ => exact absurd hnd id
  | mk lf lu =>
  simp only [mergeShape, mergeRow, Row.empty, Row.fields, Row.unknown]
  constructor
  · -- FORWARD
    intro v hv
    cases v with
    | nat => simp at hv
    | bool => simp at hv
    | record entries =>
      rw [denote_shape] at hv ⊢
      obtain ⟨hnodup, hknown_m, hunknown_m⟩ := hv
      rw [knownFieldsDenote_map] at hknown_m
      rw [unknownFieldsDenote_iff] at hunknown_m ⊢
      refine ⟨hnodup, ?_, ?_⟩
      · exact (knownFieldsDenote_proj (Row.mk lf lu) entries hnd).mpr fun k hk => by
          have hk_ed : k ∈ (lf.map Prod.fst ++ []).eraseDups :=
            List.mem_eraseDups.mpr (List.mem_append_left _ hk)
          have := hknown_m k hk_ed
          simp [Row.proj, Row.fields, Row.unknown] at this
          rw [fieldCheck_merge_bot_right] at this
          exact this
      · intro k v hmem hlookup
        have hnotmem_rf : k ∉ lf.map Prod.fst :=
          fun h => by
            rw [List.mem_map] at h
            obtain ⟨⟨_, fd⟩, hmem_lf, rfl⟩ := h
            exact absurd (lookup_some_of_mem hnd hmem_lf ▸ hlookup) (by simp)
        have hnotmem_ed : k ∉ (lf.map Prod.fst ++ []).eraseDups :=
          fun h => hnotmem_rf (by simpa using List.mem_eraseDups.mp h)
        have hlookup_m := lookup_map_none
          (f := fun k => mergeFieldDesc (Row.proj (Row.mk lf lu) k) (Row.proj (Row.mk [] .bot) k)) hnotmem_ed
        have hdu := hunknown_m k v hmem hlookup_m
        -- .union lu .bot: need or_comm to move .bot to the left for simp
        simp [or_comm] at hdu; exact hdu
  · -- BACKWARD
    intro v hv
    cases v with
    | nat => simp at hv
    | bool => simp at hv
    | record entries =>
      rw [denote_shape] at hv ⊢
      obtain ⟨hnodup, hknown, hunknown⟩ := hv
      have hknown' := (knownFieldsDenote_proj (Row.mk lf lu) entries hnd).mp hknown
      rw [knownFieldsDenote_map]
      rw [unknownFieldsDenote_iff] at hunknown ⊢
      refine ⟨hnodup, ?_, ?_⟩
      · intro k hk
        have hk_lf : k ∈ Row.fieldLabels (Row.mk lf lu) := by
          have := List.mem_eraseDups.mp hk
          simp at this
          obtain ⟨fd, hmem⟩ := this
          exact List.mem_map.mpr ⟨(k, fd), hmem, rfl⟩
        have := hknown' k hk_lf
        simp [Row.proj, Row.fields, Row.unknown] at this ⊢
        rw [fieldCheck_merge_bot_right]
        exact this
      · intro k v hmem hlookup
        simp [or_comm]
        have hrf_none : lf.lookup k = none := by
          rw [List.lookup_eq_none_iff] at hlookup ⊢
          intro p hp
          exact hlookup ⟨p.1, (mergeFieldDesc (Row.proj (Row.mk lf lu) p.1) (Row.proj (Row.mk [] .bot) p.1))⟩
            (List.mem_map.mpr ⟨p.1, List.mem_eraseDups.mpr (by simp [List.mem_map.mpr ⟨p, hp, rfl⟩]), rfl⟩)
        exact hunknown k v hmem hrf_none

/- ========================================================================== -/
/-! ## Absent override (§4.1.4)

`absent 'f'` = `Opt Bot`. Merging with absent on the right is a no-op:
the left's contribution persists because the right contributes nothing. -/
/- ========================================================================== -/

theorem absent_no_remove_right (fd : FieldDesc) :
    mergeFieldDesc fd (.opt .bot) <:ᶠ fd ∧ fd <:ᶠ mergeFieldDesc fd (.opt .bot) :=
  ⟨fun val h => (fieldCheck_merge_bot_right fd val).mp h,
   fun val h => (fieldCheck_merge_bot_right fd val).mpr h⟩

theorem absent_no_remove_left (fd : FieldDesc) :
    mergeFieldDesc (.opt .bot) fd <:ᶠ fd ∧ fd <:ᶠ mergeFieldDesc (.opt .bot) fd :=
  ⟨fun val h => (fieldCheck_merge_bot_left fd val).mp h,
   fun val h => (fieldCheck_merge_bot_left fd val).mpr h⟩


/- ========================================================================== -/
/-! ## Merge monotonicity -/
/- ========================================================================== -/

/-- **Merge is monotone on field descriptors.** If `fd₁ ≤ fd₁'` and `fd₂ ≤ fd₂'`
(in the `fieldSub` sense), then `mergeFieldDesc fd₁ fd₂ ≤ mergeFieldDesc fd₁' fd₂'`.

This is the field-level building block for shape merge monotonicity.

Proof by case analysis on `fd₂` (the right input, which determines the merge structure):
- `fd₂ = Req`: merge produces `Req r₂` regardless of fd₁. The result depends only
  on `h₂`, which transfers `r₂` to `fd₂'`'s type.
  - `fd₂' = Req`: direct transfer via `h₂`.
  - `fd₂' = Opt`: the merged result has a union; inject via `Or.inr`.
- `fd₂ = Opt`:
  - `fd₂' = Req`: `h₂ : fieldSub (.opt _) (.req _)` is impossible (True → False at none),
    so `absurd` closes the goal.
  - `fd₂' = Opt`: merge produces a union in both source and target. Use `Or.imp`
    with `fieldSub_at_some h₁` and `fieldSub_at_some h₂` to transfer each disjunct.
    If `h₁ : fieldSub (.opt _) (.req _)`, that's also impossible → `absurd`. -/
theorem mergeFieldDesc_mono {fd₁ fd₁' fd₂ fd₂' : FieldDesc}
    (h₁ : fieldSub fd₁ fd₁') (h₂ : fieldSub fd₂ fd₂') :
    fieldSub (mergeFieldDesc fd₁ fd₂) (mergeFieldDesc fd₁' fd₂') := by
  intro val hval
  cases fd₂ with
  | req r₂ =>
    cases fd₂' with
    | req r₂' =>
      -- merge _ (.req r₂) = .req r₂; merge _ (.req r₂') = .req r₂'
      -- Just apply h₂ to transfer the field check.
      cases fd₁ <;> cases fd₁' <;>
        simp [mergeFieldDesc] at hval ⊢ <;> exact h₂ val hval
    | opt r₂' =>
      -- merge _ (.req r₂) = .req r₂; merge _ (.opt r₂') has a union
      -- h₂ transfers r₂ → r₂'; inject into right disjunct via Or.inr.
      cases fd₁ <;> cases fd₁' <;>
        simp [mergeFieldDesc] at hval ⊢ <;> cases val <;>
        simp_all [fieldCheck] <;> exact Or.inr (h₂ (some _) hval)
  | opt r₂ =>
    cases fd₂' with
    | req r₂' =>
      -- h₂ : fieldSub (.opt r₂) (.req r₂'). At val=none: True → False.
      -- This makes h₂ self-contradictory, so the goal is vacuously true.
      exact absurd (h₂ none trivial) id
    | opt r₂' =>
      -- Both sides have unions. Case split on all descriptor combinations and val.
      cases fd₁ <;> cases fd₁' <;>
        simp [mergeFieldDesc] at hval ⊢ <;> cases val <;>
        simp_all [fieldCheck]
      -- Remaining goals are either:
      -- - Or.imp case: transfer each disjunct via h₁ and h₂
      -- - Contradiction: h₁ maps True → False at none (opt → req)
      all_goals first
        | exact Or.imp (fieldSub_at_some h₁ _) (fieldSub_at_some h₂ _) ‹_›
        | exact absurd (h₁ none trivial) id

/-- **Merge is monotone on shapes.** If the left rows are ordered and the right rows are
ordered, then their merges are ordered.

We take the [Shape] rule's structural premises directly:
- `hlu : sub left₁.unknown left₂.unknown` -- unknown types are ordered
- `hru : sub right₁.unknown right₂.unknown`
- `hlf : ∀ k, fieldSub (proj left₁ k) (proj left₂ k)` -- per-field ordering
- `hrf : ∀ k, fieldSub (proj right₁ k) (proj right₂ k)`

These cannot be extracted from semantic `sub (.shape left) (.shape left')`
in degenerate cases (e.g., when a shape has an uninhabited required field,
making semantic subtyping vacuously true while the structural conditions fail). -/
theorem merge_mono
    {lf₁ lf₂ : List (String × FieldDesc)} {lu₁ lu₂ : BaseTy}
    {rf₁ rf₂ : List (String × FieldDesc)} {ru₁ ru₂ : BaseTy}
    (hlu : sub lu₁ lu₂) (hru : sub ru₁ ru₂)
    (hlf : ∀ k, fieldSub (Row.proj (.mk lf₁ lu₁) k) (Row.proj (.mk lf₂ lu₂) k))
    (hrf : ∀ k, fieldSub (Row.proj (.mk rf₁ ru₁) k) (Row.proj (.mk rf₂ ru₂) k)) :
    sub (mergeShape (.mk lf₁ lu₁) (.mk rf₁ ru₁)) (mergeShape (.mk lf₂ lu₂) (.mk rf₂ ru₂)) := by
  -- Unfold to expose the mapped key list structure
  simp only [mergeShape, mergeRow, Row.fields, Row.unknown] at *
  intro v hv
  cases v with
  | nat => simp at hv
  | bool => simp at hv
  | record entries =>
    -- Unfold shape denotation to triple: NoDup ∧ known ∧ unknown
    rw [denote_shape] at hv ⊢
    obtain ⟨hnodup, hknown₁, hunknown₁⟩ := hv
    -- Convert both sides to fieldCheck / membership form
    rw [knownFieldsDenote_map] at hknown₁ ⊢
    rw [unknownFieldsDenote_iff] at hunknown₁ ⊢
    refine ⟨hnodup, ?_, ?_⟩
    · -- KNOWN FIELDS: for each k ∈ keys₂, show fieldCheck (merge₂ k) (lookup k entries)
      intro k hk₂
      -- mergeFieldDesc_mono gives us: merge₁(k) ≤ merge₂(k) in fieldSub
      have hmono := mergeFieldDesc_mono (hlf k) (hrf k)
      by_cases hk₁ : k ∈ (lf₁.map Prod.fst ++ rf₁.map Prod.fst).eraseDups
      · -- CASE k ∈ keys₁: hknown₁ gives fieldCheck(merge₁ k)(lookup k entries).
        -- Apply hmono to transfer to merge₂.
        exact hmono _ (hknown₁ k hk₁)
      · -- CASE k ∉ keys₁: k is unknown in shape₁.
        -- proj left₁ k = .opt lu₁ and proj right₁ k = .opt ru₁
        -- (because k is not in either field list).
        have hlf₁_none : lf₁.lookup k = none :=
          lookup_none_of_not_mem_keys (fun h => hk₁ (List.mem_eraseDups.mpr (List.mem_append_left _ h)))
        have hrf₁_none : rf₁.lookup k = none :=
          lookup_none_of_not_mem_keys (fun h => hk₁ (List.mem_eraseDups.mpr (List.mem_append_right _ h)))
        -- After substituting the none lookups, hmono becomes:
        -- fieldSub (.opt (.union lu₁ ru₁)) (merge₂ k)
        simp [Row.proj, Row.fields, Row.unknown, hlf₁_none, hrf₁_none, mergeFieldDesc] at hmono
        apply hmono
        -- Need: fieldCheck (.opt (.union lu₁ ru₁)) (lookup k entries)
        cases hlook : entries.lookup k with
        | none => simp [fieldCheck]   -- .opt at none: True
        | some v =>
          simp [fieldCheck]
          -- Need: denote lu₁ v ∨ denote ru₁ v
          -- From hunknown₁: since k ∉ keys₁, the merged₁ field list has no entry for k,
          -- so hunknown₁ gives denote (.union lu₁ ru₁) v for any (k, v) ∈ entries.
          have hmem := mem_of_lookup_eq_some hlook
          have hmlookup := lookup_map_none
            (f := fun k => mergeFieldDesc (Row.proj (Row.mk lf₁ lu₁) k) (Row.proj (Row.mk rf₁ ru₁) k)) hk₁
          have := hunknown₁ k v hmem hmlookup
          simp at this
          exact this
    · -- UNKNOWN FIELDS: for each (k, v) ∈ entries with k ∉ keys₂,
      -- show denote (.union lu₂ ru₂) v
      intro k v hmem hlookup₂
      simp
      by_cases hk₁ : k ∈ (lf₁.map Prod.fst ++ rf₁.map Prod.fst).eraseDups
      · -- CASE k ∈ keys₁, k ∉ keys₂:
        -- Use NoDup entries to convert (k, v) ∈ entries → lookup k entries = some v.
        have hlook := lookup_some_of_mem (α := Val) hnodup hmem
        -- hknown₁ gives fieldCheck(merge₁ k)(some v)
        have hfc₁ := hknown₁ k hk₁
        rw [hlook] at hfc₁
        -- k ∉ keys₂ → lf₂.lookup k = none and rf₂.lookup k = none
        -- Extract from hlookup₂ via List.lookup_eq_none_iff
        have hlf₂_none : lf₂.lookup k = none := by
          rw [List.lookup_eq_none_iff]
          intro p hp
          rw [List.lookup_eq_none_iff] at hlookup₂
          have := hlookup₂ ⟨p.1, mergeFieldDesc (Row.proj (Row.mk lf₂ lu₂) p.1) (Row.proj (Row.mk rf₂ ru₂) p.1)⟩
            (List.mem_map.mpr ⟨p.1, List.mem_eraseDups.mpr (List.mem_append_left _ (List.mem_map.mpr ⟨p, hp, rfl⟩)), rfl⟩)
          simp [bne_iff_ne] at this ⊢
          exact this
        have hrf₂_none : rf₂.lookup k = none := by
          rw [List.lookup_eq_none_iff]
          intro p hp
          rw [List.lookup_eq_none_iff] at hlookup₂
          have := hlookup₂ ⟨p.1, mergeFieldDesc (Row.proj (Row.mk lf₂ lu₂) p.1) (Row.proj (Row.mk rf₂ ru₂) p.1)⟩
            (List.mem_map.mpr ⟨p.1, List.mem_eraseDups.mpr (List.mem_append_right _ (List.mem_map.mpr ⟨p, hp, rfl⟩)), rfl⟩)
          simp [bne_iff_ne] at this ⊢
          exact this
        -- After substituting none lookups, hmono becomes:
        -- fieldSub (merge₁ k) (.opt (.union lu₂ ru₂))
        have hmono := mergeFieldDesc_mono (hlf k) (hrf k)
        simp [Row.proj, Row.fields, Row.unknown, hlf₂_none, hrf₂_none, mergeFieldDesc] at hmono
        -- Apply hmono to hfc₁ and extract the disjunction
        have := hmono (some v) hfc₁
        simp [fieldCheck] at this
        exact this
      · -- CASE k ∉ keys₁: k is unknown in shape₁ too.
        -- hunknown₁ gives denote (.union lu₁ ru₁) v directly.
        have hmlookup := lookup_map_none
          (f := fun k => mergeFieldDesc (Row.proj (Row.mk lf₁ lu₁) k) (Row.proj (Row.mk rf₁ ru₁) k)) hk₁
        have hdu := hunknown₁ k v hmem hmlookup
        -- Transfer via hlu and hru: denote lu₁ v → denote lu₂ v (or ru₁ → ru₂)
        simp at hdu ⊢
        exact hdu.elim (fun h => Or.inl (hlu _ h)) (fun h => Or.inr (hru _ h))

/- ========================================================================== -/
/-! ## Lookup on mapped lists -/
/- ========================================================================== -/

/-- Lookup on a mapped key list succeeds when the key is present.
Unlike a general association list, every entry `(k, f k)` in
`ks.map (fun k => (k, f k))` has value determined by the key, so
first-match always returns `f k` -- no `Nodup` assumption needed.

Proof by induction on `ks`. If `k = k'` (head), `lookup_cons_self` applies.
Otherwise skip the head via `lookup_cons_ne'` and recurse. -/
theorem lookup_map_mem {ks : List String} {f : String → β} {k : String}
    (h : k ∈ ks) : (ks.map (fun k => (k, f k))).lookup k = some (f k) := by
  induction ks with
  | nil => simp at h                    -- k ∈ [] is False
  | cons k' rest ih =>
    simp only [List.map_cons]            -- unfold map at the head
    rcases List.mem_cons.mp h with rfl | hmem
    · exact List.lookup_cons_self        -- k = k': found at head
    · by_cases hne : k = k'
      · subst hne; exact List.lookup_cons_self  -- also k = k'
      · rw [lookup_cons_ne' (f k') _ hne]       -- k ≠ k': skip head
        exact ih hmem                            -- recurse

/- ========================================================================== -/
/-! ## Row.proj on merged rows -/
/- ========================================================================== -/

/-- **Key characterization**: projecting a field from a merged row equals
merging the individual projections.

For `mergeRow left right`, the field list is
`(left.fields.keys ++ right.fields.keys).eraseDups.map (fun k => (k, mergeFieldDesc (proj left k) (proj right k)))`.

- If `k` is in this key list: lookup finds `mergeFieldDesc (proj left k) (proj right k)`
  directly (via `lookup_map_mem`), and `proj` returns it.
- If `k` is NOT in the key list: `k` is in neither field list, so
  `proj left k = .opt left.unknown` and `proj right k = .opt right.unknown`, and
  `mergeFieldDesc (.opt left.unknown) (.opt right.unknown) = .opt (.union left.unknown right.unknown)`,
  which equals `proj (mergeRow left right) k = .opt (.union left.unknown right.unknown)`. -/
private theorem proj_mergeRow_mk (lf : List (String × FieldDesc)) (lu : BaseTy)
    (rf : List (String × FieldDesc)) (ru : BaseTy) (k : String) :
    Row.proj (mergeRow (.mk lf lu) (.mk rf ru)) k =
    mergeFieldDesc (Row.proj (.mk lf lu) k) (Row.proj (.mk rf ru) k) := by
  simp only [mergeRow, Row.proj, Row.fields, Row.unknown]
  by_cases hk : k ∈ (lf.map Prod.fst ++ rf.map Prod.fst).eraseDups
  · rw [lookup_map_mem hk]
  · rw [lookup_map_none hk]
    simp [lookup_none_of_not_mem_keys (fun h => hk (List.mem_eraseDups.mpr (List.mem_append_left _ h))),
          lookup_none_of_not_mem_keys (fun h => hk (List.mem_eraseDups.mpr (List.mem_append_right _ h))),
          mergeFieldDesc]

theorem proj_mergeRow (left right : Row) (k : String) :
    Row.proj (mergeRow left right) k =
    mergeFieldDesc (Row.proj left k) (Row.proj right k) := by
  cases left with
  | mk lf lu => cases right with
    | mk rf ru => exact proj_mergeRow_mk lf lu rf ru k
    | var _ => exact proj_mergeRow_mk lf lu [] .bot k
  | var _ => cases right with
    | mk rf ru => exact proj_mergeRow_mk [] .bot rf ru k
    | var _ => exact proj_mergeRow_mk [] .bot [] .bot k


/- ========================================================================== -/
/-! ## mergeFieldDesc associativity -/
/- ========================================================================== -/

/-- `mergeFieldDesc` is associative (left-to-right ≤ right-to-left).

The proof is a 2×2×2×2 case split (3 field descriptors × val = some/none).
After `simp_all` reduces each case, the remaining goals are `∨`-reassociations:
`(A ∨ B) ∨ C → A ∨ (B ∨ C)`, handled by `rcases` on the nested `Or`. -/
theorem mergeFieldDesc_assoc (fd₁ fd₂ fd₃ : FieldDesc) :
    fieldSub (mergeFieldDesc (mergeFieldDesc fd₁ fd₂) fd₃)
             (mergeFieldDesc fd₁ (mergeFieldDesc fd₂ fd₃)) := by
  intro val hval
  cases fd₁ <;> cases fd₂ <;> cases fd₃ <;> cases val <;>
    simp_all [mergeFieldDesc, fieldCheck]
  all_goals first
    | exact Or.inl ‹_› | exact Or.inr ‹_›
    | rcases ‹_ ∨ _› with (h | h) | h   -- (A ∨ B) ∨ C
      · exact Or.inl h                    -- A → A ∨ (B ∨ C)
      · exact Or.inr (Or.inl h)           -- B → A ∨ (B ∨ C)
      · exact Or.inr (Or.inr h)           -- C → A ∨ (B ∨ C)
    | rcases ‹_ ∨ _› with h | h          -- binary Or fallback
      · exact Or.inl h
      · exact Or.inr h

/-- Reverse direction: right-to-left ≤ left-to-right. Same structure but
reassociates `A ∨ (B ∨ C) → (A ∨ B) ∨ C`. -/
theorem mergeFieldDesc_assoc' (fd₁ fd₂ fd₃ : FieldDesc) :
    fieldSub (mergeFieldDesc fd₁ (mergeFieldDesc fd₂ fd₃))
             (mergeFieldDesc (mergeFieldDesc fd₁ fd₂) fd₃) := by
  intro val hval
  cases fd₁ <;> cases fd₂ <;> cases fd₃ <;> cases val <;>
    simp_all [mergeFieldDesc, fieldCheck]
  all_goals first
    | exact Or.inl ‹_› | exact Or.inr ‹_›
    | rcases ‹_ ∨ _› with h | h | h      -- A ∨ (B ∨ C)
      · exact Or.inl (Or.inl h)           -- A → (A ∨ B) ∨ C
      · exact Or.inl (Or.inr h)           -- B → (A ∨ B) ∨ C
      · exact Or.inr h                    -- C → (A ∨ B) ∨ C
    | rcases ‹_ ∨ _› with h | h
      · exact Or.inl h
      · exact Or.inr h

/- ========================================================================== -/
/-! ## Per-key characterization of mergeShape denotation
/- ========================================================================== -/

These use `shape_denote_iff` + `proj_mergeRow` to convert between the structural
denotation and a uniform per-key view. -/

/-- If a record inhabits a merged shape, then for every key `k`, the per-key
`fieldCheck` holds. Uses `shape_denote_iff` + `proj_mergeRow`. -/
theorem mergeShape_denote_implies_fieldCheck
    (left right : Row)
    (entries : List (String × Val))
    (h : denote (mergeShape left right) (.record entries)) (k : String) :
    fieldCheck (mergeFieldDesc (Row.proj left k) (Row.proj right k)) (entries.lookup k) := by
  rw [mergeShape, shape_denote_iff _ _ (mergeRow_nodupKeys left right)] at h
  rw [← proj_mergeRow]; exact h.2 k

/-- If NoDup entries and every key satisfies the merged field descriptor,
then the record inhabits the merged shape. -/
theorem fieldCheck_implies_mergeShape_denote
    (left right : Row)
    (entries : List (String × Val))
    (hnodup : (entries.map Prod.fst).Nodup)
    (hall : ∀ k, fieldCheck (mergeFieldDesc (Row.proj left k) (Row.proj right k)) (entries.lookup k)) :
    denote (mergeShape left right) (.record entries) := by
  rw [mergeShape, shape_denote_iff _ _ (mergeRow_nodupKeys left right)]
  exact ⟨hnodup, fun k => by rw [proj_mergeRow]; exact hall k⟩

/- ========================================================================== -/
/-! ## Merge associativity -/
/- ========================================================================== -/

/-- Extract NoDup from a mergeShape denotation. -/
private theorem mergeShape_nodup {left right : Row} {entries : List (String × Val)}
    (h : denote (mergeShape left right) (.record entries)) :
    (entries.map Prod.fst).Nodup := by
  simp only [mergeShape, mergeRow] at h
  rw [denote_shape] at h; exact h.1

/-- **Merge is associative** up to sub-equivalence:
`merge(merge(A, B), C) ≡ merge(A, merge(B, C))`.

Both directions follow the same 3-step pattern:
1. **Extract**: `mergeShape_denote_implies_fieldCheck` gives per-key fieldCheck
   from the input denotation.
2. **Rewrite**: `proj_mergeRow` (applied twice, once for each nested merge)
   converts the per-key descriptor from `mergeFieldDesc (proj (mergeRow ...) ...)  ...`
   to the triply-nested form `mergeFieldDesc (mergeFieldDesc ... ...) ...`.
3. **Reassociate**: `mergeFieldDesc_assoc` (or `_assoc'`) shuffles the nesting.
4. **Reconstruct**: `fieldCheck_implies_mergeShape_denote` rebuilds the
   output denotation from the per-key checks. -/
theorem merge_assoc (a b c : Row) :
    sub (mergeShape (mergeRow a b) c)
        (mergeShape a (mergeRow b c))
    ∧ sub (mergeShape a (mergeRow b c))
          (mergeShape (mergeRow a b) c) := by
  constructor
  · -- Forward: merge(merge(A,B), C) ≤ merge(A, merge(B,C))
    intro v hv
    cases v with
    | nat => simp only [mergeShape] at hv; rw [denote_shape_nat] at hv; exact absurd hv id
    | bool => simp only [mergeShape] at hv; rw [denote_shape_bool] at hv; exact absurd hv id
    | record entries =>
      -- Reconstruct RHS denotation from per-key checks
      apply fieldCheck_implies_mergeShape_denote _ _ _ (mergeShape_nodup hv)
      intro k
      -- Rewrite RHS proj via proj_mergeRow (inner merge on the right)
      rw [proj_mergeRow]
      -- Extract per-key check from LHS, rewrite via proj_mergeRow (inner merge on the left)
      have := mergeShape_denote_implies_fieldCheck _ _ _ hv k
      rw [proj_mergeRow] at this
      -- this : fieldCheck (mergeFieldDesc (mergeFieldDesc (proj a k) (proj b k)) (proj c k)) ...
      -- goal : fieldCheck (mergeFieldDesc (proj a k) (mergeFieldDesc (proj b k) (proj c k))) ...
      exact mergeFieldDesc_assoc _ _ _ _ this
  · -- Backward: merge(A, merge(B,C)) ≤ merge(merge(A,B), C)
    -- Symmetric, using mergeFieldDesc_assoc' for the reverse reassociation
    intro v hv
    cases v with
    | nat => simp only [mergeShape] at hv; rw [denote_shape_nat] at hv; exact absurd hv id
    | bool => simp only [mergeShape] at hv; rw [denote_shape_bool] at hv; exact absurd hv id
    | record entries =>
      apply fieldCheck_implies_mergeShape_denote _ _ _ (mergeShape_nodup hv)
      intro k
      rw [proj_mergeRow]
      have := mergeShape_denote_implies_fieldCheck _ _ _ hv k
      rw [proj_mergeRow] at this
      exact mergeFieldDesc_assoc' _ _ _ _ this
