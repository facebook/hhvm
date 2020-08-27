<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// OK: one case type
class PU2 {
  enum X {
    case type T;
  }
}
