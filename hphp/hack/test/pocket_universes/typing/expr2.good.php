<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// OK: apply atom to case value
class PU {
  enum E {
    case int v;
    :@X(
      v = 1
    );
    :@Y(
      v = 2
    );
  }

  public static function test(): int {
    return self:@E::v(:@X);
  }
}
