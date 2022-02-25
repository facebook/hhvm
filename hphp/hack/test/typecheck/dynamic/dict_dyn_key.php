<?hh

function f(dynamic $d) : void {
  $dict = dict[];
  $dict[$d] = 1;
  $dict[1];
}
