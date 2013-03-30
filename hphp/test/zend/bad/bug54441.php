<?php

trait Foo {
  public function bar() {}
}

class Boo {
  use Foo {
    bar as dontKnow; 
    dontKnow as protected;
  }
}

?>