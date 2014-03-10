<?hh
function bar($x) { return !$x; }
function main() {
  $v0 = Vector {};
  $v1 = Vector {1};
  var_dump(array_filter(
    Vector {7, 0, '', '0', 'foo', true, false, $v0, $v1}, 'bar'));
  var_dump(array_filter(
    Map {11 => 7, 22 => 0, 33 => '', 44 => '0', 55 => 'foo', 66 => true,
         77 => false, 88 => $v0, 99 => $v1},
    'bar'));
  var_dump(array_filter(
    Map {11 => 7, 22 => '0', 33 => 'foo', 44 => true, 55 => $v1}, 'bar'));
  var_dump(array_filter(
    Set {7, "0", "foo"}, 'bar'));
  var_dump(array_filter(
    Pair {7, ""}, 'bar'));
  var_dump(array_filter(
    Pair {"0", "foo"}, 'bar'));
  var_dump(array_filter(
    Pair {$v0, $v1}, 'bar'));
  echo "========\n";
  var_dump(array_filter(
    Vector {7, 0, "", "0", "foo", true, false, $v0, $v1}));
  var_dump(array_filter(
    Map {11 => 7, 22 => 0, 33 => "", 44 => "0", 55 => "foo", 66 => true,
         77 => false, 88 => $v0, 99 => $v1}));
  var_dump(array_filter(
    Map {11 => 7, 22 => 0, 33 => "", 44 => "0", 55 => false, 66 => $v0}));
  var_dump(array_filter(
    Set {7, 0, "", "0"}));
  var_dump(array_filter(
    Set {0, "", "0", "foo"}));
  var_dump(array_filter(
    Pair {7, ""}));
  var_dump(array_filter(
    Pair {"0", "foo"}));
  var_dump(array_filter(
    Pair {$v0, $v1}));
}
main();

