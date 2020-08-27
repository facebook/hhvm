<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum Param {
    case type T;
    case T data;
  }

  public function g<TP as this:@Param>(TP $in): TP:@T {
    return this:@Param::data($in);
  }
}
