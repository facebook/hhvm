import Shapes.Row.Normalize
import Shapes.Splat.Decomp
import Shapes.Row.Decide

/- ========================================================================== -/
/-! # Multi-splat inference and sole-splat condition

When a shape splat contains multiple type variables, the inference algorithm
replaces all but the rightmost with their current lower bound, then solves
for the rightmost using the fresh-variable trick.

## Main results

- **Sole-splat sufficiency**: replacing a type variable with its determined
  lower bound in multi-splat occurrences is sound (`normalize_substAt_mono`).
- **Sole-splat necessity**: without it, two distinct valid solutions exist. -/
/- ========================================================================== -/

/- ========================================================================== -/
/-! ## Sole-splat sufficiency -/
/- ========================================================================== -/

/-- Replacing a type variable with a lower bound gives a smaller result.
If the subtyping check passes with the actual value, it passes with the
lower bound — the result can only get smaller. -/
theorem sole_splat_sub_sound {rs : List Row} {i : Nat} {L V target : Row}
    (hi : i < rs.length)
    (hnd : ∀ r ∈ rs, Row.NoDupKeys r)
    (hLV : L <:ʳ V)
    (htarget : normalize (rs.set i V) <:ʳ target) :
    normalize (rs.set i L) <:ʳ target :=
  row_sub_trans (normalize_substAt_mono hi hnd hLV) htarget

/-- Dual: for supertype position. -/
theorem sole_splat_super_sound {rs : List Row} {i : Nat} {L V source : Row}
    (hi : i < rs.length)
    (hnd : ∀ r ∈ rs, Row.NoDupKeys r)
    (hLV : L <:ʳ V)
    (hsource : source <:ʳ normalize (rs.set i L)) :
    source <:ʳ normalize (rs.set i V) :=
  row_sub_trans hsource (normalize_substAt_mono hi hnd hLV)

/- ========================================================================== -/
/-! ## The sole-splat predicate

A list of rows satisfies the sole-splat condition when every row variable
that appears in any element also appears as the ONLY variable in at least
one element. This ensures the inference algorithm can determine each
type variable independently. -/
/- ========================================================================== -/

/-- A row has no free row variables (is ground). -/
def Row.noVars : Row → Prop
  | .mk _ _ => True
  | .var _ => False

/-- Count the number of row variables in a list of rows. -/
def varCount : List Row → Nat
  | [] => 0
  | (.var _) :: rest => 1 + varCount rest
  | (.mk _ _) :: rest => varCount rest

/-- A position in a splat list is a "sole occurrence" for a variable:
the list element at that position is the variable, and no other element
is a variable. -/
def isSoleSplat (rs : List Row) (i : Nat) : Prop :=
  ∃ (hi : i < rs.length), (∃ n, rs[i] = .var n) ∧ varCount rs = 1

/-- The sole-splat well-formedness condition: for a function with parameters
`params` (each a splat list) and return type `ret` (a splat list), every
type variable that appears in any splat has a sole-splat occurrence in
at least one parameter or the return type. -/
def SoleSplatWF (params : List (List Row)) (ret : List Row) : Prop :=
  ∀ n, (∃ rs ∈ params, .var n ∈ rs) ∨ (.var n ∈ ret) →
    ∃ rs, (rs ∈ params ∨ rs = ret) ∧ .var n ∈ rs ∧ varCount rs = 1

/- ========================================================================== -/
/-! ## Sole-splat necessity

Without the sole-splat condition, two distinct valid solutions exist for
the same constraint, demonstrating that inference is non-deterministic.

Counterexample: `F <:ᵇ shape(...T₁, ...T₂)` where F = shape('x' => nat, 'y' => bool).

Solution A: T₁ = shape('x' => nat), T₂ = shape('y' => bool)
Solution B: T₁ = shape(),           T₂ = shape('x' => nat, 'y' => bool) -/
/- ========================================================================== -/

private def F_ex : Row := .mk [("x", .req .nat), ("y", .req .bool)] .bot
private def T₁a : Row := .mk [("x", .req .nat)] .bot
private def T₂a : Row := .mk [("y", .req .bool)] .bot
private def T₁b : Row := Row.empty
private def T₂b : Row := .mk [("x", .req .nat), ("y", .req .bool)] .bot

