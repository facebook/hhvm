<?hh

function foo() {}

function getFun() {
  return __hhvm_intrinsics\launder_value(foo<>);
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

function varrayCast($x) {
  return varray($x);
}

function W($f) {
  try {
    var_dump($f());
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

function test($x): void {
  W(() ==> stringCast($x));
  W(() ==> boolCast($x));
  W(() ==> doubleCast($x));
  W(() ==> intCast($x));
  W(() ==> varrayCast($x));
}

<<__EntryPoint>>
function main(): void {
  test(getStr());
  test(getFun());
}
