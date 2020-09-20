<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class Foo {
  final enum Field {
    case type T;
    case string ident;
    case T default_value;
  }
}
