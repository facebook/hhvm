<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__LateInit>> private dict $d;
  private static bool $g = false;

  public function foo() :mixed{
    if (!A::$g) return;
    if (__hhvm_intrinsics\launder_value(true)) {
      $this->d = __hhvm_intrinsics\launder_value(dict['abc' => 456]);
    }
  }

  public function bar() :mixed{
    A::$g = true;
    return $this->d['abc'];
  }
}

<<__EntryPoint>>
function main() :mixed{
  $a = new A();
  try { $a->bar(); } catch (Exception $e) {}
  $a->foo();
  var_dump($a->bar());
}
