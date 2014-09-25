<?php

class test {}

$c = new ReflectionClass('test');
try {
  var_dump($c->getStaticPropertyValue('notfound', 'default'));
  var_dump($c->getStaticPropertyValue('notfound', null));
  var_dump($c->getStaticPropertyValue('notfound'));
} catch (ReflectionException $e) {
  echo $e->getMessage(), PHP_EOL;
}
