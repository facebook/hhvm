<?hh // strict

class C {
  public static function foo(): void {}
}

type CAlias = C;

function new_c(classname<CAlias> $c): void {
  $c::foo();
}

function bad_new_c(typename<CAlias> $c): void {
  $c::foo();
}
