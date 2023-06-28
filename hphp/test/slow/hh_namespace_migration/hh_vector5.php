<?hh

class Vector {
  public function foo() :mixed{
    echo "foo\n";
  }
}


<<__EntryPoint>>
function main_hh_vector5() :mixed{
$x = new \Vector();
$x->foo();
}
