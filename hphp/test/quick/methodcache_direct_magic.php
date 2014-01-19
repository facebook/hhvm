<?php

class one {
  public function __call($x, $y) {
    echo "one::__call: $x, $y\n";
  }
}

class two {
  public function __call($x, $y) {
    echo "two::__call: $x, $y\n";
  }
}

class goer {
  public function go($o) {
    $o->__call('heh', 'heh');
  }
}

function main() {
  $go = new goer;
  $one = new one;
  $two = new two;
  foreach (array(1,2) as $k) {
    $go->go($one);
    $go->go($one);
    $go->go($one);
    $go->go($two);
    $go->go($two);
    $go->go($two);
  }
}

class base {
  public function __call($x, $y) {
    echo "base::__call: $x, $y: " . get_called_class() . "\n";
  }
}

class related_one extends base {}
class related_two extends base {}

function main2() {
  $go = new goer;
  $one = new related_one;
  $two = new related_two;
  foreach (array(1,2) as $k) {
    $go->go($one);
    $go->go($one);
    $go->go($one);
    $go->go($two);
    $go->go($two);
    $go->go($two);
  }
}

main();
main2();
