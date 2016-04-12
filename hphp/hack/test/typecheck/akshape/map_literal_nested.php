<?hh //strict

/**
 * Shape field types in dynamically set nested positions are not
 * overwritten, but grow.
 */
function test(int $i, int $j): void {
  $a = array('x' => 4);
  $b = array($a);

  take_int($b[$i]['x']);

  $b[$i]['x'] = 'zzz';

  // we don't know if $i == $j or $i != $j, so the argument to take_string
  // below can be both int or string, and should be an error
  take_string($b[$j]['x']);
}

function take_int(int $_): void {}
function take_string(int $_): void {}
