<?hh

function foo(?int $a, ?string $b): bool {
  /* HH_IGNORE[12001] */ return $a === $b;
  //  ^ hover-at-caret
}
