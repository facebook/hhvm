<?hh

function id($x) :mixed{
  var_dump($x);
  return $x;
}
<<__EntryPoint>> function main(): void {
$arr = vec[null, true, false, 0, 1, 0.0, 1.0, "", "foo", vec[], vec[1]];

for ($i = 0; $i < count($arr); ++$i) {
  $x = $arr[$i];
  $y = id($x) ? id($x) : "blarg";
  echo "---------\n";
  var_dump($y);
  echo "\n\n";
}

echo "********************\n";

for ($i = 0; $i < count($arr); ++$i) {
  $x = $arr[$i];
  $y = id($x) ?: "blarg";
  echo "---------\n";
  var_dump($y);
  echo "\n\n";
}
}
