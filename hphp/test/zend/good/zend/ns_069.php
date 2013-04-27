<?php

namespace foo;

class Test {
  static function f() {
    var_dump((binary)__NAMESPACE__);
    include __DIR__ . '/ns_069.inc';
    var_dump((binary)__NAMESPACE__);
  }
}

Test::f();

?>
===DONE===