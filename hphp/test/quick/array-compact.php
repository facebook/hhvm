<?hh
function f($a) {
  $a["four"] = 4;
  return $a;
}
<<__EntryPoint>> function main(): void {
$a = darray[1=>1, 2=>2, 3=>3];
unset($a[1]);
unset($a[2]);
unset($a[3]);
var_dump(f($a));
}
