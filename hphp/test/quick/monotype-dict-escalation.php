<?hh

function escalate_keys() :mixed{
  $base = __hhvm_intrinsics\launder_value(17);
  $f = () ==> dict['a' => vec[$base * 1], 'b' => vec[$base * 2]];
  $a = json_decode('"a"');
  $c = json_decode('"c"');

  $x = $f();
  print(json_encode($x[$a])."\n");

  $x = $f();
  $x[$a][0]++;
  print(json_encode($x)."\n");

  $x = $f();
  unset($x[$a]);
  print(json_encode($x)."\n");

  $x = $f();
  $x[$c] = vec[$base * 3];
  print(json_encode($x)."\n");
}

function escalate_vals() :mixed{
  $base = __hhvm_intrinsics\launder_value(17);
  $x = dict['a' => $base * 1, 'b' => $base * 2, 'c' => $base * 3];
  unset($x['a']);
  foreach ($x as $k => $v) {
    $x[$k] = (string)$v;
  }
  print(json_encode($x)."\n");
}

<<__EntryPoint>>
function main() :mixed{
  escalate_keys();
  escalate_vals();
}
