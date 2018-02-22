<?hh // strict

interface I {}

trait MyTrait {
  public function __construct() {}

  public function equals(I $other): this {
    return $other instanceof $this;
  }
}
