<?php

class foo {
  private $prop;
  public function __construct($s) {
    $this->prop = $s;
  }
}

function main() {
  new foo('hi');
}
main();
echo "done\n";
