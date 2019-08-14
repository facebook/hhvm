<?hh // strict

function get(): int {
  return 1;
}

function f(array<int> $vec = array(get(), 2)): void{}
