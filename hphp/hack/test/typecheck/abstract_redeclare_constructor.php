<?hh

abstract class MyParent {
  public function __construct(): void {}
}

abstract class MyChild extends MyParent {
  abstract public function __construct(): void;
}
