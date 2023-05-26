<?hh

module a.b;

<<__EntryPoint>>
function main_throw() {
  try {
    foo();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }

  try {
    __hhvm_intrinsics\launder_value("foo")();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
