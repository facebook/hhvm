<?hh

function test(dynamic $d, Set<arraykey> $s) : void {
  $s[] = $d;
  hh_expect_equivalent<Set<arraykey>>($s);
}
