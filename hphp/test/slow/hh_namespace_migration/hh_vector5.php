<?hh

class Vector {
  public function foo() {
    echo "foo\n";
  }
}


<<__EntryPoint>>
function main_hh_vector5() {
$x = new \Vector();
$x->foo();
}
