<?hh

function main($x, $y, $z, $t) {
  $a = (int)$x;
  $b = (int)$y;
  $c = (int)$z;
  $d = (int)$t;

  var_dump($a);
  var_dump($b);
  var_dump($c);
  var_dump($d);
}
<<__EntryPoint>> function main_entry(): void {
main(0.0, 0.5, 0.25, 0.75);
main(-0.0, -0.5, -0.25, -0.75);
main(20000000000000000000.0,
     9223372036854775807.0,
     18446744073709551615.0,
     9223372036854775806.0);
main(-20000000000000000000.0,
     -9223372036854775807.0,
     -18446744073709551615.0,
     -9223372036854775806.0);
main(NAN, -NAN, INF, -INF);
}
