<?php

function f($arg0, $arg1) {
 var_dump($arg0, $arg1);
 }
function g(&$arg0, $arg1) {
 var_dump($arg0, $arg1);
 }
class Af {
  function f($f, $var) {
    $f($this, $$var = 5);
  }
  function g($f, $var) {
    $f($this, $var++);
  }
}
class Ag {
  function f($f, $var) {
    $f(&$this, $$var = 5);
  }
  function g($f, $var) {
    $thiz = $this;
    $f(&$thiz, $var++);
  }
}

<<__EntryPoint>>
function main_1099() {
$af = new Af;
$ag = new Ag;
$af->f('f', 'this');
$ag->f('g', 'this');
$af->g('f', 30);
$ag->g('g', 30);
}
