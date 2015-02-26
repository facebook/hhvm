<?php


class foo {
  public static function setBlob($x) {
    $this->blob = $x;
  }
}

function main() {
  (new foo)->setBlob(0);
}
main();
