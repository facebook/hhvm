<?php

class C {
  function g() {
    $ex = new Exception();
    $bt = $ex->getTrace();
    foreach ($bt as $k => $_) {
      $frame = $bt[$k];
      unset($frame['file']);
      unset($frame['line']);
      unset($frame['args']);
      ksort($frame);
      $bt[$k] = $frame;
    }
    var_dump($bt);
  }
  function f() {
    $this->g();
  }
}
$obj = new C;
$obj->f();
echo "------------------------\n";
Exception::setTraceOptions(true);
$obj->f();
