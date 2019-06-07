<?hh

trait Hello {
  public function sayHello() {
    echo 'Hello ';
  }
}
trait World {
  public function sayWorld() {
    echo 'World';
  }
  use Hello;
}
class MyHelloWorld {
  use World;
  public function sayExclamationMark() {
    echo "!\n";
  }
}

<<__EntryPoint>>
function main_1946() {
$o = new MyHelloWorld();
$o->sayHello();
$o->sayWorld();
$o->sayExclamationMark();
}
