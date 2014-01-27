<?php

function test() {
  $x = new ReflectionFunction(function($a, $b = null) { });
  $params = $x->getParameters();
  $p1 = $params[1];
  var_dump($p1->getDefaultValueText());
}

test();
