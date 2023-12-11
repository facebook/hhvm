<?hh

<<__EntryPoint>>
function main() :mixed{
  $a = vec[0];
  $a = HH\array_mark_legacy($a);
  $a[0] += 10;
  var_dump($a);
  var_dump(HH\is_array_marked_legacy($a));
  $a[] = 42;
  var_dump($a);
  var_dump(HH\is_array_marked_legacy($a));
}
