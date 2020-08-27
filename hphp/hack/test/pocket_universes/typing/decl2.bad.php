<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// KO: v mapped to an incorrect expression
class PU9 {
  enum X {
    case int v;
    :@X(
      v = 1[1]
    );
  }
}
