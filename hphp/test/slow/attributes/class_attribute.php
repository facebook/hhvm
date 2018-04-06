<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class __Attribute__Cat implements HH\ClassAttribute {
  public function __construct(public string $sprop) {}
}

class __Attribute__Dog implements HH\ClassAttribute {
  public function __construct(public int $j, public string $opt = "default") {}
}

<<Cat("especial")>>
class C {}

<<Dog(7)>>
class D {}

<<Cat(42)>> // incorrect type
class X {}

function reflect(): void {
  $rc = new ReflectionClass("C");
  $ac = $rc->getAttributeClass(__Attribute__Cat::class);
  var_dump($ac->sprop);

  $rc = new ReflectionClass("D");
  $ac = $rc->getAttributeClass(__Attribute__Dog::class);
  var_dump($ac->j);
  var_dump($ac->opt);

  $rc = new ReflectionClass("X");
  $ac = $rc->getAttributeClass(__Attribute__Cat::class);
}

echo reflect();
