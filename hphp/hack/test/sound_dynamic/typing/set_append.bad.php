<?hh

function test(dynamic $d, Set<int> $s) : void {
  $s[] = $d;
  hh_show($s);
}
