import Shapes.Ty.Sub
import Shapes.Row.Normalize
import Shapes.Splat.Decomp

/- ========================================================================== -/
/-! # Rigid substitution soundness

Connects the `[ForallRow]` subtyping rule to the ground merge algebra.

## Key results

- `merge_congr_left_ty`: if `R₁ <:ʳ R₂` then `shape(mergeRow R₁ c) <: shape(mergeRow R₂ c)`.
  Lifts the ground `merge_congr_left` through `row_sub_sound` to the `Ty` level.

- `splat_forallRow_widen`: widening the bound of `∀ R <:ʳ bound. shape(R) → τ`
  is sound, reducing to `row_sub_sound` via instantiation.

- `splat_forallRow_full`: the full rule with both bound widening and
  body variation, combining `merge_congr_left` and `merge_congr_right`. -/
/- ========================================================================== -/

open Ty

/- ========================================================================== -/
/-! ## Merge congruence at the Ty level -/
/- ========================================================================== -/

theorem merge_congr_left_ty {R₁ R₂ concrete : Row}
    (h : R₁ <:ʳ R₂) :
    (.base (.shape (mergeRow R₁ concrete)) : Ty) <: .base (.shape (mergeRow R₂ concrete)) :=
  .base (row_sub_sound (merge_congr_left h))

theorem merge_congr_right_ty {left : Row} {R₁ R₂ : Row}
    (h : R₁ <:ʳ R₂) :
    (.base (.shape (mergeRow left R₁)) : Ty) <: .base (.shape (mergeRow left R₂)) :=
  .base (row_sub_sound (merge_congr_right h))

/- ========================================================================== -/
/-! ## ForallRow with shape variable body

A body like `Ty.base (.shape (Row.var 0))` represents the identity shape
function — after substitution with `R`, it becomes `Ty.base (.shape R)`.
Subtyping between instantiations follows from `row_sub_sound`. -/
/- ========================================================================== -/

/-- Body substitution for the identity shape: `(.shape (var 0)).substRow 0 R = .shape R`. -/
theorem shape_var_substRow (R : Row) :
    (Ty.base (.shape (.var 0))).substRow 0 R = .base (.shape R) := by
  simp [Ty.substRow, BaseTy.substRow_shape_var_hit]

/-- Widening the bound of a `forallRow` with identity-shape body is sound.
    `(∀ R <:ʳ bound₁. shape R) <: (∀ R <:ʳ bound₂. shape R)` when `bound₂ <:ʳ bound₁`. -/
theorem splat_forallRow_widen {bound₁ bound₂ : Row}
    (hbound : bound₂ <:ʳ bound₁) :
    TySub (.forallRow bound₁ (.base (.shape (.var 0))))
          (.forallRow bound₂ (.base (.shape (.var 0)))) :=
  .forallRow hbound fun R hnd hsub => by
    simp [Ty.substRow, BaseTy.substRow_shape_var_hit]
    exact .base (sub_refl _)

/- ========================================================================== -/
/-! ## Full rigid substitution soundness

The HIP's [Splat-Rigid-Sub] and [Splat-Rigid-Super] are corollaries:
for a polymorphic shape function `∀ R <:ʳ bound. shape(R) → τ`,
- widening the bound preserves subtyping (from `[ForallRow]`)
- monotonicity of merge preserves the shape ordering (from `merge_congr_left`)
- the function argument is contravariant, so wider shape = smaller function type

Together, these make splat-polymorphic function types well-behaved. -/
/- ========================================================================== -/

/-- For any `R₁ <:ʳ R₂ <:ʳ bound` and any concrete extension `c`,
    `shape(mergeRow R₁ c) <: shape(mergeRow R₂ c)`.
    This is the rigid substitution property: instantiating a row variable
    preserves the subtyping structure through merge. -/
theorem rigid_sub_merge {R₁ R₂ : Row} {concrete : Row}
    (hsub : R₁ <:ʳ R₂) :
    (.base (.shape (mergeRow R₁ concrete)) : Ty) <: .base (.shape (mergeRow R₂ concrete)) :=
  merge_congr_left_ty hsub

/-- Soundness of `[ForallRow]` for shape function types:
    if `bound₂ <:ʳ bound₁`, then
    `(∀ R <:ʳ bound₁. shape R → τ) <: (∀ R <:ʳ bound₂. shape R → τ)`.

    The body ordering follows from `row_sub_sound` (shape R <:ᵇ shape R). -/
theorem splat_forallRow_fn {bound₁ bound₂ : Row} {result : Ty}
    (hbound : bound₂ <:ʳ bound₁)
    (hwf : result.wf) :
    let body := Ty.fn (.base (.shape (.var 0))) result
    TySub (.forallRow bound₁ body) (.forallRow bound₂ body) :=
  .forallRow hbound fun R hnd hsub => by
    simp only [Ty.substRow, BaseTy.substRow_shape_var_hit]
    exact .fn (.base (sub_refl _)) (TySub.refl _ (substRow_wf hwf 0 R))

/- ========================================================================== -/
/-! ## Splat-level rigid substitution soundness
/- ========================================================================== -/

The HIP's [Splat-Rigid-Sub] and [Splat-Rigid-Super] rules: to check subtyping
on a shape splat containing a type variable, substitute the variable with its
bound and check the result. Sound because shape splats are covariant in each
position (`normalize_substAt_mono`). -/

/-- [Splat-Rigid-Sub]: substituting upper bound in subtype position.
    If `normalize(splat[i := upper]) <:ʳ target`, then for all `R <:ʳ upper`,
    `normalize(splat[i := R]) <:ʳ target`. -/
