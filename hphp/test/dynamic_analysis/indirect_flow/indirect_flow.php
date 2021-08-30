<?hh

function __source(): int {
  return 1;
}

function identity(int $input): int {
  return $input;
}

function __sink($input): void {}

<<__EntryPoint>> function main(): void {
  $data = __source();
  $derp = identity($data);
  __sink($derp);
}
