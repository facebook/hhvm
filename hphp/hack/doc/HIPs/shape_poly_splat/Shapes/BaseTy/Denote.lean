import Shapes.BaseTy

/- ========================================================================== -/
/-! # Denotational semantics for base types

The denotation `denote : BaseTy → Val → Prop` is a `def` (a function),
not an `inductive` relation. This is a deliberate choice: semantic subtyping
is defined as `sub τ₁ τ₂ := ∀ v, denote τ₁ v → denote τ₂ v`, which makes
subtyping proofs ordinary functions. Reflexivity is the identity function,
transitivity is composition, and the union/bot/top rules are `Or.inl`/`Or.inr`
and `False.elim` — no induction on derivation trees.

An inductive subtyping relation would require proving admissibility of
reflexivity and transitivity by induction on the derivation. With the
denotational approach, these are definitional. The cost is that `denote`
is `@[irreducible]` (from the mutual block) and must be accessed through
equation lemmas, not unfolding. -/
/- ========================================================================== -/

mutual
  /-- A value inhabits a type. Shapes require a record with unique keys
  whose entries satisfy the declared fields and whose undeclared entries
  satisfy the unknown-field type. Row variables are uninhabited. -/
  def denote : BaseTy → Val → Prop
    | .bot, _ => False
    | .top, _ => True
    | .nat, .nat _ => True
    | .nat, .bool _ => False
    | .nat, .record _ => False
    | .bool, .bool _ => True
    | .bool, .nat _ => False
    | .bool, .record _ => False
    | .union a b, v => denote a v ∨ denote b v
    | .shape (.mk fields unk), .record entries =>
        (entries.map Prod.fst).Nodup
        ∧ knownFieldsDenote fields entries
        ∧ unknownFieldsDenote fields unk entries
    | .shape (.var _), .record _ => False
    | .shape _, .nat _ => False
    | .shape _, .bool _ => False

  /-- Every declared field is satisfied: required fields must be present
  with a value of the right type; optional fields, if present, must have
  a value of the right type. Operates on List(String × FieldDesc) rather than
  Row to satisfy the termination checker -/
  def knownFieldsDenote
    (fields: List (String × FieldDesc))
    (entries: List (String × Val)) : Prop :=
    match fields with
    | [] =>
        -- if the shape declares no fields, there's nothing to check about declared fields.
        True
    | (l, .req τ) :: rest =>
        /- if we have a required field with label l and type τ, our entries MUST
        have some associated value v such that it inhabit the field type τ -/
        (∃ v, entries.lookup l = some v ∧ denote τ v)
        ∧ knownFieldsDenote rest entries
    | (l, .opt τ) :: rest =>
        /- if we have an optional field with label k and type t, then IF our entries
        have an associated value v, it must be denoted by t. Absent is fine. -/
        (∀ v, entries.lookup l = some v → denote τ v)
        ∧ knownFieldsDenote rest entries

  /-- Every record entry not declared in the field list has a value
  inhabiting the unknown-field type. This is what makes a shape open
  (`τ = top`: anything goes) or closed (`τ = bot`: no extra fields). -/
  def unknownFieldsDenote
    (fields: List (String × FieldDesc))
    (τ: BaseTy)
    (entries: List (String × Val)):  Prop :=
    match entries with
    | [] =>
      -- if there are no entires, there is nothing to check
      True
    | (k, v) :: rest =>
      /-  if this entry's key k is not declared in the field list
      (lookup returns none), then its value v must inhabit the
      unknown-field type τ. -/
      (fields.lookup k = none → denote τ v)
      ∧ unknownFieldsDenote fields τ rest
end

notation "⟦" τ "⟧" => denote τ

/- ========================================================================== -/
/-! ## Named equation lemmas for `denote`

The mutual block generates `@[irreducible]` definitions with auto-named
equation lemmas (`denote_nat` etc). We alias them with meaningful names
and register as `@[simp]`. -/
/- ========================================================================== -/

