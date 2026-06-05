import Shapes.Row.Sub
import Shapes.Row.Decide

/- ========================================================================== -/
/-! # Fresh-variable decomposition for splat inference

When a splat subtyping check involves a type variable T, the inference
algorithm decomposes it into per-field constraints using fresh variables.
The requiredness of each fresh variable depends on which side T is on:

- **Super-right** (T rightmost in supertype): `Opt` fresh — T might
  contribute, making the supertype wider. Unconditionally complete.
- **Sub-right** (T rightmost in subtype): `Req` fresh — T must dominate
  whatever concrete is to its left. Unconditionally complete.

This duality is the central result. The `Opt`/`Req` choice is dual because:
- `Opt` weakens T's contribution (wider supertype → easier to be below)
- `Req` strengthens T's contribution (narrower subtype → easier to be above)

Combined with `absent` in T's bound to exclude fields T shouldn't have,
both directions are unconditionally complete with no compatibility condition. -/
/- ========================================================================== -/

/- ========================================================================== -/
/-! ## Soundness -/
/- ========================================================================== -/

/-- Soundness (super direction): per-field checks ⇒ `F <:ʳ mergeRow C L`. -/
theorem decomp_sound {F C L : Row}
    (hndF : Row.NoDupKeys F)
    (hunknown : F.unknown <:ᵇ (mergeRow C L).unknown)
    (hfields : ∀ k, Row.proj F k <:ᶠ mergeFieldDesc (Row.proj C k) (Row.proj L k)) :
    F <:ʳ mergeRow C L :=
  ⟨hndF, mergeRow_nodupKeys C L, hunknown,
   fun k => by rw [proj_mergeRow]; exact hfields k⟩

/-- Soundness (sub direction): per-field checks ⇒ `mergeRow C L <:ʳ F`. -/
theorem decomp_sound_sub {F C L : Row}
    (hndF : Row.NoDupKeys F)
    (hunknown : (mergeRow C L).unknown <:ᵇ F.unknown)
    (hfields : ∀ k, mergeFieldDesc (Row.proj C k) (Row.proj L k) <:ᶠ Row.proj F k) :
    mergeRow C L <:ʳ F :=
  ⟨mergeRow_nodupKeys C L, hndF, hunknown,
   fun k => by rw [proj_mergeRow]; exact hfields k⟩

/-- Biconditional: row subtyping through merge ↔ per-field checks (when R is known). -/
theorem decomp_iff {F C R : Row} (hndF : Row.NoDupKeys F) :
    F <:ʳ mergeRow C R ↔
    (F.unknown <:ᵇ (mergeRow C R).unknown ∧
     ∀ k, Row.proj F k <:ᶠ mergeFieldDesc (Row.proj C k) (Row.proj R k)) :=
  ⟨fun h => ⟨h.2.2.1, fun k => by have := h.2.2.2 k; rw [proj_mergeRow] at this; exact this⟩,
   fun ⟨hu, hf⟩ => decomp_sound hndF hu hf⟩

/- ========================================================================== -/
/-! ## Super-right completeness: Opt fresh (unconditional)
/- ========================================================================== -/

When T is rightmost in the supertype, it might override the concrete part's
fields. Use `Opt` fresh variables: T's contribution is optional, making the
supertype wider. This is the permissive direction — easier to be below. -/

/-- Replacing any field descriptor with `Opt fd.ty` in the right position of
merge makes the result wider (a larger supertype). -/
theorem mergeFieldDesc_weaken_right (C fd : FieldDesc) :
    mergeFieldDesc C fd <:ᶠ mergeFieldDesc C (.opt fd.ty) := by
  intro val hval
  cases C <;> cases fd <;> cases val <;>
    simp_all [mergeFieldDesc, fieldCheck, FieldDesc.ty]
  all_goals first
    | exact Or.inr ‹_›
    | exact ‹_›

/-- **Super-right completeness.** If `F <:ʳ mergeRow C R`, the Opt-weakened
per-field constraints hold unconditionally.

Example: `{x: int, y: bool} <: shape(...{x: int}, ...T)`
- At `x`: `Req int <:ᶠ mergeFieldDesc(Req int, Opt #f) = Req(int | #f)` ✓
- At `y`: T gets lower bound `{y: bool}` -/
theorem decomp_complete_super {F C R : Row}
    (h : F <:ʳ mergeRow C R) (k : String) :
    Row.proj F k <:ᶠ mergeFieldDesc (Row.proj C k) (.opt (Row.proj R k).ty) := by
  have hfk := h.2.2.2 k
  rw [proj_mergeRow] at hfk
  exact fun val hv => mergeFieldDesc_weaken_right (Row.proj C k) (Row.proj R k) val (hfk val hv)


/- ========================================================================== -/
/-! ## Sub-right completeness: Req fresh (unconditional)

When T is rightmost in the subtype, it dominates whatever concrete is to
its left. Use `Req` fresh variables: since `mergeFieldDesc(_, Req t) = Req t`,
the concrete part is irrelevant. No compatibility condition needed.

Combined with `absent` in T's bound to exclude concrete fields, T only gets
the fields it actually needs. Fields with `absent` produce no fresh variable;
the concrete part stands alone at those fields. -/
/- ========================================================================== -/

