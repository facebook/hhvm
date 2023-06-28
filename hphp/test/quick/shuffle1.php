<?hh

function get($a, $idx) :mixed{
  $max = 10;
  for ($i = 0; $i < $max; $i++) {
    $r = $a[$idx];
    $r = $a[0];
    $r = $a[1];
    $r = $a[2];
  }
  return $a[$idx];
}
<<__EntryPoint>> function main(): void {
$a = varray['a', 2, false];
var_dump(get($a, 1));

$a = darray['a'=>'b', 1=>'c', 2=>4, 'c'=>3, 0=>'hello'];
var_dump(get($a, 1));
}
