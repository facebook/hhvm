import Shapes.BaseTy.Denote

/- ========================================================================== -/
/-! # Inhabited types

If `BaseTy.isEmpty t = false` and `t` is well-formed (all embedded shapes have
unique field keys), there exists a value `v` such that `denote t v`.

This result characterizes the relationship between the boolean emptiness
checker (`BaseTy.isEmpty`) and the semantic denotation (`denote`). The
soundness direction (`isEmpty_sound` in `Row/Decide.lean`) says `isEmpty = true`
implies no value exists. This file proves the converse for well-formed types:
`isEmpty = false` implies a value DOES exist.

The proof constructs a concrete witness value for each non-empty type. -/
/- ========================================================================== -/


/- ========================================================================== -/
/-! ## Witness construction

`witness` produces a concrete `Val` for any `BaseTy`. It is total — it
returns *something* even for empty types (e.g., `.nat 0` for `Bot`).
The interesting work is in the proof (`witness_denote`) that the returned
value actually inhabits the type when `isEmpty = false` and `wf` holds.

`witnessFields` builds record entries for shape types: one entry per
required field (with a recursively constructed witness), optional fields
skipped (they need not be present). -/
/- ========================================================================== -/

mutual
  private def witness : BaseTy → Val
    | .nat => .nat 0
    | .bool => .bool true
    | .top => .nat 0
    | .bot => .nat 0
    | .union a b => if a.isEmpty then witness b else witness a
    | .shape (.mk fields _) => .record (witnessFields fields)
    | .shape (.var _) => .nat 0

  private def witnessFields : List (String × FieldDesc) → List (String × Val)
    | [] => []
    | (k, .req t) :: rest => (k, witness t) :: witnessFields rest
    | (_, .opt _) :: rest => witnessFields rest
end

/- ========================================================================== -/
/-! ## Properties of witnessFields
/- ========================================================================== -/

These lemmas characterize the record produced by `witnessFields`:
- Its keys are a subset of the field list's keys (only Req fields appear)
- Every key in the result corresponds to a Req field in the input
- If the input has unique keys, so does the result
- Lookup at a Req field returns the witness for that field's type
- Lookup at an Opt field returns none (Opt fields are skipped)
- Lookup at a key not in the field list returns none -/

/-- Keys of `witnessFields` are a subset of the original field keys. -/
private theorem witnessFields_keys_sub {fields : List (String × FieldDesc)} {k : String}
    (hmem : k ∈ (witnessFields fields).map Prod.fst) :
    k ∈ fields.map Prod.fst := by
  induction fields with
  | nil => simp [witnessFields] at hmem
  | cons hd rest ih =>
    obtain ⟨k', fd⟩ := hd
    cases fd with
    | opt _ =>
      simp only [witnessFields] at hmem
      exact List.mem_cons_of_mem _ (ih hmem)
    | req t =>
      simp only [witnessFields, List.map_cons] at hmem
      rcases List.mem_cons.mp hmem with rfl | hmem
      · exact List.mem_cons.mpr (Or.inl rfl)
      · exact List.mem_cons.mpr (Or.inr (ih hmem))

/-- Every key in `witnessFields` came from a Req field. -/
private theorem witnessFields_keys_req {fields : List (String × FieldDesc)} {k : String}
    (hmem : k ∈ (witnessFields fields).map Prod.fst) :
    ∃ t, (k, FieldDesc.req t) ∈ fields := by
  induction fields with
  | nil => simp [witnessFields] at hmem
  | cons hd rest ih =>
    obtain ⟨k', fd⟩ := hd
    cases fd with
    | opt _ =>
      simp only [witnessFields] at hmem
      obtain ⟨t, h⟩ := ih hmem; exact ⟨t, List.mem_cons.mpr (Or.inr h)⟩
    | req t =>
      simp only [witnessFields, List.map_cons] at hmem
      rcases List.mem_cons.mp hmem with rfl | hmem
      · exact ⟨t, List.mem_cons.mpr (Or.inl rfl)⟩
      · obtain ⟨t', h⟩ := ih hmem; exact ⟨t', List.mem_cons.mpr (Or.inr h)⟩

