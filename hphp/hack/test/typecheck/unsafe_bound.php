<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

namespace HH_FIXME {
  type MISSING_TYPE_IN_HIERARCHY = mixed;
}
namespace Test {
type UNSAFE = \HH_FIXME\MISSING_TYPE_IN_HIERARCHY;

interface IE {
  abstract const type TID as UNSAFE;
  public function getID(): this::TID;
}

abstract class Base<T as IE, Tv> {
  final protected function __construct(
    private (function(T): Tv) $f,
  ): void { }
}

final class Derived<T as IE, Tv> extends Base<T, Tv>
{
    final public static function foo(): this {
      $x = meth_caller(IE::class, 'getID');
      return new static($x);
    }
}
}
