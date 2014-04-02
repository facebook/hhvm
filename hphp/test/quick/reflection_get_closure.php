<?php
class Bar {
  private $base;
  public function __construct($base) {
    $this->base = $base;
  }

  public function baz($a) {
    return $a + $this->base;
  }
  public static function foo($a) {
    return $a + 1;
  }
}
function foo($a) {
  return $a + 2;
}

$foo = function ($a) {
  return $a + 3;
};

$bar = new Bar(10);
// Dynamic methods
$rf = new ReflectionMethod($bar, 'baz');
var_dump($rf->getClosure() === NULL);
var_dump(call_user_func($rf->getClosure($bar), 1));

$rf = new ReflectionMethod('Bar', 'baz');
var_dump($rf->getClosure() === NULL);
var_dump(call_user_func($rf->getClosure($bar), 1));

$rf = new ReflectionMethod('Bar::baz');
var_dump($rf->getClosure() === NULL);
var_dump(call_user_func($rf->getClosure($bar), 1));

// Static methods
$rf = new ReflectionMethod($bar, 'foo');
var_dump(call_user_func($rf->getClosure(), 1));

$rf = new ReflectionMethod('Bar', 'foo');
var_dump(call_user_func($rf->getClosure(), 1));

$rf = new ReflectionMethod('Bar::foo');
var_dump(call_user_func($rf->getClosure(), 1));

// Function
$rf = new ReflectionFunction('foo');
var_dump(call_user_func($rf->getClosure(), 1));

// Closure
$rf = new ReflectionFunction($foo);
var_dump(call_user_func($rf->getClosure(), 1));
