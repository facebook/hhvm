open util/graph[Set]

/**
 * Model for validating ApproxSet.ml
 */

check all_sets_are_wellformed {
  Set in Wellformed
} for 5

run ComplexSets{
  Set in Wellformed
  some Set.elems && !one Set.elems
  all s: Union | s.left != s.right
  all s: Inter | s.left != s.right
  some Union
  some Inter
} for 5

sig Elem {}

abstract sig Set {
  elems: set Elem,
  children: set Set,
  relate: Set -> one Relation,
}{
  // Fact: A set is considered wellformed iff its relationship
  // with all other Set is wellformed
  this in Wellformed <=> all s: Set {
    wellformed[@relate[this, s], elems, s.@elems]
  }
}

fact "Sets are not cyclic" {
  dag[children]
}

sig Wellformed in Set {}

sig Atom extends Set {}{
  // Atoms do not have children
  no children

  // We take as an assumption that the relationship between
  // Atoms themselves are wellformed
  all a: Atom {
    wellformed[@relate[this, a], elems, a.@elems]
  }

  // Defining relatin to sets
  all u: Union {
    @relate[this, u] = @relate[u, this].flip
  }
	all i: Inter {
    @relate[this, i] = @relate[i, this].flip
  }
  all c: Complement {
    @relate[this, c] = @relate[c, this].flip
  }
}

sig Union extends Set {
  left: one Set,
  right: one Set
}{
  children = left + right
  elems = left.@elems + right.@elems
  all s: Set {
   @relate[this, s] = union[@relate[left, s], @relate[right, s]]
  }
}

sig Inter extends Set {
  left: one Set,
  right: one Set
}{
  children = left + right
  elems = left.@elems & right.@elems
  all s: Set {
   @relate[this, s] = inter[@relate[left, s], @relate[right, s]]
  }
}

sig Complement extends Set {
  comp: one Atom
}{
  children = comp
  elems = Elem - comp.@elems
  all s: Set {
   @relate[this, s] = complement[@relate[comp, s]]
  }
}

abstract sig Relation {}

one sig Equal, Subset, Superset, Disjoint, Unknown extends Relation {}

pred wellformed[r:one Relation, set1: set univ, set2: set univ] {
  (r = Equal => set1 = set2)
  (r = Subset => set1 in set2)
  (r = Superset => set2 in set1)
  (r = Disjoint => disj[set1, set2])
}

fun flip[r: one Relation]: one Relation {
  r = Subset => Superset
  else r = Superset => Subset
  else r
}

fun complement[r: one Relation]: one Relation {
	match[
		r,
			Equal    -> Disjoint
		+ Subset   -> Unknown
		+ Superset -> Disjoint
		+ Disjoint -> Superset
  ]
}

fun union[r1: one Relation, r2: one Relation]: one Relation {
  match2[
		r1,
		r2,
    	Equal ->
			{ Equal    -> Equal
			+ Subset   -> Equal
			+ Superset -> Superset
			+ Disjoint -> Superset
			+ Unknown  -> Superset
			}
		+ Subset ->
			{ Subset   -> Subset
			+ Superset -> Superset
			+ Disjoint -> Unknown
			+ Unknown  -> Unknown
			}
		+ Superset ->
			{ Superset -> Superset
			+ Disjoint -> Superset
			+ Unknown  -> Superset
			}
		+ Disjoint ->
			{ Disjoint -> Disjoint
			+ Unknown -> Unknown
			}
		+ Unknown -> Unknown -> Unknown
  ]
}

fun inter[r1: one Relation, r2: one Relation]: one Relation {
  match2[
		r1,
		r2,
    	Equal ->
			{ Equal    -> Equal
			+ Subset   -> Subset
			+ Superset -> Equal
			+ Disjoint -> Disjoint
			+ Unknown  -> Subset
			}
		+ Subset ->
			{ Subset   -> Subset
			+ Superset -> Subset
			+ Disjoint -> Disjoint
			+ Unknown  -> Subset
			}
		+ Superset ->
			{ Superset -> Superset
			+ Disjoint -> Disjoint
			+ Unknown  -> Unknown
			}
		+ Disjoint ->
			{ Disjoint -> Disjoint
			+ Unknown -> Disjoint
			}
		+ Unknown -> Unknown -> Unknown
  ]
}

fun match[
	r: one Relation,
	relation: Relation -> lone Relation,
]: one Relation {
  some relation[r] => relation[r] else Unknown
}

fun match2[
	l: one Relation,
	r: one Relation,
	relation: Relation -> Relation -> lone Relation,
]: one Relation {
	one relation[l, r] => relation[l, r] else
  one relation[r, l] => relation[r, l] else
  Unknown
}
