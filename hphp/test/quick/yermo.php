<?php

class blah {
  private function breaker() {
    static $x = 0;
    return $x++ == 0 ? array() : null;
  }

  public function foo() {
    $x = 0;
    $y = 0;

    if (($top_var_ref =& $this->breaker()) === NULL) {
      echo "hi\n";
    }
    echo "ok\n";
  }
}

function main() {
  $x = new blah();
  $x->foo();
  $x->foo();
}
main();
