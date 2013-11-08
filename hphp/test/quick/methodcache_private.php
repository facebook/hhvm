<?php

class one {
  public function lol() {}

  public function doit($o) {
    $o->heh();
  }

  private function heh() {
    echo "one\n";
  }
}

class two extends one {
  private function heh() {
    echo "two\n";
  }
}

function main() {
  $one = new one;
  $two = new two;
  $one->doit($two);
  $one->doit($one);
  $one->doit($one);
  $one->doit($two);
}
main();
