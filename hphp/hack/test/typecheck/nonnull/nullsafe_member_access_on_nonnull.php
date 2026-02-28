<?hh

function f(nonnull $x): void {
  $_ = $x?->foo();
}
