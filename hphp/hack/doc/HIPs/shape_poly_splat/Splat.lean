-- Splat: a verified, decidable subtyping/subrow system with rightmost-wins spread semantics.
--
-- Layer map (each layer builds on the one above; the decision is stratified ground-first):
--   • Foundations  — Syntax, Subtyping, Denotation: terms, the ground `SubBase` relation, the
--       step-indexed value model and soundness.
--   • Frame+kernel — PerLabel, Corner: the per-label symbolic projection `rowFieldAt`, and the
--       box-agnostic heart — one parameter's contribution reduces to ≤4 box corners (`peel_step`).
--   • Ground bounds — Sound, Complete, Decide: fixed box, cartesian `cornerAssigns`, `decidableSemSubRow`.
--   • General bounds — Coupled, CoupledComplete: the box-shift extension (topological enumeration,
--       `decidableSemSubRow_coupled`) — the only genuinely coupled-specific machinery.
--   • Optimization kernel — Irrelevance: box-agnostic liveness/masking irrelevance (shared).
--   • Ground optimizations — GroundOpt: the corner cuts on the fixed-box enumeration, read directly.
--   • Coupled optimizations — CoupledOpt: the same cuts threaded through the topological enumeration,
--       matching the implementation's `corners_for`.
-- A reader can take the ground path (Foundations → kernel → Ground bounds → Irrelevance → GroundOpt)
-- as a complete, self-contained story before the General/Coupled layers extend it.
--
-- Representation (mirrors the OCaml `repr.ml`): a `SimpleRow`'s `unknown` is *opt-only* (a bare
-- `Base`, so the infinitely many absent labels are always optional).  Bottom is therefore not a row
-- at all — it is `Base.bot`, handled at the *base* level (`SubBase.bot : SubBase .bot t`, and `canon`
-- collapsing any splat carrying `spread .bot` to `.bot`).  `SemSubRow` is the parametric *row*
-- comparison one layer above that: it is faithfully field-by-field (`rowFieldAt`), and it coincides
-- with the bottom-collapsing semantics on every `Γ` whose parameter *upper* bounds are non-bottom
-- (`NotBotRow`) — exactly the ⊥-super case the implementation dispatches at the base level.  The
-- coupled completeness therefore takes that non-bottom-upper-bound hypothesis, under which every
-- completeness witness is a (non-bottom) simple-row shape.
import Splat.Syntax
import Splat.Subtyping
import Splat.Denotation
import Splat.Subrow.PerLabel
import Splat.Subrow.Corner
import Splat.Subrow.Sound
import Splat.Subrow.Complete
import Splat.Subrow.Decide
import Splat.Subrow.Coupled
import Splat.Subrow.CoupledComplete
import Splat.Subrow.Irrelevance
import Splat.Subrow.GroundOpt
import Splat.Subrow.CoupledOpt
