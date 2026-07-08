import Splat.Subrow.Coupled

/-!
# Completeness for the coupled case — the topological witness

The completeness direction (`SemSubRow → corner check`) for coupled bounds.  A failing corner is
realized by a `Compatible` witness built in *topological* order (`buildEnv`), whose self-consistency
(`buildEnv_self`) replaces the ground-bound assumption of the §33 `completeWitness`.

This file first establishes `evalRow_congr` (evaluation reads `ρ` only at the parameters a `Ground`
row mentions — absent from the redo so far), then the builder and the completeness lemma.
-/

namespace Splat

/- ---------------------------------------------------------------------------------------------- -/
/- ## 57a. The opt-only collapse (witness reconstruction only)                                    -/
/- ---------------------------------------------------------------------------------------------- -/

/- To *build* the completeness witness we must reconstruct a parameter's (non-bottom) upper-bound
   row as a concrete `SimpleRow`, so that one field can be overridden at the failing label.  The
   collapse folds `mergeSimple` (rightmost-wins) over a rigid-free-substituted splat, exactly as the
   per-label `rowFieldAt` folds `mergeField`.  Bottom has no opt-only simple-row form, so the `.bot`
   arm returns the junk `emptyRow`; the collapse is only ever *used* on non-bottom rows (the
   `NotBotRow`/`NotBotEnv` conditions), where the `bridge` below makes it faithful. -/
mutual
  def collapseRow : Row → SimpleRow
    | .simple fs u => normalize fs u
    | .splat es    => collapseSplat es
  def collapseSplat : List SplatElem → SimpleRow
    | []      => emptyRow
    | e :: es => collapseFold es (collapseElem e)
  def collapseFold : List SplatElem → SimpleRow → SimpleRow
    | [],      acc => acc
    | e :: es, acc => collapseFold es (mergeSimple acc (collapseElem e))
  def collapseElem : SplatElem → SimpleRow
    | .spread b => collapseBase b
  def collapseBase : Base → SimpleRow
    | .shape r   => collapseRow r
    | .bot       => emptyRow    -- bottom has no opt-only simple-row form (junk; `NotBot` rules out)
    | .top       => emptyRow
    | .prim _    => emptyRow
    | .union _ _ => emptyRow
    | .rigid _   => emptyRow
end

/-- Evaluate a row to a simple row under a ground instantiation: substitute then collapse. -/
def evalRow (ρ : GEnv) (r : Row) : SimpleRow := collapseRow (substRow ρ r)

/- The literal (pre-substitution) *non-bottom* condition: no reachable `spread .bot`.  A `.rigid` is
   non-bottom literally (its value is supplied by the env, guarded separately by `NotBotEnv`). -/
mutual
  def NotBotBase : Base → Prop
    | .bot     => False
    | .shape r => NotBotRow r
    | _        => True
  def NotBotRow : Row → Prop
    | .simple _ _ => True
    | .splat es   => NotBotElems es
  def NotBotElems : List SplatElem → Prop
    | []      => True
    | e :: es => NotBotElem e ∧ NotBotElems es
  def NotBotElem : SplatElem → Prop
    | .spread b => NotBotBase b
end

/-- An instantiation is `NotBotEnv` when every parameter collapses to a non-bottom base. -/
def NotBotEnv (ρ : GEnv) : Prop := ∀ α, NotBotBase (ρ α)

/-- `projOpt` distributes over `mergeSimple` at an optional label (`none` is the merged unknown;
    `some l` is `proj_mergeSimple`). -/
theorem projOpt_mergeSimple (a b : SimpleRow) (l : Option Label) :
    projOpt (mergeSimple a b) l = mergeField (projOpt a l) (projOpt b l) := by
  cases l with
  | none     => rfl
  | some lbl => exact proj_mergeSimple a b lbl

/- ---------------------------------------------------------------------------------------------- -/
/- ## 57b. The bridge: the collapse's projection equals the per-label field                        -/
/- ---------------------------------------------------------------------------------------------- -/

/- **Ground-base bridge.**  A non-bottom *ground* base projects (via collapse) to its per-label
   field, resolved by the dummy assignment `gOf` uses.  (`.rigid` maps both sides to `.opt .bot`.) -/
mutual
  theorem projOpt_collapseBase (l : Option Label) :
      ∀ b : Base, NotBotBase b → projOpt (collapseBase b) l = baseFieldAt (fun _ => .opt .bot) l b
    | .bot,       h => absurd h (by simp [NotBotBase])
    | .top,       _ => by cases l <;> rfl
    | .prim _,    _ => by cases l <;> rfl
    | .union _ _, _ => by cases l <;> rfl
    | .rigid _,   _ => by cases l <;> rfl
    | .shape (.simple _ _), _ => by cases l <;> rfl
    | .shape (.splat es), h => projOpt_collapseSplat l es h

  theorem projOpt_collapseSplat (l : Option Label) :
      ∀ es : List SplatElem, NotBotElems es →
        projOpt (collapseSplat es) l = splatFieldAt (fun _ => .opt .bot) l es
    | [],      _ => by cases l <;> rfl
    | e :: es, h =>
        projOpt_collapseFold l es h.2 (collapseElem e) (elemFieldAt (fun _ => .opt .bot) l e)
          (by cases e with | spread b => exact projOpt_collapseBase l b h.1)

  theorem projOpt_collapseFold (l : Option Label) :
      ∀ (es : List SplatElem), NotBotElems es → ∀ (accS : SimpleRow) (accF : FieldDesc),
        projOpt accS l = accF →
        projOpt (collapseFold es accS) l = splatFoldAt (fun _ => .opt .bot) l es accF
    | [],      _, _,    _,    hinv => hinv
    | e :: es, h, accS, accF, hinv =>
        projOpt_collapseFold l es h.2 (mergeSimple accS (collapseElem e))
          (mergeField accF (elemFieldAt (fun _ => .opt .bot) l e))
          (by rw [projOpt_mergeSimple, hinv]
              exact congrArg (mergeField accF)
                (by cases e with | spread b => exact projOpt_collapseBase l b h.1))
