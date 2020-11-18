<?hh

class Foo {}

function getClass() {
  return __hhvm_intrinsics\launder_value(Foo::class);
}

function getStr() {
  return __hhvm_intrinsics\launder_value('Foo');
}

function stringCast($x) {
  return (string) $x;
}

function boolCast($x) {
    return (bool) $x;
}

function doubleCast($x) {
    return (float)$x;
}

function intCast($x) {
    return (int) $x;
}

function varrayCast($x) {
  return varray($x);
}

function test($x): void {
  var_dump(stringCast($x));
  var_dump(boolCast($x));
  var_dump(doubleCast($x));
  var_dump(intCast($x));
  try {
    var_dump(varrayCast($x));
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main(): void {
  test(getStr());
  test(getClass());
}
