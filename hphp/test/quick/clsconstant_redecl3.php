<?php

interface A {
  const FOO = 'FOO';
}

abstract class B implements A {
  const BAR = 'BAR';

  abstract public function test();
}

class C extends B {
  const FOO = 'DOH';

  public function test() { echo self::FOO . "\n"; }
}
