import Shapes.BaseTy.Denote

/- ========================================================================== -/
/-! # Base-type and field-descriptor subtyping -/
/- ========================================================================== -/

/-- Semantic subtyping: τ₁ is a subtype of τ₂ if every value inhabiting τ₁ also inhabits τ₂. -/
def sub (τ₁ τ₂ : BaseTy) : Prop := ∀ v, denote τ₁ v → denote τ₂ v

infixl:50 " <:ᵇ " => sub

/-! ## Base-type subtyping properties -/

/-- `τ <:ᵇ τ` -/
theorem sub_refl (τ : BaseTy) : τ <:ᵇ τ := fun _ h => h

/-- `a <:ᵇ b → b <:ᵇ c → a <:ᵇ c` -/
theorem sub_trans {a b c : BaseTy} (hab : a <:ᵇ b) (hbc : b <:ᵇ c) : a <:ᵇ c :=
  fun v ha => hbc v (hab v ha)

/-- `⊥ <:ᵇ τ` -/
theorem bot_sub (τ : BaseTy) : .bot <:ᵇ τ := fun v h =>
  (denote_bot v ▸ h).elim

/-- `τ <:ᵇ ⊤` -/
theorem sub_top (τ : BaseTy) : τ <:ᵇ .top := fun v _ =>
  denote_top v ▸ trivial

/-- `s <:ᵇ s ∪ t` -/
theorem sub_union_l (s t: BaseTy) : s <:ᵇ (.union s t) := fun v hs =>
  denote_union v s t ▸ Or.inl hs

/-- `b <:ᵇ a ∪ b` -/
theorem sub_union_r (s t : BaseTy) : t <:ᵇ (.union s t) := fun v ht =>
  denote_union v s t ▸ Or.inr ht

/-- `a <:ᵇ c → b <:ᵇ c → a ∪ b <:ᵇ c` -/
theorem union_sub {a b c : BaseTy} (hac : a <:ᵇ c) (hbc : b <:ᵇ c) : (.union a b) <:ᵇ c :=
  fun v hab =>
    (denote_union v a b ▸ hab).elim (hac v) (hbc v)

/-- `⊥ ∪ τ <:ᵇ τ` -/
theorem sub_union_bot_left (t : BaseTy) : .union .bot t <:ᵇ t :=
  fun v h => by simp at h; exact h

/-- `τ ∪ ⊥ <:ᵇ τ` -/
theorem sub_union_bot_right (t : BaseTy) : .union t .bot <:ᵇ t :=
  fun v h => by simp at h; exact h

/- ========================================================================== -/
/-! ## Field-descriptor subtyping -/
/- ========================================================================== -/

/-- Subtyping on field descriptors: `fd₁` is below `fd₂` when any lookup result
satisfying `fd₁` also satisfies `fd₂`. -/
def fieldSub (fd₁ fd₂ : FieldDesc) : Prop :=
  ∀ val, fieldCheck fd₁ val → fieldCheck fd₂ val

infixl:50 " <:ᶠ " => fieldSub

theorem fieldSub_refl (fd : FieldDesc) : fd <:ᶠ fd := fun _ h => h

theorem fieldSub_trans {fd₁ fd₂ fd₃ : FieldDesc}
    (h₁₂ : fd₁ <:ᶠ fd₂) (h₂₃ : fd₂ <:ᶠ fd₃) : fd₁ <:ᶠ fd₃ :=
  fun val h => h₂₃ val (h₁₂ val h)

/-- [Field-Req]: `Req τ₁ <:ᶠ Req τ₂` when `τ₁ <:ᵇ τ₂`. -/
theorem fieldSub_req {t₁ t₂ : BaseTy} (h : t₁ <:ᵇ t₂) : (.req t₁) <:ᶠ (.req t₂) :=
  fun val hval => by cases val <;> simp [fieldCheck] at hval ⊢ <;> exact h _ hval

/-- [Field-Opt]: `Opt τ₁ <:ᶠ Opt τ₂` when `τ₁ <:ᵇ τ₂`. -/
theorem fieldSub_opt {t₁ t₂ : BaseTy} (h : t₁ <:ᵇ t₂) : (.opt t₁) <:ᶠ (.opt t₂) :=
  fun val hval => by cases val <;> simp [fieldCheck] at hval ⊢ <;> exact h _ hval

/-- [Field-Req-Opt]: `Req τ₁ <:ᶠ Opt τ₂` when `τ₁ <:ᵇ τ₂`. -/
theorem fieldSub_req_opt {t₁ t₂ : BaseTy} (h : t₁ <:ᵇ t₂) : (.req t₁) <:ᶠ (.opt t₂) :=
  fun val hval => by cases val <;> simp [fieldCheck] at hval ⊢ <;> exact h _ hval

/-- [Field-Opt-Req, always fails]: an optional field never subtypes a required field. -/
theorem not_fieldSub_opt_req (t₁ t₂ : BaseTy) : ¬ ((.opt t₁) <:ᶠ (.req t₂)) :=
  fun h => absurd (h none trivial) id

/-- Specialization of `fieldSub` at `some v`. Used in the monotonicity proof
to avoid type inference issues when Lean can't determine the `Option Val` argument. -/
theorem fieldSub_at_some {fd₁ fd₂ : FieldDesc} (h : fieldSub fd₁ fd₂) (v : Val) :
    fieldCheck fd₁ (some v) → fieldCheck fd₂ (some v) :=
  h (some v)
