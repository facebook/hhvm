import Shapes.Ty

/- ========================================================================== -/
/-! # Polymorphic subtyping

Semantic subtyping (`Ty.sub`) and rule-based subtyping (`TySub`).
Soundness: every rule-derivable judgment holds in the semantic model. -/
/- ========================================================================== -/

namespace Ty

/- ========================================================================== -/
/-! ## Semantic subtyping -/
/- ========================================================================== -/

def sub : Ty → Ty → Prop
  | .base a, .base b => a <:ᵇ b
  | .fn a₁ b₁, .fn a₂ b₂ => sub a₂ a₁ ∧ sub b₁ b₂
  | .forallRow b₁ bd₁, .forallRow b₂ bd₂ =>
      b₂ <:ʳ b₁ ∧ ∀ R, Row.NoDupKeys R → R <:ʳ b₂ → sub (bd₁.substRow 0 R) (bd₂.substRow 0 R)
  | _, _ => False
termination_by τ₁ τ₂ => τ₁.size + τ₂.size
decreasing_by all_goals (simp +arith [size, substRow_size])

infixl:50 " <:ₛ " => Ty.sub

theorem sub_refl : (τ : Ty) → τ.wf → τ <:ₛ τ
  | .base t, _ => by unfold Ty.sub; exact _root_.sub_refl t
  | .fn a b, ⟨ha, hb⟩ => by unfold Ty.sub; exact ⟨sub_refl a ha, sub_refl b hb⟩
  | .forallRow bound body, ⟨hnd, hwf⟩ => by
      unfold Ty.sub; exact ⟨row_sub_refl hnd, fun R _ _ =>
        sub_refl (body.substRow 0 R) (substRow_wf hwf 0 R)⟩
termination_by τ => τ.size
decreasing_by all_goals (simp +arith [size, substRow_size])

theorem sub_trans {τ₁ τ₂ τ₃ : Ty}
    (h₁₂ : τ₁ <:ₛ τ₂) (h₂₃ : τ₂ <:ₛ τ₃) : τ₁ <:ₛ τ₃ := by
  match τ₁, τ₂, τ₃ with
  | .base _, .base _, .base _ =>
    unfold Ty.sub at h₁₂ h₂₃ ⊢; exact _root_.sub_trans h₁₂ h₂₃
  | .fn _ _, .fn _ _, .fn _ _ =>
    unfold Ty.sub at h₁₂ h₂₃ ⊢
    exact ⟨sub_trans h₂₃.1 h₁₂.1, sub_trans h₁₂.2 h₂₃.2⟩
  | .forallRow _ _, .forallRow _ _, .forallRow _ _ =>
    unfold Ty.sub at h₁₂ h₂₃ ⊢
    exact ⟨row_sub_trans h₂₃.1 h₁₂.1, fun R hnd hsub =>
      sub_trans (h₁₂.2 R hnd (row_sub_trans hsub h₂₃.1)) (h₂₃.2 R hnd hsub)⟩
  | .base _, .fn _ _, _ | .base _, .forallRow _ _, _
  | .fn _ _, .base _, _ | .fn _ _, .forallRow _ _, _
  | .forallRow _ _, .base _, _ | .forallRow _ _, .fn _ _, _ =>
    unfold Ty.sub at h₁₂; exact absurd h₁₂ id
  | .base _, .base _, .fn _ _ | .base _, .base _, .forallRow _ _
  | .fn _ _, .fn _ _, .base _ | .fn _ _, .fn _ _, .forallRow _ _
  | .forallRow _ _, .forallRow _ _, .base _
  | .forallRow _ _, .forallRow _ _, .fn _ _ =>
    unfold Ty.sub at h₂₃; exact absurd h₂₃ id
termination_by τ₂.size
decreasing_by all_goals (simp +arith [size, substRow_size])

/- ========================================================================== -/
/-! ## Rule-based subtyping -/
/- ========================================================================== -/

inductive TySub : Ty → Ty → Prop where
  | base {a b : BaseTy} :
      a <:ᵇ b → TySub (.base a) (.base b)
  | fn {a₁ a₂ b₁ b₂ : Ty} :
      TySub a₂ a₁ → TySub b₁ b₂ →
      TySub (.fn a₁ b₁) (.fn a₂ b₂)
  | forallRow {bound₁ bound₂ : Row} {body₁ body₂ : Ty} :
      bound₂ <:ʳ bound₁ →
      (∀ R, Row.NoDupKeys R → R <:ʳ bound₂ →
        TySub (body₁.substRow 0 R) (body₂.substRow 0 R)) →
      TySub (.forallRow bound₁ body₁) (.forallRow bound₂ body₂)

infixl:50 " <: " => TySub

/- ========================================================================== -/
/-! ## Reflexivity -/
/- ========================================================================== -/

theorem TySub.refl : (τ : Ty) → τ.wf → τ <: τ
  | .base t, _ => .base (_root_.sub_refl t)
  | .fn a b, ⟨ha, hb⟩ => .fn (TySub.refl a ha) (TySub.refl b hb)
  | .forallRow bound body, ⟨hnd, hwf⟩ =>
      .forallRow (row_sub_refl hnd) fun R _ _ =>
        TySub.refl (body.substRow 0 R) (substRow_wf hwf 0 R)
termination_by τ => τ.size
decreasing_by all_goals (simp +arith [size, substRow_size])

/- ========================================================================== -/
/-! ## Transitivity -/
/- ========================================================================== -/

theorem TySub.trans {τ₁ τ₂ τ₃ : Ty}
    (h₁₂ : τ₁ <: τ₂) (h₂₃ : τ₂ <: τ₃) : τ₁ <: τ₃ :=
  match h₁₂, h₂₃ with
  | .base hab, .base hbc => .base (_root_.sub_trans hab hbc)
  | .fn ha₂₁ hb₁₂, .fn ha₃₂ hb₂₃ =>
      .fn (TySub.trans ha₃₂ ha₂₁) (TySub.trans hb₁₂ hb₂₃)
  | .forallRow hbound₁₂ hbody₁₂, .forallRow hbound₂₃ hbody₂₃ =>
      .forallRow (row_sub_trans hbound₂₃ hbound₁₂) fun R hnd hsub =>
        TySub.trans (hbody₁₂ R hnd (row_sub_trans hsub hbound₂₃)) (hbody₂₃ R hnd hsub)
termination_by τ₂.size
decreasing_by all_goals (simp +arith [size, substRow_size])

/- ========================================================================== -/
/-! ## Soundness -/
/- ========================================================================== -/

theorem TySub.sound {τ₁ τ₂ : Ty} (h : τ₁ <: τ₂) : τ₁ <:ₛ τ₂ := by
  induction h with
  | base hab => unfold Ty.sub; exact hab
  | fn _ _ iha ihb => unfold Ty.sub; exact ⟨iha, ihb⟩
  | forallRow hbound hbody ih => unfold Ty.sub; exact ⟨hbound, ih⟩

end Ty
