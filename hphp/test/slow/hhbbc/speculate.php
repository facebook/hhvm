<?hh

function test(?string $a, ?string $b) :mixed{
  echo "Foo\n";
  if (
    ($a === null && $b === null) ||
    ($a !== null && $b !== null)
  ) {
    return 0;
  }

  if ($b !== null) {
    return 42;
  } else {
    return 24;
  }
}
<<__EntryPoint>> function main(): void {
var_dump(test("a", null));
var_dump(test(null, "b"));
}
