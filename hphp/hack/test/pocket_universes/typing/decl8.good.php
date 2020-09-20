<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// OK: v mapped to an int
class PU8 {
  enum X {
    case int v;
    :@X(
      v = 1
    );
  }
}
