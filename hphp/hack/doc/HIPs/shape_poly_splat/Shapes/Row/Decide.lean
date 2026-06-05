import Shapes.Row.Sub

/- ========================================================================== -/
/-! # Concrete `row_sub` checking

## How to use

To prove a concrete `row_sub` goal:

    example : r₁ <:ʳ r₂ := rowSubBool_sound (by native_decide)

This works in two steps:
1. `native_decide` compiles `rowSubBool r₁ r₂` to native code, runs it,
   and checks that the result is `true`. This produces a proof of
   `rowSubBool r₁ r₂ = true` — a `Bool` equality, which is trivially
   decidable.
2. `rowSubBool_sound` is a proved theorem that converts
   `rowSubBool r₁ r₂ = true` into `r₁ <:ʳ r₂`.

## Why soundness only?

A `Decidable (row_sub r₁ r₂)` instance would need BOTH directions:
- Soundness: `rowSubBool = true → row_sub` (proved)
- Completeness: `row_sub → rowSubBool = true` (not proved)

Completeness requires showing that when semantic `sub` holds between
two inhabited shapes, the structural `row_sub` conditions also hold.
This is true but requires constructing test records with fresh keys
to probe individual fields — substantial infrastructure we don't
have yet. Since `native_decide` only needs to evaluate the boolean
(soundness lifts the `true` case), we get full functionality without
completeness.

## Why fuel?

`subGo` recurses through `BaseTy → shape → Row → FieldDesc → BaseTy`.
This cycle crosses the mutual inductive, and Lean's structural recursion
checker can't verify termination through the `Row.proj`/`List.all`
indirections. A fuel parameter (`Nat` counting down) sidesteps this.
The fuel 1000 in `rowSubBool` is arbitrary but far exceeds any
realistic type nesting depth.

## Why equation lemmas?

`subGo` uses a `where` clause for `goField`/`goFields`. Lean 4 compiles
where-clause helpers via well-founded recursion wrappers, making them
opaque to `simp` and `rfl` in proof mode — even though the function
evaluates correctly at runtime. We provide manual `@[simp]` lemmas
proved via `simp only [subGo]` (which DOES unfold when both
constructor arguments are concrete). These lemmas are only needed
for the `subGo_sound` proof, not for `native_decide` evaluation. -/
/- ========================================================================== -/

/- ========================================================================== -/
/-! ## Decidable Row.NoDupKeys -/
/- ========================================================================== -/

instance instDecidableNoDupKeys : (r : Row) → Decidable (Row.NoDupKeys r)
  | .mk fs _ => by unfold Row.NoDupKeys; exact inferInstance
  | .var _ => isFalse id

/- ========================================================================== -/
/-! ## isEmpty: conservative uninhabitedness check

Returns `true` only when the type is provably uninhabited:
- `bot` has no values
- `union a b` where both sides are empty
- `shape (var _)` (row variables have denotation `False`)
- `shape (mk fields _)` where some required field has an empty type

Returns `false` conservatively — the type might still be uninhabited
(e.g., shapes with conflicting duplicate keys). This is fine: `false`
means "I don't know", and soundness only needs the `true → uninhabited`
direction. The checker uses `isEmpty` as a catch-all for cases where
no structural sub rule applies (see `subGo`). -/
/- ========================================================================== -/

/-- `isEmpty a = true → ∀ v, ¬denote a v`: no value inhabits an empty type. -/
theorem BaseTy.isEmpty_sound {a : BaseTy} (h : a.isEmpty = true) (v : Val) :
    ¬denote a v := by
  match a, v with
  | .bot, _ => simp
  | .nat, _ | .bool, _ | .top, _ => simp [isEmpty] at h
  | .union a b, v =>
    simp [isEmpty, Bool.and_eq_true] at h
    intro hv; simp at hv; exact hv.elim (isEmpty_sound h.1 _) (isEmpty_sound h.2 _)
  | .shape (.var _), v => cases v <;> simp
  | .shape (.mk _ _), .nat _ | .shape (.mk _ _), .bool _ => simp
  | .shape (.mk fields unk), .record entries =>
    simp [isEmpty] at h
    intro hv; rw [denote_shape] at hv; obtain ⟨_, hknown, _⟩ := hv
    exact go_sound h ((knownFieldsDenote_iff fields entries).mp hknown)
