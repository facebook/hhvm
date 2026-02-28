<?hh

/* Tests the 8 function matrix of:
 *   vf(), vfk(), vm(), vmk(),
 *   mf(), mfk(), mm(), mmk(),
 *
 * Using static wait handles which yield results in various types
 */

<<__EntryPoint>>
function main_hh_async_filter_map() :mixed{
  $vals = vec[
    NULL,
    true,
    false,
    1,
    2.0,
    "Hello World",
    vec[3.14],
  ];

  $typeMap = dict[
    'v' => ($a ==> Vector::fromArray($a)),
    'm' => ($a ==> Map::fromArray($a)),
  ];
  $keyMap = dict[
    'f' => (
      async function($v) {
        await HH\Asio\later();
        return is_scalar($v);
      }
    ),
    'm' => (
      async function($v) {
        await HH\Asio\later();
        return vec[$v];
      }
    ),
    'fk' => (
      async function($k, $v) {
        await HH\Asio\later();
        return (bool)($k % 2);
      }
    ),
    'mk' => (
      async function($k, $v) {
        await HH\Asio\later();
        return dict[$k => $v];
      }
    ),
  ];

  $funcMap = dict[
    "vf" => HH\Asio\vf<>,
    "vm" => HH\Asio\vm<>,
    "vfk" => HH\Asio\vfk<>,
    "vmk" => HH\Asio\vmk<>,
    "mf" => HH\Asio\mf<>,
    "mm" => HH\Asio\mm<>,
    "mfk" => HH\Asio\mfk<>,
    "mmk" => HH\Asio\mmk<>,
  ];

  foreach ($typeMap as $type => $gen) {
    foreach ($keyMap as $ktype => $kgen) {
      $name = "{$type}{$ktype}";
      $wh = ($funcMap[$name])($gen($vals), $kgen);
      echo "HH\\Asio\\{$name}() => ";
      var_dump(\HH\Asio\join($wh));
    }
  }
}
