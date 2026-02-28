<?hh

trait MY_TRAIT1 {
  public function sayHello() :mixed{
    echo "Hello from MY_TRAIT1\n";
  }
}
trait MY_TRAIT2 {
  use MY_TRAIT1;
  public function sayHello() :mixed{
    echo "Hello from MY_TRAIT2\n";
  }
}
class MY_CLASS {
  use MY_TRAIT2;
}
<<__EntryPoint>> function main(): void {
$o = new MY_CLASS;
$o->sayHello();
}
