<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class PU {
  enum E {
    case type T;
    :@I(
      type T = int
    );
    :@S(
      type T = string
    );
  }

  public static function dump<TF as this:@E>(TF $x, TF:@T $v): void {
    var_dump($v);
  }

  public static function main(): void {
    static::dump(:@I, 42);
    static::dump(:@S, "foo");
  }
}
