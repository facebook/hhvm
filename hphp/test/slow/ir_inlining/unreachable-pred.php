<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function func3(string $name) :mixed{
  if (class_exists($name, false) || interface_exists($name, false)) {
    return true;
  }
  if ($name == "blahblah") {
    foreach ($name as $v) {}
  }
  return false;
}

function func2(string $child, string $parent) :mixed{
  return func3($child) && func3($child, $parent);
}

function starts_with($x, $y): bool {
  $len = strlen($y);
  return !$len || 0 === strncmp($x, $y, $len);
}

abstract class Base1Cls {}
abstract class Interface1 {}
abstract class FooBase {}

class Base1Foobaz1 extends Base1Cls {}
class Foobaz2 extends Base1Cls {}
class Foobaz3 extends FooBase {}

abstract class DefBase {

<<__Memoize>>
static function func1(string $id): bool {
  return starts_with($id, 'Base1')
    || func2($id, Base1Cls::class)
    || func2($id, Interface1::class);
}

}

class DefChild1 extends DefBase {}
class DefChild2 extends DefBase {}

function loop($str, $count) :mixed{
  for ($i = 0; $i < 100; $i++) { starts_with('helloworld', 'hello'); }
  for ($i = 0; $i < $count; $i++) {
    DefBase::func1($str);
  }
}

function test() :mixed{
  loop('Base1Foobaz1', 35);
  loop('Foobaz2', 35);
  loop('Foobaz3', 35);
}

<<__EntryPoint>>
function main_unreachable_pred() :mixed{
test();
echo "DONE\n";
}
