<?php

trait A {
  abstract protected static function AAbsProtStat();

  private static function APrivStat1() {}
  private static function APrivStat2() {}

  public function APublic1() {}
  public function APublic2() {}
}

trait B {
  abstract protected static function BAbsProtStat();

  private static function BPrivStat1() {}
  private static function BPrivStat2() {}

  public function BPublic1() {}
  public function BPublic2() {}
}

class C {
  use B;
  use A;

  protected static function BAbsProtStat() {}
  protected static function AAbsProtStat() {}
}

class D extends C {
  public function foo() {}
}

$cls = new ReflectionClass('D');
var_dump(array_map(
  function($meth) { return $meth->getName(); },
  $cls->getMethods()
));
