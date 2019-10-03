<?hh

function ref(inout $x, $i, $j) { echo ++$x[$i][$j], "\n"; }
function non($x) { echo $x, "\n"; }

function foo($f, $ref) {
  $x = vec[0, vec[1]];
  $ref ? $f(inout $x, 1, 0) : $f($x[1][0]);
  try {
    $ref ? $f(inout $x, 1, 2) : $f($x[1][2]);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
  try {
    $ref ? print("skip\n") : $f($x[1][2][3]);
  } catch (Exception $e) {
    echo "Catch: ", $e->getMessage(), "\n";
  }
}
<<__EntryPoint>> function main(): void {
foo('non', false);
foo('ref', true);
}
