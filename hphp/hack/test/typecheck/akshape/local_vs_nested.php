<?hh //strict

/**
 * Difference between nested and local-var shape-like arrays
 */
function test(int $i, int $j, int $k): void {

  $a = array();
  $a['aaa'] = 4;
  $a['aaa'] = 'aaa'; // $a is a local variable, so at this point we can forget
  // about 'aaa' prevoiusly storing an int
  hh_show($a);
  take_string($a['aaa']); // no error

  $v = Vector {array()};
  $v[$i]['aaa'] = 4;
  $v[$j]['aaa'] = 'aaa'; // we don't know anything about $i, $j and $k, so
  // $v[$k]['aaa'] can be both string and int

  hh_show($v[$k]);
  take_string($v[$k]['aaa']); // should be an error

}

function take_string(string $_): void;
