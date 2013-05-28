<?php

define('FOO', 1);
class a {
  static $b = FOO;
}
function foo() {
  static $a;
  static $a = FOO;
  echo $a;
}
foo();
echo a::$b;
