<?hh
function main() {
  $a = Pair { 10, "a" };
  var_dump($a->linearSearch(10));
  var_dump($a->linearSearch("a"));
  var_dump($a->linearSearch(null));
}

<<__EntryPoint>>
function main_pair_api() {
main();
}
