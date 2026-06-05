import Shapes.BaseTy.Merge

/- ========================================================================== -/
/-! # Expressions and runtime merge

The expression language for shape splats. The key construct is `merge`,
which performs rightmost-wins combination at runtime on record values. -/
/- ========================================================================== -/

inductive Expr where
  | nat    : Nat → Expr
  | bool   : Bool → Expr
  | record : List (String × Expr) → Expr
  | field  : Expr → String → Expr
  | merge  : Expr → Expr → Expr

/- ========================================================================== -/
/-! ## Runtime merge on record entries

`mergeEntries left right` produces a record with rightmost-wins semantics:
for each key in both records, the right value wins. The result has unique
keys (assuming both inputs do).

The definition mirrors `mergeRow`: deduplicate the combined key set, then
for each key, take the rightmost value. -/
/- ========================================================================== -/

private def pickVal (left right : List (String × Val)) (k : String) : Val :=
  match right.lookup k, left.lookup k with
  | some v, _      => v
  | none,   some v => v
  | none,   none   => .nat 0  -- unreachable when k ∈ keys of left ∪ right

def mergeEntries (left right : List (String × Val)) : List (String × Val) :=
  let allKeys := (left.map Prod.fst ++ right.map Prod.fst).eraseDups
  allKeys.map fun k => (k, pickVal left right k)

/- ========================================================================== -/
/-! ## Properties of mergeEntries -/
/- ========================================================================== -/

theorem mergeEntries_nodup (left right : List (String × Val)) :
    ((mergeEntries left right).map Prod.fst).Nodup := by
  simp [mergeEntries, List.map_map]
  show ((left.map Prod.fst ++ right.map Prod.fst).eraseDups.map fun k => k).Nodup
  simp; exact eraseDups_nodup _

theorem mergeEntries_lookup_right {left right : List (String × Val)}
    {k : String} {v : Val}
    (h : right.lookup k = some v) :
    (mergeEntries left right).lookup k = some v := by
  have hk : k ∈ (left.map Prod.fst ++ right.map Prod.fst).eraseDups :=
    List.mem_eraseDups.mpr (List.mem_append_right _
      (List.mem_map.mpr ⟨(k, v), mem_of_lookup_eq_some h, rfl⟩))
  simp only [mergeEntries]
  rw [lookup_map_mem hk]
  simp [pickVal, h]

theorem mergeEntries_lookup_left {left right : List (String × Val)}
    {k : String} {v : Val}
    (hleft : left.lookup k = some v)
    (hright : right.lookup k = none) :
    (mergeEntries left right).lookup k = some v := by
  have hk : k ∈ (left.map Prod.fst ++ right.map Prod.fst).eraseDups :=
    List.mem_eraseDups.mpr (List.mem_append_left _
      (List.mem_map.mpr ⟨(k, v), mem_of_lookup_eq_some hleft, rfl⟩))
  simp only [mergeEntries]
  rw [lookup_map_mem hk]
  simp [pickVal, hright, hleft]

theorem mergeEntries_lookup_none {left right : List (String × Val)}
    {k : String}
    (hleft : left.lookup k = none)
    (hright : right.lookup k = none) :
    (mergeEntries left right).lookup k = none := by
  apply lookup_none_of_not_mem_keys
  intro hmem
  simp [mergeEntries, List.map_map] at hmem
  rcases hmem with ⟨v, hv⟩ | ⟨v, hv⟩
  · exact not_mem_map_fst_of_lookup_none hleft (List.mem_map.mpr ⟨(k, v), hv, rfl⟩)
  · exact not_mem_map_fst_of_lookup_none hright (List.mem_map.mpr ⟨(k, v), hv, rfl⟩)
