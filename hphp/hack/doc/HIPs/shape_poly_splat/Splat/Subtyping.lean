import Splat.Syntax

/-!
# Declarative subtyping relations and the `mergeField` algebra

The declarative subtyping relations `SubBase`/`SubField`/`SubField_row` (mutual), their
reflexivity and transitivity, monotonicity of `mergeField`/`mergeSimple`, and the
union/`mergeField` algebra used later by the corner reduction.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## Declarative subtyping relations                                                             -/
/- ---------------------------------------------------------------------------------------------- -/

/-
  `SubBase`, `SubField`, `SubField_row` are mutually recursive:
  * `SubBase.shape_sub` builds a shape subtyping out of a `SubField_row`;
  * `SubField` carries a `SubBase` between the two field types;
  * `SubField_row` lifts `SubField` pointwise over every label.

  These are *declarative* relations: each is an inductive type whose constructors *are* the
  inference rules of subtyping. They say *what* subtyping is (the laws it obeys: `refl`,
  `trans`, `top`/`bot`, the `union` rules, `shape` covariance) not *how* to check it. A value
  of type `SubBase t u` is exactly a finite proof (a derivation) assembled from those rules, so
  any term we can build is a valid one by construction (proofs-as-data). The type for a false
  statement such as `SubBase (.prim .nat) (.prim .bool)` is perfectly writable but *uninhabited*:
  no constructor sequence produces it.

  So an inhabited `SubBase t u` means only that the subtyping is *derivable* from these rules.
  That this matches *semantic* truth (every value of type `t` is usable where `u` is expected) is
  the separate soundness theorem `SubBase t u → ⟦t⟧ ⊆ ⟦u⟧`. The converse (completeness) is
  deliberately not full: some semantically-true subtypings (e.g. shape/union distributivity) are not
  derivable.

  The executable checker is recovered separately as a `Decidable (SubBase t u)` instance, giving
  `decide : Bool` with `decide = true ↔ SubBase t u` — a meaning and a checker, provably equal,
  rather than conflated.
-/
mutual

  /-- Subtyping on base types -/
  inductive SubBase : Base → Base → Prop where
    | refl        : SubBase t t
    | top         : SubBase t .top
    | bot         : SubBase .bot t
    | union_left  : SubBase t (.union t u)
    | union_right : SubBase u (.union t u)
    | union_comm  : SubBase (.union t u) (.union u t)
    | union_sub   : SubBase t v → SubBase u v → SubBase (.union t u) v
    | trans       : SubBase t u → SubBase u v → SubBase t v
    | shape_sub   : SubField_row r p →
                      SubBase (.shape (.simple r.known r.unknown))
                              (.shape (.simple p.known p.unknown))

  /-- Subfield relation.  A sub field must be at least as required (`Req ≤ Opt`) and its type
      a subtype.  There is deliberately no `opt_req` rule: an optional field can never refine
      to a required one. -/
  inductive SubField : FieldDesc → FieldDesc → Prop where
    | req_req : SubBase t u → SubField (.req t) (.req u)
    | opt_opt : SubBase t u → SubField (.opt t) (.opt u)
    | req_opt : SubBase t u → SubField (.req t) (.opt u)

  /-- Pointwise subrow on simple rows: every label's projection is a subfield.  Quantifying
      over *all* labels also constrains the `unknown` bound, since any absent label projects to it. -/
  inductive SubField_row : SimpleRow → SimpleRow → Prop where
    | mk : (∀ l, SubField (proj r l) (proj p l)) → SubField_row r p
end

/-- Extract the pointwise condition witnessed by a `SubField_row`. -/
def SubField_row.at (h : SubField_row r p) (l : Label) : SubField (proj r l) (proj p l) :=
  match h with | .mk f => f l

/- ---------------------------------------------------------------------------------------------- -/
/- ## Reflexivity                                                                                 -/
/- ---------------------------------------------------------------------------------------------- -/

-- `SubBase` reflexivity is the constructor `SubBase.refl`; the field/row versions lift it.

/-- Reflexivity of `SubField`. -/
theorem subField_refl (d : FieldDesc) : SubField d d :=
  -- pick the matching constructor for each requiredness, with `SubBase.refl` on the type
  match d with
  | .req _ => .req_req .refl
  | .opt _ => .opt_opt .refl

/-- Reflexivity of `SubField_row`. -/
theorem SubField_row.refl (r : SimpleRow) : SubField_row r r :=
  .mk fun l => subField_refl (proj r l)

/- ---------------------------------------------------------------------------------------------- -/
/- ## Transitivity                                                                                -/
/- ---------------------------------------------------------------------------------------------- -/

-- `SubBase` transitivity is the constructor `SubBase.trans`.

/-- Transitivity of `SubField`. -/
theorem subField_trans {d1 d2 d3 : FieldDesc}
    (h1 : SubField d1 d2) (h2 : SubField d2 d3) : SubField d1 d3 := by
  -- Case on both proofs.  The shared middle descriptor `d2` forces its requiredness to agree,
  -- so only four of the nine pairings are inhabited; each composes the carried base
  -- subtypings with `SubBase.trans`.  (Lean rules the other five out by the index mismatch.)
  match h1, h2 with
  | .req_req ht1, .req_req ht2 => exact .req_req (.trans ht1 ht2)
  | .req_req ht1, .req_opt ht2 => exact .req_opt (.trans ht1 ht2)
  | .opt_opt ht1, .opt_opt ht2 => exact .opt_opt (.trans ht1 ht2)
  | .req_opt ht1, .opt_opt ht2 => exact .req_opt (.trans ht1 ht2)

