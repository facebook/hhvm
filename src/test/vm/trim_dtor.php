<?php
class X {
  public function __destruct() { echo "heh\n"; }
}
function foo($x) {}

foo(new X(), new X(), new X(), new X());
echo "yep\n";
