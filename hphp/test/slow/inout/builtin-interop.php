<?hh

function main() {
  $x1 = $x2 = $x3 = array(32, 8, 19, 17, 23);
  $y1 = $y2 = $y3 = array('orange' => 12, 'apple' => 0, 'banana' => 2);

  sort(inout $x1);
  asort(inout $x2);
  usort(inout $x3, ($a, $b) ==> $a < $b);
  var_dump($x1, $x2, $x3);

  ksort(inout $y1);
  asort(inout $y2);
  usort(inout $y3, ($a, $b) ==> $a < $b);
  var_dump($y1, $y2, $y3);
}

main();
