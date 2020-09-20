<?hh
function main($a, $i) {
  unset($a[$i]);
  $a[] = 'foo';
  return $a;
}
<<__EntryPoint>> function main_entry(): void {
var_dump(main(varray['a', 'b'], 1));
}