where
  /-- If `go fields = true`, no record can satisfy `knownFieldsDenote fields`. -/
  go_sound {fields : List (String × FieldDesc)} {entries : List (String × Val)}
      (h : BaseTy.isEmpty.go fields = true)
      (hk : ∀ p ∈ fields, fieldCheck p.2 (entries.lookup p.1)) : False := by
    match fields with
    | [] => simp [BaseTy.isEmpty.go] at h
    | (_, .opt _) :: rest =>
      simp [BaseTy.isEmpty.go] at h
      exact go_sound h (fun p hp => hk p (.tail _ hp))
    | (k, .req t) :: rest =>
      simp [BaseTy.isEmpty.go, Bool.or_eq_true] at h
      have hfc := hk ⟨k, .req t⟩ (.head _)
      rw [show (⟨k, .req t⟩ : String × FieldDesc).2 = .req t from rfl] at hfc
      obtain ⟨v, _, hv⟩ := (req_iff_fieldCheck t (entries.lookup k)).mpr hfc
      exact h.elim (fun he => BaseTy.isEmpty_sound he v hv)
                    (fun hr => go_sound hr (fun p hp => hk p (.tail _ hp)))

/- ========================================================================== -/
/-! ## subGo: boolean sub checker

Checks `a <:ᵇ b` by structural case analysis on `(a, b)` with a fuel
parameter `n` that decreases at each recursive call.

### Match arm priority (earlier arms win)

1. `(0, a, _) → a.isEmpty` — out of fuel, fall back to isEmpty
2. `(_, bot, _) → true` — bot is uninhabited, sub holds vacuously
3. `(_, _, top) → true` — top accepts everything
4. `(_, nat, nat)` / `(_, bool, bool) → true` — reflexivity
5. `(n+1, union a b, c) → subGo n a c && subGo n b c` — union-left
6. `(n+1, nat, union b c) → subGo n nat b || subGo n nat c` — see below
7. `(n+1, bool, union b c)` — same as nat case
8. `(n+1, shape mk, shape mk) → isEmpty || structural` — see below
9. `(_, a, _) → a.isEmpty` — catch-all

### Union on right (arms 6–7)

`sub nat (union b c)` means every nat value lands in `b` or `c`.
Since `denote` is uniform on nat values (denote t (nat n) doesn't
depend on n), testing a single nat determines which side: if nat 0 ∈ b
then all nats ∈ b. So `sub nat b ∨ sub nat c` is equivalent to
`sub nat (union b c)`. Same for bool. This does NOT work for `top`
or `shape` — those fall to the catch-all.

### Shape vs shape (arm 8)

`isEmpty || structural`: if the left shape is uninhabited, sub holds
vacuously. Otherwise, check the structural `row_sub` conditions:
unknown sub, Row.NoDupKeys on both sides, and per-field sub over the
finite union of field keys.

### Catch-all (arm 9)

Returns `a.isEmpty`. This is SOUND: if `isEmpty = true`, the type
is uninhabited and sub holds vacuously. If `isEmpty = false`, the
checker returns `false` — it gives up rather than attempting a proof.
This means the checker is INCOMPLETE (it may return `false` when sub
actually holds), but that's fine: we only need soundness. -/
/- ========================================================================== -/