private theorem F_ex_nodupkeys : Row.NoDupKeys F_ex := by
  simp [Row.NoDupKeys, F_ex]

private theorem fieldSub_bot_union (t : BaseTy) :
    (.opt .bot) <:ᶠ (.opt (.union t .bot)) :=
  fun val h => by cases val <;> simp_all [fieldCheck]

private theorem fieldSub_nat_union_bot :
    (.req .nat) <:ᶠ (.req (.union .nat .bot)) :=
  fun val h => by cases val <;> simp_all [fieldCheck]

/-- Solution A is valid: F <:ʳ mergeRow T₁a T₂a. -/
private theorem check_field (fd : FieldDesc) : fd <:ᶠ mergeFieldDesc fd (.opt .bot) := by
  cases fd <;> intro val hval <;> cases val <;> simp_all [mergeFieldDesc, fieldCheck]

private theorem fieldSub_via_bot (fd : FieldDesc) :
    fd <:ᶠ mergeFieldDesc fd (.opt .bot) :=
  fun val h => (fieldCheck_merge_bot_right fd val).mpr h

private theorem fieldSub_merge_bot_l (fd : FieldDesc) :
    fd <:ᶠ mergeFieldDesc (.opt .bot) fd :=
  fun val h => (fieldCheck_merge_bot_left fd val).mpr h

