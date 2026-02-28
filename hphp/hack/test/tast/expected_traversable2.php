<?hh

function traversable(): Traversable<num> {
  return keyset[]; // The inferred key type should be int
}

class C {}

function keyed_traversable(): KeyedTraversable<C, mixed> {
  return dict[]; // The inferred key type should be nothing
}