def subGo : Nat → BaseTy → BaseTy → Bool
  | 0, a, _ => a.isEmpty
  | _, .bot, _ => true
  | _, _, .top => true
  | _, .nat, .nat => true
  | _, .bool, .bool => true
  | n + 1, .union a b, c => subGo n a c && subGo n b c
  | n + 1, .nat, .union b c => subGo n .nat b || subGo n .nat c
  | n + 1, .bool, .union b c => subGo n .bool b || subGo n .bool c
  | n + 1, .shape (.mk f₁ u₁), .shape (.mk f₂ u₂) =>
      (.shape (.mk f₁ u₁) : BaseTy).isEmpty ||
      (subGo n u₁ u₂ &&
       decide (Row.NoDupKeys (.mk f₁ u₁)) &&
       decide (Row.NoDupKeys (.mk f₂ u₂)) &&
       goFields n f₁ u₁ f₂ u₂ (f₁.map Prod.fst ++ f₂.map Prod.fst).eraseDups)
  | _, a, _ => a.isEmpty
where
  goField (n : Nat) : FieldDesc → FieldDesc → Bool
    | .req t₁, .req t₂ | .opt t₁, .opt t₂ | .req t₁, .opt t₂ => subGo n t₁ t₂
    | .opt _, .req _ => false
  goFields (n : Nat) (f₁ : List (String × FieldDesc)) (u₁ : BaseTy)
      (f₂ : List (String × FieldDesc)) (u₂ : BaseTy) : List String → Bool
    | [] => true
    | k :: rest =>
        goField n (Row.proj (.mk f₁ u₁) k) (Row.proj (.mk f₂ u₂) k) &&
        goFields n f₁ u₁ f₂ u₂ rest

/- ========================================================================== -/
/-! ### Equation lemmas for `subGo`

One `@[simp]` lemma per match arm, proved by `simp only [subGo]`
(which reduces when constructors are concrete). Used only by the
`subGo_sound` proof — `native_decide` doesn't need them.

The union-left arm needs one lemma per RHS constructor because
the `.top` arm has higher priority: `subGo (n+1) (.union a b) .top`
matches `.top` (→ true), not union-left (→ && of recursions).
A general `subGo_union_l` equation would be false at `.top`. -/
/- ========================================================================== -/

-- Base cases
@[simp] theorem subGo_zero : subGo 0 a b = a.isEmpty := by
  simp only [subGo]; try rfl
@[simp] theorem subGo_bot : subGo (n + 1) .bot b = true := by
  simp only [subGo]; try rfl
@[simp] theorem subGo_top_r : subGo (n + 1) a .top = true := by
  cases a <;> simp only [subGo] <;> rfl
@[simp] theorem subGo_nat_nat : subGo (n + 1) .nat .nat = true := by
  simp only [subGo]; try rfl
@[simp] theorem subGo_bool_bool : subGo (n + 1) .bool .bool = true := by
  simp only [subGo]; try rfl

-- Union on left (one lemma per RHS constructor, since .top has priority)
@[simp] theorem subGo_union_l_nat : subGo (n + 1) (.union a b) .nat =
    (subGo n a .nat && subGo n b .nat) := by simp only [subGo]
@[simp] theorem subGo_union_l_bool : subGo (n + 1) (.union a b) .bool =
    (subGo n a .bool && subGo n b .bool) := by simp only [subGo]
@[simp] theorem subGo_union_l_bot : subGo (n + 1) (.union a b) .bot =
    (subGo n a .bot && subGo n b .bot) := by simp only [subGo]
@[simp] theorem subGo_union_l_union : subGo (n + 1) (.union a b) (.union c d) =
    (subGo n a (.union c d) && subGo n b (.union c d)) := by simp only [subGo]
@[simp] theorem subGo_union_l_shape : subGo (n + 1) (.union a b) (.shape r) =
    (subGo n a (.shape r) && subGo n b (.shape r)) := by
  cases r <;> simp only [subGo]

-- Union on right (nat, bool only — top/shape/etc. fall to catch-all)
@[simp] theorem subGo_nat_union : subGo (n + 1) .nat (.union b c) =
    (subGo n .nat b || subGo n .nat c) := by simp only [subGo]
@[simp] theorem subGo_bool_union : subGo (n + 1) .bool (.union b c) =
    (subGo n .bool b || subGo n .bool c) := by simp only [subGo]

