<?hh

function __source(): int {
  return 1;
}

function identity(int $input, bool $flag): int {
  return $input;
}

function __sink($input): void {}

<<__EntryPoint>> function main(): void {
  $data = __source();
  $derp = identity($data, /* flag */ false);
  __sink($derp);
}
