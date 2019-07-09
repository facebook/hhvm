<?hh

class Bob {
}
class Loblaw {
}
// Interfaces
interface BaseOne {
}
interface BaseTwo {
}

interface Iface extends BaseOne, BaseTwo {
}

class Base implements BaseOne {
}
class Fancy implements Iface {
}

<<__EntryPoint>> function main(): void {
  $a = new Bob();

  // instanceof constant
  var_dump($a is Bob);
  var_dump($a is Loblaw);

  // instanceof string variable
  $bob = "Bob";
  $loblaw = "Loblaw";
  var_dump($a instanceof $bob);
  var_dump($a instanceof $loblaw);

  // instanceof object
  $bob = new Bob();
  $loblaw = new Loblaw();
  var_dump($a instanceof $bob);
  var_dump($a instanceof $loblaw);

  $b = new Base();
  var_dump($b is BaseOne);

  // Follow the interface hierarchy up
  $f = new Fancy();
  var_dump($f is BaseTwo);
}
