<?hh // strict

// Interfaces and traits extending PPL classes must be PPL
<<__PPL>>
abstract class RequireMe {
}

interface BaseInterface {
  require extends RequireMe;
}
