<?php
class Foo {
  private $bar;
  function __construct($bar) {
    $this->bar = $bar;
  }
  function __destruct() {
    echo __METHOD__,"\n";
    unset($this->bar);
  }
}

class Bar {
  function __destruct() {
    echo __METHOD__,"\n";
    unset($this->bar);
  }
}
function main() {
$y = new Bar();
$x = new Foo($y);
}
main();
?>