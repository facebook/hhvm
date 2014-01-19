<?php

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
$talker = new Talker();
$talker->fala();
?>
