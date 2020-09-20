<?hh

function foo(Vector<int> $z): Vector<int> {
  return $z;
}
class C {
  function goo(): int {
    return 0;
  }
  function blah(): this {
    return $this;
  }
}
class C1 extends C {
  function goo() {
    return 0;
  }
}
class C2 extends C1 {
  function goo(): string {
    return '0';
  }
}
interface I {
  function m(): string;
  function n(): arraykey;
}
interface I1<T> {
  function m(): T;
}
trait T {
  function t(): C {
    return new C();
  }
}
class UseT {
  use T;
}

<<__EntryPoint>>
function main_1363() {
$rf = new ReflectionFunction('foo');
var_dump($rf->getReturnTypeText());
$rc = new ReflectionClass('C');
$rm = $rc->getMethod('goo');
var_dump($rm->getReturnTypeText());
$rm = $rc->getMethod('blah');
var_dump($rm->getReturnTypeText());
$rc = new ReflectionClass('C1');
$rm = $rc->getMethod('goo');
var_dump($rm->getReturnTypeText());
$rc = new ReflectionClass('C2');
$rm = $rc->getMethod('goo');
var_dump($rm->getReturnTypeText());
$rc = new ReflectionClass('I');
$rm = $rc->getMethod('m');
var_dump($rm->getReturnTypeText());
$rm = $rc->getMethod('n');
var_dump($rm->getReturnTypeText());
$rc = new ReflectionClass('I1');
$rm = $rc->getMethod('m');
var_dump($rm->getReturnTypeText());
$rc = new ReflectionClass('UseT');
$rm = $rc->getMethod('t');
var_dump($rm->getReturnTypeText());
}
