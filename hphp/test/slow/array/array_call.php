<?hh

class C {
  static public function foo() {
    return 1;
  }
}

$c = new C;
$x = array('C', fun('var_dump'));
$x();
