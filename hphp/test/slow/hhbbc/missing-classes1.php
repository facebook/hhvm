<?hh

class A extends Missing {}

class B {
  public static function meth1() : A {
    return new A();
  }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(B::meth1());
}
