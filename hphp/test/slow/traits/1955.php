<?hh

trait A {
  public function say() {
    echo "Hello\n";
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
<<__EntryPoint>> function main() {
$talker = new Talker();
$talker->fala();
}
