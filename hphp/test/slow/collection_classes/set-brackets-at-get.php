<?hh
function brackets($x, $k) {
  try {
    var_dump($x[$k]);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}
function do_isset($x, $k) {
  try {
    var_dump(isset($x[$k]));
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}
function do_empty($x, $k) {
  try {
    var_dump(empty($x[$k]));
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}
function main() {
  $x = Set {5, 'a', 0, ''};
  $y = ImmSet {5, 'a', 0, ''};
  var_dump($x[5], $x['a'], $x[0], $x['']);
  var_dump($y[5], $y['a'], $y[0], $y['']);
  echo "----\n";
  var_dump(isset($x[5]), isset($x['a']), isset($x[0]), isset($x['']));
  var_dump(isset($y[5]), isset($y['a']), isset($y[0]), isset($y['']));
  echo "----\n";
  var_dump(empty($x[5]), empty($x['a']), empty($x[0]), empty($x['']));
  var_dump(empty($y[5]), empty($y['a']), empty($y[0]), empty($y['']));
  echo "----\n";
  brackets($x, null);
  brackets($x, 3);
  brackets($x, 'b');
  brackets($y, null);
  brackets($y, 3);
  brackets($y, 'b');
  echo "----\n";
  do_isset($x, null);
  do_isset($x, 3);
  do_isset($x, 'b');
  do_isset($y, null);
  do_isset($y, 3);
  do_isset($y, 'b');
  echo "----\n";
  do_empty($x, null);
  do_empty($x, 3);
  do_empty($x, 'b');
  do_empty($y, null);
  do_empty($y, 3);
  do_empty($y, 'b');
  echo "----\n";
  unset($x[5]);
  unset($x['a']);
  unset($x[123]);
  unset($x['b']);
  var_dump($x);
}
main();
