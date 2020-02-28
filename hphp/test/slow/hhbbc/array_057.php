<?hh

function unknown($x) {
  return $GLOBALS['asd'];
}

function foo($ids) {
  $x = varray[];
  foreach ($ids as $id) {
    $target = unknown($id);
    if ($target !== null) {
      if (!array_key_exists($target, $x)) $x[$target] = varray[];
      $x[$target][] = $id;
    }
  }
  return $x;
}

$asd = '2'.mt_rand();
function main() {
  $x = foo(varray[1,2,3]);
  foreach ($x as $k => $v) {
    var_dump($v);
  }
}

main();
