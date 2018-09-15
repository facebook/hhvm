<?php

error_reporting(E_ALL | E_STRICT | E_DEPRECATED);

// No PHP4 constructor
interface foo {
  public function foo();
}

// No PHP4 constructor
class bar implements foo {
  public function foo() {}
}

// No PHP4 constructor
trait herp {
  public function derp() {}
}

// PHP4 constructor via trait
class derp {
  use herp;
}

// direct PHP4 constructor
class MyClass {
  public function MyClass() {}
}
