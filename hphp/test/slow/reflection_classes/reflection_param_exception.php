<?php

class A {
  public static function bar($baz) {}
}

// Bad function.
try {
  new ReflectionParameter(1, 'baz');
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}

// Bad param name.
try {
  new ReflectionParameter(['A', 'bar'], 'not-baz');
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}

// Out of bound param.
new ReflectionParameter(['A', 'bar'], 0);
try {
  new ReflectionParameter(['A', 'bar'], 1);
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}
try {
  new ReflectionParameter(['A', 'bar'], -1);
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}

// Bad param type.
try {
  new ReflectionParameter(['A', 'bar'], []);
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}