-- Shape vs shape
@[simp] theorem subGo_shape_shape : subGo (n + 1) (.shape (.mk f₁ u₁)) (.shape (.mk f₂ u₂)) =
    ((.shape (.mk f₁ u₁) : BaseTy).isEmpty ||
     (subGo n u₁ u₂ && decide (Row.NoDupKeys (.mk f₁ u₁)) && decide (Row.NoDupKeys (.mk f₂ u₂)) &&
      subGo.goFields n f₁ u₁ f₂ u₂ (f₁.map Prod.fst ++ f₂.map Prod.fst).eraseDups)) := by
  simp only [subGo]; try rfl

-- Catch-all cases: all reduce to isEmpty
@[simp] theorem subGo_nat_bool : subGo (n+1) .nat .bool = BaseTy.isEmpty .nat := by simp only [subGo]; try rfl
@[simp] theorem subGo_nat_bot : subGo (n+1) .nat .bot = BaseTy.isEmpty .nat := by simp only [subGo]; try rfl
@[simp] theorem subGo_nat_shape : subGo (n+1) .nat (.shape r) = BaseTy.isEmpty .nat := by cases r <;> simp only [subGo] <;> rfl
@[simp] theorem subGo_bool_nat : subGo (n+1) .bool .nat = BaseTy.isEmpty .bool := by simp only [subGo]; try rfl
@[simp] theorem subGo_bool_bot : subGo (n+1) .bool .bot = BaseTy.isEmpty .bool := by simp only [subGo]; try rfl
@[simp] theorem subGo_bool_shape : subGo (n+1) .bool (.shape r) = BaseTy.isEmpty .bool := by cases r <;> simp only [subGo] <;> rfl
@[simp] theorem subGo_top_nat : subGo (n+1) .top .nat = BaseTy.isEmpty .top := by simp only [subGo]; try rfl
@[simp] theorem subGo_top_bool : subGo (n+1) .top .bool = BaseTy.isEmpty .top := by simp only [subGo]; try rfl
@[simp] theorem subGo_top_bot : subGo (n+1) .top .bot = BaseTy.isEmpty .top := by simp only [subGo]; try rfl
@[simp] theorem subGo_top_shape : subGo (n+1) .top (.shape r) = BaseTy.isEmpty .top := by cases r <;> simp only [subGo] <;> rfl
@[simp] theorem subGo_top_union : subGo (n+1) .top (.union b c) = BaseTy.isEmpty .top := by simp only [subGo]; try rfl
@[simp] theorem subGo_shape_nat : subGo (n+1) (.shape r) .nat = BaseTy.isEmpty (.shape r) := by cases r <;> simp only [subGo] <;> rfl
@[simp] theorem subGo_shape_bool : subGo (n+1) (.shape r) .bool = BaseTy.isEmpty (.shape r) := by cases r <;> simp only [subGo] <;> rfl
@[simp] theorem subGo_shape_bot : subGo (n+1) (.shape r) .bot = BaseTy.isEmpty (.shape r) := by cases r <;> simp only [subGo] <;> rfl
@[simp] theorem subGo_shape_union : subGo (n+1) (.shape r) (.union b c) = BaseTy.isEmpty (.shape r) := by cases r <;> simp only [subGo] <;> rfl
@[simp] theorem subGo_var_shape : subGo (n+1) (.shape (.var v)) (.shape r) = BaseTy.isEmpty (.shape (.var v)) := by cases r <;> simp only [subGo] <;> rfl
@[simp] theorem subGo_mk_var : subGo (n+1) (.shape (.mk f u)) (.shape (.var v)) = BaseTy.isEmpty (.shape (.mk f u)) := by simp only [subGo]; try rfl

