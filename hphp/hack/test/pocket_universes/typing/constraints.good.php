<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class Base {}
class Middle extends Base {}
class High extends Middle {}

class C {
  enum E {
    case type T0 as Base;
    case type T1 as Middle;
    case type T2 as High;

    :@Test0(
      type T0 = Base,
      type T1 = Middle,
      type T2 = High
    );
    :@Test1(
      type T0 = Middle,
      type T1 = High,
      type T2 = High
    );
    :@Test2(
      type T0 = High,
      type T1 = High,
      type T2 = High
    );
  }
}

class D {
  enum E {
    case type T0 super Base;
    case type T1 super Middle;
    case type T2 super High;

    :@Test0(
      type T0 = Base,
      type T1 = Middle,
      type T2 = High
    );
    :@Test1(
      type T0 = Base,
      type T1 = Base,
      type T2 = Middle
    );
    :@Test2(
      type T0 = Base,
      type T1 = Base,
      type T2 = Base
    );
  }
}

class F {
  enum E {
    case type T as Base super High;
    :@Test0(
      type T = Base
    );
    :@Test1(
      type T = Middle
    );
    :@Test2(
      type T = High
    );
  }
}
