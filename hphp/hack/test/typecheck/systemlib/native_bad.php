<?hh

<<__NativeData('A')>>
class A {
  <<__Native>>
  public function __construct(): void;
}

<<__NativeData('B')>>
class B extends A {
  public function __construct(): void {}
}
