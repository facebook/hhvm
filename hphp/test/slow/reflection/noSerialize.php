<?hh

/** Yay */
class A {
  public function __sleep() {
    throw new Exception();
  }
}

<<__EntryPoint>>
function main_no_serialize() {
var_dump((new ReflectionClass(new A))->getDocComment());
}
