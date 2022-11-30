<?hh

<<file: __EnableUnstableFeatures('type_refinements')>>

// Basic variance regression tests.

abstract class Box {
  abstract const type T;
}

abstract class Ok<+T1, -T2, T3> {
  abstract public function f1() : Box with {type T as T1};
  abstract public function f2() : Box with {type T super T2};
  abstract public function f3() : Box with {type T as T3};
  abstract public function f4() : Box with {type T super T3};
  abstract public function f5() : Box with {type T = T3};
}

abstract class Bad1<+T1, -T2> {
  abstract public function f1() : Box with {type T = T1};
}

abstract class Bad2<+T1, -T2> {
  abstract public function f1() : Box with {type T = T2};
}

abstract class Bad3<+T1, -T2> {
  abstract public function f1() : Box with {type T as T2};
}

abstract class Bad4<+T1, -T2> {
  abstract public function f1() : Box with {type T super T1};
}
