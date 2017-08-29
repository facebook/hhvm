<?hh

function f($x) { echo $x, "\n"; }

function vecTest() {
  $x = vec[0, vec[1]];
  f($x[1][0]);
  try {
    f($x[1][2]);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
  try {
    f($x[1][2][3]);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
}

function dictTest() {
  $x = dict['a' => 'foo', 'b' => dict['a' => 1]];
  f($x['b']['a']);
  try {
    f($x['b']['b']);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
  try {
    f($x['b']['b']['b']);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
}

function keysetTest() {
  $x = keyset['a', 'b'];
  f($x['b']);
  try {
    f($x['c']);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
  try {
    f($x['c']['d']);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
}

vecTest();
dictTest();
keysetTest();
