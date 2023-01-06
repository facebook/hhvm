<?hh

class A {
  public static function blah() { return "c"; }

  public function foo($a, $b, $c) {
    return __hhvm_intrinsics\launder_value(true);
  }

  public function bar() {
    return $this->foo(
      static::blah<string>,
      "a",
      "b"
     );
  }
}

<<__EntryPoint>>
function main() {
  $a = new A();
  var_dump($a->bar());
}
