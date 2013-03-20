<?php

trait Too {
  function bar() {
    $a = function () {
      var_dump(__CLASS__);
    };
    $a();
    $a = function () {
      var_dump(get_class());
    };
    $a();
    if (isset($this)) {
      $a = function () {
        var_dump(get_class($this));
      };
      $a();
    }
  }
}
class Foo { use Too; }

$f = new Foo;
echo "Between\n";
$f->bar();
echo "Between\n";
$f::bar();
echo "Between\n";
Foo::bar();
