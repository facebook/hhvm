<?php

trait T {
  public static function gen() {
    static $x;
    yield ++$x;
    yield 2;
    yield ++$x;
  }
}
class X {
 use T;
 }
class Y extends X {
}

<<__EntryPoint>>
function main_2072() {
$g = X::gen();
foreach ($g as $i) var_dump($i);
}
