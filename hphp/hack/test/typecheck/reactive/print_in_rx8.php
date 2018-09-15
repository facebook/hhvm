<?hh // strict

<<__RxLocal>>
function rxlocal(): void {
  print 1;
  echo 2;
}

interface Rx {}

class A {
  <<__RxLocal, __OnlyRxIfImpl(Rx::class)>>
  public function f(): void {
    // OK
    print 1;
    echo 1;
  }
}
