<?hh // partial

function foo(dict<int, string> $x): void {
  $x[] = "hello";
}
