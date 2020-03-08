// NOT YET IMPLEMENTED IN THE CHECKER <?hh // strict

namespace NS_new_anonymous_classes;

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

class C1 {}
interface I1 {
	public function f(): void;
	public function g(): void;
}
interface I2 {}

function main(): void {

  echo "================ Create some objects of anonymous classes type ===================\n\n";

  $v1a = new class() {
    public function __construct() {
      echo "Inside class " . __CLASS__ . "\n";
    }
  };

  var_dump($v1a);

  $v1b = new class () {
    public function __construct() {
      echo "Inside class " . __CLASS__ . "\n";
    }
  };

  var_dump($v1b);

  $v2 = new class (100) extends C1 implements I1, I2 {
    public function __construct($p) {
      echo "Inside class " . __CLASS__ . " constructor with parameter $p\n";
    }
    public function f(): void {
      echo "Inside ??::f()\n";
    }
    public function g(): void {
      echo "Inside ??::g()\n";
    }
  };

  var_dump($v2);

  $v2->f();
  $v2->g();

  echo "\n================ Comparing anonymous classes ===================\n\n";

/*
Multiple anonymous classes created in the same position (say, a loop) can be compared with `==`,
but those created elsewhere will not match as they will have a different name. */

  $identicalAnonClasses = [];
 
  for ($i = 0; $i < 2; $i++) {
    $identicalAnonClasses[$i] = new class(99) {
      public $i;
      public function __construct($i) {
        $this->i = $i;
      }
    };
  }
 
  var_dump($identicalAnonClasses[0] == $identicalAnonClasses[1]); // true
 
  $identicalAnonClasses[2] = new class(99) {
    public $i;
    public function __construct($i) {
      $this->i = $i;
    }
  };
 
  var_dump($identicalAnonClasses[0] == $identicalAnonClasses[2]); // false

  echo "\n================ End of script ===================\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
