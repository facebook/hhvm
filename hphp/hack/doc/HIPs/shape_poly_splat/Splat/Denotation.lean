import Splat.Subtyping

/-!
# Step-indexed denotational model and soundness

`Val`, the step-indexed `denBase`/`denField`/`denRow`, and soundness of the declarative relations
against it (`denBase_sound`, `subField_row_sound`).

A type denotes a set of runtime values, and subtyping soundness is *value containment*: `SubBase t u`
implies every value of `t` is also a value of `u`.  This is what justifies that the declarative
relation is the *right* notion — it tracks value substitutability.

The model is **step-indexed**: a fuel bounds how deep into nested records we look.  We need this
because at `shape` the recursive call lands on a field's type, which is *computed* by
`proj`/`normalize` rather than exposed as a constructor-child — so it is not a subterm the
structural-recursion checker can follow (the decrease is real, but invisible to it).  Fuel
decrements *only* at `shape`; every other rule recurses at the same fuel, which is exactly what
makes finite-index soundness of `union` go through.  At fuel `0` every type holds (nothing can be
observed yet), so the limit denotation `∀ n, …` is meaningful.

A rigid parameter `α` denotes an *abstract* value-set `η α`; soundness holds for every valuation
`η` (parametricity).  A `shape` of a *splat* row would need the parameter instantiation, which is
not modelled here, so it has no denotation; the ground relation only ever compares `shape`s of
*simple* rows, so that arm is never reached by the soundness proof.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 9. Runtime values and step-indexed denotation                                               -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Runtime values: a boolean, a natural, or a record (a finite list of labelled values). -/
inductive Val where
  | nat    : Nat → Val
  | bool   : Bool → Val
  | record : List (Label × Val) → Val

/-- A parameter valuation: each rigid parameter denotes an (abstract) set of values. -/
abbrev ValEnv := Ty_param → Val → Prop

/-- Whether a runtime field-slot satisfies a descriptor, given a base-denotation `P`.  A required
    field must be present and in `P`; an optional field may be absent, or present and in `P`.
    Shared by `denBase`'s shape case, `denField`, and `denRow` so they agree definitionally. -/
def fieldHolds (P : Base → Val → Prop) : FieldDesc → Option Val → Prop
  | .req t, some v => P t v
  | .req _, none   => False
  | .opt t, some v => P t v
  | .opt _, none   => True

/-- Step-indexed denotation of a base type.  Fuel decrements **only** at `shape` — the sole place
    the recursive call lands on a *computed* field type (via `proj`/`normalize`) rather than a
    constructor-child, so it is not a subterm the structural-recursion checker can follow.
    `union` recurses at the *same* fuel (its components are structural subterms), which is what
    makes finite-index soundness of `union` hold.  At fuel `0` every type holds (nothing observed
    yet).  A `rigid` leaf consults the valuation; a `shape` of a *splat* row has no denotation here
    (it needs the parameter instantiation) and is never reached by the ground soundness proof.
    Termination is lexicographic on `(fuel, sizeOf t)`. -/
def denBase : Nat → ValEnv → Base → Val → Prop
  | 0,    _, _,                     _            => True
  | _+1,  _, .top,                  _            => True
  | _+1,  _, .bot,                  _            => False
  | _+1,  _, .prim .nat,            .nat _       => True
  | _+1,  _, .prim .bool,           .bool _      => True
  | _+1,  _, .prim _,               _            => False
  | _+1,  η, .rigid α,              v            => η α v
  | n+1,  η, .union a b,            v            => denBase (n+1) η a v ∨ denBase (n+1) η b v
  | n+1,  η, .shape (.simple fs u), .record rec  =>
      ∀ l, fieldHolds (fun t v => denBase n η t v) (proj (normalize fs u) l) (rec.lookup l)
  | _+1,  _, .shape (.simple _ _),  .nat _       => False
  | _+1,  _, .shape (.simple _ _),  .bool _      => False
  | _+1,  _, .shape (.splat _),     _            => False   -- splat-row shape: needs param instantiation
termination_by fuel _ t _ => (fuel, sizeOf t)
decreasing_by all_goals simp_wf; all_goals omega

