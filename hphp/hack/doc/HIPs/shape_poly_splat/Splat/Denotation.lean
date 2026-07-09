import Splat.Subtyping

/-!
# Denotational model and soundness

`Val`, the denotation predicates `denBase`/`denField`/`denRow`, and soundness of the declarative
relations against them (`denBase_sound`, `subField_row_sound`).

A type denotes a set of runtime values, and subtyping soundness is *value containment*: `SubBase t u`
implies every value of `t` is also a value of `u`.  This is what justifies that the declarative
relation is the *right* notion — it tracks value substitutability.

The denotations are **inductive predicates**: a proof of `denBase η t v` is a finite derivation
built from the constructors below.  This avoids the termination issues of a recursive `def`
(Lean's checker cannot see through `List.lookup` to bound the recursive call at `shape`), while
giving the same semantic content — an *empty* set of applicable constructors captures the
"absent" cases (`.bot`, `.prim`/value mismatch, splat-shape) that a recursive `def` would return
`False` for.

A rigid parameter `α` denotes an *abstract* value-set `η α`; soundness holds for every valuation
`η` (parametricity).  A `shape` of a *splat* row would need the parameter instantiation, which is
not modelled here, so it has no denotation; the ground relation only ever compares `shape`s of
*simple* rows, so that arm is never reached by the soundness proof.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 9. Runtime values and denotation                                                            -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Runtime values: a boolean, a natural, or a record (a finite list of labelled values). -/
inductive Val where
  | nat    : Nat → Val
  | bool   : Bool → Val
  | record : List (Label × Val) → Val

/-- A parameter valuation: each rigid parameter denotes an (abstract) set of values. -/
abbrev ValEnv := Ty_param → Val → Prop

mutual
  /-- Denotation of a base type at a runtime value.  Each constructor is a truth rule; the absent
      cases (`.bot`, `.prim`/value mismatch, `.shape` at a non-record, `.shape` of a splat row) have
      no constructor and are therefore *uninhabited* — the inductive-predicate analogue of `False`. -/
  inductive denBase : ValEnv → Base → Val → Prop where
    | top         {η v}         : denBase η .top v
    | prim_nat    {η n}         : denBase η (.prim .nat) (.nat n)
    | prim_bool   {η b}         : denBase η (.prim .bool) (.bool b)
    | rigid       {η α v}       : η α v → denBase η (.rigid α) v
    | union_left  {η a b v}     : denBase η a v → denBase η (.union a b) v
    | union_right {η a b v}     : denBase η b v → denBase η (.union a b) v
    | shape_simple {η fs u rec} :
        (∀ l, denField η
                (match fs.lookup l with | some d => d | none => .opt u)
                (rec.lookup l)) →
        denBase η (.shape (.simple fs u)) (.record rec)

  /-- Denotation of a field descriptor against an optional runtime slot.  A `.req` field must be
      present; an `.opt` field may be present or absent.  The `req, none` case has no constructor
      (uninhabited): a required field cannot be missing. -/
  inductive denField : ValEnv → FieldDesc → Option Val → Prop where
    | req_some {η t v} : denBase η t v → denField η (.req t) (some v)
    | opt_some {η t v} : denBase η t v → denField η (.opt t) (some v)
    | opt_none {η t}   : denField η (.opt t) none
end

/-- Denotation of a simple row against a runtime record. -/
def denRow (η : ValEnv) (r : SimpleRow) (fs : List (Label × Val)) : Prop :=
  ∀ l, denField η (proj r l) (fs.lookup l)

/-- Bridge: a `shape` of a simple row denotes exactly as `denRow`.  Both sides express
    `∀ l, denField η (proj r l) (fs.lookup l)`; the shape case's inline `fs.lookup l`/`.opt u`
    fallback is definitionally `SimpleRow.proj`. -/
theorem denBase_shape (η : ValEnv) (r : SimpleRow) (fs : List (Label × Val)) :
    denBase η (.shape (.simple r.known r.unknown)) (.record fs) ↔ denRow η r fs := by
  constructor
  · intro h; cases h with | shape_simple hh => exact hh
  · intro h; exact .shape_simple h

/- ---------------------------------------------------------------------------------------------- -/
/- ## 10. Soundness of `SubBase` / `SubField` / `SubField_row`                                    -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Field soundness, given base soundness. -/
theorem denField_sound (η : ValEnv)
    (ihB : ∀ {t u}, SubBase t u → ∀ v, denBase η t v → denBase η u v)
    {d e : FieldDesc} (h : SubField d e) (ov : Option Val) (hov : denField η d ov) :
    denField η e ov := by
  cases h with
  | req_req ht =>
    cases hov with | req_some hv => exact .req_some (ihB ht _ hv)
  | opt_opt ht =>
    cases hov with
    | opt_some hv => exact .opt_some (ihB ht _ hv)
    | opt_none    => exact .opt_none
  | req_opt ht =>
    cases hov with | req_some hv => exact .opt_some (ihB ht _ hv)

/-- Row soundness, given base soundness. -/
theorem denRow_sound (η : ValEnv)
    (ihB : ∀ {t u}, SubBase t u → ∀ v, denBase η t v → denBase η u v)
    {r p : SimpleRow} (h : SubField_row r p) (fs : List (Label × Val))
    (hfs : denRow η r fs) : denRow η p fs :=
  fun l => denField_sound η ihB (h.at l) (fs.lookup l) (hfs l)

/-- **Base soundness.**  The derivation is consumed by the mutual recursor `SubBase.rec` — the
    three motives are soundness of bases, fields, and rows.  Each case reads off a constructor of
    the `denBase`/`denField` predicate, applies IHs, and rebuilds the target constructor. -/
theorem denBase_sound (η : ValEnv) :
    ∀ {a b : Base}, SubBase a b → ∀ v, denBase η a v → denBase η b v := by
  intro a b h
  refine SubBase.rec
    (motive_1 := fun a b _ => ∀ v, denBase η a v → denBase η b v)
    (motive_2 := fun d e _ => ∀ ov, denField η d ov → denField η e ov)
    (motive_3 := fun r p _ => ∀ fs, denRow η r fs → denRow η p fs)
    ?refl ?top ?bot ?union_left ?union_right ?union_sub
    ?trans ?shape_sub
    ?req_req ?opt_opt ?req_opt ?mk h
  case refl        => intro _ _ hv; exact hv
  case top         => intro _ _ _; exact .top
  case bot         => intro _ _ hv; cases hv
  case union_left  => intro _ _ _ hv; exact .union_left hv
  case union_right => intro _ _ _ hv; exact .union_right hv
  case union_sub =>
    intro _ _ _ _ _ ih1 ih2 v hv
    cases hv with
    | union_left ha  => exact ih1 v ha
    | union_right hb => exact ih2 v hb
  case trans => intro _ _ _ _ _ ih1 ih2 v hv; exact ih2 v (ih1 v hv)
  case shape_sub =>
    intro r p _ ih3 v hv
    cases hv with
    | shape_simple hh =>
        exact (denBase_shape η p _).mpr (ih3 _ ((denBase_shape η r _).mp (.shape_simple hh)))
  case req_req =>
    intro _ _ _ ih1 ov hov
    cases hov with | req_some hv => exact .req_some (ih1 _ hv)
  case opt_opt =>
    intro _ _ _ ih1 ov hov
    cases hov with
    | opt_some hv => exact .opt_some (ih1 _ hv)
    | opt_none    => exact .opt_none
  case req_opt =>
    intro _ _ _ ih1 ov hov
    cases hov with | req_some hv => exact .opt_some (ih1 _ hv)
  case mk =>
    intro _ _ _ ih2 fs hfs l
    exact ih2 l (fs.lookup l) (hfs l)

/-- Field soundness (corollary). -/
theorem denField_sound' {d e : FieldDesc} (h : SubField d e) (η : ValEnv) :
    ∀ ov, denField η d ov → denField η e ov :=
  denField_sound η (fun ht v => denBase_sound η ht v) h

/-- **Row soundness**: `SubField_row r p ⟹ ⟦r⟧ ⊆ ⟦p⟧` — every record of row `r` is a record of
    row `p`.  The headline row-soundness statement justifying `SubField_row` as the right notion
    of subrow. -/
theorem subField_row_sound {r p : SimpleRow} (h : SubField_row r p) (η : ValEnv) :
    ∀ fs, denRow η r fs → denRow η p fs :=
  denRow_sound η (fun ht v => denBase_sound η ht v) h

end Splat
