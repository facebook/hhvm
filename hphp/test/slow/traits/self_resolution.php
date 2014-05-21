<?php

trait Stuff {
  function checkSelf() {
    var_dump($this instanceof self);
  }

  function checkParent() {
    var_dump($this instanceof parent);
  }
}

class Thing {
  use Stuff;
}

class SubThing extends Thing {
  use Stuff;
}

$a = new Thing();
$a->checkSelf();
$b = new SubThing();
$b->checkSelf();
$b->checkParent();
