<?hh

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

<<__EntryPoint>>
function main_entry(): void {

  error_reporting(E_ALL);

  $t = new Foo;
  $t->__construct();
}
