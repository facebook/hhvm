<?hh // strict

abstract class P {
  abstract const type T;

  public function __construct(private this::T $t) {}
}

abstract class C extends P {
  public function __construct(this::T $t) {
    parent::__construct($t);
  }
}
