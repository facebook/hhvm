<?hh // strict

// PPL traits can only require implementing PPL interfaces

interface BaseInterface {}

<<__PPL>>
trait MyTrait {
  require implements BaseInterface;
}
