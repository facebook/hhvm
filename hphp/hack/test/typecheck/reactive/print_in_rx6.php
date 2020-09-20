<?hh // strict

interface Rx {}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function f(): void {
    // should be error
    echo 1;
  }
}
