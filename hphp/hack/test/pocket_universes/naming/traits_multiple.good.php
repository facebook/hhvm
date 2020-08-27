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

trait myTrait0 {
  use myTrait;
  enum E {
    :@S0(
      type T = string,
      val = "foo"
    );
  }
}

trait myTrait1 {
  use myTrait;
  enum E {
    :@S1(
      type T = string,
      val = "foo"
    );
  }
}

class C extends Base {
  use myTrait0, myTrait1;

  enum E {
    case string name;
    :@I(
      name = "I"
    );
    :@S(
      name = "S"
    );
    :@S0(
      name = "S"
    );
    :@S1(
      name = "S"
    );
    :@F(
      type T = float,
      val = 42.0,
      name = "F"
    );
  }
}
