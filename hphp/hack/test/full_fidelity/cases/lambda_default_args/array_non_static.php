<?hh

function get(): int {
  return 1;
}

function test(): void {
  $_ = (array<int> $vec = vec[get(), 2]): void ==> {};
}
