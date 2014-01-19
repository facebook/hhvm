<?php

class A {
  private static $priv = 7;
  function readVar() {
    A::$priv;
  }
}

function main() {
  (new A())->readVar();
  var_dump(isset(A::$priv));
}
main();
