import Shapes.Expr.Eval
import Shapes.BaseTy.Sub

/-! # Typing judgment for the ground fragment

`HasType e t` says closed ground expression `e` has base type `t`.
No variables, no functions — just records, field access, merge, and literals.
Subsumption uses semantic subtyping (`sub`). -/

mutual
  inductive HasType : Expr → BaseTy → Prop where
    | nat  : HasType (.nat n) .nat
    | bool : HasType (.bool b) .bool
    | record {fields : List (String × Expr)} {fs : List (String × FieldDesc)} :
        HasTypeFields fields fs →
        (fs.map Prod.fst).Nodup →
        HasType (.record fields) (.shape (.mk fs .bot))
    | field {e : Expr} {r : Row} {k : String} {t : BaseTy} :
        HasType e (.shape r) →
        Row.NoDupKeys r →
        (k, FieldDesc.req t) ∈ r.fields →
        HasType (.field e k) t
    | merge {e₁ e₂ : Expr} {r₁ r₂ : Row} :
        HasType e₁ (.shape r₁) →
        HasType e₂ (.shape r₂) →
        Row.NoDupKeys r₁ →
        Row.NoDupKeys r₂ →
        HasType (.merge e₁ e₂) (.shape (mergeRow r₁ r₂))
    | sub {e : Expr} {t₁ t₂ : BaseTy} :
        HasType e t₁ →
        t₁ <:ᵇ t₂ →
        HasType e t₂

  /-- Typing for field expression lists. Each expression has the declared type,
  field labels match, and all fields are required (record literals produce
  closed shapes with all-Req fields). -/
  inductive HasTypeFields : List (String × Expr) → List (String × FieldDesc) → Prop where
    | nil : HasTypeFields [] []
    | cons {k : String} {e : Expr} {t : BaseTy}
           {rest_e : List (String × Expr)} {rest_fs : List (String × FieldDesc)} :
        HasType e t →
        HasTypeFields rest_e rest_fs →
        HasTypeFields ((k, e) :: rest_e) ((k, .req t) :: rest_fs)
end
