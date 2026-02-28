<?hh

function get(): int {
  return 1;
}

function f(array<int> $vec = vec[get(), 2]): void{}
