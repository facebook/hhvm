--TEST--
Line Numbers 01
--FILE--
<?php
class :thing {
  attribute
    Thing a,
    Thing b;
  public function method() {}} function foo() {}

$r = new ReflectionFunction('foo');
echo $r->getStartLine();
--EXPECT--
6
