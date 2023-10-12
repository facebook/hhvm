<?hh

function get(): int {
  return 1;
}

function f(array<int> $vec = varray[get(), 2]): void{}
