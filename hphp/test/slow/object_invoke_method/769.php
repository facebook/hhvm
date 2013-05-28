<?php

// with type hints
class C8 {
  public function __invoke(array $a0) {
    var_dump($a0);
  }
}
$c = new C8;
$c(array(1, 2, 3));
