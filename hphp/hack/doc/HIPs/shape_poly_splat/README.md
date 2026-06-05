# shapes

Lean 4 formalization of a type system for rightmost-wins record combination with semantic subtyping.

## Metatheoretic contributions

**1. Semantic subtyping for ground types with required/optional field descriptors.** Types denote predicates on values. Subtyping is inclusion: `τ₁ <: τ₂` iff every value inhabiting `τ₁` also inhabits `τ₂`. Reflexivity is the identity function, transitivity is composition. A characterization theorem (`shape_denote_iff`) restates the list-recursive shape denotation as a uniform per-key condition, enabling compositional reasoning about combination.

**2. Rightmost-wins combination forms a monoid up to semantic equivalence.** The empty row is a two-sided identity, combination is associative, and combination is monotone with respect to subtyping in both arguments. All three properties hold up to bidirectional subtyping, not syntactic equality — the identity law is unstatable as an equation because combining a required field with an absent field produces a union type that is denotationally but not syntactically equal to the original.

**3. Structural row subtyping is sound and complete for inhabited shapes.** A structural approximation decomposes shape subtyping into per-field conditions. Sound unconditionally. Complete when the source shape is inhabited. The gap for uninhabited shapes is inherent (mechanized counterexample).

**4. The decomposition theorem is sound and complete in both directions, unconditionally.** When a subtyping check involves a type variable in a combination, the whole-row judgment decomposes into independent per-field constraints. The two directions are dual: `Opt` fresh variables for super-right (the type variable might contribute, widening the supertype), `Req` fresh variables for sub-right (the type variable dominates whatever concrete is to its left). Both are unconditionally complete.

**5. Inference is complete for sole-splat, sound for multi-splat, and inherently incomplete for multi-splat.** When every type variable has at least one parameter where it is the sole variable in the combination, inference is complete. When a combination contains multiple type variables, replacing all but the rightmost with their lower bound is sound. The incompleteness is inherent: a mechanized counterexample shows two distinct valid solutions for the same constraint.

**6. Polymorphic subtyping is reflexive, transitive, and sound.** Rule-based subtyping for functions and bounded row quantification. Reflexivity and transitivity by well-founded recursion on type size. Soundness connects rule-based to semantic subtyping.

**7. Type safety for the ground expression language.** Record construction, field access, and a merge primitive with big-step evaluation. Every well-typed closed expression evaluates to a value that inhabits its type (progress + preservation).

## Project structure

Organized hierarchically by the type operated on, with parallel structure across concepts (`BaseTy/Sub`, `Row/Sub`, `Ty/Sub`).

```
BaseTy                  -- Val, BaseTy, FieldDesc, Row (mutual inductive)
                           proj, NoDupKeys, wf, isEmpty
BaseTy/Denote           -- denote (semantic denotation)
                           fieldCheck, shape_denote_iff (per-key characterization)
BaseTy/Sub              -- sub (<:ᵇ), fieldSub (<:ᶠ), subtyping properties
BaseTy/Merge            -- mergeFieldDesc, mergeRow
                           identity, monotonicity, associativity, proj_mergeRow
BaseTy/Inhabited        -- witness construction, isEmpty_complete

Row/Sub                 -- row_sub (<:ʳ), soundness, completeness boundary
Row/Subst               -- de Bruijn row substitution, key preservation
Row/Normalize           -- normalize (n-ary merge), homomorphism, monotonicity
Row/Decide              -- boolean sub checker (subGo, rowSubBool), soundness

Ty                      -- Ty, substRow, wf, size
Ty/Sub                  -- Ty.sub (semantic), TySub (<:) (rule-based)
                           reflexivity, transitivity, soundness

Splat/Decomp            -- decomposition theorem (Opt/Req duality)
                           3×3 position matrix
Splat/Rigid             -- rigid substitution, forallRow instantiation
Splat/Infer             -- sole-splat, multi-splat soundness
                           multi-splat ambiguity counterexample

Expr                    -- expression language, mergeEntries (runtime merge)
Expr/Eval               -- big-step evaluation
Expr/Typing             -- HasType judgment (merge rule is the key novelty)
Expr/Soundness          -- type_sound, progress, type_safety
```

## Build

```
lake build                          # build everything
lake build Shapes.BaseTy.Merge      # build a single module
lake clean                          # clean build artifacts
```

Lean toolchain: `leanprover/lean4:v4.29.1`. No Mathlib dependency.
