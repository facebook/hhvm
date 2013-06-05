<?php

class A {
  private $count = 0;
  public function __toString() {
    return (string) $this->count++;
  }
}

function main() {
  $a = new A;
  var_dump((string)$a);
  var_dump((string)$a);
  var_dump((string) (new A));
}
main();
