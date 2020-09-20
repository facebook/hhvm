<?hh

/* Tests the 8 function matrix of:
 *   vf(), vfk(), vm(), vmk(),
 *   mf(), mfk(), mm(), mmk(),
 *
 * Using static wait handles which yield results in various types
 */

<<__EntryPoint>>
function main_hh_async_filter_map() {
$vals = varray[
  NULL,
  true,
  false,
  1,
  2.0,
  "Hello World",
  varray[3.14],
];

$typeMap = darray[
  'v'  => ($a ==> Vector::fromArray($a)),
  'm'  => ($a ==> Map::fromArray($a)),
];
$keyMap = darray[
  'f'  => (async function($v) {
    await HH\Asio\later();
    return is_scalar($v);
  }),
  'm'  => (async function($v) {
    await HH\Asio\later();
    return varray[$v];
  }),
  'fk' => (async function($k, $v) {
    await HH\Asio\later();
    return (bool)($k % 2);
  }),
  'mk' => (async function($k, $v) {
    await HH\Asio\later();
    return darray[$k => $v];
  }),
];

foreach ($typeMap as $type => $gen) {
  foreach ($keyMap as $ktype => $kgen) {
    $func = "HH\\Asio\\{$type}{$ktype}";
    $wh = $func($gen($vals), $kgen);
    echo "{$func}() => ";
    var_dump(\HH\Asio\join($wh));
  }

}
}
