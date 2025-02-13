<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function nc(): void {}
}

function foo(classname<A> $cls): void {
  $cls::nc(); // error
}
