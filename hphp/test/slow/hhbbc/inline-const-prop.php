<?hh

function addToMap(dict $d, int $a, int $b, int $c) :mixed{
  __hhvm_intrinsics\launder_value(false);
  if (array_key_exists($a, $d)) {
    $d[$a]['k1'][] = $b;
    $d[$a]['k2'][$b] = $c;
  } else {
    $d[$a] = dict[
      'k1' => keyset[$b],
      'k2' => dict[$b => $c]
    ];
  }
  return $d;
}

function makeMap($b) :mixed{
  $d = dict[];
  $d = addToMap($d, 310, 207, 1301);
  if ($b) $d = addToMap($d, 315, 204, 1305);
  return $d;
}

<<__EntryPoint>>
function main() :mixed{
  $d = makeMap(__hhvm_intrinsics\launder_value(false));
  var_dump($d[__hhvm_intrinsics\launder_value(310)]);
}
