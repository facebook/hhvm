<?hh

trait MyTrait {
  public function say_meth() :mixed{
    echo "meth: MyTrait\n";
  }
}
class MyBase {
  public function say_meth() :mixed{
    echo "meth: MyBase\n";
  }
}
class MyClass {
  use MyTrait;
  public function print_meth() :mixed{
    echo "meth: MyClass\n";
  }
}

<<__EntryPoint>>
function main_1988() :mixed{
$o = new MyClass();
$o->print_meth();
}
