<?hh // strict

// PPL interfaces can only be required by PPL traits

<<__PPL>>
interface BaseInterface {}

trait MyTrait {
  require implements BaseInterface;
}
