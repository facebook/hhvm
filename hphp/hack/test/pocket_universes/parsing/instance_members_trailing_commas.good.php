<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
  case type T;
  case T val;
  :@I(type T = int, val = 42);
  }
}

class D {
  enum E {
  case type T;
  case T val;
  :@I(type T = int, val = 42,);
  }
}
