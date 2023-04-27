<?hh

class A extends Missing {}

class B {
  public static function meth1() : A {
    return new A();
  }
}

<<__EntryPoint>>
function main() {
  var_dump(B::meth1());
}
