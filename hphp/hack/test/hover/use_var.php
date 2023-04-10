<?hh

function myfoo(int $x): void {
  $_ = function() use ($x) {};
  //                   ^ hover-at-caret
}
