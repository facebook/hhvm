<?hh // strict

function test(darray<string, string> $x): string {
  return "foo $x[bar]";
}
