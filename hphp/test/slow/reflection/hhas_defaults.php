<?php

function test() {
  $x = new ReflectionFunction('array_filter');
  $params = $x->getParameters();
  $p1 = $params[1];
  var_dump($p1->getDefaultValue());
  var_dump($p1->getDefaultValueText());
}

test();
