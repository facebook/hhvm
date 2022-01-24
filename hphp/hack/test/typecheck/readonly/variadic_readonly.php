<?hh // strict
function takes_varargs(string ...$x): string {
  return "";
}

function bar(readonly string $ref): void {
  takes_varargs($ref); // error
  takes_varargs("a", $ref); // error
}
