<?hh

trait ProvidesT implements I {}

trait ChildProvidesT {
  use ProvidesT;
}

abstract class Super {
  use ChildProvidesT;
}
