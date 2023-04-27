<?hh

function foo(mixed $m): void {
  $y is nonnull;
  // ^ hover-at-caret
}
