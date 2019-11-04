<?hh

function foo() {}

function getFun() {
  return __hhvm_intrinsics\launder_value(HH\fun('foo'));
}

function getStr() {
  return __hhvm_intrinsics\launder_value('foo');
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

function arrayCast($x) {
  return (array)$x;
}

function varrayCast($x) {
  return varray($x);
}

function test($x): void {
  var_dump(stringCast($x));
  var_dump(boolCast($x));
  var_dump(doubleCast($x));
  var_dump(intCast($x));
  var_dump(arrayCast($x));
  try {
    var_dump(varrayCast($x));
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main(): void {
  test(getStr());
  test(getFun());
}
