<?php

class Base {
  protected function hehe() {
    return "hehe";
  }
}

class D1 extends Base {
  public function __call($x, $y) {
    return "__call";
  }
}

function main(Base $b) {
  var_dump($b);
  $y = $b->hehe();
  echo "called\n";
  var_dump($y);
}


<<__EntryPoint>>
function main_magic_call_012() {
main(new D1);
}
