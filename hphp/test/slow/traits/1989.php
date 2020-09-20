<?hh

trait A {
  public function say() {
    echo "Hello";
  }
}
trait B {
  use A {
    A::say as fala;
  }
}
class Talker {
  use B;
}
<<__EntryPoint>> function main(): void {
$talker = new Talker();
$talker->fala();
}