/-- `Req fd.ty` is below any merge with `fd` on the right: the underlying
type `t` appears in any union the merge produces, so `Or.inr` suffices. -/
private theorem req_ty_sub_merge (C fd : FieldDesc) :
    (.req fd.ty) <:ᶠ mergeFieldDesc C fd := by
  cases C <;> cases fd <;> intro val hval <;>
    cases val <;> simp_all [mergeFieldDesc, fieldCheck, FieldDesc.ty]
  all_goals exact Or.inr ‹_›

/-- **Sub-right completeness.** If `mergeRow C R <:ʳ F`, the Req-weakened
per-field constraints hold unconditionally. Since `mergeFieldDesc(_, Req t) = Req t`,
the concrete part to the left is irrelevant.

Example: `shape(...{x: int}, ...T) <: {x: int, z: string}`
- At `x`: `mergeFieldDesc(Req int, Req #f) = Req #f <:ᶠ Req int` → `#f <: int`
  (if T has `absent 'x'`, no fresh; concrete `x: int` stands alone)
- At `z`: T gets upper bound `{z: string}` -/
theorem decomp_complete_sub {F C R : Row}
    (h : mergeRow C R <:ʳ F)
    (k : String) :
    mergeFieldDesc (Row.proj C k) (.req (Row.proj R k).ty) <:ᶠ Row.proj F k := by
  have hfk := h.2.2.2 k
  rw [proj_mergeRow] at hfk
  have heq : mergeFieldDesc (Row.proj C k) (.req (Row.proj R k).ty) = .req (Row.proj R k).ty := by
    cases Row.proj C k <;> simp [mergeFieldDesc]
  rw [heq]
  exact fieldSub_trans (req_ty_sub_merge (Row.proj C k) (Row.proj R k)) hfk

/- ========================================================================== -/
/-! ## The 3×3 matrix

After normalization, each side of a splat subtyping check is in one of
three positions: concrete (no TV), TV left (concrete definitive), or
TV right (TV dominant).

Fresh variables are introduced only when T is rightmost (dominant):
- **Super TV-right**: `Opt` fresh — T might contribute, supertype widens
- **Sub TV-right**: `Req` fresh — T dominates, concrete to left irrelevant
- **T not rightmost**: no fresh variable decomposition (concrete definitive;
  T gets direct bound constraints from non-overlapping fields)

|               | Super: concrete | Super: TV left | Super: TV right |
|---------------|-----------------|----------------|-----------------|
| Sub: concrete | direct          | direct         | **Opt** fresh   |
| Sub: TV left  | direct          | direct         | **Opt** fresh   |
| Sub: TV right | **Req** fresh   | **Req** fresh  | Req + Opt fresh |
-/
/- ========================================================================== -/

/-- Normalized splat position: concrete (no TV), TV leftmost (concrete
definitive), or TV rightmost (TV dominant). The `Row` is the concrete part. -/
inductive SplatPos where
  | concrete : Row → SplatPos
  | tvLeft   : Row → SplatPos
  | tvRight  : Row → SplatPos

/-- The effective row for a splat position, given a value `R` for the TV.
- `concrete C`: just `C`
- `tvLeft C`:  `mergeRow R C` (TV left, concrete right = definitive)
- `tvRight C`: `mergeRow C R` (concrete left, TV right = dominant) -/
def SplatPos.eval (sp : SplatPos) (R : Row) : Row :=
  match sp with
  | .concrete C => C
  | .tvLeft C   => mergeRow R C
  | .tvRight C  => mergeRow C R

/-- Soundness for any combination of splat positions. -/
theorem splat_matrix_sound {sp_sub sp_super : SplatPos} {R : Row}
    (hnd_sub : Row.NoDupKeys (sp_sub.eval R))
    (hnd_super : Row.NoDupKeys (sp_super.eval R))
    (hunknown : (sp_sub.eval R).unknown <:ᵇ (sp_super.eval R).unknown)
    (hfields : ∀ k, Row.proj (sp_sub.eval R) k <:ᶠ Row.proj (sp_super.eval R) k) :
    sp_sub.eval R <:ʳ sp_super.eval R :=
  ⟨hnd_sub, hnd_super, hunknown, hfields⟩

/-- Super-right completeness: Opt fresh, unconditional. -/
theorem splat_matrix_complete_super_right
    {sp_sub : SplatPos} {C_super : Row} {R : Row}
    (h : sp_sub.eval R <:ʳ (SplatPos.tvRight C_super).eval R) (k : String) :
    Row.proj (sp_sub.eval R) k <:ᶠ
      mergeFieldDesc (Row.proj C_super k) (.opt (Row.proj R k).ty) := by
  simp [SplatPos.eval] at h
  exact decomp_complete_super h k

/-- Sub-right completeness: Req fresh, unconditional. -/
theorem splat_matrix_complete_sub_right
    {C_sub : Row} {sp_super : SplatPos} {R : Row}
    (h : (SplatPos.tvRight C_sub).eval R <:ʳ sp_super.eval R) (k : String) :
    mergeFieldDesc (Row.proj C_sub k) (.req (Row.proj R k).ty) <:ᶠ
      Row.proj (sp_super.eval R) k := by
  simp [SplatPos.eval] at h
  exact decomp_complete_sub h k
