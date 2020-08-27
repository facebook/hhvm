<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// OK: one member
class PU1 {
  enum X {
    :@X;
  }
}
