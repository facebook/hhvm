<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

interface I {}
class Box implements I {
  public function __construct(public int $x)[] {}
}

enum class E : int {
   int A = 42;
}

class MyClass {
  public function __construct() {}

  public function get(<<__ViaLabel>> HH\MemberOf<E, int> $x, int $_): int {
    return $x;
  }
}

<<__EntryPoint>>
function main(): void {
  $obj = new MyClass();
  $obj->get#A(12);
  $obj->get#B(12); // unknown constant
}
