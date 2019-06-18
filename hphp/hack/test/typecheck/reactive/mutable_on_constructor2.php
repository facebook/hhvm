<?hh // strict

class A {

  <<__Rx, __MaybeMutable>>
  public function ok(): void {
  }

  <<__Rx, __MaybeMutable>>
  public function __construct(int $a) {
  }

}
