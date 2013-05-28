<?php

function test() {
  try {
    $classes = get_declared_classes();
    foreach ($classes as $class) {
      $r = new ReflectionClass($class);
      $t += count($r->getMethods());
    }
    var_dump('ok');
  }
 catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
test();
