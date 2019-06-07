<?hh

interface I {}

trait RequiresT {
  require implements I;
}

trait ChildRequiresT {
  use RequiresT;
}
