<?php

function bar($g) {
 return $g;
 }
class X {
  static function foo() {
    echo $this->baz(bar(1), bar(''));
  }
}