-- goField / goFields equation lemmas
@[simp] theorem goField_req_req : subGo.goField n (.req t₁) (.req t₂) = subGo n t₁ t₂ := by simp only [subGo.goField]; try rfl
@[simp] theorem goField_opt_opt : subGo.goField n (.opt t₁) (.opt t₂) = subGo n t₁ t₂ := by simp only [subGo.goField]; try rfl
@[simp] theorem goField_req_opt : subGo.goField n (.req t₁) (.opt t₂) = subGo n t₁ t₂ := by simp only [subGo.goField]; try rfl
@[simp] theorem goField_opt_req : subGo.goField n (.opt t₁) (.req t₂) = false := by simp only [subGo.goField]; try rfl
@[simp] theorem goFields_nil : subGo.goFields n f₁ u₁ f₂ u₂ [] = true := by simp only [subGo.goFields]; try rfl
@[simp] theorem goFields_cons : subGo.goFields n f₁ u₁ f₂ u₂ (k :: rest) =
    (subGo.goField n (Row.proj (.mk f₁ u₁) k) (Row.proj (.mk f₂ u₂) k) &&
     subGo.goFields n f₁ u₁ f₂ u₂ rest) := by simp only [subGo.goFields]; try rfl

/- ========================================================================== -/
/-! ## rowSubBool

Checks all four conjuncts of `row_sub`:
1. `Row.NoDupKeys r₁` — via `decide`
2. `Row.NoDupKeys r₂` — via `decide`
3. `r₁.unknown <:ᵇ r₂.unknown` — via `subGo`
4. `∀ k, Row.proj r₁ k <:ᶠ Row.proj r₂ k` — via `goFields` over the finite
   union of both field-key lists -/
/- ========================================================================== -/

def rowSubBool (r₁ r₂ : Row) : Bool :=
  match r₁, r₂ with
  | .mk f₁ u₁, .mk f₂ u₂ =>
    decide (Row.NoDupKeys (.mk f₁ u₁)) && decide (Row.NoDupKeys (.mk f₂ u₂)) &&
    subGo 1000 u₁ u₂ &&
    subGo.goFields 1000 f₁ u₁ f₂ u₂ (f₁.map Prod.fst ++ f₂.map Prod.fst).eraseDups
  | _, _ => false


/- ========================================================================== -/
/-! ## Soundness

The chain: `rowSubBool_sound` calls `subGo_sound` for the unknown-type
sub and `goFields_sound` for per-field sub. `goFields_sound` calls
`goField_sound` which calls `subGo_sound` on the inner types. Each
layer's proof mirrors the checker's structure, using the equation
lemmas to rewrite `h : checker = true` into the semantic relation.  -/
/- ========================================================================== -/

/-- `goField true → fieldSub`, given that `subGo true → sub` at the same fuel. -/
private theorem goField_sound {n : Nat} {fd₁ fd₂ : FieldDesc}
    (ih : ∀ {a b}, subGo n a b = true → a <:ᵇ b)
    (h : subGo.goField n fd₁ fd₂ = true) : fd₁ <:ᶠ fd₂ := by
  cases fd₁ <;> cases fd₂ <;> simp_all [fieldSub_req, fieldSub_opt, fieldSub_req_opt]

/-- `goFields true → per-key fieldSub`, for any key in the checked list. -/
private theorem goFields_sound {n : Nat}
    {f₁ : List (String × FieldDesc)} {u₁ : BaseTy}
    {f₂ : List (String × FieldDesc)} {u₂ : BaseTy}
    {keys : List String}
    (ih : ∀ {a b}, subGo n a b = true → a <:ᵇ b)
    (h : subGo.goFields n f₁ u₁ f₂ u₂ keys = true)
    {k : String} (hk : k ∈ keys) :
    Row.proj (.mk f₁ u₁) k <:ᶠ Row.proj (.mk f₂ u₂) k := by
  induction keys with
  | nil => contradiction
  | cons x xs ihk =>
    simp [Bool.and_eq_true] at h
    rcases List.mem_cons.mp hk with rfl | hmem
    · exact goField_sound ih h.1
    · exact ihk h.2 hmem

