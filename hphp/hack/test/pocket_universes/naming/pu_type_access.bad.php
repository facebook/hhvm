<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// See XController / XControllerPUURIBuilder from D18082628
//
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

  // No type S in TP, Param only provides T
  public final function set<TP as TBase:@Param>(TP $param, TP:@S $value): this {
    $cname = $this->classdef;
    $key = $cname::get($param);
    echo $key."\n";
    return $this;
  }

  public function foo<TP as TBase:@Param>(TP $param): TP:@X {
    while (true) {
    }
  }

  public function bar<TP as TBase:@Param>(TP $param): vec<TP:@X> {
    return vec[];
  }
}
