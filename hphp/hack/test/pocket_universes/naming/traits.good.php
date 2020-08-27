<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

abstract class Base {
  enum E {
    case type T;
    case T val;
    :@I(
      type T = int,
      val = 42
    );
  }
}


trait myTrait {
  require extends Base;
  enum E {
    :@S(
      type T = string,
      val = "foo"
    );
  }
}

class C extends Base {
  use myTrait;

  enum E {
    case string name;
    :@I(
      name = "I"
    );
    :@S(
      name = "S"
    );
    :@F(
      type T = float,
      val = 42.0,
      name = "F"
    );
  }
}
