<?hh

function call_it(vec<int> $v): void {
  $y = HH\Lib\C\count($v);
  //            ^ hover-at-caret
}
