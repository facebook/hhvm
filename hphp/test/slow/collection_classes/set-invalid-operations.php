<?hh
function test($x) {
  try {
    var_dump($x[5] = 7);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump($x['a'] = 'c');
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump(++$x[5]);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump(++$x['a']);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump($x[5] += 3);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump($x['a'] .= 'd');
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump($x[''][0] = 1);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump($x[''][] = 1);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump($x['']->prop = 123);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump($x[] = 123);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump($y = $x['a'][0]);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  var_dump($x);
}
function main() {
  test(Set {5, 'a', 0, ''});
  echo "----\n";
  test(ImmSet {5, 'a', 0, ''});
}
main();
