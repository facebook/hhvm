<?hh

function test(dynamic $d, Set<arraykey> $s) : void {
  $s[] = $d;
  hh_show($s);
}
