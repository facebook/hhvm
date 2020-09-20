<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

interface Foo {
  enum E {
    case string name;

    :@A(
      name = "A"
    );
  }
}
