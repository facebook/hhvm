import Shapes.BaseTy.Merge

/- ========================================================================== -/
/-! # Row substitution

Mutual substitution replacing `Row.var n` with a concrete row.
Pattern-matches the field list directly (like `denote`/`knownFieldsDenote`)
so Lean can verify structural termination. -/
/- ========================================================================== -/

mutual
  def BaseTy.substRow : BaseTy → Nat → Row → BaseTy
    | .nat, _, _ => .nat
    | .bool, _, _ => .bool
    | .bot, _, _ => .bot
    | .top, _, _ => .top
    | .union a b, n, r => .union (a.substRow n r) (b.substRow n r)
    | .shape (.mk fields unk), n, r =>
        .shape (.mk (substFields fields n r) (unk.substRow n r))
    | .shape (.var m), n, r =>
        .shape (if m = n then r else .var m)

  def FieldDesc.substRow : FieldDesc → Nat → Row → FieldDesc
    | .req t, n, r => .req (t.substRow n r)
    | .opt t, n, r => .opt (t.substRow n r)

  def substFields : List (String × FieldDesc) → Nat → Row → List (String × FieldDesc)
    | [], _, _ => []
    | (k, fd) :: rest, n, r => (k, fd.substRow n r) :: substFields rest n r
end

def Row.substRow : Row → Nat → Row → Row
  | .mk fields unk, n, r => .mk (substFields fields n r) (unk.substRow n r)
  | .var m, n, r => if m = n then r else .var m

/- ========================================================================== -/
/-! ## Equation lemmas -/
/- ========================================================================== -/

@[simp] theorem BaseTy.substRow_nat : BaseTy.nat.substRow n r = .nat := rfl
@[simp] theorem BaseTy.substRow_bool : BaseTy.bool.substRow n r = .bool := rfl
@[simp] theorem BaseTy.substRow_bot : BaseTy.bot.substRow n r = .bot := rfl
@[simp] theorem BaseTy.substRow_top : BaseTy.top.substRow n r = .top := rfl

@[simp] theorem BaseTy.substRow_union :
    (BaseTy.union a b).substRow n r = .union (a.substRow n r) (b.substRow n r) := rfl

@[simp] theorem BaseTy.substRow_shape_mk :
    (BaseTy.shape (.mk fields unk)).substRow n r =
    .shape (.mk (substFields fields n r) (unk.substRow n r)) := rfl

@[simp] theorem BaseTy.substRow_shape_var_hit :
    (BaseTy.shape (.var n)).substRow n r = .shape r := by
  simp [BaseTy.substRow]

@[simp] theorem BaseTy.substRow_shape_var_miss (h : m ≠ n) :
    (BaseTy.shape (.var m)).substRow n r = .shape (.var m) := by
  simp [BaseTy.substRow, h]

@[simp] theorem Row.substRow_mk {fs : List (String × FieldDesc)} {u : BaseTy} :
    (Row.mk fs u).substRow n r =
    .mk (substFields fs n r) (u.substRow n r) := rfl

@[simp] theorem Row.substRow_var_hit :
    (Row.var n).substRow n r = r := by simp [Row.substRow]

@[simp] theorem Row.substRow_var_miss (h : m ≠ n) :
    (Row.var m).substRow n r = .var m := by simp [Row.substRow, h]

@[simp] theorem substFields_nil : substFields [] n r = [] := rfl

@[simp] theorem substFields_cons :
    substFields ((k, fd) :: rest) n r =
    (k, fd.substRow n r) :: substFields rest n r := rfl

/- ========================================================================== -/
/-! ## substFields = List.map -/
/- ========================================================================== -/

theorem substFields_eq_map (fields : List (String × FieldDesc)) (n : Nat) (r : Row) :
    substFields fields n r = fields.map fun (k, fd) => (k, fd.substRow n r) := by
  induction fields with
  | nil => rfl
  | cons hd tl ih => obtain ⟨k, fd⟩ := hd; simp [ih]

/- ========================================================================== -/
/-! ## Substitution preserves field keys -/
/- ========================================================================== -/

theorem substFields_map_fst (fields : List (String × FieldDesc)) (n : Nat) (r : Row) :
    (substFields fields n r).map Prod.fst = fields.map Prod.fst := by
  induction fields with
  | nil => rfl
  | cons hd tl ih => obtain ⟨k, _⟩ := hd; simp [ih]

@[simp] theorem FieldDesc.substRow_req :
    (FieldDesc.req t).substRow n r = .req (t.substRow n r) := rfl

@[simp] theorem FieldDesc.substRow_opt :
    (FieldDesc.opt t).substRow n r = .opt (t.substRow n r) := rfl

theorem Row.substRow_preserves_NoDupKeys {row : Row} (h : Row.NoDupKeys row) (n : Nat) (r : Row) :
    Row.NoDupKeys (row.substRow n r) := by
  cases row with
  | var _ => exact absurd h id
  | mk fs u => simp [Row.substRow, Row.NoDupKeys, substFields_map_fst]; exact h

/- ========================================================================== -/
/-! ## Substitution distributes over merge -/
/- ========================================================================== -/

theorem lookup_substFields (fields : List (String × FieldDesc)) (k : String) (n : Nat) (s : Row) :
    (substFields fields n s).lookup k =
    Option.map (·.substRow n s) (fields.lookup k) := by
  induction fields with
  | nil => simp
  | cons hd tl ih =>
    obtain ⟨k', fd⟩ := hd
    simp only [substFields_cons, List.lookup_cons]
    split <;> simp_all

theorem proj_substRow_mk (fields : List (String × FieldDesc)) (unk : BaseTy)
    (k : String) (n : Nat) (s : Row) :
    Row.proj ((Row.mk fields unk).substRow n s) k =
    (Row.proj (Row.mk fields unk) k).substRow n s := by
  simp only [Row.substRow_mk, Row.proj, Row.fields, Row.unknown, lookup_substFields]
  cases fields.lookup k <;> simp

theorem proj_substRow {r : Row} (hnd : Row.NoDupKeys r) (k : String) (n : Nat) (s : Row) :
    Row.proj (r.substRow n s) k = (Row.proj r k).substRow n s := by
  cases r with
  | var _ => exact absurd hnd id
  | mk fs u => exact proj_substRow_mk fs u k n s

theorem mergeFieldDesc_substRow (fd₁ fd₂ : FieldDesc) (n : Nat) (s : Row) :
    (mergeFieldDesc fd₁ fd₂).substRow n s =
    mergeFieldDesc (fd₁.substRow n s) (fd₂.substRow n s) := by
  cases fd₁ <;> cases fd₂ <;> simp [mergeFieldDesc]
