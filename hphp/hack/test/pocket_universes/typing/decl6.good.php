<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// OK: one value
class PU6 {
  enum X {
    case string v;
  }
}
