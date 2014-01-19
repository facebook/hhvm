<?php

class C {
  public function __toString() {
    return 'bar';
  }
}
function f($x) {
  var_dump($x . '');
}
f(123);
f(new C);
