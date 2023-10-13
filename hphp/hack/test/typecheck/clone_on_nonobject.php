<?hh

class A {
  private int $foo;
  private ?A $maybeObj = null;

  public function __construct() {
    $this->foo = 5;
  }

  public function cloneMe(): A {
    return clone $this;
  }

  public function cloneMaybeObj(): A {
    if ($this->maybeObj) {
      return clone $this->maybeObj;
    } else {
      return new A();
    }
  }

  public function cloneOther(): A {
    return clone $this->foo;
  }
}
