<?hh

function main($nontop) {
  function nontop(inout $x, inout $y) {
    list($x, $y) = array($y, $x);
    $bt = array_slice(debug_backtrace(), 0, 2);
    echo implode(', ',array_map($a ==> $a['function'], $bt))."\n";
    return 'foo';
  }

  $x1 = 24;
  $y1 = 42;
  $a = nontop(inout $x1, inout $y1);
  var_dump($a, $x1, $y1);

  $x2 = 18;
  $y2 = 81;
  $a = $nontop(inout $x2, inout $y2);
  var_dump($a, $x2, $y2);
}

main('nontop');
