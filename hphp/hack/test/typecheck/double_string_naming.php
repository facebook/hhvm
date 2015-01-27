<?hh // strict

function test(array<string, string> $x): string {
  return "foo $x[bar]";
}
