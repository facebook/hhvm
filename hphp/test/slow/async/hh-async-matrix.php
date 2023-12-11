<?hh

/* Tests the 20 function matrix of:
 *   v(),  vf(),  vm(),  vfk(),  vmk(),
 *   m(),  mf(),  mm(),  mfk(),  mmk(),
 *   vw(), vfw(), vmw(), vfkw(), vmkw(),
 *   mw(), mfw(), mmw(), mfkw(), mmkw(),
 *
 * Using static wait handles which yield results in various types
 */

<<__EntryPoint>>
function main_hh_async_matrix() :mixed{
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
  'v'  => new Vector($vals),
  'm'  => new Map($vals),
];

$callMap = dict[
  ''  => (($f, $inputs) ==>
    $f($inputs->map(async $input ==> $input))),
  'f' => (($f, $inputs) ==>
    $f($inputs, async $input ==> (bool) $input)),
  'm' => (($f, $inputs) ==>
    $f($inputs, async $input ==> var_export($input, true))),
  'fk' => (($f, $inputs) ==>
    $f($inputs, async ($k, $v) ==> (bool) $v)),
  'mk' => (($f, $inputs) ==>
    $f($inputs, async ($k, $v) ==> var_export(dict[$k => $v], true))),
];
$wrapMap = dict[
  ''  => ($r ==> $r),
  'w' => ($r ==> (is_array($r) ? array_map($o ==> $o->getResult(), $r)
                               : $r->map($o ==> $o->getResult()))),
];

foreach ($typeMap as $type => $inputs) {
  foreach ($callMap as $call => $cgen) {
    foreach ($wrapMap as $wrap => $wgen) {
      $func = "HH\\Asio\\{$type}{$call}{$wrap}";
      $wh = $cgen($func, $inputs);
      echo "{$func}() => ";
      var_dump($wgen(\HH\Asio\join($wh)));
    }
  }
}
}
