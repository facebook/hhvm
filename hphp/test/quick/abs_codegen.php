<?hh

function main($a, $b, $c, $d) :mixed{
  $x = abs($a);
  $y = abs($b);
  $z = abs($c);
  $t = abs($d);

  var_dump($x);
  var_dump($y);
  var_dump($z);
  var_dump($t);
}
<<__EntryPoint>> function main_entry() :mixed{
main(5, -5, 5.5, -5.5);
main(1729382256910270464, -1729382256910270464,
     4611686018427387904, -4611686018427387904);
main(0, 0.0, -0.0, false);
main(dict[], vec[1], true, false);
}
