<?hh
<<__EntryPoint>> function main(): void {
$keys = varray[null, true, false, 0, 100, 0.0, 1238.93498];

foreach ($keys as $key) {
  $a = darray[];
  $a[$key] = 123;
  var_dump($a);
  var_dump($a[$key]);
}
}