-- Named aliases for auto-generated equation lemmas.
-- IMPORTANT: numbering follows match arm order in `denote` above.
-- If you reorder the arms, these must be updated.
abbrev denote_bot          := @denote.eq_1   -- ⟦⊥⟧(v) = False
abbrev denote_top          := @denote.eq_2   -- ⟦⊤⟧(v) = True
abbrev denote_nat          := @denote.eq_3   -- ⟦Nat⟧(nat n) = True
abbrev denote_nat_bool     := @denote.eq_4   -- ⟦Nat⟧(bool _) = False
abbrev denote_nat_record   := @denote.eq_5   -- ⟦Nat⟧(record _) = False
abbrev denote_bool         := @denote.eq_6   -- ⟦Bool⟧(bool b) = True
abbrev denote_bool_nat     := @denote.eq_7   -- ⟦Bool⟧(nat _) = False
abbrev denote_bool_record  := @denote.eq_8   -- ⟦Bool⟧(record _) = False
abbrev denote_union        := @denote.eq_9   -- ⟦a ∪ b⟧(v) = ⟦a⟧(v) ∨ ⟦b⟧(v)
abbrev denote_shape        := @denote.eq_10  -- ⟦shape(mk fields unk)⟧(record entries) = ...
abbrev denote_shape_var    := @denote.eq_11  -- ⟦shape(var _)⟧(record _) = False
abbrev denote_shape_nat    := @denote.eq_12  -- ⟦shape _⟧(nat _) = False
abbrev denote_shape_bool   := @denote.eq_13  -- ⟦shape _⟧(bool _) = False

abbrev knownFields_nil     := @knownFieldsDenote.eq_1
abbrev knownFields_req     := @knownFieldsDenote.eq_2
abbrev knownFields_opt     := @knownFieldsDenote.eq_3

abbrev unknownFields_nil   := @unknownFieldsDenote.eq_1
abbrev unknownFields_cons  := @unknownFieldsDenote.eq_2

attribute [simp] denote_nat denote_bool denote_bot denote_top denote_union
  denote_shape denote_shape_var denote_shape_nat denote_shape_bool
  denote_nat_bool denote_nat_record denote_bool_nat denote_bool_record
  knownFields_nil knownFields_req knownFields_opt
  unknownFields_nil unknownFields_cons


/- ========================================================================== -/
/-! ## Lookup helpers -/
/- ========================================================================== -/

