sig Elems {}
sig Set {
  elems: set Elems,
}

pred is_subset[a: Set, b: Set] {
  a.elems in b.elems
}

pred is_superset[a: Set, b: Set] {
  b.elems in a.elems
}

pred is_disjoint[a: Set, b: Set] {
  disj[a.elems, b.elems]
}

check {
  all a, b, ab, c: Set {
    {
     ab.elems = a.elems + b.elems
    } => {
      a.is_subset[c] && b.is_subset[c] => ab.is_subset[c]
      a.is_superset[c] || b.is_superset[c] => ab.is_superset[c]
      a.is_disjoint[c] && b.is_disjoint[c] => ab.is_disjoint[c]
    }
    {
     ab.elems = a.elems & b.elems
    } => {
      a.is_subset[c] || b.is_subset[c] => ab.is_subset[c]
      a.is_superset[c] && b.is_superset[c] => ab.is_superset[c]
      a.is_disjoint[c] || b.is_disjoint[c] => ab.is_disjoint[c]
    }
		{
     c.elems = Elems - a.elems
    } => {
      a.is_disjoint[b] => c.is_superset[b]
      a.is_superset[b] => c.is_disjoint[b]
    }
  }
}