/-- Transitivity of `SubField_row`. -/
theorem SubField_row.trans {r r' r'' : SimpleRow}
    (h1 : SubField_row r r') (h2 : SubField_row r' r'') : SubField_row r r'' :=
  -- pointwise: chain the two subfields at each label
  .mk fun l => subField_trans (h1.at l) (h2.at l)

/- ---------------------------------------------------------------------------------------------- -/
/- ## Monotonicity of `mergeField`                                                                -/
/- ---------------------------------------------------------------------------------------------- -/

/-- `union` is monotone in both arguments: given `asub <: asup` and `bsub <: bsup` we have
 `(asub|bsub) <: (asup|bsup)` -/
theorem subBase_union_mono {asub asup bsub bsup : Base}
    (ha : SubBase asub asup) (hb : SubBase bsub bsup) : SubBase (.union asub bsub) (.union asup bsup) :=
  .union_sub -- If asub <: (asup|bsup) and bsub <: (asup|bsup) then (asub|bsub) <: (asup|bsup)
    (.trans ha .union_left)  -- asub <: asup <: (asup|bsup)
    (.trans hb .union_right) -- bsub <: bsup <: (asup|bsup)

/-- `mergeField` is monotone in both arguments w.r.t. `SubField` — the per-label engine that
    makes the whole splat/corner machinery monotone in each type parameter's field value. -/
theorem mergeField_mono {fdl_sub fdl_sup fdr_sub fdr_sup : FieldDesc}
    (hl : SubField fdl_sub fdl_sup) (hr : SubField fdr_sub fdr_sup) :
    SubField (mergeField fdl_sub fdr_sub) (mergeField fdl_sup fdr_sup) := by
  -- Case on the RIGHT pair first: `mergeField` is right-biased (a `Req` on the right wins).
  match fdr_sub, fdr_sup, hr with
  | .req t, .req t', .req_req ht =>
    -- right is `Req`: `mergeField _ (.req _) = .req _` discards its left argument (rightmost-
    -- wins).  Both goal merges have a `Req` right field, so each collapses to its right field
    -- and the left subtyping `hl` is never used.
    show SubField (.req t) (.req t')
    exact .req_req ht

  | .opt t, .opt t', .opt_opt ht =>
    -- right is `Opt`: the merge keeps the LEFT requiredness and unions the types;
    -- case on the left to read that requiredness off, then union-monotonicity finishes
    match fdl_sub, fdl_sup, hl with
    | .req u, .req u', .req_req hu =>
      show SubField (.req (.union u t)) (.req (.union u' t'))
      exact .req_req (subBase_union_mono hu ht)
    | .req u, .opt u', .req_opt hu =>
      show SubField (.req (.union u t)) (.opt (.union u' t'))
      exact .req_opt (subBase_union_mono hu ht)
    | .opt u, .opt u', .opt_opt hu =>
      show SubField (.opt (.union u t)) (.opt (.union u' t'))
      exact .opt_opt (subBase_union_mono hu ht)

  | .req t, .opt t', .req_opt ht =>
    -- right goes `Req t ↦ Opt t'`: the LEFT merge (right `Req t`) is masked to `.req t`, while
    -- the RIGHT merge (right `Opt t'`) unions in `t'`.  In every left-case the result type only
    -- grows, via `ht : SubBase t t'` composed with `union_right`.
    match fdl_sub, fdl_sup, hl with
    | .req u, .req u', .req_req hu =>
      show SubField (.req t) (.req (.union u' t'))
      exact .req_req (.trans ht .union_right)
    | .req u, .opt u', .req_opt hu =>
      show SubField (.req t) (.opt (.union u' t'))
      exact .req_opt (.trans ht .union_right)
    | .opt u, .opt u', .opt_opt hu =>
      show SubField (.req t) (.opt (.union u' t'))
      exact .req_opt (.trans ht .union_right)

/-- `mergeSimple` is monotone w.r.t. `SubField_row` — the whole-row corollary of
    `mergeField_mono`.  Because `proj` distributes over `mergeSimple` (`proj_mergeSimple`),
    each label reduces to a `mergeField` of the two sides' projections, where `mergeField_mono`
    applies pointwise. -/
theorem mergeSimple_mono {lsub lsup rsub rsup : SimpleRow}
    (hl : SubField_row lsub lsup) (hr : SubField_row rsub rsup) :
    SubField_row (mergeSimple lsub rsub) (mergeSimple lsup rsup) := by
  -- it suffices to compare projections at every label
  apply SubField_row.mk
  intro lbl
  -- push `proj` through both merges, turning each into a `mergeField` of projections
  rw [proj_mergeSimple, proj_mergeSimple]
  -- then merge-monotonicity finishes, fed the left and right pointwise subfields
  exact mergeField_mono (hl.at lbl) (hr.at lbl)

end Splat
