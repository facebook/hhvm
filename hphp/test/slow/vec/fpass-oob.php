<?hh
<<__DynamicallyCallable>>
function ref(inout $x, $i, $j) :mixed{ echo ++$x[$i][$j], "\n"; }
<<__DynamicallyCallable>>
function non($x) :mixed{ echo $x, "\n"; }
function foo($f, $ref) :mixed{
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
foo(non<>, false);
foo(ref<>, true);
}