/-- Denotation of a field descriptor against an optional runtime slot. -/
def denField (n : Nat) (η : ValEnv) (d : FieldDesc) (ov : Option Val) : Prop :=
  fieldHolds (fun t v => denBase n η t v) d ov

/-- Denotation of a simple row against a runtime record. -/
def denRow (n : Nat) (η : ValEnv) (r : SimpleRow) (fs : List (Label × Val)) : Prop :=
  ∀ l, denField n η (proj r l) (fs.lookup l)

/-- The "true" denotations are the limits over all fuel. -/
def denBaseLim  (η : ValEnv) (t : Base)      (v : Val)                  : Prop := ∀ n, denBase  n η t v
def denFieldLim (η : ValEnv) (d : FieldDesc) (ov : Option Val)         : Prop := ∀ n, denField n η d ov
def denRowLim   (η : ValEnv) (r : SimpleRow) (fs : List (Label × Val)) : Prop := ∀ n, denRow   n η r fs

/-- Bridge: a `shape` of a simple row denotes exactly as `denRow` at one less fuel.  Holds
    definitionally thanks to the shared `fieldHolds` and `proj_normalize_simplerow`. -/
theorem denBase_shape (η : ValEnv) (m : Nat) (r : SimpleRow) (fs : List (Label × Val)) :
    denBase (m+1) η (.shape (.simple r.known r.unknown)) (.record fs) = denRow m η r fs := by
  -- both sides reduce to `∀ l, fieldHolds (denBase m η ·) (proj r l) (fs.lookup l)`; the row's
  -- own fields project the same after `normalize` (`proj_normalize_simplerow`)
  simp only [denBase, denRow, denField, proj_normalize_simplerow]

/- ---------------------------------------------------------------------------------------------- -/
/- ## 10. Soundness of `SubBase` / `SubField` / `SubField_row`                                    -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Field soundness at a fixed fuel, given base soundness at that fuel. -/
theorem denField_sound (η : ValEnv) (n : Nat)
    (ihB : ∀ {t u}, SubBase t u → ∀ v, denBase n η t v → denBase n η u v)
    {d e : FieldDesc} (h : SubField d e) (ov : Option Val) (hov : denField n η d ov) :
    denField n η e ov := by
  -- the shared middle requiredness limits the cases; a present slot uses `ihB`, an absent slot
  -- is unchanged (`req`→`req`/`opt`) or vacuous (`req`→`opt` has `hov : False`)
  cases h with
  | req_req ht => cases ov with | some v => exact ihB ht v hov | none => exact hov
  | opt_opt ht => cases ov with | some v => exact ihB ht v hov | none => exact hov
  | req_opt ht => cases ov with | some v => exact ihB ht v hov | none => exact False.elim hov

/-- Row soundness at a fixed fuel, given base soundness at that fuel. -/
theorem denRow_sound (η : ValEnv) (n : Nat)
    (ihB : ∀ {t u}, SubBase t u → ∀ v, denBase n η t v → denBase n η u v)
    {r p : SimpleRow} (h : SubField_row r p) (fs : List (Label × Val))
    (hfs : denRow n η r fs) : denRow n η p fs :=
  -- pointwise: at each label, the subfield holds by `denField_sound`
  fun l => denField_sound η n ihB (h.at l) (fs.lookup l) (hfs l)

/-- **Base soundness, step-indexed.**  Proved by induction on fuel; the derivation is consumed by
    the mutual recursor `SubBase.rec` (the three motives are soundness of bases, fields, and rows at
    the current fuel).  Only `shape_sub` drops a fuel level, where it appeals to `denRow_sound` at
    the lower level via the fuel IH `ih`. -/
