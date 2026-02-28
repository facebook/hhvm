<?hh

function foo(bool $coinflip): ?string  {
  return $coinflip ? "foo" : null;
}

function bar(string $prefix): string {
  return $prefix."bar";
}

<<__EntryPoint>>
function main(): void {
  var_dump(foo(true) |?> bar($$));
  var_dump(foo(false) |?> bar($$));
}
