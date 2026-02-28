<?hh

function foo(vec<int> $v): void {
  HH\Lib\C\any($v);
  //       ^ hover-at-caret
}
