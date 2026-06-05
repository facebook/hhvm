import Shapes.Row.Sub
import Shapes.Row.Subst

/- ========================================================================== -/
/-! # Polymorphic type layer

`Ty` adds function types and bounded row quantification on top of the
ground `BaseTy` layer. -/
/- ========================================================================== -/

inductive Ty where
  | base : BaseTy → Ty
  | fn : Ty → Ty → Ty
  | forallRow : Row → Ty → Ty

namespace Ty

/- ========================================================================== -/
/-! ## Substitution at the Ty level -/
/- ========================================================================== -/

def substRow : Ty → Nat → Row → Ty
  | .base t, n, r => .base (t.substRow n r)
  | .fn a b, n, r => .fn (a.substRow n r) (b.substRow n r)
  | .forallRow bound body, n, r =>
      .forallRow (bound.substRow n r) (body.substRow (n + 1) r)

/- ========================================================================== -/
/-! ## Well-formedness -/
/- ========================================================================== -/

def wf : Ty → Prop
  | .base _ => True
  | .fn a b => a.wf ∧ b.wf
  | .forallRow bound body => Row.NoDupKeys bound ∧ body.wf

theorem substRow_wf {τ : Ty} (hwf : τ.wf) (n : Nat) (r : Row) :
    (τ.substRow n r).wf := by
  induction τ generalizing n with
  | base => trivial
  | fn a b iha ihb => exact ⟨iha hwf.1 n, ihb hwf.2 n⟩
  | forallRow bound body ih => exact ⟨Row.substRow_preserves_NoDupKeys hwf.1 n r, ih hwf.2 (n + 1)⟩

/- ========================================================================== -/
/-! ## Size measure -/
/- ========================================================================== -/

def size : Ty → Nat
  | .base _ => 1
  | .fn a b => 1 + a.size + b.size
  | .forallRow _ body => 1 + body.size

theorem substRow_size : ∀ (τ : Ty) (n : Nat) (r : Row),
    (τ.substRow n r).size = τ.size := by
  intro τ n r
  induction τ generalizing n with
  | base => simp [substRow, size]
  | fn a b iha ihb => simp [substRow, size, iha, ihb]
  | forallRow bound body ih => simp [substRow, size, ih]

end Ty
