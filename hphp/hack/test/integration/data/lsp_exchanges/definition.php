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
