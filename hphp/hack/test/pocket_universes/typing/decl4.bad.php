<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

// KO: type error in v (dependent case)
class PU11 {
  enum X {
    case type T;
    case Vector<T> v;
    :@X(
      v = 1
    );
  }
}
