<?hh // strict
interface Rx1 {
}

abstract class A {

  public function mayberx(): int {
    // ok - condition matches
    return $this->mayberx2();
  }


  public abstract function mayberx2(): int;
}
