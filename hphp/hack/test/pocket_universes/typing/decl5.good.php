<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// OK: T mapped to a valid type
class PU4 {
  enum X {
    case type T;
    :@X(
      type T = int
    );
  }
}
