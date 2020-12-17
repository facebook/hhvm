<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__RxLocal>>
function rxlocal()[rx_local]: void {
  print 1; // ERROR
  echo 2;  // ERROR
}

interface Rx {}

class A {
  <<__RxLocal, __OnlyRxIfImpl(Rx::class)>>
  public function f()[rx_local]: void {  // TODO(coeffects) abstract ctx
    echo 3;  // ERROR
    print 4; // ERROR
  }
}
