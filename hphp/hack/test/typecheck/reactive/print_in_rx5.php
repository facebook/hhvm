<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface Rx {}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function f()[rx]: void {  // TODO(coeffects) migrate to abstract ctx
    // should be error
    print 1;
  }
}
