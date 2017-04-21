<?hh
function try_idx($c, $k, $d) {
  try {
    var_dump(idx($c, $k, $d));
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}
function test($a) {
  echo "== ", (is_array($a) ? "array" : substr(get_class($a), 3)), " ==\n";
  try_idx($a, '0', 3);
  try_idx($a, '1', 4);
  try_idx($a, '2', 5);
  try_idx($a, 0, 6);
  try_idx($a, 1, 7);
  try_idx($a, 2, 8);
  try_idx($a, 'hello', 9);
  try_idx($a, 'world', 10);
  try_idx($a, '', 11);
  try_idx($a, 1.01, 12);
  try_idx($a, null, 13);
}
function main() {
  //test(array('1' => '2', 1 => 2, 'hello' => 'world', '' => 'empty'));
  test(Vector {'2', 2, 'world', 'empty'});
  test(ImmVector {'2', 2, 'world', 'empty'});
  test(Map {'1' => '2', 1 => 2, 'hello' => 'world', '' => 'empty'});
  test(ImmMap {'1' => '2', 1 => 2, 'hello' => 'world', '' => 'empty'});
  test(Set {'1', 1, 'hello', ''});
  test(ImmSet {'1', 1, 'hello', ''});
  test(Pair {'2', ''});
}
main();
