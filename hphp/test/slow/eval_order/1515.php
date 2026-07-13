<?hh

function f($a) :mixed{
 echo "test$a\n";
 return 1;
 }
function bug2($a, $b) :mixed{
  $k1 = $a; $a++; $r = isset($b[f($k1)]);
  if ($r) { $k2 = $a; $a++; $r = isset($b[f($k2)]); }
  if ($r) { $k3 = $a; $a++; $r = isset($b[f($k3)]); }
  return $r;
}

<<__EntryPoint>>
function main_1515() :mixed{
bug2(0, vec[]);
}
