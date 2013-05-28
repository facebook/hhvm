<?php

class Bar{
}

class Foo {
  private $x;
  private $y;

  public function __construct($x) {
    $this->x = $x;
  }

  public function setX($x) {
    $this->x = $x;
  }

  public function getX() {
    return $this->x;
  }

  public function setXVerified(string $str) {
    $this->x = $str;
  }

  public function setY(Bar $k) {
    $this->y = $k;
  }
}

function main() {
  $k = new Foo("something");
  echo $k->getX();
  echo "\n";
  $k->setX("foo");
  echo $k->getX();
  echo "\n";

  $k->setXVerified("string");
  echo $k->getX();
  echo "\n";

  $k->setY(new Bar);
}
main();
