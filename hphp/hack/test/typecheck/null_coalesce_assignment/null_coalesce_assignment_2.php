<?hh // strict

function f(): ?int {
  return null;
}

function g(): int {
  $x = f();
  return $x ??= 0;
}
