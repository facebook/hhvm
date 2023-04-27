<?hh

function wildcard_vector(Traversable<int> $xs): void {
  $_ = new Vector<_>($xs);
}

function wildcard_map(KeyedTraversable<int, bool> $xs): void {
  $_ = new Map<_, _>($xs);
}
