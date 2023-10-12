<?hh // strict

abstract class C {

  public abstract static function f(): void;

}

function test(): void {
  C::f();
}
