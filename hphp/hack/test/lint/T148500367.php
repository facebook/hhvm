<?hh

function essential_nonnull1(?int $key, vec<vec<int>> $v): void {
  // redundant nonnull shouldn't fire
  $v[$key as nonnull][] = 42;
}

function essential_nonnull2(?int $key, vec<vec<int>> $v): void {
  // redundant nonnull shouldn't fire
  $v[$key as nonnull][42] = 42;
}
