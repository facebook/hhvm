<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function vec_map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  return vec[];
}

abstract class Base {

  abstract const type TParams as shape(...);
  public function __construct(protected this::TParams $params) {
  }

}
final class Derived extends Base {

  const type TParams = shape(
    'max_int' => int,
  );

  static public function foo(vec<this> $v): void {
    $w = vec_map($v,
      $se ==> { return $se->params['max_int']; });
  }

}
