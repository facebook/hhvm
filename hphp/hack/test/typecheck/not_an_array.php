<?hh // partial

function takes_array(varray $arg): void {}

function foo(dict<int, string> $x): void {
  takes_array($x);
}
