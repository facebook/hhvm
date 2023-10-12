<?hh // strict

function f(): ?int {
  return null;
}

function g(): int {
  $x = f();
  $x ??= 0;
  return $x;
}
