<?hh
function f($a) :mixed{
  $a["four"] = 4;
  return $a;
}
<<__EntryPoint>> function main(): void {
$a = dict[1=>1, 2=>2, 3=>3];
unset($a[1]);
unset($a[2]);
unset($a[3]);
var_dump(f($a));
}
