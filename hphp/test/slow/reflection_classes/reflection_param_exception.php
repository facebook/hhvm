<?php

class A {
  public static function bar($baz) {}
}

// Bad function.
try {
  new ReflectionParameter(1, 'baz');
} catch (ReflectionException $e) {
  echo 'caught1';
}

// Bad param name.
try {
  new ReflectionParameter(['A', 'bar'], 'not-baz');
} catch (ReflectionException $e) {
  echo 'caught2';
}

// Out of bound param.
new ReflectionParameter(['A', 'bar'], 0);
try {
  new ReflectionParameter(['A', 'bar'], 1);
} catch (ReflectionException $e) {
  echo 'caught3';
}
try {
  new ReflectionParameter(['A', 'bar'], -1);
} catch (ReflectionException $e) {
  echo 'caught4';
}

// Bad param type.
try {
  new ReflectionParameter(['A', 'bar'], []);
} catch (ReflectionException $e) {
  echo 'caught5';
}
