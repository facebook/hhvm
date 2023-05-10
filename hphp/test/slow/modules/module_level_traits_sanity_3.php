<?hh

trait T1 {}

<<__ModuleLevelTrait>>
trait T2 {
  use T1;
}