/-- `subGo n a b = true → a <:ᵇ b`. By induction on fuel. Each match
arm uses the corresponding equation lemma to rewrite `h`, then applies
the semantic subtyping lemma. -/
theorem subGo_sound {n : Nat} {a b : BaseTy}
    (h : subGo n a b = true) : a <:ᵇ b := by
  induction n generalizing a b with
  | zero =>
    -- subGo 0 a b = a.isEmpty. If true, a is uninhabited → sub vacuously.
    simp at h; exact fun v hv => absurd hv (BaseTy.isEmpty_sound h v)
  | succ n ih =>
    match a, b with
    -- Priority arms: bot/top/same-constructor
    | .bot, _ => exact bot_sub _
    | _, .top => exact sub_top _
    | .nat, .nat | .bool, .bool => exact sub_refl _
    -- Union on left: decompose. One case per RHS constructor since the
    -- .top arm has higher priority than union-left.
    | .union a' b', c =>
      match c with
      | .top => exact sub_top _
      | .nat => rw [subGo_union_l_nat] at h; simp [Bool.and_eq_true] at h; exact union_sub (ih h.1) (ih h.2)
      | .bool => rw [subGo_union_l_bool] at h; simp [Bool.and_eq_true] at h; exact union_sub (ih h.1) (ih h.2)
      | .bot => rw [subGo_union_l_bot] at h; simp [Bool.and_eq_true] at h; exact union_sub (ih h.1) (ih h.2)
      | .union c d => rw [subGo_union_l_union] at h; simp [Bool.and_eq_true] at h; exact union_sub (ih h.1) (ih h.2)
      | .shape r => rw [subGo_union_l_shape] at h; simp [Bool.and_eq_true] at h; exact union_sub (ih h.1) (ih h.2)
    -- Nat/bool against union on right: one side must accept all nats/bools
    | .nat, .union b' c' =>
      rw [subGo_nat_union] at h
      rcases (by simp at h; exact h : _ ∨ _) with hb | hc
      · exact sub_trans (ih hb) (sub_union_l _ _)
      · exact sub_trans (ih hc) (sub_union_r _ _)
    | .bool, .union b' c' =>
      rw [subGo_bool_union] at h
      rcases (by simp at h; exact h : _ ∨ _) with hb | hc
      · exact sub_trans (ih hb) (sub_union_l _ _)
      · exact sub_trans (ih hc) (sub_union_r _ _)
    -- Shape vs shape: isEmpty fallback OR structural row check
    | .shape (.mk f₁ u₁), .shape (.mk f₂ u₂) =>
      rw [subGo_shape_shape] at h
      rcases (by simp at h; exact h : _ ∨ _) with hempty | hstruct
      · -- Left shape is uninhabited → sub holds vacuously
        exact fun v hv => absurd hv (BaseTy.isEmpty_sound hempty v)
      · -- Structural check passed → extract row_sub components
        have hs := hstruct; simp at hs
        obtain ⟨⟨⟨hu, hnd₁⟩, hnd₂⟩, hfl⟩ := hs
        apply row_sub_sound
        refine ⟨hnd₁, hnd₂, ih hu, fun k => ?_⟩
        -- For keys in both field lists: goFields gives the proof
        by_cases hk : k ∈ (f₁.map Prod.fst ++ f₂.map Prod.fst).eraseDups
        · exact goFields_sound ih hfl hk
        · -- For keys in neither: both project to Opt unknown, sub from unknowns
          have hk₁ := fun hm => hk (List.mem_eraseDups.mpr (List.mem_append_left _ hm))
          have hk₂ := fun hm => hk (List.mem_eraseDups.mpr (List.mem_append_right _ hm))
          simp [Row.proj, Row.fields, Row.unknown,
                lookup_none_of_not_mem_keys hk₁, lookup_none_of_not_mem_keys hk₂]
          exact fieldSub_opt (ih hu)
    -- Catch-all: subGo returns isEmpty. If isEmpty = true, the type is
    -- uninhabited so sub holds vacuously. If isEmpty = false, subGo
    -- returned false, contradicting h.
    | .nat, .bool | .nat, .bot => simp [BaseTy.isEmpty] at h
    | .nat, .shape r => rw [subGo_nat_shape] at h; simp [BaseTy.isEmpty] at h
    | .bool, .nat | .bool, .bot => simp [BaseTy.isEmpty] at h
    | .bool, .shape r => rw [subGo_bool_shape] at h; simp [BaseTy.isEmpty] at h
    | .top, .nat | .top, .bool | .top, .bot => simp [BaseTy.isEmpty] at h
    | .top, .shape r => rw [subGo_top_shape] at h; simp [BaseTy.isEmpty] at h
    | .top, .union _ _ => rw [subGo_top_union] at h; simp [BaseTy.isEmpty] at h
    | .shape r, .union _ _ =>
      rw [subGo_shape_union] at h; exact fun v hv => absurd hv (BaseTy.isEmpty_sound h v)
    | .shape r, .nat =>
      rw [subGo_shape_nat] at h; exact fun v hv => absurd hv (BaseTy.isEmpty_sound h v)
    | .shape r, .bool =>
      rw [subGo_shape_bool] at h; exact fun v hv => absurd hv (BaseTy.isEmpty_sound h v)
    | .shape r, .bot =>
      rw [subGo_shape_bot] at h; exact fun v hv => absurd hv (BaseTy.isEmpty_sound h v)
    | .shape (.var _), .shape _ =>
      rw [subGo_var_shape] at h; exact fun v hv => absurd hv (BaseTy.isEmpty_sound h v)
    | .shape (.mk _ _), .shape (.var _) =>
      rw [subGo_mk_var] at h; exact fun v hv => absurd hv (BaseTy.isEmpty_sound h v)

