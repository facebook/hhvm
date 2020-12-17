<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface Rx {}

class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx::class)>>
  public function f()[rx_shallow]: void { // TODO(coeffects) abstract ctx
    // should be error
    echo 1;
  }
}
