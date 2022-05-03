<?hh

<<__EntryPoint>>
function main() {
  $cl = () ==> __hhvm_intrinsics\launder_value(false);
  $cl();

  try {
    $f = HH\dynamic_class_meth(get_class($cl), "__invoke");
    echo "ERROR: shouldn't be able to call dynamically\n";
    $f();
  } catch (Exception $e) {
    echo "Caught!\n";
  }

  try {
    $m = class_meth('Closure$main', '__invoke');
    $m();
  } catch (Exception $e) {
    echo "Caught!\n";
  }
}
