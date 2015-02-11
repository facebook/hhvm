<?hh

abstract class C1 {
  abstract const int X;
}
function test_polymorphism(C1 $inst): string {
  return $inst::X;
}
