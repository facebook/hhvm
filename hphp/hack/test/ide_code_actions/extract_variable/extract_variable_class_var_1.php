<?hh

class A {
  // we should not offer the "Extract Variable" refactor here
  int $x = /*range-start*/2222/*range-end*/;
}
