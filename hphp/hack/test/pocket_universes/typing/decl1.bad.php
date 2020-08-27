<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// KO: T mapped to an invalid (unknown) type
class PU5 {
  enum X {
    case type T;
    :@X(
      type T = U
    );
  }
}