theorem denBase_sound (η : ValEnv) :
    ∀ n {a b : Base}, SubBase a b → ∀ v, denBase n η a v → denBase n η b v := by
  intro n
  induction n with
  | zero =>
    -- at fuel `0` the target denotation is `True`, so there is nothing to prove
    intro a b _ v _; simp only [denBase]
  | succ m ih =>
    intro a b h
    refine SubBase.rec
      (motive_1 := fun a b _ => ∀ v, denBase (m+1) η a v → denBase (m+1) η b v)
      (motive_2 := fun d e _ => ∀ ov, denField (m+1) η d ov → denField (m+1) η e ov)
      (motive_3 := fun r p _ => ∀ fs, denRow (m+1) η r fs → denRow (m+1) η p fs)
      ?refl ?top ?bot ?union_left ?union_right ?union_comm ?union_sub ?trans ?shape_sub
      ?req_req ?opt_opt ?req_opt ?mk h
    case refl => intro t v hv; exact hv
    case top => intro t v _; simp only [denBase]
    case bot => intro t v hv; simp only [denBase] at hv
    case union_left => intro t u v hv; simp only [denBase]; exact Or.inl hv
    case union_right => intro t u v hv; simp only [denBase]; exact Or.inr hv
    case union_comm => intro t u v hv; simp only [denBase] at hv ⊢; exact Or.symm hv
    case union_sub =>
      intro t u w _ _ ih1 ih2 v hv
      simp only [denBase] at hv
      rcases hv with hv | hv
      · exact ih1 v hv
      · exact ih2 v hv
    case trans => intro t u w _ _ ih1 ih2 v hv; exact ih2 v (ih1 v hv)
    case shape_sub =>
      intro r p hrow _ih3 v hv
      cases v with
      | record fs =>
          -- shape drops a level: rewrite both `shape`s to `denRow` at fuel `m`, then use the fuel IH
          rw [denBase_shape] at hv ⊢
          exact denRow_sound η m (fun ht v => ih ht v) hrow fs hv
      | nat k => simp only [denBase] at hv
      | bool b => simp only [denBase] at hv
    case req_req =>
      intro t u _ ih1 ov hov
      cases ov with | some v => exact ih1 v hov | none => exact hov
    case opt_opt =>
      intro t u _ ih1 ov hov
      cases ov with | some v => exact ih1 v hov | none => exact hov
    case req_opt =>
      intro t u _ ih1 ov hov
      cases ov with | some v => exact ih1 v hov | none => exact False.elim hov
    case mk =>
      intro r p _ ih2 fs hfs l
      exact ih2 l (fs.lookup l) (hfs l)

/-- Field soundness, step-indexed (corollary). -/
theorem denField_sound' {d e : FieldDesc} (h : SubField d e) (η : ValEnv) (n : Nat) :
    ∀ ov, denField n η d ov → denField n η e ov :=
  denField_sound η n (fun ht v => denBase_sound η n ht v) h

/-- Row soundness, step-indexed (corollary). -/
theorem denRow_sound' {r p : SimpleRow} (h : SubField_row r p) (η : ValEnv) (n : Nat) :
    ∀ fs, denRow n η r fs → denRow n η p fs :=
  denRow_sound η n (fun ht v => denBase_sound η n ht v) h

/- ---------------------------------------------------------------------------------------------- -/
/- ## 11. Soundness at the limit (value containment)                                              -/
/- ---------------------------------------------------------------------------------------------- -/

/-- `SubBase t u ⟹ ⟦t⟧ ⊆ ⟦u⟧`: every value of `t` is a value of `u`. -/
theorem denBaseLim_sound {t u : Base} (h : SubBase t u) (η : ValEnv) :
    ∀ v, denBaseLim η t v → denBaseLim η u v :=
  fun v hv n => denBase_sound η n h v (hv n)

/-- `SubField d e ⟹ ⟦d⟧ ⊆ ⟦e⟧`. -/
theorem denFieldLim_sound {d e : FieldDesc} (h : SubField d e) (η : ValEnv) :
    ∀ ov, denFieldLim η d ov → denFieldLim η e ov :=
  fun ov hov n => denField_sound' h η n ov (hov n)

/-- `SubField_row r p ⟹ ⟦r⟧ ⊆ ⟦p⟧`: every record of row `r` is a record of row `p`.  This is the
    headline row-soundness statement justifying `SubField_row` as the right notion of subrow. -/
theorem subField_row_sound {r p : SimpleRow} (h : SubField_row r p) (η : ValEnv) :
    ∀ fs, denRowLim η r fs → denRowLim η p fs :=
  fun fs hf n => denRow_sound' h η n fs (hf n)

end Splat
