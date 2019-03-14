<?hh // partial

function takes_array(array $arg): void {}

function foo(vec<string> $arg): void {
  takes_array($arg);
}
