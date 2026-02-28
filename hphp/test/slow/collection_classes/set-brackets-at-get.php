<?hh
function brackets($x, $k) :mixed{
  try {
    var_dump($x[$k]);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}
function do_isset($x, $k) :mixed{
  try {
    var_dump(isset($x[$k]));
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}
function do_empty($x, $k) :mixed{
  try {
    var_dump(!($x[$k] ?? false));
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}
function main() :mixed{
  $x = Set {5, 'a', 0, ''};
  $y = ImmSet {5, 'a', 0, ''};
  var_dump($x[5], $x['a'], $x[0], $x['']);
  var_dump($y[5], $y['a'], $y[0], $y['']);
  echo "----\n";
  var_dump(isset($x[5]), isset($x['a']), isset($x[0]), isset($x['']));
  var_dump(isset($y[5]), isset($y['a']), isset($y[0]), isset($y['']));
  echo "----\n";
  var_dump(!($x[5] ?? false), !($x['a'] ?? false), !($x[0] ?? false), !($x[''] ?? false));
  var_dump(!($y[5] ?? false), !($y['a'] ?? false), !($y[0] ?? false), !($y[''] ?? false));
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

<<__EntryPoint>>
function main_set_brackets_at_get() :mixed{
main();
}
