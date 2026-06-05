
/- ========================================================================== -/
/-! # Value domain

The semantic universe for the denotational model of subtyping.
Each `BaseTy` denotes a predicate on `Val` — a value inhabits a type
when the predicate holds.

Records are association lists `List (String × Val)`. The shape denotation
requires records to have unique keys (enforced by a `Nodup` condition in
`BaseTy/Denote.lean`), reflecting that records are finite maps. -/
/- ========================================================================== -/

/-- Runtime values.
- `nat n`: a natural number
- `bool b`: a boolean
- `record entries`: a record (finite map from string labels to values) -/
inductive Val where
  | nat : Nat → Val
  | bool : Bool → Val
  | record : List (String × Val) → Val

mutual
  /-- Base types (Nat, Bool, Bot, Top, Union, Shape) --/
  inductive BaseTy where
    | nat : BaseTy
    | bool : BaseTy
    | bot : BaseTy
    | top : BaseTy
    | union : BaseTy → BaseTy → BaseTy
    | shape : Row → BaseTy

  /-- A row is the content of a shape type: a list of labelled field descriptors
  plus an unknown-field type. -/
  inductive Row where
    | mk : List (String × FieldDesc) → BaseTy → Row
    | var : Nat → Row

  /-- A field descriptor is a known, labelled field of a row with an associated
  requiredness -/
  inductive FieldDesc where
    | req : BaseTy → FieldDesc
    | opt : BaseTy → FieldDesc
end

-- Rows ------------------------------------------------------------------------

namespace Row

def fields (ρ: Row) : List (String × FieldDesc) :=
  match ρ with
  | Row.mk fs _ => fs
  | Row.var _ => []

def unknown (ρ: Row): BaseTy :=
  match ρ with
  | Row.mk _ u => u
  | Row.var _ => .bot

def fieldLabels (ρ : Row) : List String :=
  ρ.fields.map Prod.fst

/-- Project a field descriptor from a row. Returns `Opt unknown` for missing keys. -/
def proj (r : Row) (k : String) : FieldDesc :=
  match r.fields.lookup k with
  | some fd => fd
  | none => .opt r.unknown

/-- A row has unique field keys. Variables are not ground, so NoDupKeys is False. -/
def NoDupKeys (ρ: Row): Prop :=
  match ρ with
  | Row.mk fs _ => (fs.map Prod.fst).Nodup
  | Row.var _ => False

/-- The empty row: no fields, closed (unknown = bot). Identity element for merge. -/
def empty : Row := .mk [] .bot

end Row

-- Field descriptors -----------------------------------------------------------

namespace FieldDesc

def ty : FieldDesc → BaseTy
  | .req t => t
  | .opt t => t

end FieldDesc



/- ========================================================================== -/
/-! ## eraseDups produces Nodup -/
/- ========================================================================== -/

/-- Reversing a Nodup list preserves Nodup. -/
private theorem nodup_reverse (l : List String) (h : l.Nodup) : l.reverse.Nodup := by
  rw [List.nodup_iff_count] at h ⊢; intro a; rw [List.count_reverse]; exact h a

/-- The internal loop of `List.eraseDupsBy` preserves Nodup on the accumulator.
We unfold to the loop level because `eraseDupsBy` is defined via a tail-recursive
helper that Lean's simp can't see through directly. -/
private theorem eraseDupsBy_loop_nodup (input acc : List String)
    (hacc : acc.Nodup) :
    (List.eraseDupsBy.loop (fun (x y : String) => x == y) input acc).Nodup := by
  induction input generalizing acc with
  | nil => rw [List.eraseDupsBy.loop.eq_1]; exact nodup_reverse acc hacc
  | cons a rest ih =>
    rw [List.eraseDupsBy.loop.eq_2]
    split
    · exact ih acc hacc
    · next h =>
      apply ih; rw [List.nodup_cons]
      exact ⟨fun hmem => by simp at h; exact h a hmem rfl, hacc⟩

/-- `List.eraseDups` produces a list with no duplicate elements.
Used by `mergeRow_nodupKeys` to show that combined rows have unique keys. -/
theorem eraseDups_nodup (l : List String) : l.eraseDups.Nodup := by
  unfold List.eraseDups List.eraseDupsBy
  exact eraseDupsBy_loop_nodup l [] List.nodup_nil

/- ========================================================================== -/
/-! ## Well-formedness: all embedded shapes have unique field keys -/
/- ========================================================================== -/

mutual
  def BaseTy.wf : BaseTy → Prop
    | .nat | .bool | .bot | .top => True
    | .union a b => a.wf ∧ b.wf
    | .shape r => r.wfRow

  def Row.wfRow : Row → Prop
    | .var _ => True
    | .mk fields unk => Row.NoDupKeys (.mk fields unk) ∧ unk.wf ∧ fieldsWf fields

  def fieldsWf : List (String × FieldDesc) → Prop
    | [] => True
    | (_, .req t) :: rest => t.wf ∧ fieldsWf rest
    | (_, .opt t) :: rest => t.wf ∧ fieldsWf rest
end

/- ========================================================================== -/
/-! ## isEmpty -/
/- ========================================================================== -/

/-- A base type is empty (uninhabited) if no value can satisfy its denotation.
Shapes are empty when any required field has an empty type. -/
def BaseTy.isEmpty : BaseTy → Bool
  | .bot => true
  | .nat | .bool | .top => false
  | .union a b => a.isEmpty && b.isEmpty
  | .shape (.var _) => true
  | .shape (.mk fields _) => go fields
where go : List (String × FieldDesc) → Bool
  | [] => false
  | (_, .req t) :: rest => t.isEmpty || go rest
  | (_, .opt _) :: rest => go rest
