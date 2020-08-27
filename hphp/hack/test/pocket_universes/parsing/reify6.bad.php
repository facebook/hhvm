<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case type T;
    :@A (reify type T = int);
  }
}
