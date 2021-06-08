<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

interface I {}
class Box implements I {
  public function __construct(public int $x)[] {}
}

enum class EE : I {
   Box A = new Box(42);
}

function get<T>(<<__ViaLabel>> HH\MemberOf<EE, T> $z) : T {
  return $z;
}

function set<T>(<<__ViaLabel>> HH\MemberOf<EE, T> $z, T $val): void {}

class MyClass {
  public static function foo<T>(<<__ViaLabel>> HH\MemberOf<EE, T> $x): void {}
}

<<__EntryPoint>>
function main(): void {
  get#A();
  get#B(); // unknown constant
  set#A(new Box(13));
  set#B(new Box(13)); // unknown constant
  MyClass::foo#A();
}
