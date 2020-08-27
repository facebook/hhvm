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

  public final function set<TP as TBase:@Param>(TP $param, TP:@T $_): this {
    $cname = $this->classdef;
    $key = $cname::get($param);
    echo $key."\n";
    return $this;
  }
}
