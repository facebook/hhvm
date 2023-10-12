<?hh // strict

function mixed_method(mixed $foo): void {
  $foo->bar();
}

function mixed_property(mixed $foo): void {
  $x = $foo->bar;
}

function mixed_method_null_check(mixed $foo): void {
  $foo?->bar();
}

function nonnull_method(nonnull $foo): void {
  $foo->bar();
}
