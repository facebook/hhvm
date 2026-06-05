import Shapes.Expr

/- ========================================================================== -/
/-! # Big-step evaluation for the ground fragment

Closed ground expressions evaluate to `Val`. No functions, no variables —
just records, field access, merge, and literals. -/
/- ========================================================================== -/

mutual
  /-- Big-step evaluation. `Eval e v` means closed expression `e` evaluates
  to value `v`. -/
  inductive Eval : Expr → Val → Prop where
    | nat  : Eval (.nat n) (.nat n)
    | bool : Eval (.bool b) (.bool b)
    | record {fields : List (String × Expr)} {entries : List (String × Val)} :
        EvalFields fields entries →
        Eval (.record fields) (.record entries)
    | field {e : Expr} {entries : List (String × Val)} {k : String} {v : Val} :
        Eval e (.record entries) →
        entries.lookup k = some v →
        Eval (.field e k) v
    | merge {e₁ e₂ : Expr} {entries₁ entries₂ : List (String × Val)} :
        Eval e₁ (.record entries₁) →
        Eval e₂ (.record entries₂) →
        Eval (.merge e₁ e₂) (.record (mergeEntries entries₁ entries₂))

  /-- Evaluate a list of field expressions to a list of field values. -/
  inductive EvalFields : List (String × Expr) → List (String × Val) → Prop where
    | nil  : EvalFields [] []
    | cons {k : String} {e : Expr} {v : Val}
           {rest_e : List (String × Expr)} {rest_v : List (String × Val)} :
        Eval e v →
        EvalFields rest_e rest_v →
        EvalFields ((k, e) :: rest_e) ((k, v) :: rest_v)
end

/- ========================================================================== -/
/-! ## Properties of EvalFields -/
/- ========================================================================== -/

mutual
  theorem Eval.det {e : Expr} {v₁ v₂ : Val}
      (h₁ : Eval e v₁) (h₂ : Eval e v₂) : v₁ = v₂ := by
    cases h₁ with
    | nat => cases h₂; rfl
    | bool => cases h₂; rfl
    | record hf₁ => cases h₂ with | record hf₂ => exact congrArg Val.record (EvalFields.det hf₁ hf₂)
    | field he₁ hl₁ =>
      cases h₂ with | field he₂ hl₂ =>
      have := Eval.det he₁ he₂; cases this; rw [hl₁] at hl₂; exact Option.some.inj hl₂
    | merge he₁₁ he₁₂ =>
      cases h₂ with | merge he₂₁ he₂₂ =>
      have := Eval.det he₁₁ he₂₁; cases this
      have := Eval.det he₁₂ he₂₂; cases this; rfl

  theorem EvalFields.det {fields : List (String × Expr)} {entries₁ entries₂ : List (String × Val)}
      (h₁ : EvalFields fields entries₁) (h₂ : EvalFields fields entries₂) : entries₁ = entries₂ := by
    cases h₁ with
    | nil => cases h₂; rfl
    | cons he₁ hrest₁ =>
      cases h₂ with | cons he₂ hrest₂ =>
      have := Eval.det he₁ he₂; subst this
      have := EvalFields.det hrest₁ hrest₂; subst this; rfl
end

theorem EvalFields.map_fst {fields : List (String × Expr)} {entries : List (String × Val)}
    (h : EvalFields fields entries) :
    entries.map Prod.fst = fields.map Prod.fst := by
  cases h with
  | nil => rfl
  | cons he hrest => simp [EvalFields.map_fst hrest]
