<?php
class a {
  static $b = FOO;
}
function foo() {
  static $a;
  static $a = FOO;
  echo $a;
}


const FOO = 1;
<<__EntryPoint>>
function main_1408() {
foo();
echo a::$b;
}
