<?hh

trait Hello {
  public function sayHello() :mixed{
    echo 'Hello ';
  }
}
trait World {
  public function sayWorld() :mixed{
    echo 'World';
  }
}
class MyHelloWorld {
  use Hello, World;
  public function sayExclamationMark() :mixed{
    echo "!\n";
  }
}

<<__EntryPoint>>
function main_1945() :mixed{
$o = new MyHelloWorld();
$o->sayHello();
$o->sayWorld();
$o->sayExclamationMark();
}