end

/- **The substitution bridge.**  For a `Ground`, literally-non-bottom row under a `NotBotEnv`
   instantiation, the collapse of the substituted row projects to the per-label field with the
   parameters resolved by `gOf ρ l`.  At a `.rigid`, the substituted base `ρ α` is ground and
   non-bottom, so the ground-base bridge (with `gOf`'s dummy) closes it. -/
mutual
  theorem projOpt_collapseBase_subst (ρ : GEnv) (hρ : NotBotEnv ρ) (l : Option Label) :
      ∀ b : Base, GroundSpread b → NotBotBase b →
        projOpt (collapseBase (substBase ρ b)) l = baseFieldAt (gOf ρ l) l b
    | .rigid α, _, _ => by
        show projOpt (collapseBase (ρ α)) l = gOf ρ l α
        exact projOpt_collapseBase l (ρ α) (hρ α)
    | .bot,       _, h => absurd h (by simp [NotBotBase])
    | .top,       _, _ => by cases l <;> rfl
    | .prim _,    _, _ => by cases l <;> rfl
    | .union _ _, _, _ => by cases l <;> rfl
    | .shape (.simple fs u), hg, _ => by
        show projOpt (normalize (substFields ρ fs) (substBase ρ u)) l = projOpt (normalize fs u) l
        rw [substFields_fixed ρ hg.1, hg.2 ρ]
    | .shape (.splat es), hg, hnb => projOpt_collapseSplat_subst ρ hρ l es hg hnb

  theorem projOpt_collapseElem_subst (ρ : GEnv) (hρ : NotBotEnv ρ) (l : Option Label) :
      ∀ e : SplatElem, GroundElem e → NotBotElem e →
        projOpt (collapseElem (substElem ρ e)) l = elemFieldAt (gOf ρ l) l e
    | .spread b, hg, hnb => projOpt_collapseBase_subst ρ hρ l b hg hnb

  theorem projOpt_collapseSplat_subst (ρ : GEnv) (hρ : NotBotEnv ρ) (l : Option Label) :
      ∀ es : List SplatElem, GroundElems es → NotBotElems es →
        projOpt (collapseSplat (substElems ρ es)) l = splatFieldAt (gOf ρ l) l es
    | [],      _,  _   => by cases l <;> rfl
    | e :: es, hg, hnb =>
        projOpt_collapseFold_subst ρ hρ l es hg.2 hnb.2
          (collapseElem (substElem ρ e)) (elemFieldAt (gOf ρ l) l e)
          (projOpt_collapseElem_subst ρ hρ l e hg.1 hnb.1)

  theorem projOpt_collapseFold_subst (ρ : GEnv) (hρ : NotBotEnv ρ) (l : Option Label) :
      ∀ (es : List SplatElem), GroundElems es → NotBotElems es →
        ∀ (accS : SimpleRow) (accF : FieldDesc), projOpt accS l = accF →
        projOpt (collapseFold (substElems ρ es) accS) l = splatFoldAt (gOf ρ l) l es accF
    | [],      _,  _,   _,    _,    hinv => hinv
    | e :: es, hg, hnb, accS, accF, hinv =>
        projOpt_collapseFold_subst ρ hρ l es hg.2 hnb.2
          (mergeSimple accS (collapseElem (substElem ρ e)))
          (mergeField accF (elemFieldAt (gOf ρ l) l e))
          (by rw [projOpt_mergeSimple, hinv, projOpt_collapseElem_subst ρ hρ l e hg.1 hnb.1])
end

/-- The **bridge** (dispatcher): the collapse of a `Ground`, non-bottom row projects to its per-label
    field under `gOf ρ l`. -/
theorem projOpt_evalRow (ρ : GEnv) (hρ : NotBotEnv ρ) (l : Option Label) :
    ∀ r : Row, GroundRow r → NotBotRow r →
      projOpt (evalRow ρ r) l = rowFieldAt (gOf ρ l) l r
  | .simple fs u, hg, _ => by
      show projOpt (normalize (substFields ρ fs) (substBase ρ u)) l = projOpt (normalize fs u) l
      rw [substFields_fixed ρ hg.1, hg.2 ρ]
  | .splat es, hg, hnb => projOpt_collapseSplat_subst ρ hρ l es hg hnb

/-- The bridge at an ordinary label. -/
theorem proj_evalRow (ρ : GEnv) (hρ : NotBotEnv ρ) (l : Label) (r : Row)
    (hg : GroundRow r) (hnb : NotBotRow r) :
    proj (evalRow ρ r) l = rowFieldAt (gOf ρ (some l)) (some l) r :=
  projOpt_evalRow ρ hρ (some l) r hg hnb

/- **Evaluation congruence.**  The collapse reads `ρ` only at the parameters the row mentions (for a
   `Ground` row); the junk arms are `ρ`-independent. -/
mutual
  theorem collapseBase_substBase_congr {ρ ρ' : GEnv} :
      ∀ b : Base, GroundSpread b → (∀ α, baseMentions b α → ρ α = ρ' α) →
        collapseBase (substBase ρ b) = collapseBase (substBase ρ' b)
    | .rigid β,              _,  h => by
        show collapseBase (ρ β) = collapseBase (ρ' β); rw [h β rfl]
    | .bot,                  _,  _ => rfl
    | .top,                  _,  _ => rfl
    | .prim _,               _,  _ => rfl
    | .union _ _,            _,  _ => rfl
    | .shape (.simple fs u), hg, _ => by
        show collapseRow (.simple (substFields ρ fs) (substBase ρ u))
           = collapseRow (.simple (substFields ρ' fs) (substBase ρ' u))
        rw [substFields_fixed ρ hg.1, substFields_fixed ρ' hg.1, hg.2 ρ, hg.2 ρ']
    | .shape (.splat es),    hg, h => collapseSplat_substElems_congr es hg h

  theorem collapseElem_substElem_congr {ρ ρ' : GEnv} :
      ∀ e : SplatElem, GroundElem e → (∀ α, elemMentions e α → ρ α = ρ' α) →
        collapseElem (substElem ρ e) = collapseElem (substElem ρ' e)
    | .spread b, hg, h => collapseBase_substBase_congr b hg h

  theorem collapseSplat_substElems_congr {ρ ρ' : GEnv} :
      ∀ es : List SplatElem, GroundElems es → (∀ α, splatMentions es α → ρ α = ρ' α) →
        collapseSplat (substElems ρ es) = collapseSplat (substElems ρ' es)
    | [],      _,  _ => rfl
    | e :: es, hg, h =>
        collapseFold_substElems_congr es hg.2 (fun α ha => h α (Or.inr ha))
          (collapseElem (substElem ρ e)) (collapseElem (substElem ρ' e))
          (collapseElem_substElem_congr e hg.1 (fun α ha => h α (Or.inl ha)))

  theorem collapseFold_substElems_congr {ρ ρ' : GEnv} :
      ∀ (es : List SplatElem), GroundElems es → (∀ α, splatMentions es α → ρ α = ρ' α) →
        ∀ (acc acc' : SimpleRow), acc = acc' →
        collapseFold (substElems ρ es) acc = collapseFold (substElems ρ' es) acc'
    | [],      _,  _, _,   _,    hacc => hacc
    | e :: es, hg, h, acc, acc', hacc =>
        collapseFold_substElems_congr es hg.2 (fun α ha => h α (Or.inr ha))
          (mergeSimple acc (collapseElem (substElem ρ e)))
          (mergeSimple acc' (collapseElem (substElem ρ' e)))
          (by rw [hacc, collapseElem_substElem_congr e hg.1 (fun α ha => h α (Or.inl ha))])
end

/-- **Evaluation congruence (ground rows).** -/
theorem evalRow_congr {ρ ρ' : GEnv} (r : Row) (hg : GroundRow r)
    (h : ∀ α, rowMentions r α → ρ α = ρ' α) : evalRow ρ r = evalRow ρ' r := by
  cases r with
  | simple fs u =>
      show collapseRow (.simple (substFields ρ fs) (substBase ρ u))
         = collapseRow (.simple (substFields ρ' fs) (substBase ρ' u))
      rw [substFields_fixed ρ hg.1, substFields_fixed ρ' hg.1, hg.2 ρ, hg.2 ρ']
  | splat es => exact collapseSplat_substElems_congr es hg h

/- ---------------------------------------------------------------------------------------------- -/
/- ## 58. The topological env builder                                                             -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The upper-bound row of `T` (a `⊤`-row if undeclared). -/
def ubRow (Γ : TyParamEnv) (T : Ty_param) : Row :=
  match Γ T with
  | some b => rowOfBase b.upper
  | none   => .simple [] .top

/-- The upper-bound row mentions only the parameters `T` depends on. -/
theorem rowMentions_ubRow {Γ : TyParamEnv} {T α : Ty_param} (h : rowMentions (ubRow Γ T) α) :
    DependsOn Γ T α := by
  cases hb : Γ T with
  | none   => rw [ubRow, hb] at h; cases h
  | some b => rw [ubRow, hb] at h; exact ⟨b, hb, Or.inr h⟩

/-- The witness base for `T`: its upper-bound row (evaluated under `ρ`) with the field at `l₀`
    overridden to the corner `c T`.  Collapses to `rowUpdate (evalRow ρ ubRow) l₀ (c T)`. -/
def witnessBase (Γ : TyParamEnv) (l₀ : Label) (c : Ty_param → FieldDesc) (ρ : GEnv) (T : Ty_param) :
    Base :=
  .shape (.simple ((l₀, c T) :: (evalRow ρ (ubRow Γ T)).known) (evalRow ρ (ubRow Γ T)).unknown)

theorem collapseBase_witnessBase (Γ : TyParamEnv) (l₀ : Label) (c : Ty_param → FieldDesc) (ρ : GEnv)
    (T : Ty_param) :
    collapseBase (witnessBase Γ l₀ c ρ T) = rowUpdate (evalRow ρ (ubRow Γ T)) l₀ (c T) := rfl

/-- **Topological env builder.**  Fold over `Ts`, setting each `T` to its `witnessBase` under the
    prefix already built. -/
def buildEnv (Γ : TyParamEnv) (l₀ : Label) (c : Ty_param → FieldDesc) : List Ty_param → GEnv → GEnv
  | [],      ρ => ρ
  | T :: Ts, ρ => buildEnv Γ l₀ c Ts (Function.update ρ T (witnessBase Γ l₀ c ρ T))

theorem buildEnv_cons (Γ : TyParamEnv) (l₀ : Label) (c : Ty_param → FieldDesc) (T : Ty_param)
    (Ts : List Ty_param) (ρ : GEnv) :
    buildEnv Γ l₀ c (T :: Ts) ρ
      = buildEnv Γ l₀ c Ts (Function.update ρ T (witnessBase Γ l₀ c ρ T)) := rfl

/-- The builder leaves parameters outside the list untouched. -/
theorem buildEnv_notMem (Γ : TyParamEnv) (l₀ : Label) (c : Ty_param → FieldDesc) :
    ∀ (Ts : List Ty_param) (ρ : GEnv) (T : Ty_param), T ∉ Ts → buildEnv Γ l₀ c Ts ρ T = ρ T
  | [],      _, _, _  => rfl
  | U :: Us, ρ, T, hT => by
      rw [List.mem_cons, not_or] at hT
      rw [buildEnv_cons, buildEnv_notMem Γ l₀ c Us _ T hT.2, Function.update_of_ne hT.1]

/-- **Self-consistency of the builder.**  For `TopoSorted` (acyclic) ground upper bounds, the finalized
    value of each `T ∈ Ts` collapses to its upper bound *under the full env* — later updates do not
    touch the parameters `T`'s bound mentions (`rowMentions_ubRow` + `TopoSorted` + `evalRow_congr`). -/
theorem buildEnv_self (Γ : TyParamEnv) (l₀ : Label) (c : Ty_param → FieldDesc) :
    ∀ (Ts : List Ty_param), Ts.Nodup → TopoSorted Γ Ts → (∀ U ∈ Ts, GroundRow (ubRow Γ U)) →
      ∀ (ρ : GEnv) (T : Ty_param), T ∈ Ts →
        collapseBase (buildEnv Γ l₀ c Ts ρ T)
          = rowUpdate (evalRow (buildEnv Γ l₀ c Ts ρ) (ubRow Γ T)) l₀ (c T)
  | U :: Us, hnodup, htopo, hground, ρ, T, hT => by
    have hUnotin : U ∉ Us := (List.nodup_cons.mp hnodup).1
    have hnodup' : Us.Nodup := (List.nodup_cons.mp hnodup).2
    obtain ⟨htopoHd, htopoTl⟩ : (∀ α, DependsOn Γ U α → α ∉ U :: Us) ∧ TopoSorted Γ Us := htopo
    rcases List.mem_cons.mp hT with rfl | hTUs
    · -- head: value set now; the bound's eval is unchanged under the full env
      rw [buildEnv_cons, buildEnv_notMem Γ l₀ c Us _ T hUnotin, Function.update_self,
          collapseBase_witnessBase]
      have heq : evalRow ρ (ubRow Γ T)
               = evalRow (buildEnv Γ l₀ c Us (Function.update ρ T (witnessBase Γ l₀ c ρ T)))
                   (ubRow Γ T) :=
        evalRow_congr (ubRow Γ T) (hground T List.mem_cons_self) (fun α hα => by
          have hnotin := htopoHd α (rowMentions_ubRow hα)
          rw [List.mem_cons, not_or] at hnotin
          rw [buildEnv_notMem Γ l₀ c Us _ α hnotin.2, Function.update_of_ne hnotin.1])
      rw [heq]
    · -- below the head: IH on `Us` (the full env coincides with this prefix's env)
      rw [buildEnv_cons,
          buildEnv_self Γ l₀ c Us hnodup' htopoTl
            (fun U' hU' => hground U' (List.mem_cons_of_mem _ hU')) _ T hTUs]

/- ---------------------------------------------------------------------------------------------- -/
/- ## 59. The witness is non-bottom; merge preserves rigid-freeness                               -/
/- ---------------------------------------------------------------------------------------------- -/

/-- `mergeField` preserves rigid-freeness (`substField` is a `mergeField` homomorphism). -/
theorem mergeField_rigidFree {a b : FieldDesc} (ha : RigidFreeField a) (hb : RigidFreeField b) :
    RigidFreeField (mergeField a b) := by
  intro ρ'; rw [substField_mergeField, ha ρ', hb ρ']

/-- Every `witnessBase` is a *simple-row shape*, hence non-bottom. -/
theorem notBotBase_witnessBase (Γ : TyParamEnv) (l₀ : Label) (c : Ty_param → FieldDesc) (ρ : GEnv)
    (T : Ty_param) : NotBotBase (witnessBase Γ l₀ c ρ T) := by
  unfold witnessBase; exact trivial

/-- **The builder preserves `NotBotEnv`** — each parameter it touches becomes a (non-bottom)
    simple-row shape; the rest keep `ρ`'s value. -/
theorem notBotEnv_buildEnv (Γ : TyParamEnv) (l₀ : Label) (c : Ty_param → FieldDesc) :
    ∀ (Ts : List Ty_param) (ρ : GEnv), NotBotEnv ρ → NotBotEnv (buildEnv Γ l₀ c Ts ρ)
  | [],      ρ, hρ => hρ
  | T :: Ts, ρ, hρ => by
      rw [buildEnv_cons]
      refine notBotEnv_buildEnv Γ l₀ c Ts _ ?_
      intro α
      by_cases hα : α = T
      · rw [hα, Function.update_self]; exact notBotBase_witnessBase Γ l₀ c ρ T
      · rw [Function.update_of_ne hα]; exact hρ α

/- ---------------------------------------------------------------------------------------------- -/
/- ## 60. Coupled completeness                                                                    -/
/- ---------------------------------------------------------------------------------------------- -/

/-- The witness realizes the corner: its field at `l₀` for a `Ts`-parameter is exactly `c T`. -/
theorem gOf_buildEnv (Γ : TyParamEnv) (l₀ : Label) (c : Ty_param → FieldDesc) (Ts : List Ty_param)
    (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (hground : ∀ U ∈ Ts, GroundRow (ubRow Γ U))
    (ρ : GEnv) (hnbρ : NotBotEnv (buildEnv Γ l₀ c Ts ρ)) {T : Ty_param} (hT : T ∈ Ts) :
    gOf (buildEnv Γ l₀ c Ts ρ) (some l₀) T = c T := by
  show baseFieldAt (fun _ => .opt .bot) (some l₀) (buildEnv Γ l₀ c Ts ρ T) = c T
  rw [← projOpt_collapseBase (some l₀) _ (hnbρ T)]
  show proj (collapseBase (buildEnv Γ l₀ c Ts ρ T)) l₀ = c T
  rw [buildEnv_self Γ l₀ c Ts hnodup htopo hground ρ T hT, proj_rowUpdate, if_pos rfl]

/-- **Coupled completeness.**  For `Ground`, non-bottom rows and non-bottom bounds, `SemSubRow`
    forces the per-label goal at a corner `c` that lies (at `l₀`) in each parameter's coupled box.
    Realize `c` by the topological `buildEnv` witness — every witness base is a (non-bottom)
    simple-row shape (`notBotEnv_buildEnv`), so the collapse-projection bridge (`proj_evalRow`)
    applies; `Compatible` follows (upper bound by `rowUpdate` refinement at `l₀`, lower elsewhere by
    consistency `hconsist`) — then read `SemSubRow` off at `l₀` and rewrite by relevance
    (`rowFieldAt_congr` + `gOf_buildEnv`). -/
theorem subrow_corner_complete_coupled (r p : Row) (Γ : TyParamEnv) (Ts : List Ty_param) (l₀ : Label)
    (c : Ty_param → FieldDesc) (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (hclosed : BoundClosed Γ Ts)
    (_hr : GroundRow r) (_hp : GroundRow p)
    (hTs_all : ∀ α b, Γ α = some b → α ∈ Ts)
    (hub_ground : ∀ U ∈ Ts, GroundRow (ubRow Γ U))
    (hlb_ground : ∀ T b, Γ T = some b → GroundRow (rowOfBase b.lower))
    (hnb_l : ∀ T b, Γ T = some b → NotBotRow (rowOfBase b.lower))
    (hnb_u : ∀ T b, Γ T = some b → NotBotRow (rowOfBase b.upper))
    (hconsist : ∀ T b, Γ T = some b → ∀ ρ,
      SubField_row (evalRow ρ (rowOfBase b.lower)) (evalRow ρ (rowOfBase b.upper)))
    (hcbox : ∀ T b, Γ T = some b →
      SubField (rowFieldAt c (some l₀) (rowOfBase b.lower)) (c T) ∧
      SubField (c T) (rowFieldAt c (some l₀) (rowOfBase b.upper)))
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (hsem : SemSubRow r p Γ) :
    SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p) := by
  set ρc : GEnv := buildEnv Γ l₀ c Ts (fun _ => .shape (.simple [] .bot)) with hρc
  have hnbρ : NotBotEnv ρc := by
    rw [hρc]; exact notBotEnv_buildEnv Γ l₀ c Ts _ (fun _ => trivial)
  have hgOf : ∀ T ∈ Ts, gOf ρc (some l₀) T = c T := by
    intro T hT; rw [hρc]; exact gOf_buildEnv Γ l₀ c Ts hnodup htopo hub_ground _ (hρc ▸ hnbρ) hT
  -- the witness's projection of a ground, non-bottom, `Ts`-mentioning row at `l₀` equals `rowFieldAt c`
  have hbridge : ∀ bd : Row, GroundRow bd → NotBotRow bd → (∀ α, rowMentions bd α → α ∈ Ts) →
      proj (evalRow ρc bd) l₀ = rowFieldAt c (some l₀) bd := by
    intro bd hbdg hbdnb hbdrel
    rw [proj_evalRow ρc hnbρ l₀ bd hbdg hbdnb]
    exact rowFieldAt_congr (some l₀) bd (fun α hα => hgOf α (hbdrel α hα))
  -- bound rows mention only `Ts` (`BoundClosed`)
  have hboundrel : ∀ T b, Γ T = some b → T ∈ Ts →
      (∀ α, rowMentions (rowOfBase b.lower) α → α ∈ Ts) ∧
      (∀ α, rowMentions (rowOfBase b.upper) α → α ∈ Ts) :=
    fun T b hb hT => ⟨fun α hα => hclosed T hT α ⟨b, hb, Or.inl hα⟩,
                      fun α hα => hclosed T hT α ⟨b, hb, Or.inr hα⟩⟩
  -- the witness is `Compatible`
  have hcompat : Compatible ρc Γ := by
    intro T b hb l
    have hT : T ∈ Ts := hTs_all T b hb
    obtain ⟨hrelL, hrelU⟩ := hboundrel T b hb hT
    have hub_g : GroundRow (rowOfBase b.upper) := by simpa only [ubRow, hb] using hub_ground T hT
    -- `gOf` at `T`: the upper bound (evaluated under `ρc`), overridden at `l₀`
    have hgl : gOf ρc (some l) T = proj (rowUpdate (evalRow ρc (rowOfBase b.upper)) l₀ (c T)) l := by
      show baseFieldAt (fun _ => .opt .bot) (some l) (ρc T) = _
      rw [← projOpt_collapseBase (some l) _ (hnbρ T)]
      show proj (collapseBase (ρc T)) l = _
      rw [hρc, buildEnv_self Γ l₀ c Ts hnodup htopo hub_ground _ T hT]
      simp only [ubRow, hb]
    show SubField (rowFieldAt (gOf ρc (some l)) (some l) (rowOfBase b.lower)) (gOf ρc (some l) T) ∧
         SubField (gOf ρc (some l) T) (rowFieldAt (gOf ρc (some l)) (some l) (rowOfBase b.upper))
    rw [← proj_evalRow ρc hnbρ l (rowOfBase b.lower) (hlb_ground T b hb) (hnb_l T b hb),
        ← proj_evalRow ρc hnbρ l (rowOfBase b.upper) hub_g (hnb_u T b hb), hgl, proj_rowUpdate]
    obtain ⟨hb1, hb2⟩ := hcbox T b hb
    by_cases hl : l = l₀
    · subst hl
      rw [if_pos rfl, hbridge (rowOfBase b.lower) (hlb_ground T b hb) (hnb_l T b hb) hrelL,
          hbridge (rowOfBase b.upper) hub_g (hnb_u T b hb) hrelU]
      exact ⟨hb1, hb2⟩
    · rw [if_neg hl]
      exact ⟨(hconsist T b hb ρc).at l, subField_refl _⟩
  -- read `SemSubRow` off at `l₀`, then rewrite both per-label fields to `rowFieldAt c` by relevance
  have key := hsem ρc hcompat l₀
  show SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p)
  rw [← rowFieldAt_congr (some l₀) r (fun α hα => hgOf α (hrelr α hα)),
      ← rowFieldAt_congr (some l₀) p (fun α hα => hgOf α (hrelp α hα))]
  exact key

/- ---------------------------------------------------------------------------------------------- -/
/- ## 61. The per-label field of a ground row under a rigid-free assignment is rigid-free          -/
/- ---------------------------------------------------------------------------------------------- -/

/-- Projecting a row with rigid-free known fields and rigid-free `unknown` base gives a rigid-free
    field. -/
theorem projOpt_rigidFree {R : SimpleRow} (hk : GroundFields R.known) (hu : RigidFreeBase R.unknown)
    (l : Option Label) : RigidFreeField (projOpt R l) := by
  cases l with
  | none   => exact rigidFreeField_opt hu
  | some _ => exact rigidFreeField_proj hk hu _

/- The per-label field of a `Ground` row, under an assignment that is rigid-free everywhere, is
   rigid-free: spread parameters contribute `g α` (rigid-free), inline rows project to rigid-free
   fields (`Ground`), and the fold preserves it (`mergeField_rigidFree`). -/
mutual
  theorem baseFieldAt_rigidFree {g : Ty_param → FieldDesc} (hg : ∀ α, RigidFreeField (g α))
      (l : Option Label) : ∀ b : Base, GroundSpread b → RigidFreeField (baseFieldAt g l b)
    | .rigid α,              _  => hg α
    | .bot,                  _  => fun _ => rfl
    | .top,                  _  => fun _ => rfl
    | .prim _,               _  => fun _ => rfl
    | .union _ _,            _  => fun _ => rfl
    | .shape (.simple _ _), hgr => projOpt_rigidFree (groundFields_normalize hgr.1 hgr.2) hgr.2 l
    | .shape (.splat es),   hgr => splatFieldAt_rigidFree hg l es hgr

  theorem elemFieldAt_rigidFree {g : Ty_param → FieldDesc} (hg : ∀ α, RigidFreeField (g α))
      (l : Option Label) : ∀ e : SplatElem, GroundElem e → RigidFreeField (elemFieldAt g l e)
    | .spread b, hgr => baseFieldAt_rigidFree hg l b hgr

  theorem splatFieldAt_rigidFree {g : Ty_param → FieldDesc} (hg : ∀ α, RigidFreeField (g α))
      (l : Option Label) : ∀ es : List SplatElem, GroundElems es → RigidFreeField (splatFieldAt g l es)
    | [],      _   => fun _ => rfl
    | e :: es, hgr =>
        splatFoldAt_rigidFree hg l es hgr.2 (elemFieldAt g l e) (elemFieldAt_rigidFree hg l e hgr.1)

  theorem splatFoldAt_rigidFree {g : Ty_param → FieldDesc} (hg : ∀ α, RigidFreeField (g α))
      (l : Option Label) : ∀ (es : List SplatElem), GroundElems es → ∀ (acc : FieldDesc),
        RigidFreeField acc → RigidFreeField (splatFoldAt g l es acc)
    | [],      _,   _,   hacc => hacc
    | e :: es, hgr, acc, hacc =>
        splatFoldAt_rigidFree hg l es hgr.2 (mergeField acc (elemFieldAt g l e))
          (mergeField_rigidFree hacc (elemFieldAt_rigidFree hg l e hgr.1))
end

/-- **The per-label field of a ground row under a rigid-free assignment is rigid-free.** -/
theorem rowFieldAt_rigidFree {g : Ty_param → FieldDesc} (hg : ∀ α, RigidFreeField (g α))
    (l : Option Label) (r : Row) (hgr : GroundRow r) : RigidFreeField (rowFieldAt g l r) := by
  cases r with
  | simple _ _ => exact projOpt_rigidFree (groundFields_normalize hgr.1 hgr.2) hgr.2 l
  | splat es   => exact splatFieldAt_rigidFree hg l es hgr

/- ---------------------------------------------------------------------------------------------- -/
/- ## 62. Enumerated corners are rigid-free and in-box                                            -/
/- ---------------------------------------------------------------------------------------------- -/

/-- A coupled box (under a rigid-free assignment, ground bounds) has rigid-free endpoints. -/
theorem fieldBoundsAt_rigidFree {Γ : TyParamEnv} {fa : Ty_param → FieldDesc} {l : Option Label}
    {T : Ty_param} (hfa : ∀ α, RigidFreeField (fa α))
    (hbg : ∀ b, Γ T = some b → GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper)) :
    RigidFreeField (fieldBoundsAt Γ fa l T).1 ∧ RigidFreeField (fieldBoundsAt Γ fa l T).2 := by
  cases hb : Γ T with
  | none   => simp only [fieldBoundsAt, hb]; exact ⟨fun _ => rfl, fun _ => rfl⟩
  | some b =>
    simp only [fieldBoundsAt, hb]
    obtain ⟨hl, hu⟩ := hbg b hb
    exact ⟨rowFieldAt_rigidFree hfa l _ hl, rowFieldAt_rigidFree hfa l _ hu⟩

variable [∀ a b : Base, Decidable (SubBase a b)]

/-- **Enumerated corners are rigid-free and in their (own) box.**  Every corner the topological
    enumeration produces is rigid-free (corner of a rigid-free box, §61 + §59 helpers) and lies in its
    coupled box computed under itself — the box being `TopoSorted`-stable: it reads only `T`'s
    dependencies, which lie outside `T :: rest` (so the corner equals the initial assignment there,
    `coupledCornerAssigns_off`), letting `rowFieldAt_congr` rewrite box@`fa` to box@`c`. -/
theorem coupledCornerAssigns_complete_props (Γ : TyParamEnv) (l₀ : Label) :
    ∀ (Ts : List Ty_param), Ts.Nodup → TopoSorted Γ Ts →
      (∀ T b, Γ T = some b → GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper)) →
      ∀ (fa : Ty_param → FieldDesc), (∀ α, RigidFreeField (fa α)) →
      ∀ (c : Ty_param → FieldDesc), c ∈ coupledCornerAssigns Γ (some l₀) Ts fa →
        (∀ T ∈ Ts, RigidFreeField (c T)) ∧
        (∀ T b, Γ T = some b → T ∈ Ts →
          SubField (rowFieldAt c (some l₀) (rowOfBase b.lower)) (c T) ∧
          SubField (c T) (rowFieldAt c (some l₀) (rowOfBase b.upper)))
  | [],        _,      _,     _,   _,  _,   c, _  =>
      ⟨fun T hT => absurd hT (by simp), fun T b _ hT => absurd hT (by simp)⟩
  | T :: rest, hnodup, htopo, hbg, fa, hfa, c, hc => by
    have hTnotin : T ∉ rest := (List.nodup_cons.mp hnodup).1
    have hnodup' : rest.Nodup := (List.nodup_cons.mp hnodup).2
    obtain ⟨htopoHd, htopoTl⟩ : (∀ α, DependsOn Γ T α → α ∉ T :: rest) ∧ TopoSorted Γ rest := htopo
    rw [coupledCornerAssigns, List.mem_flatMap] at hc
    obtain ⟨d, hd, hcd⟩ := hc
    -- the chosen corner equals `c T`, and `c` agrees with `fa` outside `T :: rest`
    have hcT : c T = d := by
      rw [coupledCornerAssigns_off Γ (some l₀) rest _ c hcd T hTnotin, Function.update_self]
    have hcfa : ∀ α, α ∉ T :: rest → c α = fa α := by
      intro α hα
      rw [List.mem_cons, not_or] at hα
      rw [coupledCornerAssigns_off Γ (some l₀) rest _ c hcd α hα.2, Function.update_of_ne hα.1]
    have hboxrf := fieldBoundsAt_rigidFree (l := some l₀) hfa (fun b hb => hbg T b hb)
    have hcTrf : RigidFreeField (c T) := hcT ▸ rigidFreeField_of_mem_rigidCorners hboxrf.1 hboxrf.2 hd
    -- recurse on the rest (the updated assignment is still rigid-free)
    have hfa' : ∀ α, RigidFreeField (Function.update fa T d α) := by
      intro α; by_cases hα : α = T
      · subst hα; rw [Function.update_self]; exact hcT ▸ hcTrf
      · rw [Function.update_of_ne hα]; exact hfa α
    obtain ⟨ihcg, ihcbox⟩ := coupledCornerAssigns_complete_props Γ l₀ rest hnodup' htopoTl hbg
      (Function.update fa T d) hfa' c hcd
    refine ⟨fun U hU => (List.mem_cons.mp hU).elim (fun h => h ▸ hcTrf) (ihcg U), fun U b hb hU => ?_⟩
    rcases List.mem_cons.mp hU with rfl | hUrest
    · -- head: box@`c` = box@`fa` (bound mentions are external), corner in box by `mem_rigidCorners`
      have hbox : fieldBoundsAt Γ fa (some l₀) U
                = (rowFieldAt c (some l₀) (rowOfBase b.lower),
                   rowFieldAt c (some l₀) (rowOfBase b.upper)) := by
        simp only [fieldBoundsAt, hb]
        rw [rowFieldAt_congr (some l₀) (rowOfBase b.lower)
              (fun α hα => (hcfa α (htopoHd α ⟨b, hb, Or.inl hα⟩)).symm),
            rowFieldAt_congr (some l₀) (rowOfBase b.upper)
              (fun α hα => (hcfa α (htopoHd α ⟨b, hb, Or.inr hα⟩)).symm)]
      obtain ⟨hb1, hb2⟩ := mem_rigidCorners hd
      rw [hbox] at hb1 hb2
      exact hcT ▸ ⟨hb1, hb2⟩
    · exact ihcbox U b hb hUrest

/- ---------------------------------------------------------------------------------------------- -/
/- ## 63. The coupled decidability characterization                                               -/
/- ---------------------------------------------------------------------------------------------- -/

/-- **The coupled characterization.**  For `Ground` rows over `TopoSorted`/`BoundClosed` ground
    (coupled) bounds with `Ts` = the declared parameters, `SemSubRow r p Γ` is equivalent to the finite
    corner check: at each label in the finite list (rows' + bounds' labels, plus a fresh `lf`), the
    per-label goal holds at every assignment the coupled enumeration produces from a fixed rigid-free
    default.  `→` is completeness (§60 `subrow_corner_complete_coupled`, the enumerated corners being
    rigid-free + in-box by §62); `←` is soundness (§56 `semSubRow_of_coupledCorners_finite`). -/
theorem semSubRow_iff_coupledCorners (r p : Row) (Γ : TyParamEnv) (Ts : List Ty_param)
    (dflt : Ty_param → FieldDesc) (lf : Label)
    (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (hclosed : BoundClosed Γ Ts)
    (hr : GroundRow r) (hp : GroundRow p)
    (hdecl : ∀ U ∈ Ts, ∃ bs, Γ U = some bs) (hTs_all : ∀ α b, Γ α = some b → α ∈ Ts)
    (hground : ∀ T b, Γ T = some b → GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper))
    (hnb : ∀ T b, Γ T = some b → NotBotRow (rowOfBase b.lower) ∧ NotBotRow (rowOfBase b.upper))
    (hcons : ∀ (l : Option Label) (U : Ty_param) (acc' : Ty_param → FieldDesc),
      SubBase (fieldBoundsAt Γ acc' l U).1.base (fieldBoundsAt Γ acc' l U).2.base)
    (hconsist : ∀ T b, Γ T = some b → ∀ ρ,
      SubField_row (evalRow ρ (rowOfBase b.lower)) (evalRow ρ (rowOfBase b.upper)))
    (hdflt : ∀ α, RigidFreeField (dflt α))
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts)
    (hlf : lf ∉ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts) :
    SemSubRow r p Γ ↔
      ∀ l₀ ∈ rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts ++ [lf],
        ∀ c ∈ coupledCornerAssigns Γ (some l₀) Ts dflt,
          SubField (rowFieldAt c (some l₀) r) (rowFieldAt c (some l₀) p) := by
  have hub_ground : ∀ U ∈ Ts, GroundRow (ubRow Γ U) := by
    intro U hU; obtain ⟨b, hb⟩ := hdecl U hU; simp only [ubRow, hb]; exact (hground U b hb).2
  constructor
  · -- completeness: each enumerated corner is realized by the topological witness
    intro hsem _l₀ _ c hcmem
    obtain ⟨_, hcbox⟩ :=
      coupledCornerAssigns_complete_props Γ _l₀ Ts hnodup htopo hground dflt hdflt c hcmem
    exact subrow_corner_complete_coupled r p Γ Ts _l₀ c hnodup htopo hclosed hr hp hTs_all hub_ground
      (fun T b hb => (hground T b hb).1) (fun T b hb => (hnb T b hb).1) (fun T b hb => (hnb T b hb).2)
      hconsist (fun T b hb => hcbox T b hb (hTs_all T b hb)) hrelr hrelp hsem
  · -- soundness: the finite check
    intro H
    exact semSubRow_of_coupledCorners_finite r p Γ Ts dflt lf hnodup htopo hclosed hr hp hdecl
      hcons hground hrelr hrelp hlf H

/-- **Coupled decidability.**  Given the F-sub leaf decider `[∀ a b, Decidable (SubBase a b)]`,
    `SemSubRow` is decidable for `Ground` rows over `TopoSorted`/`BoundClosed` ground (coupled) bounds:
    `semSubRow_iff_coupledCorners` reduces it to a finite, concrete per-label per-corner `SubField`
    check.  Noncomputable only because the fresh label is chosen classically. -/
noncomputable def decidableSemSubRow_coupled (r p : Row) (Γ : TyParamEnv) (Ts : List Ty_param)
    (dflt : Ty_param → FieldDesc)
    (hnodup : Ts.Nodup) (htopo : TopoSorted Γ Ts) (hclosed : BoundClosed Γ Ts)
    (hr : GroundRow r) (hp : GroundRow p)
    (hdecl : ∀ U ∈ Ts, ∃ bs, Γ U = some bs) (hTs_all : ∀ α b, Γ α = some b → α ∈ Ts)
    (hground : ∀ T b, Γ T = some b → GroundRow (rowOfBase b.lower) ∧ GroundRow (rowOfBase b.upper))
    (hnb : ∀ T b, Γ T = some b → NotBotRow (rowOfBase b.lower) ∧ NotBotRow (rowOfBase b.upper))
    (hcons : ∀ (l : Option Label) (U : Ty_param) (acc' : Ty_param → FieldDesc),
      SubBase (fieldBoundsAt Γ acc' l U).1.base (fieldBoundsAt Γ acc' l U).2.base)
    (hconsist : ∀ T b, Γ T = some b → ∀ ρ,
      SubField_row (evalRow ρ (rowOfBase b.lower)) (evalRow ρ (rowOfBase b.upper)))
    (hdflt : ∀ α, RigidFreeField (dflt α))
    (hrelr : ∀ α, rowMentions r α → α ∈ Ts) (hrelp : ∀ α, rowMentions p α → α ∈ Ts) :
    Decidable (SemSubRow r p Γ) :=
  let h := exists_fresh_label (rowLabels r ++ rowLabels p ++ coupledBoundLabels Γ Ts)
  decidable_of_iff _
    (semSubRow_iff_coupledCorners r p Γ Ts dflt h.choose hnodup htopo hclosed hr hp hdecl hTs_all
      hground hnb hcons hconsist hdflt hrelr hrelp h.choose_spec).symm

end Splat
