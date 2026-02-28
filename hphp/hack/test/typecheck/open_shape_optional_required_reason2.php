<?hh

type S = shape(
  'field' => string,
  ...
);

interface I<+T super shape(...)> {}

final class C implements I<S> {}
