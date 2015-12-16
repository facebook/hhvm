<?hh // strict

function f(): ?int {
  // UNSAFE_BLOCK
}

function g(): int {
  return f() ?? 0;
}
