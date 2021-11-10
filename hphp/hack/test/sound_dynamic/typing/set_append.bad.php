<?hh

function test(dynamic $d, Set<int> $s) : void {
  $s[] = $d;
  hh_expect_equivalent<Set<int>>($s);
}
