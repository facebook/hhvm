<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

type UNSAFE_<T> = T;

// This is a hack to mimic Tany on bounds of type constants
/* HH_FIXME[4102] */
/* HH_FIXME[4101] */
type UNSAFE = UNSAFE_;

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
