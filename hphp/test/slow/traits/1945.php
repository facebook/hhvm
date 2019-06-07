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
}
class MyHelloWorld {
  use Hello, World;
  public function sayExclamationMark() {
    echo "!\n";
  }
}

<<__EntryPoint>>
function main_1945() {
$o = new MyHelloWorld();
$o->sayHello();
$o->sayWorld();
$o->sayExclamationMark();
}