theorem solution_a : F_ex <:ʳ mergeRow T₁a T₂a := by
  refine ⟨F_ex_nodupkeys, mergeRow_nodupKeys .., bot_sub _, ?_⟩
  intro k; rw [proj_mergeRow]
  by_cases hx : k = "x"
  · subst hx; exact fieldSub_via_bot _
  · by_cases hy : k = "y"
    · subst hy; exact fieldSub_merge_bot_l _
    · -- All projections reduce to .opt .bot
      have hF : Row.proj F_ex k = .opt .bot := by
        simp [Row.proj, F_ex, Row.fields, Row.unknown, lookup_cons_ne' _ _ hx, lookup_cons_ne' _ _ hy]
      have hT₁ : Row.proj T₁a k = .opt .bot := by
        simp [Row.proj, T₁a, Row.fields, Row.unknown, lookup_cons_ne' _ _ hx]
      have hT₂ : Row.proj T₂a k = .opt .bot := by
        simp [Row.proj, T₂a, Row.fields, Row.unknown, lookup_cons_ne' _ _ hy]
      rw [hF, hT₁, hT₂]; exact fieldSub_merge_bot_l _

/-- Solution B is valid: F <:ʳ mergeRow T₁b T₂b. -/
theorem solution_b : F_ex <:ʳ mergeRow T₁b T₂b := by
  refine ⟨F_ex_nodupkeys, mergeRow_nodupKeys .., bot_sub _, ?_⟩
  intro k; rw [proj_mergeRow]
  by_cases hx : k = "x"
  · subst hx; exact fieldSub_merge_bot_l _
  · by_cases hy : k = "y"
    · subst hy; exact fieldSub_merge_bot_l _
    · -- All projections reduce to .opt .bot
      have hF : Row.proj F_ex k = .opt .bot := by
        simp [Row.proj, F_ex, Row.fields, Row.unknown, lookup_cons_ne' _ _ hx, lookup_cons_ne' _ _ hy]
      have hT₁ : Row.proj T₁b k = .opt .bot := by
        simp [Row.proj, T₁b, Row.empty, Row.fields, Row.unknown]
      have hT₂ : Row.proj T₂b k = .opt .bot := by
        simp [Row.proj, T₂b, Row.fields, Row.unknown, lookup_cons_ne' _ _ hx, lookup_cons_ne' _ _ hy]
      rw [hF, hT₁, hT₂]; exact fieldSub_merge_bot_l _

/- ========================================================================== -/
/-! ### Decision procedure for concrete `row_sub`

The manual proofs above decompose `row_sub` by hand: provide Row.NoDupKeys,
unknown sub, then case-split on every key for per-field sub. This is
mechanical but verbose (~15 lines per goal).

`Shapes.Decide` provides a boolean checker `rowSubBool` that evaluates
all four `row_sub` conjuncts computationally, plus a soundness theorem
`rowSubBool_sound : rowSubBool r₁ r₂ = true → r₁ <:ʳ r₂`.

The proof pattern:
1. `native_decide` evaluates `rowSubBool r₁ r₂` to `true` (a `Bool`
   equality, which Lean can check by running the compiled function)
2. `rowSubBool_sound` lifts that `true` into a proof of `r₁ <:ʳ r₂`

This works for any concrete rows — no manual case analysis needed. -/
/- ========================================================================== -/

example : F_ex <:ʳ mergeRow T₁a T₂a := rowSubBool_sound (by native_decide)
example : F_ex <:ʳ mergeRow T₁b T₂b := rowSubBool_sound (by native_decide)

/-- The two solutions are distinct. -/
theorem solutions_distinct : T₁a ≠ T₁b := by
  simp [T₁a, T₁b, Row.empty]

/-- **Sole-splat necessity.** Without the sole-splat condition, two distinct
valid solutions exist for the same subtyping constraint. No deterministic
single-pass strategy can prefer one over the other. -/
theorem multi_splat_ambiguous :
    (F_ex <:ʳ mergeRow T₁a T₂a) ∧
    (F_ex <:ʳ mergeRow T₁b T₂b) ∧
    T₁a ≠ T₁b :=
  ⟨solution_a, solution_b, solutions_distinct⟩

/- ========================================================================== -/
/-! ## Multi-splat soundness

The multi-splat algorithm (§4.5.2) handles splats with multiple type
variables by replacing all but the rightmost with their current lower
bound, then solving the single-variable problem. This is sound because
`normalize` is monotone (`normalize_substAt_mono`): substituting a lower
bound gives the smallest normalized result. In supertype position, a
smaller supertype is the hardest case; in subtype position, a smaller
subtype is the easiest.

Together with `multi_splat_ambiguous`, this establishes:
- The multi-splat strategy is a **sound** approximation: it never
  accepts an invalid instantiation.
- It is **incomplete**: it may reject valid instantiations
  (`multi_splat_ambiguous` shows two distinct valid solutions; the
  algorithm can only find one).
- The sole-splat fragment is the **complete** case: when each TV has a
  sole-splat occurrence, its value is fully determined. -/

/-- **Multi-splat soundness (supertype position).** If the check passes
after replacing a TV at position `i` with its lower bound `L`, then it
passes for any instantiation `R ≥ L`. The list `rs` may contain other
TVs that have already been solved — the theorem applies regardless.

Proof: `L <:ʳ R` gives `normalize(rs[i:=L]) <:ʳ normalize(rs[i:=R])`
by monotonicity, and `source <:ʳ normalize(rs[i:=L])` by hypothesis. -/
/- ========================================================================== -/
theorem multi_splat_sound_super {rs : List Row} {i : Nat} {L source : Row}
    (hi : i < rs.length)
    (hnd : ∀ r ∈ rs, Row.NoDupKeys r)
    (hcheck : source <:ʳ normalize (rs.set i L))
    {R : Row} (hR : L <:ʳ R) :
    source <:ʳ normalize (rs.set i R) :=
  row_sub_trans hcheck (normalize_substAt_mono hi hnd hR)

/-- **Multi-splat soundness (subtype position).** Dual: if the check passes
with a value `V` at position `i`, then it passes for any smaller `L ≤ V`.
A smaller subtype is easier to fit below the target. -/
theorem multi_splat_sound_sub {rs : List Row} {i : Nat} {V target : Row}
    (hi : i < rs.length)
    (hnd : ∀ r ∈ rs, Row.NoDupKeys r)
    (hcheck : normalize (rs.set i V) <:ʳ target)
    {L : Row} (hL : L <:ʳ V) :
    normalize (rs.set i L) <:ʳ target :=
  row_sub_trans (normalize_substAt_mono hi hnd hL) hcheck
