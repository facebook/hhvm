<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cat implements HH\ClassAttribute {
  public function __construct(public string $sprop)[] {}
}

class Dog implements HH\ClassAttribute {
  public function __construct(public int $j, public string $opt = "default")[] {}
}

class Emu implements HH\EnumAttribute {
  public function __construct(public int $k)[] {}
}

<<Cat("especial")>>
class C {}

<<Dog(7)>>
class D {}

<<Emu(23)>>
enum E: int {}

<<Cat(42)>> // incorrect type
class X {}

function reflect(): void {
  $rc = new ReflectionClass("C");
  $ac = $rc->getAttributeClass(Cat::class);
  var_dump($ac->sprop);

  $rc = new ReflectionClass("D");
  $ac = $rc->getAttributeClass(Dog::class);
  var_dump($ac->j);
  var_dump($ac->opt);

  $rc = new ReflectionClass("E");
  $ac = $rc->getAttributeClass(Emu::class);
  var_dump($ac->k);

  $rc = new ReflectionClass("X");
  $ac = $rc->getAttributeClass(Cat::class);
}
<<__EntryPoint>> function main(): void {
echo reflect();
}
