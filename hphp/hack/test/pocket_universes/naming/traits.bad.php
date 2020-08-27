<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>
trait MyTrait {
  enum E {
    case type T;
  }
}

trait MyTrait2 {
  enum E {
    case string v;
  }
}
