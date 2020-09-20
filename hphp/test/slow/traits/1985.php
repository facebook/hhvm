<?hh

trait Hello {
  public function sayHello() {
    echo "Hello\n";
  }
}
trait Hello1 {
  use Hello;
}
trait Hello2 {
  use Hello;
}
class MyClass {
  use Hello1, Hello2 {
    Hello1::sayHello insteadof Hello2;
  }
}

<<__EntryPoint>>
function main_1985() {
$o = new MyClass();
$o->sayHello();
}
