<?hh // strict
function takes_mutable(string ...$x): string {
  return "";
}

function test(readonly string $x): void {
  takes_mutable($x);
}
