<?hh

function foo() :mixed{}

function getFun() :mixed{
  return __hhvm_intrinsics\launder_value(foo<>);
}

function getStr() :mixed{
  return __hhvm_intrinsics\launder_value('foo');
}

function stringCast($x) :mixed{
  return (string) $x;
}

function boolCast($x) :mixed{
    return (bool) $x;
}

function doubleCast($x) :mixed{
    return (float)$x;
}

function intCast($x) :mixed{
    return (int) $x;
}

function varrayCast($x) :mixed{
  return varray($x);
}

function W($f) :mixed{
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
