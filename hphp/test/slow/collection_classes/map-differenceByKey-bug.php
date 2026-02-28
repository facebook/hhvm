<?hh
function main() :mixed{
  $x = Map {};
  $y = Map {'a' => 1, 'b' => 2};
  $x->differenceByKey($y);
  var_dump($x);
}

<<__EntryPoint>>
function main_map_difference_by_key_bug() :mixed{
main();
}
