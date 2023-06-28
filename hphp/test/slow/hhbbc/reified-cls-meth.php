<?hh

class A {
  public static function blah() :mixed{ return "c"; }

  public function foo($a, $b, $c) :mixed{
    return __hhvm_intrinsics\launder_value(true);
  }

  public function bar() :mixed{
    return $this->foo(
      static::blah<string>,
      "a",
      "b"
     );
  }
}

<<__EntryPoint>>
function main() :mixed{
  $a = new A();
  var_dump($a->bar());
}
