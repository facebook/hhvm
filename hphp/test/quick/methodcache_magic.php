<?php

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

class base {
  public function __call($x, $y) {
    echo "base::__call: $x, $y " . get_called_class() . "\n";
  }
}

class one extends base {
}

class two extends base {
}

class goer {
  public function go($o) {
    $o->magic();
  }
}

function main() {
  $go = new goer;
  $one = new one;
  $two = new two;
  $go->go($one);
  foreach (array(1,2,3) as $_) {
    foreach (array($one, $two) as $o) {
      $go->go($o);
      $go->go($o);
    }
  }
}

main();
