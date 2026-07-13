<?hh
function test($x) :mixed{
  try {
    $x[5] = 7; var_dump($x[5]);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    $x['a'] = 'c'; var_dump($x['a']);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    ++$x[5]; var_dump($x[5]);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    ++$x['a']; var_dump($x['a']);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    $x[5] += 3; var_dump($x[5]);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    $x['a'] .= 'd'; var_dump($x['a']);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    $x[''][0] = 1; var_dump($x[''][0]);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    $x[''][] = 1; var_dump(1);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    $x['']->prop = 123; var_dump(123);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    $t = 123; $x[] = $t; var_dump($t);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    $y = $x['a'][0]; var_dump($y);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  var_dump($x);
}
function main() :mixed{
  test(Set {5, 'a', 0, ''});
  echo "----\n";
  test(ImmSet {5, 'a', 0, ''});
}

<<__EntryPoint>>
function main_set_invalid_operations() :mixed{
main();
}
