<?php

function test() {
  (new X)->foo();
}

apc_add('foo', 0);

function setup() {
  $i = apc_inc('foo', 1);
  var_dump($i);

  $text = "";
  for ($t = 0; $t < $i; $t++) {
    // define a number of classes that should use
    // the same amount of memory as X, so that X will
    // be allocated at a different address on each request
    $text .= "class X$t extends Y { function bar() {} }\n";
  }
  $text .= "class Y { const C = $i; }\n";

  $file = __FILE__ . ".$i.inc";
  file_put_contents($file, "<?php $text");
  include $file;
  unlink($file);
  class X extends Y {
    private $priv = 42;
    function foo() {
      var_dump($this->priv + self::C);
    }
  }
}

setup();
test();
