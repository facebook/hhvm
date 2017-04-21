<?hh // strict

function foo(dict<string, string> $x): void {
  unset($x['a']);
}
