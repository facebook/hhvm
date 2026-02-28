<?hh

function accept_string(string $_): void {}

abstract class MyParentClass {
  public function __construct(public string $str) {}
}

abstract class IntermClass extends MyParentClass {}

final class MySubClass extends IntermClass {
  public function __construct(string $str) {
    accept_string($this->str);
    parent::__construct($str);
  }
}
