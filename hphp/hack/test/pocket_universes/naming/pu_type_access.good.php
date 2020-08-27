<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

abstract class Base {
  enum Param {
    case type T;
    case string key;
  }

  public final static function get(this:@Param $x): string {
    return static:@Param::key($x);
  }
}

final class C<TBase as Base> {
  public function __construct(private classname<TBase> $classdef) {}

  public final function set<TP as TBase:@Param>(TP $_, TP:@T $_): this {
    return $this;
  }

  public function foo<TP as TBase:@Param>(TP $param): TP:@T {
    while (true) {
    }
  }

  public function bar<TP as TBase:@Param>(TP $param): vec<TP:@T> {
    return vec[];
  }
}
