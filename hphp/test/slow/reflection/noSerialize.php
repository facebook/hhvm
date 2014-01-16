<?PHP

/** Yay */
class A {
  public function __sleep() {
    throw new Exception();
  }
}
var_dump((new ReflectionClass(new A))->getDocComment());
