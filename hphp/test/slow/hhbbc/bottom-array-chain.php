<?hh

function bar($a, $b) :mixed{
  return __hhvm_intrinsics\launder_value(array_key_exists($b, $a));
}

function foo($v) :mixed{
  $m = dict[];
  foreach ($v as $t) {
    $a1 = $t[0];
    $a2 = $t[1];
    if (!bar($a2, $a1)) {
      $m[$a1] = dict[];
    }
    if (bar($m[$a1], $a2)) {
      $m[$a1][$a2]++;
    } else {
      $m[$a1][$a2] = 1;
    }
  }
  return $m;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(foo(__hhvm_intrinsics\launder_value(vec[])));
}
