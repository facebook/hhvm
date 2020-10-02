<?hh // partial

function takes_array(varray $arg): void {}

function foo(vec<string> $arg): void {
  takes_array($arg);
}
