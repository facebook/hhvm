<?hh

function a_definition(): int {
  return b_definition();
}

function b_definition(): int {
  return 42;
}

class BB {
  public function __construct(int $i) {}
}

class CC extends BB {
}

class DD extends CC {
}

class EE extends DD {
  public function __construct() {
    parent::__construct(1);
  }
}

class FF {}

function test(): void {
  $bb = new BB(1); // should go to B::__construct
  $cc = new CC(1); // should offer choice B::__construct or C
  $dd = new DD(1); // should offer choice B::__construct or D
  $ee = new EE(); // should go to E::__construct
  $ff = new FF(); // should go to F
}

class TakesString {
  public function __construct(string $s) {}
}

class HasString {
  const MyString = "myString";
}

function testClassMemberInsideConstructorInvocation(): void {
  $x = new TakesString(HasString::MyString);
}

class MyEnumClassKind {}
enum class MyEnumClass : MyEnumClassKind {
  MyEnumClassKind First = new MyEnumClassKind();
  MyEnumClassKind Second = new MyEnumClassKind();
}

<<__DynamicallyReferenced>>
    class HasAnnos { // span should start at the beginning of this line (not include the attribute)
  <<__Memoize>>
  public function foo(): void {} // span should start at the beginning of this line (not include the attribute)

  // the span for `bar` should NOT include part of the span for the property $prop;
  private int $prop = 1; <<__Memoize>> public function bar(): void {}
}
<<__Memoize>>
function has_anno(): void { // span should start on this line
  echo '';
}
