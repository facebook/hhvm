<?hh

function __source(): int {
  return 1;
}

function __sink($input): void {}

<<__EntryPoint>>
function main(): void {
  $data = __source();
  $derp = $data;
  __sink($derp);
}
