<?hh

function miter_foo(&$ar) {
  if (!is_array($ar)) {
    global $x;
    ++$x;
    $ar = array(1);
    return;
  }
  foreach ($ar as &$v) {
    miter_bar($v);
  }
}

function miter_bar(&$ar) {
  if (!is_array($ar)) {
    global $x;
    ++$x;
    $ar = array(1);
    return;
  }
  foreach ($ar as &$v) {
    miter_foo($ar);
  }
}

function array_blowup($arr) {
  $ret = array();
  for ($i = 1; $i <= count($arr); ++$i) {
    $ret[] =& array_slice($arr, 0, $i);
  }
  return $ret;
}

function repeat($fn, $accum, $k) {
  while ($k--) {
    $accum = $fn($accum);
  }
  return $accum;
}

function main() {
  $x = repeat($x ==> array_blowup($x), array(1,2,3,4,5), 3);
  miter_foo($x);
}

main();
var_dump($x);
