//// file1.php
<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class B extends A {
  enum F {
    :@A(key = "A");
  }
}

//// file2.php
<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class A {
  enum F {
    case string key;

    :@A( key = "A" );
  }
}

class A2 extends A {
  enum F {
    :@A(key = "A");
  }
}

class D {
  enum F {
    case type T;
    case T key;
    :@I(type T = int, type T = int, key = 42);
    :@J(type T = int, key = 42, key = 42);
    :@K(type T = int, type T = float, key = 42.0, key = 42);
  }
}