/-- `rowSubBool r₁ r₂ = true → r₁ <:ʳ r₂`. Decomposes the boolean
conjunction into the four `row_sub` conjuncts. For per-field sub,
keys outside both field lists get `Opt unknown` on both sides,
reduced to unknown sub via `fieldSub_opt`. -/
theorem rowSubBool_sound {r₁ r₂ : Row}
    (h : rowSubBool r₁ r₂ = true) : r₁ <:ʳ r₂ := by
  match r₁, r₂ with
  | .mk f₁ u₁, .mk f₂ u₂ =>
    simp [rowSubBool, Bool.and_eq_true, decide_eq_true_eq] at h
    obtain ⟨⟨⟨hnd₁, hnd₂⟩, hu⟩, hfl⟩ := h
    refine ⟨hnd₁, hnd₂, subGo_sound hu, fun k => ?_⟩
    by_cases hk : k ∈ (f₁.map Prod.fst ++ f₂.map Prod.fst).eraseDups
    · exact goFields_sound (fun h => subGo_sound h) hfl hk
    · have hk₁ := fun hm => hk (List.mem_eraseDups.mpr (List.mem_append_left _ hm))
      have hk₂ := fun hm => hk (List.mem_eraseDups.mpr (List.mem_append_right _ hm))
      simp [Row.proj, Row.fields, Row.unknown,
            lookup_none_of_not_mem_keys hk₁, lookup_none_of_not_mem_keys hk₂]
      exact fieldSub_opt (subGo_sound hu)
  | .var _, _ | _, .var _ => simp [rowSubBool] at h

/- ========================================================================== -/
/-! ## Usage examples

Positive: `rowSubBool_sound (by native_decide)` — the boolean evaluates
to true, soundness converts to a proof.

Negative: extract a counterexample from `row_sub`'s `∀ k` clause by
picking a key `k` and a value `val` that satisfies `Row.proj r₁ k` but not
`Row.proj r₂ k`. This is manual (the checker doesn't help with refutation). -/
/- ========================================================================== -/

private def F_test : Row := .mk [("x", .req .nat), ("y", .req .bool)] .bot
private def T1_test : Row := .mk [("x", .req .nat)] .bot
private def T2_test : Row := .mk [("y", .req .bool)] .bot

example : F_test <:ʳ mergeRow T1_test T2_test :=
  rowSubBool_sound (by native_decide)

example : ¬ (Row.mk [("x", .req .nat)] .bot <:ʳ
              Row.mk [("x", .req .bool)] .bot) := fun h =>
  absurd (h.2.2.2 "x" (some (.nat 0)) (by simp [Row.proj, Row.fields, fieldCheck]))
    (by simp [Row.proj, Row.fields, fieldCheck])
