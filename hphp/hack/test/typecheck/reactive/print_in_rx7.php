<?hh // strict

interface Rx {}

class A {
  <<__RxShallowIfImplements(Rx::class)>>
  public function f(): void {
    // should be error
    echo 1;
  }
}
