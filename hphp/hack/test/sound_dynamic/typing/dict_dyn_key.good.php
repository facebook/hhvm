<?hh

function f(dynamic $d) : void {
  $dict = dict[];
  $dict[$d] = 1;
  hh_expect_equivalent<dict<(arraykey&dynamic), int>>($dict);
  $dict[1];
}