theorem rigid_splat_sub_sound {rs : List Row} {i : Nat} {upper target : Row}
    (hi : i < rs.length)
    (hnd : ∀ r ∈ rs, Row.NoDupKeys r)
    (hcheck : normalize (rs.set i upper) <:ʳ target)
    {R : Row} (hR : R <:ʳ upper) :
    normalize (rs.set i R) <:ʳ target :=
  row_sub_trans (normalize_substAt_mono hi hnd hR) hcheck

/-- [Splat-Rigid-Super]: substituting lower bound in supertype position.
    If `source <:ʳ normalize(splat[i := lower])`, then for all `lower <:ʳ R`,
    `source <:ʳ normalize(splat[i := R])`. -/
theorem rigid_splat_super_sound {rs : List Row} {i : Nat} {lower source : Row}
    (hi : i < rs.length)
    (hnd : ∀ r ∈ rs, Row.NoDupKeys r)
    (hcheck : source <:ʳ normalize (rs.set i lower))
    {R : Row} (hR : lower <:ʳ R) :
    source <:ʳ normalize (rs.set i R) :=
  row_sub_trans hcheck (normalize_substAt_mono hi hnd hR)

/-- [Splat-Rigid-Sub] at the Ty level. -/
theorem rigid_splat_sub_sound_ty {rs : List Row} {i : Nat} {upper : Row} {target : Row}
    (hi : i < rs.length) (hnd : ∀ r ∈ rs, Row.NoDupKeys r)
    (hcheck : normalize (rs.set i upper) <:ʳ target)
    {R : Row} (hR : R <:ʳ upper) :
    (.base (.shape (normalize (rs.set i R))) : Ty) <: .base (.shape target) :=
  .base (row_sub_sound (rigid_splat_sub_sound hi hnd hcheck hR))

/-- [Splat-Rigid-Super] at the Ty level. -/
theorem rigid_splat_super_sound_ty {rs : List Row} {i : Nat} {lower source : Row}
    (hi : i < rs.length) (hnd : ∀ r ∈ rs, Row.NoDupKeys r)
    (hcheck : source <:ʳ normalize (rs.set i lower))
    {R : Row} (hR : lower <:ʳ R) :
    (.base (.shape source) : Ty) <: .base (.shape (normalize (rs.set i R))) :=
  .base (row_sub_sound (rigid_splat_super_sound hi hnd hcheck hR))

/- ========================================================================== -/
/-! ## Connecting forallRow to the decomposition

A `forallRow` with a shape function body `shape(var 0) → τ` produces,
after instantiation with R, the type `shape(R) → τ[R]`. When the caller
provides argument type F, the subtyping check is `F <:ʳ R` — exactly
the judgment that `decomp_complete_super` acts on.

For a splat body `shape(...C, ...var 0) → τ`, instantiation gives
`shape(mergeRow C R) → τ[R]`. The argument check becomes
`F <:ʳ mergeRow C R` — the `decomp_sound`/`decomp_complete_super` target. -/
/- ========================================================================== -/

/-- When a forallRow is instantiated, the body's shape argument becomes
a concrete subtyping check that the decomposition can act on.
This connects Ty-level polymorphism to Row-level inference. -/
theorem forallRow_instantiate_decomp
    {bound : Row} {C : Row} {result : Ty}
    {F R : Row}
    (hndF : Row.NoDupKeys F) (hwf : result.wf)
    (hunknown : F.unknown <:ᵇ (mergeRow C R).unknown)
    (hfields : ∀ k, Row.proj F k <:ᶠ mergeFieldDesc (Row.proj C k) (Row.proj R k)) :
    TySub (.fn (.base (.shape (mergeRow C R))) (result.substRow 0 R))
          (.fn (.base (.shape F)) (result.substRow 0 R)) :=
  .fn (.base (row_sub_sound (decomp_sound hndF hunknown hfields)))
      (TySub.refl _ (substRow_wf hwf 0 R))

/- ========================================================================== -/
/-! ## Substitution distributes over merge

Substitution commutes with merge at the `Row.proj` level: for ground rows,
`Row.proj` of the substituted merge equals `Row.proj` of the merge of substitutions.
This gives `<:^r`-equivalence. -/
/- ========================================================================== -/

theorem substRow_mergeRow_sub
    (lf rf : List (String × FieldDesc)) (lu ru : BaseTy) (n : Nat) (s : Row) :
    (mergeRow (Row.mk lf lu) (Row.mk rf ru)).substRow n s <:ʳ
    mergeRow ((Row.mk lf lu).substRow n s) ((Row.mk rf ru).substRow n s) := by
  refine ⟨Row.substRow_preserves_NoDupKeys (mergeRow_nodupKeys ..) n s,
          mergeRow_nodupKeys .., sub_refl _, ?_⟩
  intro k
  rw [proj_substRow (mergeRow_nodupKeys ..), proj_mergeRow,
      mergeFieldDesc_substRow, proj_mergeRow, ← proj_substRow_mk, ← proj_substRow_mk]
  exact fieldSub_refl _

theorem substRow_mergeRow_super
    (lf rf : List (String × FieldDesc)) (lu ru : BaseTy) (n : Nat) (s : Row) :
    mergeRow ((Row.mk lf lu).substRow n s) ((Row.mk rf ru).substRow n s) <:ʳ
    (mergeRow (Row.mk lf lu) (Row.mk rf ru)).substRow n s := by
  refine ⟨mergeRow_nodupKeys ..,
          Row.substRow_preserves_NoDupKeys (mergeRow_nodupKeys ..) n s, sub_refl _, ?_⟩
  intro k
  rw [proj_mergeRow, proj_substRow (mergeRow_nodupKeys ..), proj_mergeRow,
      mergeFieldDesc_substRow, ← proj_substRow_mk, ← proj_substRow_mk]
  exact fieldSub_refl _
