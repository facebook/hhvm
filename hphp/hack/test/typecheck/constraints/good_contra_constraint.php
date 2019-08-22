<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
/* HH_FIXME[4336] */
function filterInstance<Tu, Tv super Tu>(
  Traversable<Tv> $vector,
  classname<Tu> $classname,
): Vector<Tu> {
}

class Base {}
class Derived extends Base {}

function genTextAttachment(Vector<Base> $x): Derived {
  $v = filterInstance($x, Derived::class);
  return $v[0];
}
