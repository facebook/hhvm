<?php

function t($instance) {
  try {
    var_dump(clone $instance);
  } catch (Exception $e) {
    echo get_class($e) . ': ' . $e->getMessage() . "\n";
  }
}

t(new ReflectionParameter('t', 'instance'));
t(new ReflectionProperty('ReflectionProperty', 'name'));
t(new ReflectionExtension('zlib'));
t(new ReflectionClass('ReflectionClass'));
t(new ReflectionFunction('t'));
t(new ReflectionMethod('ReflectionMethod', '__construct'));
