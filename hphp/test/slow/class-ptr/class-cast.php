<?hh

class Foo {}

function getClass() :mixed{
  return __hhvm_intrinsics\launder_value(Foo::class);
}

function getStr() :mixed{
  return __hhvm_intrinsics\launder_value('Foo');
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
