<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class Param<reify T> {
  public function __construct(public T $x) {}
}

class Foo {
  enum E {
    case type reify S;
    case Param<S> data;

    :@I(
      type S = int,
      data = new Param<int>(42)
    );

  }
}
