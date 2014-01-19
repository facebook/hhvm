<?php

trait Hello {
  public function say() {
    echo 'Hello ';
  }
}
trait World {
  public function say() {
    echo 'World';
  }
}
class MyHelloWorld {
  use Hello {
    Hello::say as sayHello;
  }
  public function say() {
    $this->sayHello();
    $this->sayWorld();
    echo "!\n";
  }
  use World {
    World::say as sayWorld;
  }
}
$o = new MyHelloWorld();
$o->say();
