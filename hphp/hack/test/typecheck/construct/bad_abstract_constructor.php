<?hh

interface HasConstructor {
  public function __construct(int $_);
}

abstract class MyBase implements HasConstructor {}

class MyClass extends MyBase {}
