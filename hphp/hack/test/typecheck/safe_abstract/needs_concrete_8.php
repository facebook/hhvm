<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function nc(): void {}
}

function foo(concreteclassname<A> $cls): void {
  $cls::nc(); // ok
}
