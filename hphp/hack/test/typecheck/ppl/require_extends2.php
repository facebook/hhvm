<?hh // strict

// PPL interfaces and traits require extending PPL classes
abstract class RequireMe {
}

<<__PPL>>
interface BaseInterface {
  require extends RequireMe;
}
