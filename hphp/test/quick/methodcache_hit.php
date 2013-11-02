<?php

class one {
  public function foo() { echo "foo\n"; }
}
class doer {
  public function junk($x) {
    $x->foo();
  }
}

function main() {
  $x = new doer;
  $o = new one;
  $x->junk($o);
  $x->junk($o);
  $x->junk($o);
}
main();
