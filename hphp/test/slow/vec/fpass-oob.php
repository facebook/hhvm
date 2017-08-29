<?hh

function ref(&$x) { echo ++$x, "\n"; }
function non($x) { echo $x, "\n"; }

function foo($f) {
  $x = vec[0, vec[1]];
  $f($x[1][0]);
  try {
    $f($x[1][2]);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
  try {
    $f($x[1][2][3]);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
}

foo('non');
foo('ref');
