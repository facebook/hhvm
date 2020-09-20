<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case string key;
    :@I(
      key = "I"
    );
  }
}

class E extends C {}

class D extends E {
  enum E {

    :@I();
  }
}

class F {
  enum X {
    :@A;
    :@B();
  }
}

class G extends F {
  enum X {
    :@A;
  }
}

class R extends F {
  enum X {
    :@A;
    :@A;
  }
}
