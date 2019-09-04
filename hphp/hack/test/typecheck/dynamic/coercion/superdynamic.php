<?hh // strict

function foo<T super dynamic>(): T {
  return 5 as dynamic;
}

function bar(): dynamic {
  return 5;
}

function baz<T1 super dynamic as T2, T2 as T3, T3>(): ?T3 {
  return 5 as dynamic;
}
