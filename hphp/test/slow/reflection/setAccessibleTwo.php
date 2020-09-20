<?hh

class A {
  private $b = 'b';
}
class C {
  static private $d = 'd';
}
<<__EntryPoint>>
function entrypoint_setAccessibleTwo(): void {
  error_reporting(E_ALL);

  $prop1 = (new ReflectionClass('A'))->getProperty('b');
  $prop2 = (new ReflectionClass('A'))->getProperty('b');
  $prop1->setAccessible(true);
  $prop2->getValue(new A);

  $prop1 = (new ReflectionClass('C'))->getProperty('d');
  $prop2 = (new ReflectionClass('C'))->getProperty('d');
  $prop1->setAccessible(true);
  $prop2->getValue('C');
}
