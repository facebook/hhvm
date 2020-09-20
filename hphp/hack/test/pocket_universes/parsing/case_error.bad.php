<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class Foo
{
  enum Field {
  case 16 toto;
  case tata 15;
  case class ident;
  not_atom_nor_case type T;
  }
}
