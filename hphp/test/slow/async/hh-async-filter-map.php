<?hh
/* Tests the 8 function matrix of:
 *   vf(), vfk(), vm(), vmk(),
 *   mf(), mfk(), mm(), mmk(),
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
  'v'  => ($a ==> Vector::fromArray($a)),
  'm'  => ($a ==> Map::fromArray($a)),
);
$keyMap = array(
  'f'  => (async function($v) {
    await HH\Asio\later();
    return is_scalar($v);
  }),
  'm'  => (async function($v) {
    await HH\Asio\later();
    return array($v);
  }),
  'fk' => (async function($k, $v) {
    await HH\Asio\later();
    return (bool)($k % 2);
  }),
  'mk' => (async function($k, $v) {
    await HH\Asio\later();
    return array($k => $v);
  }),
);

foreach ($typeMap as $type => $gen) {
  foreach ($keyMap as $ktype => $kgen) {
    $func = "HH\\Asio\\{$type}{$ktype}";
    $wh = $func($gen($vals), $kgen);
    echo "{$func}() => ";
    var_dump($wh->join());
  }

}
