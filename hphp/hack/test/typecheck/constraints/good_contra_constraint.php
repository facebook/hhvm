<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function filterInstance<Tu, Tv super Tu>(
  Traversable<Tv> $vector,
  classname<Tu> $classname,
): Vector<Tu> {
  return Vector {};
}

class Base {}
class Derived extends Base {}

function genTextAttachment(Vector<Base> $x): Derived {
  $v = filterInstance($x, Derived::class);
  return $v[0];
}
