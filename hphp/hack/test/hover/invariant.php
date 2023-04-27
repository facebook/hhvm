<?hh

function foo(int $x): void {
  invariant($x === 1, "Expected x to be one!");
  // ^ hover-at-caret
}
