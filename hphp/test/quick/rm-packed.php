<?hh
function main($a, $i) :mixed{
  unset($a[$i]);
  $a[] = 'foo';
  return $a;
}
<<__EntryPoint>> function main_entry(): void {
var_dump(main(vec['a', 'b'], 1));
}
