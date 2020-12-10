<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface Rx {}

class A {
  <<__RxLocal, __OnlyRxIfImpl(Rx::class)>>
  public function f(): void {
    // ERROR
    print 1;
  }
}
