<?hh

error_reporting(E_ALL);

trait A {
   public function __construct() {
     echo 'a';
   }
}

trait B {
   public function __construct() {
     echo 'b';
   }
}

trait C {
   public function __construct() {
     echo 'c';
   }
}

class Foo {
    use C, A, B {
		B::__construct insteadof A, C;

	}
}

$t = new Foo;
$t->__construct();

