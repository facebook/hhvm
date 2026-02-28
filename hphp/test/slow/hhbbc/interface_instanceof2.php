<?hh

abstract class Base {}

interface IHeh {}
interface IHeh2 {}

class D1 extends Base implements IHeh, IHeh2 {}
class D2 extends Base implements IHeh, IHeh2 {}
class D3 extends Base implements IHeh2 {}


function f() :mixed{
  return new D1();
}

function main() : IHeh {
  if (__hhvm_intrinsics\launder_value(1)) {
    $o = f();
    invariant($o is IHeh, "asdf");
  } else {
    $o = new D2();
  }
  return $o;
}

<<__EntryPoint>>
function main_interface_instanceof() :mixed{
  main();
  echo "Hello\n";
}
