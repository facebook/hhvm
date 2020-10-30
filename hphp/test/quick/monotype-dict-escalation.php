<?hh

<<__EntryPoint>>
function main() {
  $base = __hhvm_intrinsics\launder_value(17);
  $x = dict['a' => $base * 1, 'b' => $base * 2, 'c' => $base * 3];
  unset($x['a']);
  foreach ($x as $k => $v) {
    $x[$k] = (string)$v;
  }
  print(json_encode($x)."\n");
}
