<?php


class bt {
  function __destruct() {
    global $bt;
    $bt = debug_backtrace();
    var_dump($bt);
  }
}

function foo() {
  if (isset($GLOBALS['notset'])) {
    var_dump(func_get_args());
  }
  $x = new bt;
  throw new Exception;
}

function main() {
  try {
    foo(new bt);
  } catch (Exception $ex) {
    global $bt;
    var_dump($bt);
  }
}

main();
