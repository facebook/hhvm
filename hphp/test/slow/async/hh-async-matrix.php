<?hh
/* Tests the 16 function matrix of:
 *   a(),  ac(),  aw(),  acw(),
 *   v(),  vc(),  vw(),  vcw(),
 *   m(),  mc(),  mw(),  mcw(),
 *   va(), vac(), vaw(), vacw(),
 *
 * Using static wait handles which yield results in various types
 */

$vals = array(
  NULL,
  true,
  false,
  1,
  2.0,
  "Hello World",
  array(3.14),
  (object)array(4, 8, 15, 16, 23, 42),
);

$typeMap = array(
  'a'  => ($a ==> $a),
  'v'  => ($a ==> Vector::fromArray($a)),
  'm'  => ($a ==> Map::fromArray($a)),
  'va' => null,
);
$callMap = array(
  ''  => (($f, $g) ==> $f($g(array_map($v ==> HH\Asio\val($v), $vals)))),
  'c' => (($f, $g) ==> $f($k ==> HH\Asio\val($vals[$k]), array_keys($vals))),
);
$vaCallMap = array(
  ''  => (function($f, $g) use ($vals) {
    $args = array_map($v ==> HH\Asio\val($v), $vals);
    return $f(...$args);
  }),
  'c' => (function($f, $g) use ($vals) {
    $keys = array_keys($vals);
    return $f($k ==> HH\Asio\val($vals[$k]), ...$keys);
  }),
);
$wrapMap = array(
  ''  => ($r ==> $r),
  'w' => ($r ==> (is_array($r) ? array_map($o ==> $o->getResult(), $r)
                               : $r->map($o ==> $o->getResult()))),
);

foreach ($typeMap as $type => $gen) {
  $cm = ($type === 'va') ? $vaCallMap : $callMap;
  foreach ($cm as $call => $cgen) {
    foreach ($wrapMap as $wrap => $wgen) {
      $func = "HH\\Asio\\{$type}{$call}{$wrap}";
      $wh = $cgen($func, $gen);
      echo "{$func}() => ";
      var_dump($wgen($wh->join()));
    }
  }
}
