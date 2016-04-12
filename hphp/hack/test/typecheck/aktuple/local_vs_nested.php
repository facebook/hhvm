<?hh //strict

/**
 * Difference between nested and local-var tuple-like arrays
 */
function test(int $i, int $j, int $k): void {

  $a = array(4);
  $a[0] = 'aaa'; // $a is a local variable, so at this point we can forget
  // about 0 prevoiusly storing an int
  hh_show($a);
  take_string($a[0]); // no error

  $v = Vector {array(4)};
  $v[$i][0] = 'aaa'; // we don't know anything about $i and $j, so
  // $v[$j][0] can be both string and int
  hh_show($v[$j]);
  take_string($v[$j][0]); // should be an error

}

function take_string(string $_): void;
