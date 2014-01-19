<?php

class one {
  public function doit($o) {
    // Method cache should still dispatch to one::heh for $o :: two
    $o->heh();
  }

  private function heh() {
    echo "one\n";
  }
}

class two extends one {
  // You can override a private function with a public one in php.
  // (But you can't change it to static.)
  public function heh() {
    echo "two\n";
  }
}

function main() {
  $one = new one;
  $two = new two;
  $one->doit($one);
  $one->doit($two);
  $one->doit($two);
  $one->doit($one);
  $one->doit($one);
  $one->doit($two);
  $one->doit($two);
}
main();
