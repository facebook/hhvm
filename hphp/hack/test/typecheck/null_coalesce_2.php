<?hh // strict

function f(): ?int {
  return null;
}

function g(): int {
  return f() ?? 0;
}
