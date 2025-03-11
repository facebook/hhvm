<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function nc(): void {}
}

function foo(concrete<classname<A>> $cls): void {
  $cls::nc(); // ok
}
//
