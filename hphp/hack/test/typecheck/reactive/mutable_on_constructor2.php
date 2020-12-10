<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {

  <<__Rx, __MaybeMutable>>
  public function ok(): void {
  }

  <<__Rx, __MaybeMutable>>
  public function __construct(int $a) {
  }

}
