<?php

interface A {
  const FOO = 'FOO';
}

abstract class B implements A {
  const BAR = 'BAR';

  abstract public function test();
}

class C extends B {
  // This is actually fine. It wouldn't be if C directly implemented A
  const FOO = 'DOH';

  public function test() { echo self::FOO . "\n"; }
}

var_dump(C::FOO);
