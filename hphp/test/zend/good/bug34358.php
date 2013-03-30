<?php
class foo {
  function bar() {
    $ref = &$this;
  }
}
$x = new foo();
$x->bar();
echo "ok\n";
?>