<?hh

class A {
  public function __construct() {
    echo "In A::__construct\n";
  }
}
class B extends A {
}
class A2 {
  public function __construct() {
    echo "In A2::__construct\n";
  }
}
class B2 extends A2 {
  public function __construct() {
    echo "In B2::__construct\n";
    parent::__construct();
  }
}
class C {
}
class D extends C {
  public function __construct() {
    echo "In D::__construct\n";
  }
}

<<__EntryPoint>>
function main_1721() :mixed{
$obj = new B();
$obj = new B2();
$obj = new D;
}