/-- `witnessFields` preserves key uniqueness. -/
private theorem witnessFields_nodup {fields : List (String × FieldDesc)}
    (hnd : (fields.map Prod.fst).Nodup) :
    ((witnessFields fields).map Prod.fst).Nodup := by
  induction fields with
  | nil => simp [witnessFields]
  | cons hd rest ih =>
    obtain ⟨k, fd⟩ := hd
    rw [List.map_cons, List.nodup_cons] at hnd
    obtain ⟨hnotin, hnd_rest⟩ := hnd
    cases fd with
    | opt _ => simp only [witnessFields]; exact ih hnd_rest
    | req t =>
      simp only [witnessFields, List.map_cons, List.nodup_cons]
      exact ⟨fun hmem => hnotin (witnessFields_keys_sub hmem), ih hnd_rest⟩

/-- Looking up a Req field in `witnessFields` returns the witness for that
field's type. This is the key lemma connecting the constructed record to
the type-level field declarations. -/
private theorem witnessFields_lookup_req {fields : List (String × FieldDesc)} {k : String}
    {t : BaseTy} (hnd : (fields.map Prod.fst).Nodup)
    (hmem : (k, FieldDesc.req t) ∈ fields) :
    (witnessFields fields).lookup k = some (witness t) := by
  induction fields with
  | nil => simp at hmem
  | cons hd rest ih =>
    obtain ⟨k', fd⟩ := hd
    rw [List.map_cons, List.nodup_cons] at hnd
    obtain ⟨hnotin, hnd_rest⟩ := hnd
    rcases List.mem_cons.mp hmem with h | hmem_rest
    · obtain ⟨rfl, hfd_eq⟩ := Prod.mk.inj h
      cases fd with
      | opt => simp at hfd_eq
      | req t' =>
        have := FieldDesc.req.inj hfd_eq; subst this
        simp [witnessFields]
    · have hk_ne : k ≠ k' := fun heq => by
        subst heq; exact hnotin (List.mem_map.mpr ⟨(k, FieldDesc.req t), hmem_rest, rfl⟩)
      cases fd with
      | opt _ => simp only [witnessFields]; exact ih hnd_rest hmem_rest
      | req t' =>
        simp only [witnessFields, List.lookup_cons]
        rw [show (k == k') = false from by simp [hk_ne]]
        exact ih hnd_rest hmem_rest

/-- Looking up an Opt field in `witnessFields` returns none — optional
fields are skipped. Uses `witnessFields_keys_req` to derive a contradiction:
if the key WERE present, it would correspond to a Req field, but NoDupKeys
means the same key can't be both Req and Opt. -/
private theorem witnessFields_lookup_opt {fields : List (String × FieldDesc)} {k : String}
    {t : BaseTy} (hnd : (fields.map Prod.fst).Nodup)
    (hmem : (k, FieldDesc.opt t) ∈ fields) :
    (witnessFields fields).lookup k = none := by
  apply lookup_none_of_not_mem_keys
  intro hmem_keys
  obtain ⟨t', hmem_req⟩ := witnessFields_keys_req hmem_keys
  have h1 := lookup_some_of_mem hnd hmem
  have h2 := lookup_some_of_mem hnd hmem_req
  rw [h1] at h2; exact absurd h2 (by simp)

/-- Looking up a key not in the field list returns none. -/
private theorem witnessFields_lookup_none {fields : List (String × FieldDesc)} {k : String}
    (hnotin : k ∉ fields.map Prod.fst) :
    (witnessFields fields).lookup k = none :=
  lookup_none_of_not_mem_keys (fun h => hnotin (witnessFields_keys_sub h))

/- ========================================================================== -/
/-! ## Extraction lemmas
/- ========================================================================== -/

These extract facts from the `isEmpty.go` and `fieldsWf` predicates
for individual fields, given membership in the field list. -/

/-- If `go fields = false` (no required field has an empty type) and
`(k, Req t) ∈ fields`, then `t.isEmpty = false`. -/
private theorem go_false_req {fields : List (String × FieldDesc)} {k : String} {t : BaseTy}
    (hgo : BaseTy.isEmpty.go fields = false)
    (hmem : (k, FieldDesc.req t) ∈ fields) :
    t.isEmpty = false := by
  induction fields with
  | nil => simp at hmem
  | cons hd rest ih =>
    obtain ⟨k', fd⟩ := hd
    rcases List.mem_cons.mp hmem with h | hmem_rest
    · obtain ⟨rfl, hfd_eq⟩ := Prod.mk.inj h
      cases fd with
      | opt => simp at hfd_eq
      | req t' =>
        have := FieldDesc.req.inj hfd_eq; subst this
        simp [BaseTy.isEmpty.go, Bool.or_eq_false_iff] at hgo; exact hgo.1
    · cases fd with
      | opt _ => simp [BaseTy.isEmpty.go] at hgo; exact ih hgo hmem_rest
      | req t' =>
        simp [BaseTy.isEmpty.go, Bool.or_eq_false_iff] at hgo; exact ih hgo.2 hmem_rest

/-- If `fieldsWf fields` and `(k, Req t) ∈ fields`, then `t.wf`. -/
private theorem fieldsWf_mem_req {fields : List (String × FieldDesc)} {k : String} {t : BaseTy}
    (hwf : fieldsWf fields) (hmem : (k, FieldDesc.req t) ∈ fields) :
    t.wf := by
  induction fields with
  | nil => simp at hmem
  | cons hd rest ih =>
    obtain ⟨k', fd⟩ := hd
    rcases List.mem_cons.mp hmem with h | hmem_rest
    · obtain ⟨rfl, hfd_eq⟩ := Prod.mk.inj h
      cases fd with
      | opt => simp at hfd_eq
      | req t' => have := FieldDesc.req.inj hfd_eq; subst this; exact hwf.1
    · cases fd <;> exact ih hwf.2 hmem_rest

/- ========================================================================== -/
/-! ## Main theorem

`witness_denote` proves that `witness t` actually inhabits `t` when
`t.wf` and `t.isEmpty = false`. The proof is by well-founded recursion
on `sizeOf t`.

The interesting case is shapes: the witness record is `witnessFields fields`,
which has one entry per Req field. We show it satisfies `shape_denote_iff`
by checking each key:
- Req fields: `witnessFields_lookup_req` gives the witness value,
  and the recursive call gives `denote t' (witness t')`.
- Opt fields: `witnessFields_lookup_opt` gives `none`, and
  `fieldCheck (Opt _) none = True`.
- Undeclared fields: `witnessFields_lookup_none` gives `none`, same. -/
/- ========================================================================== -/

private theorem witness_denote (τ : BaseTy) (hwf : τ.wf) (hne : τ.isEmpty = false) :
    denote τ (witness τ) := by
  match τ, hwf, hne with
  | .nat, _, _ => simp [witness]
  | .bool, _, _ => simp [witness]
  | .top, _, _ => simp [witness]
  | .bot, _, h => simp [BaseTy.isEmpty] at h
  | .union a b, ⟨ha_wf, hb_wf⟩, h =>
    simp only [witness]
    cases ha : a.isEmpty with
    | false =>
      simp; exact Or.inl (witness_denote a ha_wf ha)
    | true =>
      simp
      have hb : b.isEmpty = false := by simp [BaseTy.isEmpty, ha] at h; exact h
      exact Or.inr (witness_denote b hb_wf hb)
  | .shape (.var _), _, h => simp [BaseTy.isEmpty] at h
  | .shape (.mk fields unk), hwf, h =>
    have hnd : (fields.map Prod.fst).Nodup := hwf.1
    have hfields_wf : fieldsWf fields := hwf.right.right
    simp only [witness]
    rw [shape_denote_iff _ _ hwf.1]
    refine ⟨witnessFields_nodup hnd, fun k => ?_⟩
    cases hlook : fields.lookup k with
    | none =>
      have hnotin := not_mem_map_fst_of_lookup_none hlook
      simp [Row.proj, Row.fields, Row.unknown, hlook]
      rw [witnessFields_lookup_none hnotin]
      simp [fieldCheck]
    | some fd =>
      have hmem := mem_of_lookup_eq_some hlook
      cases fd with
      | req t' =>
        simp [Row.proj, Row.fields, hlook]
        rw [witnessFields_lookup_req hnd hmem]
        simp [fieldCheck]
        exact witness_denote t' (fieldsWf_mem_req hfields_wf hmem) (go_false_req h hmem)
      | opt t' =>
        simp [Row.proj, Row.fields, hlook]
        rw [witnessFields_lookup_opt hnd hmem]
        simp [fieldCheck]
termination_by sizeOf τ
decreasing_by
  all_goals simp_wf
  all_goals first
    | omega
    | (have := List.sizeOf_lt_of_mem ‹(_, FieldDesc.req _) ∈ _›; simp_all; omega)

/-- Well-formed non-empty types are inhabited. -/
theorem isEmpty_complete (t : BaseTy) (hwf : t.wf) (hne : t.isEmpty = false) :
    ∃ v, denote t v :=
  ⟨witness t, witness_denote t hwf hne⟩