/-- If `List.lookup` returns `some v`, then `(k, v)` is in the list.
Proof by induction on the list. In the cons case, `List.lookup_cons` splits
on whether the key matches: if yes, the head is the witness; if no, recurse. -/
theorem mem_of_lookup_eq_some {k : String} {v : α} {entries : List (String × α)}
    (h : entries.lookup k = some v) : (k, v) ∈ entries := by
  induction entries with
  | nil => simp [List.lookup] at h  -- lookup on [] is none, contradicts h
  | cons hd tl ih =>
    obtain ⟨k', v'⟩ := hd           -- destructure the head pair
    rw [List.lookup_cons] at h       -- unfold lookup at the cons case
    split at h                       -- split on the BEq match (k == k')
    · next heq =>                    -- case k == k' = true:
      rw [beq_iff_eq] at heq; subst heq  -- convert BEq to Prop equality, substitute
      have := Option.some.inj h; subst this  -- extract v = v' from some v = some v'
      exact .head _                  -- (k, v) is the head of the list
    · exact .tail _ (ih h)           -- case k == k' = false: recurse on the tail

/-- Skip past a cons with a different key during lookup.
`List.lookup_cons` gives a `match` on `k₁ == k₂`; we manually evaluate the
BEq to `false` using `h : k₁ ≠ k₂`. -/
@[simp] theorem lookup_cons_ne' {k₁ k₂ : String} (v : α) (tl : List (String × α))
    (h : k₁ ≠ k₂) :
    List.lookup k₁ ((k₂, v) :: tl) = List.lookup k₁ tl := by
  simp [List.lookup_cons]            -- unfold to the match on k₁ == k₂
  rw [show (k₁ == k₂) = false from by simp [h]]  -- evaluate k₁ == k₂ to false

/-- If `List.lookup k fields = none`, then `k` is not among the keys.
Proof by induction. In the cons case, `split at h` handles the BEq match:
the `true` case contradicts `h` (lookup found something); the `false` case
extracts `k ≠ k'` and recurses. -/
theorem not_mem_map_fst_of_lookup_none {fields : List (String × α)} {k : String}
    (h : fields.lookup k = none) : k ∉ fields.map Prod.fst := by
  induction fields with
  | nil => simp
  | cons hd tl ih =>
    obtain ⟨k', v⟩ := hd
    simp [List.lookup_cons] at h
    split at h
    · next heq => exact absurd h (by simp)  -- k == k' = true but lookup = none: contradiction
    · next hne =>
      simp
      -- Need: k ≠ k' (from hne) ∧ k ∉ tl.map Prod.fst (from ih)
      exact ⟨fun heq => by subst heq; simp at hne,  -- k = k' contradicts hne
             fun x hmem => ih h (List.mem_map.mpr ⟨(k, x), hmem, rfl⟩)⟩  -- recurse

/-- Converse of `not_mem_map_fst_of_lookup_none`: if `k` is not among the keys,
then `List.lookup k` returns `none`. Uses `List.lookup_eq_none_iff` which
characterizes `lookup = none` via `BEq` inequality against all pairs. -/
theorem lookup_none_of_not_mem_keys {fields : List (String × α)} {k : String}
    (h : k ∉ fields.map Prod.fst) : fields.lookup k = none := by
  rw [List.lookup_eq_none_iff]       -- rewrite to: ∀ p ∈ fields, (k != p.1) = true
  intro ⟨k', v⟩ hmem
  simp [bne_iff_ne]                  -- convert BEq inequality to ≠
  intro heq                          -- suppose k = k'
  exact h (heq ▸ List.mem_map.mpr ⟨(k', v), hmem, rfl⟩)  -- then k ∈ keys, contradiction

/-- For a list built by mapping a function over keys, lookup returns `none`
when the key is not in the original key list.
Proof by induction: in the cons case, `h.1 : k ≠ k'` lets us evaluate the
BEq to false and skip the head. -/
theorem lookup_map_none {ks : List String} {f : String → β} {k : String}
    (h : k ∉ ks) : (ks.map (fun k => (k, f k))).lookup k = none := by
  induction ks with
  | nil => rfl                       -- empty list: lookup is none
  | cons k' rest ih =>
    simp [List.lookup_cons] at h ⊢   -- unfold lookup; h gives k ≠ k' ∧ k ∉ rest
    have hne : (k == k') = false := by simp [h.1]  -- convert k ≠ k' to BEq false
    rw [hne]                         -- evaluate the match branch to the tail case
    exact ih h.2                     -- recurse on the tail


/- ========================================================================== -/
/-! ## Row.NoDupKeys lookup lemmas -/
/- ========================================================================== -/

/-- For unique-key lists, membership determines lookup: if `(k, v) ∈ fields`
and keys are unique, then `fields.lookup k = some v`.
Proof by induction. The key step: in the cons case with `(k, v)` in the tail,
`k ≠ k₂` (otherwise `k` would appear twice in the keys, contradicting Nodup),
so we skip the head via `lookup_cons_ne'` and recurse. -/
theorem lookup_some_of_mem {fields : List (String × α)} {k : String} {v : α}
    (hnd : (fields.map Prod.fst).Nodup) (hmem : (k, v) ∈ fields) :
    fields.lookup k = some v := by
  induction fields with
  | nil => simp at hmem              -- (k, v) ∈ [] is False
  | cons hd tl ih =>
    obtain ⟨k2, v2⟩ := hd
    simp at hnd                      -- Nodup at cons gives: k2 ∉ tl keys ∧ Nodup tl keys
    obtain ⟨hnotin, hnd_tl⟩ := hnd
    rcases List.mem_cons.mp hmem with ⟨rfl, rfl⟩ | hmem_tl
    · exact List.lookup_cons_self    -- (k, v) is the head: lookup finds it immediately
    · -- (k, v) is in the tail: k ≠ k2 (otherwise k2 appears twice, contradicting Nodup)
      have hne : k ≠ k2 := fun heq => by subst heq; exact hnotin v hmem_tl
      rw [lookup_cons_ne' v2 tl hne] -- skip the head
      exact ih hnd_tl hmem_tl        -- recurse on the tail

/-- For unique-key rows, `proj` returns the actual field descriptor
when `(k, fd) ∈ r.fields`. Follows directly from `lookup_some_of_mem`. -/
theorem proj_of_mem {r : Row} {k : String} {fd : FieldDesc}
    (hnd : Row.NoDupKeys r) (hmem : (k, fd) ∈ r.fields) :
    Row.proj r k = fd := by
  cases r with
  | var _ => exact absurd hnd id
  | mk fs u =>
    have hnd' : (fs.map Prod.fst).Nodup := hnd
    simp [Row.proj, Row.fields, lookup_some_of_mem hnd' hmem]

/- ========================================================================== -/
/-! ## fieldCheck: lookup-based field denotation

`fieldCheck` is the lookup-based equivalent of the per-field check that
`knownFieldsDenote` performs structurally. It checks whether an `Option Val`
(the result of looking up a key in a record) satisfies a field descriptor. -/
/- ========================================================================== -/

/-- Does a lookup result satisfy a field descriptor?
- Required + present: the value must have the right type.
- Required + absent: always fails (required field missing).
- Optional + present: the value must have the right type.
- Optional + absent: always succeeds (optional field may be missing). -/
def fieldCheck (fd : FieldDesc) (val : Option Val) : Prop :=
  match fd, val with
  | .req t, some v => denote t v
  | .req _, none => False
  | .opt t, some v => denote t v
  | .opt _, none => True

@[simp] theorem fieldCheck_req_some : fieldCheck (.req t) (some v) = denote t v := rfl
@[simp] theorem fieldCheck_req_none : fieldCheck (.req t) none = False := rfl
@[simp] theorem fieldCheck_opt_some : fieldCheck (.opt t) (some v) = denote t v := rfl
@[simp] theorem fieldCheck_opt_none : fieldCheck (.opt t) none = True := rfl



/- ========================================================================== -/
/-! ## Characterization lemmas

These connect the list-recursive `knownFieldsDenote`/`unknownFieldsDenote`
with the lookup-based `fieldCheck`/`proj`. -/
/- ========================================================================== -/


/-- The `∃ v, lookup = some v ∧ denote t v` form used by `knownFieldsDenote` for
required fields is equivalent to `fieldCheck (.req t) lookup`.
Proof: case split on the lookup result. `some v` gives the existential directly;
`none` gives `False ↔ False`. -/
theorem req_iff_fieldCheck (t : BaseTy) (lookup : Option Val) :
    (∃ v, lookup = some v ∧ denote t v) ↔ fieldCheck (.req t) lookup := by
  cases lookup <;> simp [fieldCheck]

/-- The `∀ v, lookup = some v → denote t v` form used by `knownFieldsDenote` for
optional fields is equivalent to `fieldCheck (.opt t) lookup`.
Proof: case split. `some v` gives `denote t v ↔ denote t v`; `none` gives `True ↔ True`. -/
theorem opt_iff_fieldCheck (t : BaseTy) (lookup : Option Val) :
    (∀ v, lookup = some v → denote t v) ↔ fieldCheck (.opt t) lookup := by
  cases lookup <;> simp [fieldCheck]

/-- **Key characterization for merge proofs.** For field lists built by mapping
a function over a key list (as `mergeFields` produces), `knownFieldsDenote`
equivalent to checking `fieldCheck (f k) (entries.lookup k)` for each key.

Proof by induction on the key list `ks`. At each step:
- Case split on `f k` (Req or Opt) to apply the right equation lemma.
- Use `req_iff_fieldCheck` or `opt_iff_fieldCheck` to convert the structural
  form to the `fieldCheck` form.
- The inductive hypothesis `ih` handles the tail.
- The constructor/intro split handles the ↔ by separating the head from the tail. -/
theorem knownFieldsDenote_map
    (ks : List String) (f : String → FieldDesc) (entries : List (String × Val)) :
    knownFieldsDenote (ks.map (fun k => (k, f k))) entries ↔
    ∀ k ∈ ks, fieldCheck (f k) (entries.lookup k) := by
  induction ks with
  | nil => simp                      -- empty key list: both sides are True
  | cons k rest ih =>
    cases hfk : f k with             -- case split on the field descriptor at key k
    | req t =>
      -- knownFieldsDenote at cons with Req: uses eq_2 (∃ v, ...)
      simp only [List.map_cons, knownFields_req, ih, hfk, req_iff_fieldCheck]
      constructor
      · intro ⟨hk, hrest⟩ k2 hk2    -- forward: destructure head ∧ tail
        rcases List.mem_cons.mp hk2 with rfl | hk2
        · rw [hfk]; exact hk         -- k2 = k: use head hypothesis
        · exact hrest k2 hk2         -- k2 ∈ rest: use tail hypothesis
      · intro h                       -- backward: split the ∀ into head and tail
        exact ⟨by rw [← hfk]; exact h k (List.mem_cons.mpr (Or.inl rfl)),
               fun k2 hk2 => h k2 (List.mem_cons.mpr (Or.inr hk2))⟩
    | opt t =>
      -- knownFieldsDenote at cons with Opt: uses eq_3 (∀ v, ...)
      -- Same structure as the Req case but with opt_iff_fieldCheck.
      simp only [List.map_cons, knownFields_opt, ih, hfk, opt_iff_fieldCheck]
      constructor
      · intro ⟨hk, hrest⟩ k2 hk2
        rcases List.mem_cons.mp hk2 with rfl | hk2
        · rw [hfk]; exact hk
        · exact hrest k2 hk2
      · intro h
        exact ⟨by rw [← hfk]; exact h k (List.mem_cons.mpr (Or.inl rfl)),
               fun k2 hk2 => h k2 (List.mem_cons.mpr (Or.inr hk2))⟩

/-- Characterization for arbitrary field lists (not necessarily built by map).
`knownFieldsDenote` is equivalent to checking `fieldCheck p.2 (entries.lookup p.1)`
for each pair `p` in the field list.

Same proof structure as `knownFieldsDenote_map` but iterates over pairs
rather than keys. -/
theorem knownFieldsDenote_iff
    (fields : List (String × FieldDesc)) (entries : List (String × Val)) :
    knownFieldsDenote fields entries ↔
    ∀ p ∈ fields, fieldCheck p.2 (entries.lookup p.1) := by
  induction fields with
  | nil => simp
  | cons hd rest ih =>
    obtain ⟨k, fd⟩ := hd
    cases fd with
    | req t =>
      simp only [knownFields_req, ih, req_iff_fieldCheck]
      constructor
      · intro ⟨hk, hrest⟩ p hp
        rcases List.mem_cons.mp hp with ⟨rfl, rfl⟩ | hp
        · exact hk
        · exact hrest p hp
      · intro h
        exact ⟨h (k, .req t) (List.mem_cons.mpr (Or.inl rfl)),
               fun p hp => h p (List.mem_cons.mpr (Or.inr hp))⟩
    | opt t =>
      simp only [knownFields_opt, ih, opt_iff_fieldCheck]
      constructor
      · intro ⟨hk, hrest⟩ p hp
        rcases List.mem_cons.mp hp with ⟨rfl, rfl⟩ | hp
        · exact hk
        · exact hrest p hp
      · intro h
        exact ⟨h (k, .opt t) (List.mem_cons.mpr (Or.inl rfl)),
               fun p hp => h p (List.mem_cons.mpr (Or.inr hp))⟩

/-- Characterization for unique-key rows via `proj`.
For NoDupKeys rows, `knownFieldsDenote` is equivalent to checking
`fieldCheck (proj r k) (entries.lookup k)` for each key `k` in `r.fieldLabels`.

This converts the pair-based `knownFieldsDenote_iff` to a key-based form
using `proj_of_mem`: for NoDup lists, each key maps to exactly one descriptor,
which is what `proj` returns. -/
theorem knownFieldsDenote_proj
    (r : Row) (entries : List (String × Val))
    (hnd : Row.NoDupKeys r) :
    knownFieldsDenote r.fields entries ↔
    ∀ k ∈ r.fieldLabels, fieldCheck (Row.proj r k) (entries.lookup k) := by
  rw [knownFieldsDenote_iff]         -- convert to the pair-based form
  constructor
  · intro h k hk
    simp [Row.fieldLabels, List.mem_map] at hk  -- k ∈ fieldLabels ↔ ∃ fd, (k, fd) ∈ r.fields
    obtain ⟨fd, hmem⟩ := hk
    rw [proj_of_mem hnd hmem]        -- proj returns fd for this key (NoDupKeys)
    exact h (k, fd) hmem             -- apply the pair-based hypothesis
  · intro h p hmem
    rw [← proj_of_mem hnd hmem]      -- rewrite fd back to proj
    exact h p.1 (List.mem_map.mpr ⟨p, hmem, rfl⟩)  -- key is in fieldLabels

/-- Characterization for `unknownFieldsDenote`: the list-recursive check is
equivalent to a membership-based quantification over entries.

Proof by induction on entries. In the cons case, the head `(k, v)` contributes
one conjunct (its own check) and the tail is handled by the IH. The ↔ is split
by separating the head from the tail in both directions. -/
theorem unknownFieldsDenote_iff
    (fields : List (String × FieldDesc)) (unk : BaseTy) (entries : List (String × Val)) :
    unknownFieldsDenote fields unk entries ↔
    (∀ k v, (k, v) ∈ entries → fields.lookup k = none → denote unk v) := by
  induction entries with
  | nil => simp                      -- empty entries: both sides are True
  | cons entry tl ih =>
    obtain ⟨k, v⟩ := entry
    simp [ih]                        -- unfold unknownFieldsDenote at cons; rewrite tail with IH
    constructor
    · intro ⟨hkv, htl⟩ k2 v2 hmem hflook
      rcases hmem with ⟨rfl, rfl⟩ | hmem  -- (k2, v2) is head or in tail
      · exact hkv hflook             -- head: use hkv
      · exact htl k2 v2 hmem hflook  -- tail: use htl from IH
    · intro h
      exact ⟨fun hf => h k v (Or.inl ⟨rfl, rfl⟩) hf,   -- head case
             fun k2 v2 hmem hflook => h k2 v2 (Or.inr hmem) hflook⟩  -- tail case


/- ========================================================================== -/
/-! ## Per-key characterization of shape denotation -/
/- ========================================================================== -/

/-- A record inhabits a shape iff it has unique keys and every key (declared or not)
satisfies the projected field descriptor. This combines `knownFieldsDenote_proj`
and `unknownFieldsDenote_iff` into a single per-key quantification. -/
theorem shape_denote_iff (r : Row) (entries : List (String × Val)) (hnd : Row.NoDupKeys r) :
    denote (.shape r) (.record entries) ↔
    (entries.map Prod.fst).Nodup ∧ ∀ k, fieldCheck (Row.proj r k) (entries.lookup k) := by
  cases r with
  | var _ => exact absurd hnd id
  | mk fields unk =>
  rw [denote_shape]
  constructor
  · intro ⟨hnodup, hknown, hunknown⟩
    have hknown' := (knownFieldsDenote_proj (Row.mk fields unk) entries hnd).mp hknown
    rw [unknownFieldsDenote_iff] at hunknown
    exact ⟨hnodup, fun k => by
      by_cases hk : k ∈ Row.fieldLabels (Row.mk fields unk)
      · exact hknown' k hk
      · have hflookup : fields.lookup k = none := lookup_none_of_not_mem_keys hk
        rw [show Row.proj (Row.mk fields unk) k = .opt unk from by simp [Row.proj, Row.fields, Row.unknown, hflookup]]
        cases hlook : entries.lookup k with
        | none => simp [fieldCheck]
        | some v => simp [fieldCheck]; exact hunknown k v (mem_of_lookup_eq_some hlook) hflookup⟩
  · intro ⟨hnodup, hall⟩
    refine ⟨hnodup, ?_, ?_⟩
    · exact (knownFieldsDenote_proj (Row.mk fields unk) entries hnd).mpr
        fun k hk => hall k
    · rw [unknownFieldsDenote_iff]
      intro k v hmem hlookup
      have hfc := hall k
      have hlook := lookup_some_of_mem hnodup hmem
      simp [Row.proj, Row.fields, Row.unknown, hlookup, fieldCheck] at hfc
      rw [hlook] at hfc; simp at hfc
      exact hfc
