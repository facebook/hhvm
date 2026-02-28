<?hh

/** Yay */
class A {
  public function __sleep() :mixed{
    throw new Exception();
  }
}

<<__EntryPoint>>
function main_no_serialize() :mixed{
var_dump((new ReflectionClass(new A))->getDocComment());
}
