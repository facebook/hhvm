<?hh // partial

function takes_array(array $arg): void {}

function foo(dict<int, string> $x): void {
  takes_array($x);
}
