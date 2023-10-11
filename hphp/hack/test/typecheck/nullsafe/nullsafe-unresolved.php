<?hh

function test(): void {
  $a = make_nullable_unresolved();
  $f = $a?->f();
  expect_nonnull($f);
  $b = $a?->b;
  expect_nonnull($b);
}

function make_nullable_unresolved<T>(): ?T {
  return null;
}

function expect_nonnull(Container<mixed> $container): void {}
