<?hh

function f($x) :mixed{ echo $x, "\n"; }

function vecTest() :mixed{
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

function dictTest() :mixed{
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

function keysetTest() :mixed{
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


<<__EntryPoint>>
function main_unreachable_minstr() :mixed{
vecTest();
dictTest();
keysetTest();
}
